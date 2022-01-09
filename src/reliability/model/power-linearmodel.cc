/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "ns3/log.h"
#include "ns3/traced-value.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include "ns3/power-linearmodel.h"
#include <ns3/performance-model.h>
#include <iterator>
#include <string>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream> //istringstream


NS_LOG_COMPONENT_DEFINE ("PowerLinearModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PowerLinearModel);

TypeId
PowerLinearModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PowerLinearModel")
    .SetParent<PowerModel> ()
    .SetGroupName ("Power")
    .AddConstructor<PowerLinearModel> ()
    .AddAttribute ("A",
                   "Parameter A.",
                   DoubleValue (6.0957e-11),    //1.21914316767465*0.0000001
                   MakeDoubleAccessor (&PowerLinearModel::SetA,
                                       &PowerLinearModel::GetA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("B",
                   "Parameter B.",
                   DoubleValue (4.7775e-05),    //0.0955492791339378
                   MakeDoubleAccessor (&PowerLinearModel::SetB,
                                       &PowerLinearModel::GetB),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("C",
                   "Parameter C.",
                   DoubleValue (4.7775e-05),    //0.0955492791339378
                   MakeDoubleAccessor (&PowerLinearModel::SetC,
                                       &PowerLinearModel::GetC),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("f",
                   "Frequency.",
                   DoubleValue (2.4*1000000000),
                   MakeDoubleAccessor (&PowerLinearModel::SetFrequency,
                                       &PowerLinearModel::GetFrequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("cpu_util",
                   "CPU Utilization",
                   DoubleValue (1.0),    // default
                   MakeDoubleAccessor (&PowerLinearModel::SetUtilization,
                                       &PowerLinearModel::GetUtilization),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("IdlePowerW",
                   "Idle Power Consumption of Cpu",
                   DoubleValue (2.8),    // default
                   MakeDoubleAccessor (&PowerLinearModel::SetIdlePowerW,
                                       &PowerLinearModel::GetIdlePowerW),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("CpuPower",
                     "CPU power consumption of the device.",
                     MakeTraceSourceAccessor (&PowerLinearModel::m_cpupower),
                     "ns3::TracedValueCallback::Double")
  ; 
  return tid;
}

PowerLinearModel::PowerLinearModel ()
{
  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  m_powerUpdateInterval = Seconds (0.045);
  m_temperatureModel = NULL;      // TemperatureModel
  m_performanceModel = NULL;
  m_currentState = 0;
}

PowerLinearModel::~PowerLinearModel ()
{
  NS_LOG_FUNCTION (this);
}


void
PowerLinearModel::RegisterTemperatureModel (Ptr<TemperatureModel> temperatureModel)
{
  m_temperatureModel = temperatureModel;
}

void
PowerLinearModel::RegisterPerformanceModel (Ptr<PerformanceModel> performanceModel)
{
  m_performanceModel = performanceModel;
}

double
PowerLinearModel::GetPower (void) const
{
  NS_LOG_FUNCTION (this);
  return m_cpupower;
}


void
PowerLinearModel::SetA (double A)
{
  NS_LOG_FUNCTION (this);
  m_A = A;
}

double
PowerLinearModel::GetA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_A;
}

void
PowerLinearModel::SetB (double B)
{
  NS_LOG_FUNCTION (this);
  m_B = B;
}

double
PowerLinearModel::GetB (void) const
{
  NS_LOG_FUNCTION (this);
  return m_B;
}

void
PowerLinearModel::SetC (double C)
{
  NS_LOG_FUNCTION (this);
  m_C = C;
}

double
PowerLinearModel::GetC (void) const
{
  NS_LOG_FUNCTION (this);
  return m_C;
}

void
PowerLinearModel::SetFrequency (double frequency)
{
  NS_LOG_FUNCTION (this);
  m_frequency = frequency;
}

double
PowerLinearModel::GetFrequency (void) const
{
  NS_LOG_FUNCTION (this);
  return m_frequency;
}

void
PowerLinearModel::SetUtilization (double cpu_util)
{
  NS_LOG_FUNCTION (this);
  m_util = cpu_util;
}

double
PowerLinearModel::GetUtilization (void) const
{
  NS_LOG_FUNCTION (this);
  return m_util;
}

void
PowerLinearModel::SetIdlePowerW (double idlePowerW)
{
  NS_LOG_FUNCTION (this << idlePowerW);
  m_idlePowerW = idlePowerW;
}

double
PowerLinearModel::GetIdlePowerW (void) const
{
  NS_LOG_FUNCTION (this);
  return m_idlePowerW;
}

void
PowerLinearModel::SetApplication(std::string n0, const DoubleValue &v0)
{
  m_appname = n0;
  m_datasize = v0.Get();
  m_performanceModel->SetApplication(m_appname,m_datasize);
}

void
PowerLinearModel::SetState (int state)
{
  NS_LOG_FUNCTION (this << state);
  m_currentState = state;
}

int
PowerLinearModel::GetState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

double
PowerLinearModel::GetEnergy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_energy;
}

void
PowerLinearModel::RunApp()
{
  Time now = Simulator::Now ();
  m_powerUpdateEvent = Simulator::Schedule (now,&PowerLinearModel::UpdatePower,this);
  m_currentState = 1;
  m_exectime = m_performanceModel->GetExecTime();
  Simulator::Schedule (Seconds(m_exectime),&PowerLinearModel::TerminateApp,this);
  NS_LOG_DEBUG ("PowerLinearModel:Application scheduled successfully!" << " at time = " << Simulator::Now ());
  NS_LOG_DEBUG ("PowerLinearModel:Application will be terminated in " << m_exectime << " seconds ");
}

void
PowerLinearModel::TerminateApp()
{
 m_powerUpdateEvent.Cancel ();
 m_cpupower = m_idlePowerW;
 m_currentState = 0;
   NS_LOG_DEBUG ("PowerLinearModel:Application terminated successfully!" << " at time = " << Simulator::Now ());
}


void
PowerLinearModel::UpdatePower ()
{
  NS_LOG_FUNCTION ("m_A:" << m_A << " m_B:" << m_B << " m_C:" << m_C << "m_frequency:" << m_frequency << "m_util:" << m_util);
  NS_LOG_DEBUG ("PowerLinearModel:Updating power" << " at time = " << Simulator::Now ());
  if (Simulator::IsFinished ())
    {
      return;
    }
  m_powerUpdateEvent.Cancel ();

  if(m_appname == "LinearRegression")
  {m_A = 0.0;
    m_B = 1.83*pow(10,-1);
    m_C = (9.72)*pow(10,1);
    m_energy = m_B*m_datasize + m_C;
  }
  if(m_appname == "AdaBoost")
  {m_A = 0.0;
    m_B = 8.13*pow(10,-4);
    m_C = (1.94)*pow(10,1);
    m_energy = m_B*m_datasize + m_C;
  }
  if(m_appname == "MedianFilter")
  {m_A = 0.0;
    m_B = 8.13*pow(10,-4);
    m_C = (1.94)*pow(10,1);
    m_energy = m_B*m_datasize + m_C;
  }
  if(m_appname == "NeuralNetwork")
  {m_A = 0.0;
    m_B = 8.13*pow(10,-4);
    m_C = (1.94)*pow(10,1);
    m_energy = m_B*m_datasize + m_C;
  }
  m_cpupower = m_energy/m_exectime;

  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();

  m_temperatureModel->UpdateTemperature (m_cpupower);
  m_powerUpdateEvent = Simulator::Schedule (m_powerUpdateInterval,&PowerLinearModel::UpdatePower,this);
}

void
PowerLinearModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_temperatureModel = NULL;      // TemperatureModel
  m_performanceModel = NULL;      // PerformanceModel

}



} // namespace ns3

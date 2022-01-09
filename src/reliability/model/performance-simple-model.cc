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
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include "performance-simple-model.h"


NS_LOG_COMPONENT_DEFINE ("PerformanceSimpleModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PerformanceSimpleModel);

TypeId
PerformanceSimpleModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PerformanceSimpleModel")
    .SetParent<PerformanceModel> ()
    .SetGroupName("Performance")
    .AddConstructor<PerformanceSimpleModel> ()
    .AddAttribute ("A",
                   "Coeff A.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&PerformanceSimpleModel::SetA,
                                       &PerformanceSimpleModel::GetA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("B",
                   "Coeff B.",
                   DoubleValue (2.28*pow(10,-4)),
                   MakeDoubleAccessor (&PerformanceSimpleModel::SetB,
                                       &PerformanceSimpleModel::GetB),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("C",
                   "Coeff C.",
                   DoubleValue (9.38*pow(10,-1)),
                   MakeDoubleAccessor (&PerformanceSimpleModel::SetC,
                                       &PerformanceSimpleModel::GetC),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("datasize",
                   "Data Size",
                   DoubleValue (100),
                   MakeDoubleAccessor (&PerformanceSimpleModel::SetDataSize,
                                       &PerformanceSimpleModel::GetDataSize),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("ExecutionTime",
                     "Execution Time of the App.",
                     MakeTraceSourceAccessor (&PerformanceSimpleModel::m_exectime),
                     "ns3::TracedValue::DoubleCallback")
  ; 
  return tid;
}

PerformanceSimpleModel::PerformanceSimpleModel ()
{
  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  m_throughput = 0.0;
}

PerformanceSimpleModel::~PerformanceSimpleModel ()
{
  NS_LOG_FUNCTION (this);
}

double
PerformanceSimpleModel::GetExecTime (void) const
{
  NS_LOG_FUNCTION (this);
  return m_exectime;
}

double
PerformanceSimpleModel::GetThroughput (void) const
{
  NS_LOG_FUNCTION (this);
  return m_throughput;
}

void
PerformanceSimpleModel::SetThroughput (double throughput)
{
  NS_LOG_FUNCTION (this);
  m_throughput = throughput;
}

void
PerformanceSimpleModel::SetA (double A)
{
  NS_LOG_FUNCTION (this);
  m_A = A;
}
double
PerformanceSimpleModel::GetA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_A;
}

void
PerformanceSimpleModel::SetB (double B)
{
  NS_LOG_FUNCTION (this);
  m_B = B;
}

double
PerformanceSimpleModel::GetB (void) const
{
  NS_LOG_FUNCTION (this);
  return m_B;
}

void
PerformanceSimpleModel::SetC (double C)
{
  NS_LOG_FUNCTION (this);

  m_C = C ;
}

double
PerformanceSimpleModel::GetC (void) const
{
  NS_LOG_FUNCTION (this);
  return m_C;
}

void
PerformanceSimpleModel::SetDataSize (const DoubleValue &v0)
{
  NS_LOG_FUNCTION (this);

  m_datasize = v0.Get();
}

double
PerformanceSimpleModel::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_datasize;
}

void
PerformanceSimpleModel::SetPacketSize (const DoubleValue &v1)
{
  NS_LOG_FUNCTION (this);
  m_packetSize = v1.Get() ;
}

double
PerformanceSimpleModel::GetPacketSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_packetSize;
}

void
PerformanceSimpleModel::SetApplication (std::string m_appName, const DoubleValue &v0)
{
  NS_LOG_FUNCTION (this);
  m_datasize = v0.Get() ;

  
  if(m_deviceType == "RaspberryPi")
  {
    if(m_appName == "AdaBoost")
    {
      m_A = 0.0;
      m_B = 5.32*pow(10,-2);
      m_C = 2.40*pow(10,1);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "DecisionTree")
    {
      m_A = 0.0;
      m_B = 4.14*pow(10,-3);
      m_C = -9.82*pow(10,-1);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "RandomForest")
    {
      m_A = 3.99*pow(10,-8);
      m_B = 1.37*pow(10,-2);
      m_C = 6.80*pow(10,-2);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "kNN")
    {
      m_A = 0.0;
      m_B = 9.32*pow(10,-3);
      m_C = 4.52*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
      else if(m_appName == "LinearSVM")
    {
      m_A = 3.09*pow(10,-3);
      m_B = 5.86*pow(10,-1);
      m_C = -3.07*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "AffinityPropagation")
    {
      m_A = 6.02*pow(10,-1);
      m_B = -1.81*pow(10,0);
      m_C = 1.57*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "Birch")
    {
      m_A = 1.13*pow(10,-2);
      m_B = -1.38*pow(10,-1);
      m_C = 1.59*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "k-means")
    {
      m_A = 1.12*pow(10,-2);
      m_B = -1.51*pow(10,-1);
      m_C = 1.49*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "BayesianRegression")
    {
      m_A = 1.20*pow(10,-9);
      m_B = 4.72*pow(10,-5);
      m_C = 7.61*pow(10,-1);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "LinearRegression")
    {
      m_A = 0.0;
      m_B = 6.154*pow(10,-6);
      m_C = 0.12*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize) + m_C);
    }
    else
    {
      NS_FATAL_ERROR ("AppPowerModel:Undefined application for this device: " << m_appName);
    }
  }
  else if(m_deviceType == "Server")
  {
    if(m_appName == "AdaBoost")
    {
      m_A = 0.0;
      m_B = 4.60*pow(10,-3);
      m_C = 1.58*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "DecisionTree")
    {
      m_A = 0.0;
      m_B = 4.21*pow(10,-4);
      m_C = -3.76*pow(10,-1);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "RandomForest")
    {
      m_A = 1.96*pow(10,-8);
      m_B = 1.15*pow(10,-3);
      m_C = -1.19*pow(10,-1);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "kNN")
    {
      m_A = 0.0;
      m_B = 9.21*pow(10,-4);
      m_C = -3.19*pow(10,-1);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
      else if(m_appName == "LinearSVM")
    {
      m_A = 2.01*pow(10,-4);
      m_B = 2.95*pow(10,-2);
      m_C = -5.44*pow(10,-2);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "AffinityPropagation")
    {
      m_A = 8.27*pow(10,-2);
      m_B = 2.69*pow(10,-1);
      m_C = -1.40*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "Birch")
    {
      m_A = 1.13*pow(10,-3);
      m_B = -4.84*pow(10,-3);
      m_C = 9.26*pow(10,-2);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "k-means")
    {
      m_A = 1.40*pow(10,-3);
      m_B = -4.86*pow(10,-2);
      m_C = 7.50*pow(10,-1);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "BayesianRegression")
    {
      m_A = 1.33*pow(10,-8);
      m_B = -2.69*pow(10,-4);
      m_C = 1.38*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize/1000) + m_C);
    }
    else if(m_appName == "LinearRegression")
    {
      m_A = 0.0;
      m_B = 6.154*pow(10,-6);
      m_C = 0.12*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize) + m_C);
    }
    else
    {
      NS_FATAL_ERROR ("AppPowerModel:Undefined application for this device: " << m_appName);
    }
  }
  else if(m_deviceType == "Arduino")
  {
    if(m_appName == "MedianFilter")
    {
      m_A = 0.0;
      m_B = 1.0*pow(10,-1);
      m_C = 0.5*pow(10,0);
      m_exectime = std::max(0.1,(m_B*m_datasize)/1000 + m_C);
    }
    else
    {
      NS_FATAL_ERROR ("AppPowerModel:Undefined application for this device: " << m_appName);
    }
  }
  else if(m_deviceType == "RaspberryPi0")
  {
    if(m_appName == "LinearRegression")
    {
      m_A = 0.0;
      m_B = 5.76*pow(10,-6);
      m_C = -0.06*pow(10,0);
      m_exectime = std::max(0.1,(m_A*pow(m_datasize,2)/1000000+ m_B*m_datasize) + m_C);
    }
    else
    {
      NS_FATAL_ERROR ("AppPowerModel:Undefined application for this device: " << m_appName);
    }
  }
  else
  {
    NS_FATAL_ERROR ("AppPowerModel:Undefined device type: " << m_deviceType);
  }

}

void
PerformanceSimpleModel::SetDeviceType (std::string devicetype)
{
  NS_LOG_FUNCTION (this);
  m_deviceType = devicetype;
}

void
PerformanceSimpleModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}



} // namespace ns3
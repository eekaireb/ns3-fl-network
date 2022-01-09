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
#include "ns3/pointer.h"
#include "ns3/energy-source.h"
#include "cpu-energy-model.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/power-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CpuEnergyModel");

NS_OBJECT_ENSURE_REGISTERED (CpuEnergyModel);

TypeId
CpuEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CpuEnergyModel")
    .SetParent<DeviceEnergyModel> ()
    .SetGroupName ("Energy")
    .AddConstructor<CpuEnergyModel> ()
    .AddAttribute ("IdlePowerW",
                   "Idle Power Consumption of Cpu",
                   DoubleValue (2.8),    // default
                   MakeDoubleAccessor (&CpuEnergyModel::SetIdlePowerW,
                                       &CpuEnergyModel::GetIdlePowerW),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PowerModel", "A pointer to the attached power model.",
                   PointerValue (),
                   MakePointerAccessor (&CpuEnergyModel::m_powerModel),
                   MakePointerChecker<PowerModel> ())
    .AddTraceSource ("TotalEnergyConsumption",
                     "Total energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&CpuEnergyModel::m_totalEnergyConsumption),
                     "ns3::TracedValueCallback::Double")
  ; 
  return tid;
}


CpuEnergyModel::CpuEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  m_currentState = WifiPhyState::IDLE;  // initially IDLE
  m_nPendingChangeState = 0;
  m_isSupersededChangeState = false;
  m_energyDepletionCallback.Nullify ();
  m_cpuAppRunCallback.Nullify ();
  m_count = 0;
  m_numberOfPackets = 0;
  m_source = NULL;
  m_powerModel = NULL;
  // set callback for WifiPhy listener
  m_listener = new CpuEnergyModelPhyListener();
  m_listener->SetChangeStateCallback (MakeCallback (&DeviceEnergyModel::ChangeState, this));

}


CpuEnergyModel::~CpuEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  delete m_listener;
}


void
CpuEnergyModel::SetEnergySource (const Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}


double
CpuEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

void
CpuEnergyModel::SetIdlePowerW (double idlePowerW)
{
  NS_LOG_FUNCTION (this << idlePowerW);
  m_idlePowerW = idlePowerW;
}

double
CpuEnergyModel::GetIdlePowerW (void) const
{
  NS_LOG_FUNCTION (this);
  return m_idlePowerW;
}


WifiPhyState
CpuEnergyModel::GetCurrentState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

void
CpuEnergyModel::SetEnergyDepletionCallback (
  CpuEnergyDepletionCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("CpuEnergyModel:Setting NULL energy depletion callback!");
    }
  m_energyDepletionCallback = callback;
}

void
CpuEnergyModel::SetEnergyRechargedCallback (
  CpuEnergyRechargedCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("CpuEnergyModel:Setting NULL energy recharged callback!");
    }
  m_energyRechargedCallback = callback;
}

void
CpuEnergyModel::SetCpuAppRunCallback (
  CpuAppRunCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("CpuEnergyModel:Setting CPU app terminate callback!");
    }
  m_cpuAppRunCallback = callback;
}

void
CpuEnergyModel::SetCpuAppTerminateCallback (
  CpuAppTerminateCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("CpuEnergyModel:Setting CPU App Terminate callback!");
    }
  m_cpuAppTerminateCallback = callback;
}


void
CpuEnergyModel::SetPowerModel (const Ptr<PowerModel> model)
{
  m_powerModel = model;
  m_idlePowerW = m_powerModel->GetIdlePowerW();
}

void
CpuEnergyModel::SetPerformanceModel (const Ptr<PerformanceModel> model)
{
  m_performanceModel = model;
  m_dataSize = m_performanceModel->GetDataSize();
  m_packetSize = m_performanceModel->GetPacketSize();
  m_numberOfPackets = m_dataSize/m_packetSize;
}

void
CpuEnergyModel::ChangeState (int newState)
{
  NS_LOG_FUNCTION (this << newState);

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetNanoSeconds () >= 0); // check if duration is valid

  // energy to decrease = current * voltage * time
  double energyToDecrease = 0.0;
  switch (m_currentState)
    {
    case WifiPhyState::IDLE:
      energyToDecrease = duration.GetSeconds () * m_idlePowerW;
      break;
    case WifiPhyState::CCA_BUSY:
      energyToDecrease = duration.GetSeconds () * m_idlePowerW;
      break;
    case WifiPhyState::TX:
      energyToDecrease = duration.GetSeconds () * m_idlePowerW;
      break;
    case WifiPhyState::RX:
      m_count += 1;
      if(m_count>=m_numberOfPackets){
      if (m_powerModel->GetState()==0)
        {
          NS_LOG_DEBUG ("CpuEnergyModel:Running app" <<
                          " at time = " << Simulator::Now ());
          m_powerModel->RunApp();
          //energyToDecrease = m_powerModel->GetEnergy();
          energyToDecrease = duration.GetSeconds () * m_powerModel->GetPower();
        } else {
          NS_LOG_DEBUG ("CpuEnergyModel: BUSY, an app is still running" <<
                          " at time = " << Simulator::Now ());        
          energyToDecrease = 0;
        }
      m_count = 0;
      }
      break;
    case WifiPhyState::SWITCHING:
      energyToDecrease = duration.GetSeconds () * m_idlePowerW;
      break;
    case WifiPhyState::SLEEP:
      energyToDecrease = duration.GetSeconds () * m_idlePowerW;
      break;
    default:
      NS_FATAL_ERROR ("CpuEnergyModel:Undefined radio state: " << m_currentState);
    }

  // update total energy consumption
  m_totalEnergyConsumption += energyToDecrease;

  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();

  m_nPendingChangeState++;

  // notify energy source
  m_source->UpdateEnergySource ();

  // in case the energy source is found to be depleted during the last update, a callback might be
  // invoked that might cause a change in the Wifi PHY state (e.g., the PHY is put into SLEEP mode).
  // This in turn causes a new call to this member function, with the consequence that the previous
  // instance is resumed after the termination of the new instance. In particular, the state set
  // by the previous instance is erroneously the final state stored in m_currentState. The check below
  // ensures that previous instances do not change m_currentState.

  if (!m_isSupersededChangeState)
    {
      // update current state & last update time stamp
      SetWifiRadioState ((WifiPhyState) newState);

      // some debug message
      NS_LOG_DEBUG ("CpuEnergyModel:Total energy consumption is " <<
                    m_totalEnergyConsumption << "J");
    }

  m_isSupersededChangeState = (m_nPendingChangeState > 1);

  m_nPendingChangeState--;
}

void
CpuEnergyModel::HandleEnergyDepletion (void)
{
  // NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("CpuEnergyModel:HandleEnergyDepletion!");
  // // invoke energy depletion callback, if set.
  // if (!m_energyDepletionCallback.IsNull ())
  //   {
  //     m_energyDepletionCallback ();
  //   }
  HandleCpuAppRun();

}

void
CpuEnergyModel::HandleEnergyRecharged (void)
{
//   NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("CpuEnergyModel:HandleEnergyRecharged!");
//   // invoke energy recharged callback, if set.
//   if (!m_energyRechargedCallback.IsNull ())
//     {
//       m_energyRechargedCallback ();
//     }
  HandleCpuAppTerminate();
}

void
CpuEnergyModel::HandleCpuAppRun (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("CpuEnergyModel:App is starting!");
  // invoke energy depletion callback, if set.
  if (!m_cpuAppRunCallback.IsNull ())
    {
      m_cpuAppRunCallback ();
    }
}

void
CpuEnergyModel::HandleCpuAppTerminate (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("CpuEnergyModel:App has finished executing!");
  // invoke energy recharged callback, if set.
  if (!m_cpuAppTerminateCallback.IsNull ())
    {
      m_cpuAppTerminateCallback ();
    }
}

void CpuEnergyModel::HandleEnergyChanged()
{

}


CpuEnergyModelPhyListener *
CpuEnergyModel::GetPhyListener (void)
{
  NS_LOG_FUNCTION (this);
  return m_listener;
}



/*
 * Private functions start here.
 */

void
CpuEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_source = NULL;
  m_energyDepletionCallback.Nullify ();
  m_cpuAppRunCallback.Nullify ();

}

double
CpuEnergyModel::DoGetPower (void) const
{
  NS_LOG_FUNCTION (this);
  switch (m_currentState)
    {
    case WifiPhyState::IDLE:
      return m_idlePowerW;
    case WifiPhyState::CCA_BUSY:
      return m_idlePowerW;
    case WifiPhyState::TX:
      return m_idlePowerW;
    case WifiPhyState::RX:
      return m_idlePowerW;
    case WifiPhyState::SWITCHING:
      return m_idlePowerW;
    case WifiPhyState::SLEEP:
      return m_idlePowerW;
    default:
      NS_FATAL_ERROR ("CpuEnergyModel:Undefined radio state:" << m_currentState);
    }
}

void
CpuEnergyModel::SetWifiRadioState (const WifiPhyState state)
{
  NS_LOG_FUNCTION (this << state);
  m_currentState = state;
  std::string stateName;
  switch (state)
    {
    case WifiPhyState::IDLE:
      stateName = "IDLE";
      break;
    case WifiPhyState::CCA_BUSY:
      stateName = "CCA_BUSY";
      break;
    case WifiPhyState::TX:
      stateName = "TX";
      break;
    case WifiPhyState::RX:
      stateName = "RX";
      break;
    case WifiPhyState::SWITCHING:
      stateName = "SWITCHING";
      break;
    case WifiPhyState::SLEEP:
      stateName = "SLEEP";
      break;

      case WifiPhyState::OFF:
        stateName = "OFF";
      break;
    }
  NS_LOG_DEBUG ("CpuEnergyModel:Switching to state: " << stateName <<
                " at time = " << Simulator::Now ());
}




// -------------------------------------------------------------------------- //

CpuEnergyModelPhyListener::CpuEnergyModelPhyListener ()
{
  NS_LOG_FUNCTION (this);
  m_changeStateCallback.Nullify ();
}

CpuEnergyModelPhyListener::~CpuEnergyModelPhyListener ()
{
  NS_LOG_FUNCTION (this);
}

void
CpuEnergyModelPhyListener::SetChangeStateCallback (DeviceEnergyModel::ChangeStateCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);
  NS_ASSERT (!callback.IsNull ());
  m_changeStateCallback = callback;
}


  void CpuEnergyModelPhyListener::NotifyOff(void){}

  void CpuEnergyModelPhyListener::NotifyOn(void){}


void
CpuEnergyModelPhyListener::NotifyRxStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("CpuEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (WifiPhyState::RX);
  m_switchToIdleEvent.Cancel ();
}

void
CpuEnergyModelPhyListener::NotifyRxEndOk (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("CpuEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (WifiPhyState::IDLE);
}

void
CpuEnergyModelPhyListener::NotifyRxEndError (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("CpuEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (WifiPhyState::IDLE);
}


void
CpuEnergyModelPhyListener::NotifyTxStart (Time duration, double txPowerDbm)
{
  NS_LOG_FUNCTION (this << duration << txPowerDbm);

  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("CpuEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (WifiPhyState::TX);
  // schedule changing state back to IDLE after TX duration
  m_switchToIdleEvent.Cancel ();
  m_switchToIdleEvent = Simulator::Schedule (duration, &CpuEnergyModelPhyListener::SwitchToIdle, this);
}

void
CpuEnergyModelPhyListener::NotifyMaybeCcaBusyStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("CpuEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (WifiPhyState::CCA_BUSY);
  // schedule changing state back to IDLE after CCA_BUSY duration
  m_switchToIdleEvent.Cancel ();
  m_switchToIdleEvent = Simulator::Schedule (duration, &CpuEnergyModelPhyListener::SwitchToIdle, this);
}

void
CpuEnergyModelPhyListener::NotifySwitchingStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("CpuEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (WifiPhyState::SWITCHING);
  // schedule changing state back to IDLE after CCA_BUSY duration
  m_switchToIdleEvent.Cancel ();
  m_switchToIdleEvent = Simulator::Schedule (duration, &CpuEnergyModelPhyListener::SwitchToIdle, this);
}

void
CpuEnergyModelPhyListener::NotifySleep (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("CpuEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (WifiPhyState::SLEEP);
  m_switchToIdleEvent.Cancel ();
}

void
CpuEnergyModelPhyListener::NotifyWakeup(void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("CpuEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (WifiPhyState::IDLE);
}

/*
 * Private function state here.
 */

void
CpuEnergyModelPhyListener::SwitchToIdle (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("CpuEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (WifiPhyState::IDLE);
}

} // namespace ns3

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
 *
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

#ifndef CPU_ENERGY_MODEL_H
#define CPU_ENERGY_MODEL_H


#include "ns3/device-energy-model.h"
#include "ns3/traced-value.h"
#include "ns3/wifi-phy.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/power-model.h"
#include "ns3/performance-model.h"
#include "ns3/wifi-phy-listener.h"
#include "ns3/wifi-phy-state.h"


namespace ns3 {

class PowerModel;

/**
 * \ingroup energy
 * A WifiPhy listener class for notifying the CpuEnergyModel of Wifi radio
 * state change.
 *
 */

class CpuEnergyModelPhyListener : public WifiPhyListener
{
public:

  /**
   * Callback type for updating the power.
   */
  typedef Callback<void, double> UpdatePowerCallback;

  CpuEnergyModelPhyListener ();
  virtual ~CpuEnergyModelPhyListener ();

  /**
   * \brief Sets the change state callback. Used by helper class.
   *
   * \param callback Change state callback.
   */
  void SetChangeStateCallback (DeviceEnergyModel::ChangeStateCallback callback);

  /**
   * \brief Sets the update power callback.
   *
   * \param callback Update power callback.
   */
  void SetUpdatePowerCallback (UpdatePowerCallback callback);

  /**
   * \brief Switches the WifiRadioEnergyModel to RX state.
   *
   * \param duration the expected duration of the packet reception.
   *
   * Defined in ns3::WifiPhyListener
   */
  void NotifyRxStart (Time duration);

  /**
   * \brief Switches the WifiRadioEnergyModel back to IDLE state.
   *
   * Defined in ns3::WifiPhyListener
   *
   * Note that for the WifiRadioEnergyModel, the behavior of the function is the
   * same as NotifyRxEndError.
   */
  void NotifyRxEndOk (void);

  /**
   * \brief Switches the WifiRadioEnergyModel back to IDLE state.
   *
   * Defined in ns3::WifiPhyListener
   *
   * Note that for the WifiRadioEnergyModel, the behavior of the function is the
   * same as NotifyRxEndOk.
   */
  void NotifyRxEndError (void);

  /**
   * \brief Switches the WifiRadioEnergyModel to TX state and switches back to
   * IDLE after TX duration.
   *
   * \param duration the expected transmission duration.
   * \param txPowerDbm the nominal tx power in dBm
   *
   * Defined in ns3::WifiPhyListener
   */
  void NotifyTxStart (Time duration, double txPowerDbm);

  /**
   * \param duration the expected busy duration.
   *
   * Defined in ns3::WifiPhyListener
   */
  void NotifyMaybeCcaBusyStart (Time duration);

  /**
   * \param duration the expected channel switching duration.
   *
   * Defined in ns3::WifiPhyListener
   */
  void NotifySwitchingStart (Time duration);

  /**
   * Defined in ns3::WifiPhyListener
   */
  void NotifySleep (void);

  /**
   * Defined in ns3::WifiPhyListener
   */
  void NotifyWakeup (void);

  void NotifyOff(void);

  void NotifyOn(void);


private:
  /**
   * A helper function that makes scheduling m_changeStateCallback possible.
   */
  void SwitchToIdle (void);

  /**
   * Change state callback used to notify the WifiRadioEnergyModel of a state
   * change.
   */
  DeviceEnergyModel::ChangeStateCallback m_changeStateCallback;
  /**
   * Callback used to update the power stored in CpuEnergyModel based on
   * the power power model.
   */
  UpdatePowerCallback m_updatePowerCallback;

  EventId m_switchToIdleEvent; ///< switch to idle event
};


/**
 * \ingroup energy
 * \brief A Cpu energy model.
 *
 * 4 states are defined for the radio: TX, RX, IDLE, SLEEP. Default state is
 * IDLE.
 * The different types of transactions that are defined are:
 *  1. Tx: State goes from IDLE to TX, radio is in TX state for TX_duration,
 *     then state goes from TX to IDLE.
 *  2. Rx: State goes from IDLE to RX, radio is in RX state for RX_duration,
 *     then state goes from RX to IDLE.
 *  3. Go_to_Sleep: State goes from IDLE to SLEEP.
 *  4. End_of_Sleep: State goes from SLEEP to IDLE.
 * The class keeps track of what state the radio is currently in.
 *
 * Energy calculation: For each transaction, this model notifies EnergySource
 * object. The EnergySource object will query this model for the total current.
 * Then the EnergySource object uses the total current to calculate energy.
 *
 * Default values for power consumption are based on measurements reported in:
 *
 * Daniel Halperin, Ben Greenstein, Anmol Sheth, David Wetherall,
 * "Demystifying 802.11n power consumption", Proceedings of HotPower'10
 *
 * Power consumption in Watts (single antenna):
 *
 * \f$ P_{tx} = 1.14 \f$ (transmit at 0dBm)
 *
 * \f$ P_{rx} = 0.94 \f$
 *
 * \f$ P_{idle} = 0.82 \f$
 *
 * \f$ P_{sleep} = 0.10 \f$
 *
 * Hence, considering the default supply voltage of 3.0 V for the basic energy
 * source, the default current values in Ampere are:
 *
 * \f$ I_{tx} = 0.380 \f$
 *
 * \f$ I_{rx} = 0.313 \f$
 *
 * \f$ I_{idle} = 0.273 \f$
 *
 * \f$ I_{sleep} = 0.033 \f$
 *
 * The dependence of the power consumption in transmission mode on the nominal
 * transmit power can also be achieved through a wifi tx current model.
 *
 */

class CpuEnergyModel : public DeviceEnergyModel
{
public:
  /**
   * Callback type for energy depletion handling.
   */
  typedef Callback<void> CpuEnergyDepletionCallback;

  /**
   * Callback type for energy recharged handling.
   */
  typedef Callback<void> CpuEnergyRechargedCallback;

  /**
   * Callback type for application run handling.
   */
  typedef Callback<void> CpuAppRunCallback;

  /**
   * Callback type for application terminate handling.
   */
  typedef Callback<void> CpuAppTerminateCallback;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  CpuEnergyModel ();
  virtual ~CpuEnergyModel ();


  /**
   * \brief Sets pointer to EnergySouce installed on node.
   *
   * \param source Pointer to EnergySource installed on node.
   *
   * Implements DeviceEnergyModel::SetEnergySource.
   */
  void SetEnergySource (const Ptr<EnergySource> source);

  /**
   * \returns Total energy consumption of the wifi device.
   *
   * Implements DeviceEnergyModel::GetTotalEnergyConsumption.
   */
  double GetTotalEnergyConsumption (void) const;

  // Setter & getters for state power consumption.
  /**
   * \brief Gets A parameter.
   *
   * \returns parameter A of the power model.
   */
  double GetIdlePowerW (void) const;
  /**
   * \brief Sets IdlePowerW.
   *
   * \param IdlePowerW the idle power of the device
   */
  void SetIdlePowerW (double idlePowerW);

   /**
   * \returns Current state.
   */
   WifiPhyState GetCurrentState (void) const;

  /**
   * \param callback Callback function.
   *
   * Sets callback for energy depletion handling.
   */
  void SetEnergyDepletionCallback (CpuEnergyDepletionCallback callback);

  /**
   * \param callback Callback function.
   *
   * Sets callback for energy recharged handling.
   */
  void SetEnergyRechargedCallback (CpuEnergyRechargedCallback callback);

    /**
   * \param callback Callback function.
   *
   * Sets callback for application run handling.
   */
  void SetCpuAppRunCallback (CpuAppRunCallback callback);

  /**
   * \param callback Callback function.
   *
   * Sets callback for application terminate handling.
   */
  void SetCpuAppTerminateCallback (CpuAppTerminateCallback callback);

  /**
   * \param power model.
   */
  void SetPowerModel (const Ptr<PowerModel> model);

    /**
   * \param performance model.
   */
  void SetPerformanceModel (const Ptr<PerformanceModel> model);


  /**
   * \brief Changes state of the WifiRadioEnergyMode.
   *
   * \param newState New state the wifi radio is in.
   *
   * Implements DeviceEnergyModel::ChangeState.
   */
  void ChangeState (int newState);

  /**
   * \brief Handles energy depletion.
   *
   * Implements DeviceEnergyModel::HandleEnergyDepletion
   */
  void HandleEnergyDepletion (void);

  /**
   * \brief Handles energy recharged.
   *
   * Implements DeviceEnergyModel::HandleEnergyRecharged
   */
  void HandleEnergyRecharged (void);
  
  /**
   * \brief Handles application starting.
   *
   * Implements 
   */
  void HandleCpuAppRun (void);

  /**
   * \brief Handles application terminating.
   *
   * Implements 
   */
  void HandleCpuAppTerminate (void);
  /**
   * \returns Pointer to the PHY listener.
   */
  CpuEnergyModelPhyListener * GetPhyListener (void);

  void HandleEnergyChanged();


private:
  void DoDispose (void);

  /**
   * \returns Power draw of Cpu at current state.
   *
   * Implements DeviceEnergyModel::GetCurrentA.
   */
  double DoGetPower (void) const;

  /**
   * \param state New state the radio device is currently in.
   *
   * Sets current state. This function is private so that only the energy model
   * can change its own state.
   */
  void SetWifiRadioState (const WifiPhyState state);

  /**
   * Sets current state to IDLE.
   */
  void SetWifiToIdle (void);

  Ptr<EnergySource> m_source; ///< energy source
  Ptr<PowerModel> m_powerModel; ///< current model
  Ptr<PerformanceModel> m_performanceModel; ///< current model

  // Member variables.
  double m_idlePowerW;
  double m_dataSize;
  double m_packetSize;
  double m_count;
  double m_numberOfPackets;
  /// This variable keeps track of the total energy consumed by this model.
  TracedValue<double> m_totalEnergyConsumption;
  EventId m_powerUpdateEvent;            // power update event
  EventId m_endSleepEvent;
  // State variables.
  WifiPhyState m_currentState;  ///< current state the radio is in
  Time m_lastUpdateTime;          ///< time stamp of previous energy update

  uint8_t m_nPendingChangeState; ///< pending state change
  bool m_isSupersededChangeState; ///< superseded change state

  /// Energy depletion callback
  CpuEnergyDepletionCallback m_energyDepletionCallback;

  /// Energy recharged callback
  CpuEnergyRechargedCallback m_energyRechargedCallback;

    /// App Run callback
  CpuAppRunCallback m_cpuAppRunCallback;

  /// App Terminate callback
  CpuAppTerminateCallback m_cpuAppTerminateCallback;

  /// WifiPhy listener
  CpuEnergyModelPhyListener *m_listener;
};

} // namespace ns3

#endif /* CPU_ENERGY_MODEL_H */


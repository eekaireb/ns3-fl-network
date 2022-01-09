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

#ifndef CPU_ENERGY_MODEL_HELPER_H
#define CPU_ENERGY_MODEL_HELPER_H

#include "ns3/energy-model-helper.h"
#include "ns3/cpu-energy-model.h"

namespace ns3 {

/**
 * \ingroup energy
 * \brief Assign CpuEnergyModel to wifi devices.
 *
 * This installer installs CpuEnergyModel for only WifiNetDevice objects.
 *
 */
class CpuEnergyModelHelper : public DeviceEnergyModelHelper
{
public:
  /**
   * Construct a helper which is used to add a cpu energy model to a node
   */
  CpuEnergyModelHelper ();

  /**
   * Destroy a Cpu Energy Helper
   */
  ~CpuEnergyModelHelper ();

  /**
   * \param name the name of the attribute to set
   * \param v the value of the attribute
   *
   * Sets an attribute of the underlying Cpu energy model object.
   */
  void Set (std::string name, const AttributeValue &v);

  /**
   * \param callback Callback function for energy depletion handling.
   *
   * Sets the callback to be invoked when energy is depleted.
   */
  void SetDepletionCallback (
    CpuEnergyModel::CpuEnergyDepletionCallback callback);

  /**
   * \param callback Callback function for energy recharged handling.
   *
   * Sets the callback to be invoked when energy is recharged.
   */
  void SetRechargedCallback (
    CpuEnergyModel::CpuEnergyRechargedCallback callback);

    /**
   * \param callback Callback function forcpu app run handling.
   *
   * Sets the callback to be invoked when app is started.
   */
  void SetCpuAppRunCallback (
    CpuEnergyModel::CpuAppRunCallback callback);

  /**
   * \param callback Callback function for terminating app.
   *
   * Sets the callback to be invoked when app is finished executing.
   */
  void SetCpuAppTerminateCallback (
    CpuEnergyModel::CpuAppTerminateCallback callback);

  /**
   *
   * Set power model for the Cpu Energy Model
   */
  void SetPowerModel (Ptr<PowerModel> powerModel);

private:
  /**
   * \param device Pointer to the NetDevice to install DeviceEnergyModel.
   * \param source Pointer to EnergySource to install.
   * \returns Ptr<DeviceEnergyModel>
   *
   * Implements DeviceEnergyModel::Install.
   */
  virtual Ptr<DeviceEnergyModel> DoInstall (Ptr<NetDevice> device,
                                            Ptr<EnergySource> source) const;

private:
  ObjectFactory m_cpuEnergy; ///< cpu energy
  CpuEnergyModel::CpuEnergyDepletionCallback m_depletionCallback; ///< cpu energy depletion callback
  CpuEnergyModel::CpuEnergyRechargedCallback m_rechargedCallback; ///< cpu energy recharged callback
  CpuEnergyModel::CpuAppRunCallback m_cpuAppRunCallback; ///< cpu application run callback
  CpuEnergyModel::CpuAppTerminateCallback m_cpuAppTerminateCallback; ///< cpu application termination callback
  Ptr<PowerModel> m_powerModel; ///< power model

};

} // namespace ns3

#endif /* CPU_ENERGY_MODEL_HELPER_H */

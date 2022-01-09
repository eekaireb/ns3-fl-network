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

#include "cpu-energy-model-helper.h"
#include "ns3/wifi-net-device.h"
#include "ns3/power-model.h"
#include "ns3/wifi-phy.h"

namespace ns3 {

CpuEnergyModelHelper::CpuEnergyModelHelper ()
{
  m_cpuEnergy.SetTypeId ("ns3::CpuEnergyModel");
  m_depletionCallback.Nullify ();
  m_rechargedCallback.Nullify ();
  m_cpuAppRunCallback.Nullify ();
  m_cpuAppTerminateCallback.Nullify ();
}

CpuEnergyModelHelper::~CpuEnergyModelHelper ()
{
}

void
CpuEnergyModelHelper::Set (std::string name, const AttributeValue &v)
{
  m_cpuEnergy.Set (name, v);
}

void
CpuEnergyModelHelper::SetDepletionCallback (
  CpuEnergyModel::CpuEnergyDepletionCallback callback)
{
  m_depletionCallback = callback;
}

void
CpuEnergyModelHelper::SetRechargedCallback (
  CpuEnergyModel::CpuEnergyRechargedCallback callback)
{
  m_rechargedCallback = callback;
}

void
CpuEnergyModelHelper::SetCpuAppRunCallback (
  CpuEnergyModel::CpuAppRunCallback callback)
{
  m_cpuAppRunCallback = callback;
}

void
CpuEnergyModelHelper::SetCpuAppTerminateCallback (
  CpuEnergyModel::CpuAppTerminateCallback callback)
{
  m_cpuAppTerminateCallback = callback;
}

void
CpuEnergyModelHelper::SetPowerModel (Ptr<PowerModel> powerModel)
{
  NS_ASSERT (powerModel != NULL);
  m_powerModel = powerModel;
}


/*
 * Private function starts here.
 */

Ptr<DeviceEnergyModel>
CpuEnergyModelHelper::DoInstall (Ptr<NetDevice> device,
                                       Ptr<EnergySource> source) const
{
  NS_ASSERT (device != NULL);
  NS_ASSERT (source != NULL);
  // check if device is WifiNetDevice
  std::string deviceName = device->GetInstanceTypeId ().GetName ();
  if (deviceName.compare ("ns3::WifiNetDevice") != 0)
    {
      NS_FATAL_ERROR ("NetDevice type is not WifiNetDevice!");
    }
  Ptr<Node> node = device->GetNode ();
  Ptr<CpuEnergyModel> model = m_cpuEnergy.Create ()->GetObject<CpuEnergyModel> ();
  NS_ASSERT (model != NULL);
  // set energy source pointer
  model->SetEnergySource (source);
  // set energy depletion callback
  // if none is specified, make a callback to WifiPhy::SetSleepMode
  Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice> (device);
  Ptr<WifiPhy> wifiPhy = wifiDevice->GetPhy ();
  if (m_depletionCallback.IsNull ())
    {
      model->SetEnergyDepletionCallback (MakeCallback (&WifiPhy::SetSleepMode, wifiPhy));
    }
  else
    {
      model->SetEnergyDepletionCallback (m_depletionCallback);
    }
  // set energy recharged callback
  // if none is specified, make a callback to WifiPhy::ResumeFromSleep
  if (m_rechargedCallback.IsNull ())
    {
      model->SetEnergyRechargedCallback (MakeCallback (&WifiPhy::ResumeFromSleep, wifiPhy));
    }
  else
    {
      model->SetEnergyRechargedCallback (m_rechargedCallback);
    }
  if (m_cpuAppRunCallback.IsNull ())
    {
      model->SetCpuAppRunCallback (MakeCallback (&WifiPhy::SetSleepMode, wifiPhy));
    }
  else
    {
      model->SetCpuAppRunCallback (m_cpuAppRunCallback);
    }
  if (m_cpuAppTerminateCallback.IsNull ())
    {
      model->SetCpuAppTerminateCallback (MakeCallback (&WifiPhy::ResumeFromSleep, wifiPhy));
    }
  else
    {
      model->SetCpuAppTerminateCallback (m_cpuAppTerminateCallback);
    }
    
  // add model to device model list in energy source
  source->AppendDeviceEnergyModel (model);
  // create and register energy model phy listener
  wifiPhy->RegisterListener (model->GetPhyListener ());
  //
  Ptr<PowerModel> m_powerModel2 = node->GetObject<PowerModel>();
  Ptr<PerformanceModel> m_performanceModel2 = node->GetObject<PerformanceModel>();
  model->SetPowerModel (m_powerModel2);
  model->SetPerformanceModel (m_performanceModel2);
  m_powerModel2->AppendDeviceEnergyModel (model);
  //node->AggregateObject (model);
  return model;
}

} // namespace ns3

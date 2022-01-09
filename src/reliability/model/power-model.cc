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
#include "ns3/power-model.h"
#include "ns3/log.h"



NS_LOG_COMPONENT_DEFINE ("PowerModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PowerModel);

TypeId
PowerModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PowerModel")
    .SetParent<Object> ()
    .SetGroupName ("Power")
  ; 
  return tid;
}

PowerModel::PowerModel ()
{
  NS_LOG_FUNCTION (this);
}

PowerModel::~PowerModel ()
{
  NS_LOG_FUNCTION (this);
}

double
PowerModel::GetPower (void) const
{
  NS_LOG_FUNCTION (this);
  return 2.8;
}

void
PowerModel::AppendDeviceEnergyModel (Ptr<DeviceEnergyModel> deviceEnergyModelPtr)
{
  NS_LOG_FUNCTION (this << deviceEnergyModelPtr);
  NS_ASSERT (deviceEnergyModelPtr != NULL); // model must exist
  m_models.Add (deviceEnergyModelPtr);
  NS_LOG_DEBUG ("PowerModel: Appending power model");
  
}

void
PowerModel::RunApp ()
{
  NS_LOG_FUNCTION (this);
}

void
PowerModel::TerminateApp ()
{
  NS_LOG_FUNCTION (this);
}


void
PowerModel::UpdatePower ()
{
  NS_LOG_FUNCTION (this);
}

/*
 * Private function starts here.
 */

void
PowerModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}

/*
 * Protected function starts here.
 */

void
PowerModel::NotifyAppRun (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("PowerModel: NotifyAppRun");
  // notify all device energy models installed on node
  // DeviceEnergyModelContainer::Iterator i;
  // for (i = m_models.Begin (); i != m_models.End (); i++)
  //   {
  //     TypeId tid = TypeId ("ns3::CpuEnergyModel");
  //     NS_LOG_DEBUG ("CpuEnergyModel:Total energy consumption is " <<
  //                   tid << "J");
  //     if(tid==(*i)->GetTypeId())
  //     {
  //       NS_LOG_DEBUG ("PowerModel: NotifyAppRun");
  //       (*i)->HandleEnergyDepletion ();
  //     }
  //   }
  Ptr<DeviceEnergyModel> model = m_models.Get(0);
  model->HandleEnergyDepletion ();
  
}

void
PowerModel::NotifyAppTerminate (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("PowerModel: NotifyAppTerminate");
  // notify all device energy models installed on node
  // DeviceEnergyModelContainer::Iterator i;
  // for (i = m_models.Begin (); i != m_models.End (); i++)
  //   {
  //     TypeId tid = TypeId ("ns3::CpuEnergyModel");
  //     if(tid==(*i)->GetTypeId())
  //     {
  //       (*i)->HandleEnergyRecharged ();
  //     }
  //   }
  Ptr<DeviceEnergyModel> model = m_models.Get(0);
  model->HandleEnergyRecharged ();
}

} // namespace ns3

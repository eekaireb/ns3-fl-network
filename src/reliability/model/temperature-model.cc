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
#include "temperature-model.h"



NS_LOG_COMPONENT_DEFINE ("TemperatureModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TemperatureModel);

TypeId
TemperatureModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TemperatureModel")
    .SetParent<Object> ()
    .SetGroupName("Temperature")
  ; 
  return tid;
}

TemperatureModel::TemperatureModel ()
{
  NS_LOG_FUNCTION (this);
}

TemperatureModel::~TemperatureModel ()
{
  NS_LOG_FUNCTION (this);
}


double
TemperatureModel::GetTemperature (void) const
{
  NS_LOG_FUNCTION (this);
  return 25.0;
}

double
TemperatureModel::GetAvgTemperature (void) const
{
  NS_LOG_FUNCTION (this);
  return 0.0;
}

void
TemperatureModel::UpdateTemperature (double cpupower)
{
  NS_LOG_FUNCTION (this);
}

void
TemperatureModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}



} // namespace ns3

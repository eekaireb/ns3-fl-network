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
#include "performance-model.h"

NS_LOG_COMPONENT_DEFINE ("PerformanceModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PerformanceModel);

TypeId
PerformanceModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PerformanceModel")
    .SetParent<Object> ()
    .SetGroupName("Performance")
  ; 
  return tid;
}

PerformanceModel::PerformanceModel ()
{
  NS_LOG_FUNCTION (this);
}

PerformanceModel::~PerformanceModel ()
{
  NS_LOG_FUNCTION (this);
}

double
PerformanceModel::GetExecTime (void) const
{
  NS_LOG_FUNCTION (this);
  return 0.0;
}

double
PerformanceModel::GetThroughput (void) const
{
  NS_LOG_FUNCTION (this);
  return 0.0;
}

double
PerformanceModel::GetPacketSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 0.0;
}
double
PerformanceModel::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 0.0;
}

void
PerformanceModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}



} // namespace ns3
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

#ifndef PERFORMANCE_MODEL_H
#define PERFORMANCE_MODEL_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"
#include "ns3/node.h"
#include "ns3/double.h"

namespace ns3 {

class PerformanceModel : public Object
{
public:

  static TypeId GetTypeId (void);
  PerformanceModel () ;
  virtual ~PerformanceModel ();


  /**
  * \returns execution time.
  */
  virtual double GetExecTime (void) const;




 /**
   * Set Application
   */
  virtual void SetApplication (std::string n0, const DoubleValue &v0) = 0;
 /**
   * Set device type
   */
  virtual void SetDeviceType (std::string devicetype) = 0;

 /**
   * Set Packet Size
   */
  virtual void SetPacketSize (const DoubleValue &v1) = 0;
  virtual double GetDataSize (void) const;
  virtual double GetPacketSize (void) const;
  virtual void SetDataSize (const DoubleValue &v0) = 0;
  virtual double GetThroughput (void) const;
  virtual void SetThroughput (double throughput) = 0;

private:
  virtual void DoDispose (void);

};

} // namespace ns3

#endif /* PERFORMANCE_MODEL_H */
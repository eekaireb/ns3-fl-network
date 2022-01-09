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

#ifndef RELIABILITY_MODEL_H
#define RELIABILITY_MODEL_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"
#include "ns3/node.h"
#include "ns3/temperature-model.h"

namespace ns3 {

class ReliabilityModel : public Object
{
public:

  static TypeId GetTypeId (void);
  ReliabilityModel () ;
  virtual ~ReliabilityModel ();
  
  /**
   * \param  Pointer to temperature object attached to the device.
   *
   * Registers the Temperature Model to Power Model.
   */
  virtual void RegisterTemperatureModel (Ptr<TemperatureModel> temperatureModel) = 0;


  /**
   * Updates reliability
   */
  virtual void UpdateReliability (void);

  /**
   * \returns Current reliability
   */
  virtual double GetReliability (void) const;

private:
  virtual void DoDispose (void);


};

} // namespace ns3

#endif /* RELIABILITY_MODEL_H */

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

#ifndef TEMPERATURE_MODEL_H
#define TEMPERATURE_MODEL_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"
#include "ns3/node.h"

namespace ns3 {

class TemperatureModel : public Object
{
public:

  static TypeId GetTypeId (void);
  TemperatureModel () ;
  virtual ~TemperatureModel ();



  /**
   * Updates temperature
   */
  virtual void UpdateTemperature (double cpupower);

  /**
   * \returns Current temperature.
   */
  virtual double GetTemperature (void) const;

  /**
   * \returns Moving average of temperature.
   */
  virtual double GetAvgTemperature (void) const;

  virtual void SetDeviceType(std::string devicetype) = 0;
  virtual void SetTenv(double Tenv) = 0;
  
private:
  virtual void DoDispose (void);


};

} // namespace ns3

#endif /* TEMPERATURE_MODEL_H */

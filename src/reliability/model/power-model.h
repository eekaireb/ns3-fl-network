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

#ifndef POWER_MODEL_H
#define POWER_MODEL_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"
#include "ns3/node.h"
#include "ns3/temperature-model.h"
#include "ns3/performance-model.h"
#include "device-energy-model-container.h"  // #include "device-energy-model.h"
#include "ns3/cpu-energy-model.h"

namespace ns3 {


class PowerModel : public Object
{
public:

  static TypeId GetTypeId (void);
  PowerModel ();
  virtual ~PowerModel ();



  /**
   * \returns State of the power model (idle/busy)
   *
   * Set method is to be defined in child class only if necessary.
   */
  virtual int GetState (void) const = 0;

  /**
   * \returns Energy consumed.
   */
  virtual double GetEnergy (void) const = 0;


  /**
   * \returns Idle Power of the power model.
   */
  virtual double GetIdlePowerW (void) const = 0;

  /**
   * \param  Pointer to temperature object attached to the device.
   *
   * Registers the Temperature Model to Power Model.
   */
  virtual void RegisterTemperatureModel (Ptr<TemperatureModel> temperatureModel) = 0;

  /**
   * \param  Pointer to performance object attached to the device.
   *
   * Registers the Performance Model to Power Model.
   */
  virtual void RegisterPerformanceModel (Ptr<PerformanceModel> performanceModel) = 0;


  /**
   * \param deviceEnergyModelPtr Pointer to device energy model.
   *
   * This function appends a device energy model to the end of a list of
   * DeviceEnergyModelInfo structs.
   */
  void AppendDeviceEnergyModel (Ptr<DeviceEnergyModel> deviceEnergyModelPtr);

  virtual void SetApplication(std::string n0, const DoubleValue &v0) = 0;
  virtual void SetDeviceType(std::string devicetype) = 0;

  //virtual void SetAppName (const std::string &appname) = 0;  
  //virtual void SetDataSize (double datasize) = 0;

  /**
   * \returns Current Power.
   */
  virtual double GetPower (void) const;

  /**
   * Starts the application
   */
  virtual void RunApp ();

  /**
   * Ends the application
   */
  virtual void TerminateApp ();

  /**
   * Updates power
   */
  virtual void UpdatePower ();

private:
  virtual void DoDispose (void);


private:
  /**
   * List of device energy models installed on the same node.
   */
  DeviceEnergyModelContainer m_models;
protected:

  /**
   * This function notifies all DeviceEnergyModel of application run event. It
   * is called by the child PowerModel class when application is run.
   */
  void NotifyAppRun (void);

  /**
   * This function notifies all DeviceEnergyModel of application terminate event. It
   * is called by the child PowerMOdel class when application is terminated.
   */
  void NotifyAppTerminate (void); 
};

} // namespace ns3

#endif /* POWER_MODEL_H */

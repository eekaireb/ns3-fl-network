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

#ifndef APP_POWER_MODEL_H
#define APP_POWER_MODEL_H

#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include "ns3/temperature-model.h"
#include <ns3/performance-model.h>
#include "ns3/power-model.h"


namespace ns3 {

class AppPowerModel : public PowerModel
{
public:

  static TypeId GetTypeId (void);
  AppPowerModel ();

  virtual ~AppPowerModel ();



  /**
   * \param  Pointer to temperature object attached to the device.
   *
   * Registers the Temperature Model to Power Model.
   */
  virtual void RegisterTemperatureModel (Ptr<TemperatureModel> temperatureModel);
  /**
   * \param  Pointer to performance object attached to the device.
   *
   * Registers the Performance Model to Power Model.
   */
  virtual void RegisterPerformanceModel (Ptr<PerformanceModel> performanceModel);


  // Setter & getters.
  virtual double GetA (void) const;
  virtual void SetA (double A);
  virtual double GetB (void) const;
  virtual void SetB (double B);
  virtual double GetC (void) const;
  virtual void SetC (double C);
  virtual double GetDataSize (void) const;
  virtual void SetDataSize (double datasize);
  std::string GetAppName (void) const;
  virtual void SetAppName (const std::string &appname);  
  virtual double GetIdlePowerW (void) const;
  virtual void SetIdlePowerW (double IdlePowerW);
  virtual int GetState (void) const;
  virtual void SetState (int state);
  virtual void SetApplication(std::string appname, const DoubleValue &v0);
  virtual void SetDeviceType(std::string devicetype);
  
  /**
   * \returns Current power.
   */
  virtual double GetPower (void) const;

  /**
   * \returns Total energy to be consumed.
   */
  virtual double GetEnergy (void) const;

  /**
   * \brief Updates the power.
   *
   */
  virtual void UpdatePower ();

  /**
   * \brief Checks if CPU is idle, updates temperature.
   *
   */
  virtual void IsIdle ();

  /**
   * Starts the application
   */
  virtual void RunApp ();

  /**
   * Ends the application
   */
  virtual void TerminateApp ();


private:
  virtual void DoDispose (void);

  /**
   * Handles the application running event. 
   */
  void HandleAppRunEvent (void);

  /**
   * Handles the application terminating.
   */
  void HandleAppTerminateEvent (void);

private:

  Ptr<TemperatureModel> m_temperatureModel;
  Ptr<PerformanceModel> m_performanceModel;


  double m_A;
  double m_B;
  double m_C;
  double m_energy;
  double m_frequency;
  double m_exectime;
  int m_currentState;
  std::string m_appName;
  std::string m_deviceType;
  double m_dataSize;
  double m_idlePowerW;
  // This variable keeps track of the total energy consumed by this model.
  TracedValue<double> m_cpupower;
  // State variables.
  EventId m_powerUpdateEvent;            // energy update event
  Time m_lastUpdateTime;          // time stamp of previous energy update
  Time m_powerUpdateInterval;            // energy update interval

};

} // namespace ns3

#endif /* APP_POWER_MODEL_H */

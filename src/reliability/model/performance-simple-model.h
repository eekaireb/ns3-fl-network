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

#ifndef PERFORMANCE_SIMPLE_MODEL_H
#define PERFORMANCE_SIMPLE_MODEL_H


#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include <ns3/performance-model.h>


namespace ns3 {

class PerformanceSimpleModel : public PerformanceModel
{
public:

  static TypeId GetTypeId (void);
  PerformanceSimpleModel () ;
  virtual ~PerformanceSimpleModel ();

  // Setter & getters
  virtual double GetA (void) const;
  virtual void SetA (double A);
  virtual double GetB (void) const;
  virtual void SetB (double B);
  virtual double GetC (void) const;
  virtual void SetC (double C);
  virtual double GetDataSize (void) const;
  virtual void SetDataSize (const DoubleValue &v0);

  virtual void SetDeviceType (std::string devicetype);
  virtual void SetApplication (std::string m_appName, const DoubleValue &v0);
  virtual double GetPacketSize (void) const;
  virtual void SetPacketSize (const DoubleValue &v1);
  virtual void SetThroughput(double throughput);
  /**
   * \returns execution time.
   */
  virtual double GetExecTime (void) const;

  /**
   * \returns throughput.
   */
  virtual double GetThroughput (void) const;

private:
  virtual void DoDispose (void);

private:


  Time m_lastUpdateTime;          // time stamp of previous energy update
  double m_A;
  double m_B;
  double m_C;
  double m_datasize;
  double m_packetSize;
  std::string m_deviceType;
  TracedValue<double> m_exectime;
  double m_throughput;

};

} // namespace ns3

#endif /* PERFORMANCE_SIMPLE_MODEL_H */
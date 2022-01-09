/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Vishwesh Rege
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
 * Authors: Vishwesh Rege <vrege2012@gmail.com>
 */
#ifndef RELIABILITY_HELPER_H
#define RELIABILITY_HELPER_H

#include <ns3/node-container.h>
#include <ns3/trace-helper.h>
#include <vector>
#include <ns3/power-model.h>
#include <ns3/temperature-model.h>
#include <ns3/performance-model.h>
#include <ns3/reliability-model.h>


namespace ns3 {

/**
 * \ingroup reliability
 *
 * \brief helps to manage and create Reliability objects
 *
 * This class can help to create IEEE 802.15.4 NetDevice objects
 * and to configure their attributes during creation.  It also contains
 * additional helper functions used by client code.
 */

class ReliabilityHelper
{
public:
  /**
   * \brief Create a LrWpan helper in an empty state.  By default, a
   * SingleModelSpectrumChannel is created, with a 
   * LogDistancePropagationLossModel and a ConstantSpeedPropagationDelayModel.
   *
   * To change the channel type, loss model, or delay model, the Get/Set
   * Channel methods may be used.
   */
  ReliabilityHelper (void);

  virtual ~ReliabilityHelper (void);

  /**
   * \brief Install the reliability stack(power,performance,temperature,reliability) to a node.
   * \param node a node
   * \returns .
   */
  void Install (Ptr<Node> node);

  /**
   * \brief Install the reliability stack(power,performance,temperature,reliability) in the nodes.
   * \param container a NodeContainer
   * \returns .
   */
  void Install (NodeContainer c);
  
  void SetPowerModel (std::string type,
                       std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                       std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                       std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                       std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                       std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                       std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                       std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue (),
                       std::string n8 = "", const AttributeValue &v8 = EmptyAttributeValue (),
                       std::string n9 = "", const AttributeValue &v9 = EmptyAttributeValue ());

  void SetPerformanceModel (std::string type,
                       std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                       std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                       std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                       std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                       std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                       std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                       std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue (),
                       std::string n8 = "", const AttributeValue &v8 = EmptyAttributeValue (),
                       std::string n9 = "", const AttributeValue &v9 = EmptyAttributeValue ());
 
  void SetTemperatureModel (std::string type,
                       std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                       std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                       std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                       std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                       std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                       std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                       std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue (),
                       std::string n8 = "", const AttributeValue &v8 = EmptyAttributeValue (),
                       std::string n9 = "", const AttributeValue &v9 = EmptyAttributeValue ());

  void SetReliabilityModel (std::string type,
                       std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                       std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                       std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                       std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                       std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                       std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                       std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue (),
                       std::string n8 = "", const AttributeValue &v8 = EmptyAttributeValue (),
                       std::string n9 = "", const AttributeValue &v9 = EmptyAttributeValue ());

  void SetApplication(std::string n0, const DoubleValue &v0, const DoubleValue &v1);
  void SetDeviceType(std::string devicetype);
  void SetAmbientTemperature(double Tenv);
  
private:
  ObjectFactory m_power; //!< Object factory to create power model objects
  ObjectFactory m_performance; //!< Object factory to create performance model objects
  ObjectFactory m_temperature; //!< Object factory to create temperature model objects
  ObjectFactory m_reliability; //!< Object factory to create reliability model objects
  std::string m_appName;
  std::string m_deviceType;
  double m_dataSize;
  double m_packetSize;
  double m_Tenv;
  // Disable implicit constructors
  /**
   * \brief Copy constructor - defined and not implemented.
   */
  ReliabilityHelper (ReliabilityHelper const &);
  /**
   * \brief Copy constructor - defined and not implemented.
   * \returns
   */
  ReliabilityHelper& operator= (ReliabilityHelper const &);


};

}

#endif /* RELIABILITY_HELPER_H */

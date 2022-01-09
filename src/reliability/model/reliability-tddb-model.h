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

#ifndef RELIABILITY_TDDB_MODEL_H
#define RELIABILITY_TDDB_MODEL_H


#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include "ns3/reliability-model.h"
#include "ns3/temperature-model.h"


namespace ns3 {

class ReliabilityTDDBModel : public ReliabilityModel
{
public:

  static TypeId GetTypeId (void);
  ReliabilityTDDBModel ();
  virtual ~ReliabilityTDDBModel ();


  /**
   * \param  Pointer to temperature object attached to the device.
   *
   * Registers the Temperature Model to Power Model.
   */
  virtual void RegisterTemperatureModel (Ptr<TemperatureModel> temperatureModel);


  // Setter & getters for state power consumption.
  virtual double GetA (void) const;
  virtual void SetA (double A);
  virtual double GetB (void) const;
  virtual void SetB (double B);
  virtual double GetArea (void) const;
  virtual void SetArea (double area);

  /**
   * \returns Current reliability
   */
  virtual double GetReliability (void) const;

  // Utility functions
  virtual double g(double u , double v , double t_0 , double scale_p , double shape_p) const;
  virtual double pdf_u(double x , double mean , double sigma) const;
  virtual double pdf_v(double v , double offset , double mult , double degrees) const;
  virtual double scale_par(double T , double V , double offset_a , double mult_a , double tau_a , double tauvolt_a) const;
  virtual double shape_par(double T , double V , double mult_b , double tau_b , double offset_b , double multvolt_b) const; 
  virtual double Chi_Square_Density(double x , double n) const;
  virtual double Ln_Gamma_Function(double x) const;
  /**
   * \brief 
   *
   * \param cpupower, temperature.
   *
   * Updates reliability.
   */
  virtual void UpdateReliability ();

private:
  virtual void DoDispose (void);

private:

  double m_A;
  double m_B;
  double m_area;
  double voltage;
  //Scale-Shape Constants


  double offset_a;
  double mult_a;
  double tau_a;
  double tauvolt_a;
  double mult_b;
  double tau_b;
  double offset_b;
  double multvolt_b;

  //Reliability Parameters

  double pdf_v_offset;
  double pdf_v_mult;
  double pdf_v_degrees;
  double pdf_u_mean;
  double pdf_u_sigma;
  double scale_parameter;
  double shape_parameter;

  //Double integral domain
  double u_max;
  double u_min;
  double v_max;
  double v_min;
  double subdomain_step_u;
  double subdomain_step_v;
  int u_num_step;
  int v_num_step;
  double subdomain_area;

  double A;
  int LI_index;
  double delta_LI;
  double t_life;
  double Rd;



  Ptr<TemperatureModel> m_temperatureModel;

  // This variable keeps track of the reliability of this model.
  TracedValue<double> m_reliability;
  EventId m_reliabilityUpdateEvent;            // energy update event
  Time m_reliabilityUpdateInterval;
  // State variables.
  Time m_lastUpdateTime;          // time stamp of previous energy update

};

} // namespace ns3

#endif /* RELIABILITY_TDDB_MODEL_H */

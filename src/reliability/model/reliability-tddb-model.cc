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
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include "ns3/traced-value.h"
#include "reliability-tddb-model.h"
#include <math.h>
#include <float.h> 
#include <limits.h>

NS_LOG_COMPONENT_DEFINE ("ReliabilityTDDBModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ReliabilityTDDBModel);

TypeId
ReliabilityTDDBModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ReliabilityTDDBModel")
    .SetParent<ReliabilityModel> ()
    .SetGroupName("Reliability")
    .AddConstructor<ReliabilityTDDBModel> ()
    .AddAttribute ("A",
                   "Parameter A.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&ReliabilityTDDBModel::SetA,
                                       &ReliabilityTDDBModel::GetA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("B",
                   "Parameter B.",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&ReliabilityTDDBModel::SetB,
                                       &ReliabilityTDDBModel::GetB),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("area",
                   "Area.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&ReliabilityTDDBModel::SetArea,
                                       &ReliabilityTDDBModel::GetArea),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("Reliability",
                     "Reliability of the device.",
                     MakeTraceSourceAccessor (&ReliabilityTDDBModel::m_reliability),
                     "ns3::TracedValue::DoubleCallback")
  ; 
  return tid;
}

ReliabilityTDDBModel::ReliabilityTDDBModel ()
{
  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  m_reliabilityUpdateInterval = Seconds(30);
  m_reliability = 1.0;
  m_reliabilityUpdateEvent = Simulator::Schedule (m_reliabilityUpdateInterval,&ReliabilityModel::UpdateReliability,this);
 
  voltage = 1.1;
  offset_a = 3;   
  mult_a = 95;    
  tau_a = 0.01;
  tauvolt_a = 3;
  mult_b = 3;
  tau_b = 0.01;
  offset_b = 10; 
  multvolt_b = 7;

  pdf_v_offset = 1.8502e-005;   
  pdf_v_mult = 1.4500e-005;         
  pdf_v_degrees = 8.77;       
  pdf_u_mean = 0.65;       
  pdf_u_sigma = 0.0087;   

  u_max = 0.7;
  u_min = 0.6;
  v_max = 0.00040;  
  v_min = 0;
  subdomain_step_u = 0.0025;
  subdomain_step_v = 0.000010;
  subdomain_area = 0.000000025;
  u_num_step = 41;
  v_num_step = 41;
  A = 1;
  LI_index = 0;
  delta_LI = 30 * 24 *60 * 60 ;   //36.5 for 3 years!  //73 for 5 years;
  t_life = 5*365*24*60*60;
  Rd = 1;


}

ReliabilityTDDBModel::~ReliabilityTDDBModel ()
{
  NS_LOG_FUNCTION (this);
}


void
ReliabilityTDDBModel::RegisterTemperatureModel (Ptr<TemperatureModel> temperatureModel)
{
  m_temperatureModel = temperatureModel;
}

void
ReliabilityTDDBModel::SetA (double A)
{
  NS_LOG_FUNCTION (this);
  m_A = A;
}

double
ReliabilityTDDBModel::GetA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_A;
}

void
ReliabilityTDDBModel::SetB (double B)
{
  NS_LOG_FUNCTION (this);
  m_B = B;
}

double
ReliabilityTDDBModel::GetB (void) const
{
  NS_LOG_FUNCTION (this);
  return m_B;
}

void
ReliabilityTDDBModel::SetArea (double area)
{
  NS_LOG_FUNCTION (this);
  m_area = area;
}

double
ReliabilityTDDBModel::GetArea (void) const
{
  NS_LOG_FUNCTION (this);
  return m_area;
}

double
ReliabilityTDDBModel::GetReliability (void) const
{
  NS_LOG_FUNCTION (this);
  return m_reliability;
}

double
ReliabilityTDDBModel::g (double u , double v , double t_0 , double scale_p , double shape_p) const
{
  NS_LOG_FUNCTION (this);
	double r , f , value;
	r = log ( t_0 / scale_p ) * shape_p * u ;
	f = log ( (t_0 / scale_p)*(t_0 / scale_p) ) * (shape_p * shape_p) * v * 0.5; 
	value = exp ( r + f );
	return value;
}

double
ReliabilityTDDBModel::pdf_u(double x , double mean , double sigma) const
{
  NS_LOG_FUNCTION (this);
	double value , normalization ; 	
	normalization = (0.5 * M_SQRT1_2 * M_2_SQRTPI ) / sigma;
	value = normalization * exp ( - ( x - mean )*( x - mean ) / (2 * sigma * sigma )  );
	return value;
}
	 
double 
ReliabilityTDDBModel::pdf_v(double v , double offset , double mult , double degrees) const
{
  NS_LOG_FUNCTION (this);
	double multinv , value;
	multinv = 1/mult ; 
	value = multinv * (Chi_Square_Density (  multinv*(v - offset)  , degrees ) );
	return value;
}	 

double 
ReliabilityTDDBModel::Chi_Square_Density(double x , double n) const
{
   double n2 = 0.5 * (double) n;
   double ln_density;

   if ( x < 0.0 ) return 0.0;
   if ( x == 0.0 ) {
      if ( n == 1 ) return DBL_MAX;
      if ( n == 2 ) return 0.5;
      return 0.0;
   }
   ln_density = (n2 - 1.0) * log(0.5 * x) - 0.5 * x - Ln_Gamma_Function(n2);
   return 0.5 * exp(ln_density);
}

double 
ReliabilityTDDBModel::Ln_Gamma_Function(double x) const
{
  // For a positive argument, 0 < x <= Gamma_Function_Max_Arg() //
  // then  return log Gamma(x).                                 //

  return lgamma(x);

}


double 
ReliabilityTDDBModel::scale_par(double T , double V , double offset_a , double mult_a , double tau_a , double tauvolt_a) const
{
        double value;
        value = offset_a + mult_a * exp(-tau_a*T) * exp(-tauvolt_a*V)    ;
        return value;
}

double 
ReliabilityTDDBModel::shape_par(double T , double V , double mult_b , double tau_b , double offset_b , double multvolt_b) const
{
        double value;
        value = mult_b * exp(-tau_b*T) + offset_b - (multvolt_b*V);
        return value;
}



void
ReliabilityTDDBModel::UpdateReliability ()
{
  NS_LOG_FUNCTION (this << m_reliability << Simulator::Now ().GetSeconds ());
  double temperature = m_temperatureModel->GetAvgTemperature();

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetNanoSeconds () >= 0); // check if duration is valid


  scale_parameter = 365*24*60*60*scale_par(temperature,voltage,offset_a,mult_a,tau_a,tauvolt_a);
  shape_parameter = shape_par(temperature,voltage,mult_b,tau_b,offset_b,multvolt_b); 

	int i, j;
  double u , v , t_0 , g_value , exponential_value , pdf_u_value , pdf_v_value , 
  joint_pdf_value , integral_subdomain , g_value_prec , exponential_value_prec , 
  integral_subdomain_prec , damage;
	double Rc  , Rc_prec ;	

  LI_index++;
  u = u_min;
	v = v_min;
	t_0 = LI_index * delta_LI ; 

	for (i=0 ; i < u_num_step ; i++){
		for (j=0 ; j < v_num_step ; j++){


			g_value = g(u + 0.5*subdomain_step_u ,v + 0.5*subdomain_step_v ,t_0 , scale_parameter , shape_parameter);
			exponential_value = exp( -A*g_value);
			pdf_u_value = pdf_u( u + 0.5*subdomain_step_u , pdf_u_mean , pdf_u_sigma);
			pdf_v_value = pdf_v(  v + 0.5*subdomain_step_v ,  pdf_v_offset ,  pdf_v_mult ,  pdf_v_degrees);     
			joint_pdf_value = pdf_u_value*pdf_v_value ;
			integral_subdomain= exponential_value*joint_pdf_value*subdomain_area;
			Rc = Rc + integral_subdomain;

			if (LI_index > 1){
				g_value_prec = g(u + 0.5*subdomain_step_u ,	v + 0.5*subdomain_step_v ,t_0 - delta_LI , scale_parameter , shape_parameter);
				exponential_value_prec = exp( -A*g_value_prec);
				integral_subdomain_prec = exponential_value_prec*joint_pdf_value*subdomain_area;
				Rc_prec = Rc_prec + integral_subdomain_prec;
			}
			else{
				Rc_prec = 1 ;
			}
      v = v + subdomain_step_v;
    }//for (j=0 ; j<v_num_step ; j++)
  v = v_min;
	u = u + subdomain_step_u;
	} //for (i=0 ; i < u_num_step ; i++)
  damage = Rc_prec - Rc ;
  m_reliability = m_reliability - damage ; 

  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();
  m_reliabilityUpdateEvent = Simulator::Schedule (m_reliabilityUpdateInterval,&ReliabilityModel::UpdateReliability,this);


}

void
ReliabilityTDDBModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  //m_source = NULL;
}


} // namespace ns3

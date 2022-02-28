#include "fl-energy.h"

namespace ns3 {

    FLEnergy::FLEnergy()
    {
    }

    double FLEnergy::GetA (void) const
    {
    return m_A;
    }
    void FLEnergy::SetA (double A)
    {
    m_A=A;
    }

    double FLEnergy::GetB (void) const
    {
        return m_B;
    }
    void FLEnergy::SetB (double B)
    {
        m_B=B;
    }


    double FLEnergy::GetC (void) const{
        return m_C;
    }

    void FLEnergy::SetC (double C)
    {
        m_C=C;
    }


    double FLEnergy::GetD (void) const
    {
        return m_D;
    }


    void FLEnergy::SetD (double D)
    {
        m_D=D;
    }

    void FLEnergy::SetEpochs(double epochs)
    {
        m_epochs=epochs;
    }
    double FLEnergy::GetEpochs() const
    {
        return m_epochs;
    }

    void FLEnergy::SetModelSize(double modelSize)
    {
        m_ModelSize=modelSize;
    }
    double FLEnergy::GetModelSize() const
    {
        return m_ModelSize;
    }

    void FLEnergy::SetLearningModel(std::string name)
    {
        m_learningModel=name;
        if (m_learningModel == "MNIST")
        {
            if(m_deviceType == "4")
            {
                m_A = 8.09 * 0.0001; //constants for comp time
                m_B = 9.45;
                m_C = -1.32 * 0.0001;        // constants for comp power
                m_D =  6.49;
                m_tp = 3.9422;
            }
            else {
                m_A = 3.32 * 0.0001; //constants for comp time
                m_B = 16.6;
                m_C = -1.55 * 0.0001;        // constants for comp power
                m_D = 6.41;
                m_tp = 3.8896;
            }
            m_freq = 1500;
            m_MAC = 229300000;
        }
        else if(m_learningModel == "FashionMNIST")
        {
            if(m_deviceType == "4")
            {
                m_A = 1.08 * 0.001; //constants for comp time
                m_B = 11.4;
                m_C = -1.04 * 0.0001;        // constants for comp power
                m_D =  6.4;
                m_tp = 4.766;
            }
            else {
                m_A = 8.13 * 0.0001; //constants for comp time
                m_B = 15.7;
                m_C = -1.27 * 0.0001;        // constants for comp power
                m_D = 6.4;
                m_tp = 4.532;
            }
            m_freq = 1500;
            m_MAC = 285689600;
        }
        else if(m_learningModel == "CIFAR-10")
        {
            if(m_deviceType == "4")
            {
                m_A = 0.0108; //constants for comp time
                m_B = -4.43;
                m_C = -4.42 * 0.0001;        // constants for comp power
                m_D =  6.18;
                m_tp = 4.397;
            }
            else {
                m_A = 8.47 * 0.001; //constants for comp time
                m_B = 3.32;
                m_C = -5.65 * 0.0001;        // constants for comp power
                m_D = 6.31;
                m_tp = 4.211;
            }
            m_freq = 1500;
            m_MAC = 61572000;
        }
    }

    const std::string& FLEnergy::GetLearningModel() const
    {
        return m_learningModel;
    }


    void FLEnergy::SetDeviceType(std::string deviceType)
    {
        m_deviceType=deviceType;
    }

    const std::string & FLEnergy::GetDeviceType() const
    {
        return m_deviceType;
    }

    double FLEnergy::CalcComputationalPower()
    {
        long double n = m_C * (m_MAC * (100 + (6 * m_epochs))) * 600;
        long double d = m_freq*1000000;
        double power = (n/d) + m_D;
        return power;
    }
    double FLEnergy::CalcTransmissionPower()
    {
        return m_tp;
    }

    double FLEnergy::CalcTransmissionEnergy(double time)
    {
        return m_tp * time;
    }
    double FLEnergy::CalcComputationalEnergy(double time)
    {
        long double n = m_C * (m_MAC * (100 + (6 * m_epochs))) * 600;
        long double d = m_freq*1000000;
        double power = (n/d) + m_D;
        return power * time;
    }

    double FLEnergy::CalcComputationTime()
    {
        long double n = m_A * (m_MAC * (100 + (6 * m_epochs))) * 600;
        long double d = m_freq*1000000;
        double time = (n/d) + m_B;
        return time;
    }

}
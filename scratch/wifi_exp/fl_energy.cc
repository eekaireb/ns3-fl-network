#include "fl_energy.h"

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

    double FLEnergy::CalcComputationalPower(double time)
    {
        return 0.0;
    }
    double FLEnergy::CalcTransmissionPower(double time)
    {
        return 0.0;
    }


    double FLEnergy::CalcTransmissionEnergy(double time)
    {
        return (m_A * time) + m_B ;
    }
    double FLEnergy::CalcComputationalEnergy(double time)
    {
        return ((m_C * time) + m_D) * m_epochs;
    }

}
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"
#include "ns3/node.h"
#include "ns3/double.h"
#include <string>

namespace ns3 {
/**
 * \ingroup fl-client-session-manager
 * \brief Manages the client session
 */
class FLEnergy{
public:
    /**
    * \brief Construct energy model
    * \param inn    map of < client ids, client sessions >
    */
    FLEnergy();


    // Setter & getters
    void SetA(double A);
    double GetA (void) const;
    void SetB (double B);
    double GetB (void) const;
    void SetC (double C);
    double GetC (void) const;
    void SetD (double D);
    double GetD (void) const;
    void SetEpochs(double epochs);
    double GetEpochs() const;
    void SetModelSize(double modelSize);
    double GetModelSize() const;
    void SetLearningModel(std::string name);
    const std::string& GetLearningModel() const;
    void SetDeviceType(std::string deviceType);
    const std::string & GetDeviceType() const;
    double CalcComputationalPower();
    double CalcTransmissionPower();
    double CalcTransmissionEnergy(double time);
    double CalcComputationalEnergy(double time);
    double CalcComputationTime();

private:

    double m_A;
    double m_B;
    double m_C;
    double m_D;
    double m_tp;
    double m_freq;
    double m_epochs;
    double m_ModelSize;
    double m_MAC;
    std::string m_learningModel;
    std::string m_deviceType;

};

}


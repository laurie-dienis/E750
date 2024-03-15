#ifndef HistConfig_h
#define HistConfig_h

#include "ROOT/RDF/HistoModels.hxx"
namespace HistConfig
{

inline ROOT::RDF::TH2DModel ESilR {"hESilR", "Range vs E_{sil};BSP [mm];E_{Sil} [MeV]", 128, 0, 256, 200, 0, 15};

inline ROOT::RDF::TH2DModel Kin {"hKin", "Kinematics;#theta_{Lab} [#circ];E_{Vertex} [MeV]", 200, 0,100, 200, 0, 30};

inline ROOT::RDF::TH2DModel ThetaCMECM {
    "hCM", "#theta_{CM} vs E_{CM};#theta_{CM} [#circ];E_{CM} [MeV]", 200, 0, 60, 200, 19500, 19600};

inline ROOT::RDF::TH2DModel ThetaLabEReac {"hLab", "#theta_{Lab} vs E_{reac};#theta_{Lab} [#circ];E_{reac} [MeV]", 200, 0, 60, 200, 0, 60};

} // namespace HistConfig

#endif // !HistConfig_h

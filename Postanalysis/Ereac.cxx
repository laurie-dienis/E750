#include "ActKinematics.h"
#include "ActMergerData.h"
#include "ActSRIM.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"

#include <vector>

#include "../HistConfig.h"
void Ereac()
{
    // Read data
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"Pipe1_Tree", "./Outputs/Pipe1/elastic.root"};

    // Build all the kinematics
    std::vector<ActPhysics::Kinematics> vkins;
    for(int s = 0; s < df.GetNSlots(); s++)
        vkins.push_back({"20Ne", "1H", "1H", "20Ne", -1, 0});

    // Read srim tables
    auto* srim {new ActPhysics::SRIM};
    srim->ReadTable("light", "./Inputs/SRIM/1H_butane_110mbar.txt");

    // 1-> Define energy at the vertex
    auto def = df.Define("EVertex",
                         [&](const ActRoot::MergerData& d, double esil)
                         { return srim->EvalInitialEnergy("light", esil, d.fTrackLength); },
                         {"MergerData", "ESil"});

    // 2-> Define energy of beam at RP from BSP
    def = def.DefineSlot(
        "EReac",
        [&](unsigned int slot, double evertex, float theta)
        { return vkins[slot].ReconstructBeamEnergyFromLabKinematics(evertex, theta * TMath::DegToRad()); },
        {"EVertex", "fThetaLight"});

    // 3-> Energy at CM
    def = def.DefineSlot("ECM",
                         [&](unsigned int slot, double ereac)
                         {
                             vkins[slot].SetBeamEnergy(ereac);
                             return vkins[slot].GetECM();
                         },
                         {"EReac"});

    // 4-> ThetaCM?
    def = def.DefineSlot("ThetaCM",
                         [&](unsigned int slot, double ereac, double evertex, float theta)
                         {
                             vkins[slot].SetBeamEnergy(ereac);
                             return vkins[slot].ReconstructTheta3CMFromLab(evertex, theta);
                         },
                         {"EReac", "EVertex", "fThetaLight"});
    // Book some histograms
    auto hKin {def.Histo2D(HistConfig::Kin, "fThetaLight", "EVertex")};
    auto hCM {def.Histo2D(HistConfig::ThetaCMECM, "ThetaCM", "ECM")};
    auto hLab {def.Histo2D(HistConfig::ThetaLabEReac, "fThetaLight", "EReac")};
    auto hEReac {def.Histo1D(HistConfig::TBeam, "EReac")};
    hEReac->SetTitle("T_{beam} from BSP and E_{x} = 0");
    auto hEBeamRP {def.Histo1D(HistConfig::TBeam, "EBeamRP")};
    hEBeamRP->SetTitle("T_{beam} from SRIM and RP");

    // Plot theoretical kin at beam entrance
    ActPhysics::Kinematics theo {"20Ne", "1H", "1H", "20Ne", 89.34};
    auto* gtheo {theo.GetKinematicLine3()};

    // Plotting
    auto* c0 {new TCanvas {"c20", "Pipe 2 canvas 0"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hKin->DrawClone("colz");
    gtheo->Draw("l");
    c0->cd(2);
    hLab->DrawClone("colz");
    c0->cd(3);
    hEReac->DrawClone();
    c0->cd(4);
    hEBeamRP->DrawClone();
}

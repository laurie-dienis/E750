#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActKinematics.h"
#include "ActMergerData.h"
#include "ActSRIM.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"
#include "TString.h"

#include <vector>

#include "../HistConfig.h"
void ChannelSelection()
{
    ActRoot::DataManager datman {"../configs/data.conf", ActRoot::ModeType::EMerge};
    auto chain {datman.GetJoinedData()};
    // Enable MT
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame d {*chain};

    // Basic cuts based on multiplicity of silicon energies
    auto df {d.Filter("fSilLayers.size() == 1")};
    df = df.Filter("fSilNs.front() == 8");
    df = df.Define("ESil", "fSilEs.front() * (9.5 / 5860)");

    // Book histograms
    auto hESilR {df.Histo2D(HistConfig::ESilR, "fBSP.fCoordinates.fX", "ESil")};

    // SRIM table to compute EBeam
    auto* srim {new ActPhysics::SRIM};
    srim->ReadTable("beam", "./Inputs/SRIM/20Ne_butane_110mbar.txt");

    // Define beam energy from RP!
    // Initial beam energy
    double TIni {89.34}; // MeV
    df = df.Define("EBeamRP",
                   [&](const ActRoot::MergerData& d)
                   { return srim->Slow("beam", TIni, d.fRP.X(), d.fThetaBeam * TMath::DegToRad()); },
                   {"MergerData"});
    // Build Ex
    std::vector<ActPhysics::Kinematics> vkins;
    for(int s = 0; s < df.GetNSlots(); s++)
        vkins.push_back({"20Ne", "1H", "1H", "20Ne"});
    df = df.DefineSlot("Ex",
                       [&](unsigned int slot, double ebeam, double esil, float theta)
                       {
                           vkins[slot].SetBeamEnergy(ebeam);
                           return vkins[slot].ReconstructExcitationEnergy(esil, theta * TMath::DegToRad());
                       },
                       {"EBeamRP", "ESil", "fThetaLight"});
    // Book histograms
    auto hEBeamRP {df.Histo1D(HistConfig::TBeam, "EBeamRP")};
    hEBeamRP->SetTitle("T_{beam} from SRIM and RP");
    auto hEBeamX {df.Histo2D(HistConfig::TBeamRPx, "fRP.fCoordinates.fX", "EBeamRP")};
    auto hEx {df.Histo1D(HistConfig::Ex, "Ex")};
    auto hESilEx {df.Histo2D(HistConfig::ESilEx, "ESil", "Ex")};

    // Read cuts
    ActRoot::CutsManager<int> cuts;
    cuts.ReadCut(0, TString::Format("./Cuts/elastic.root"));
    cuts.ReadCut(1, TString::Format("./Cuts/inelastic.root"));
    if(cuts.GetCut(0)) // elastic
    {
        auto gated {df.Filter([&](float bspx, double esil) { return cuts.IsInside(0, bspx, esil); },
                              {"fBSP.fCoordinates.fX", "ESil"})};
        std::cout << "Saving elastic tree" << '\n';
        gated.Snapshot("Pipe1_Tree", "./Outputs/Pipe1/elastic.root");
    }
    if(cuts.GetCut(1)) // inelastic
    {
        auto gated {df.Filter([&](float bspx, double esil) { return cuts.IsInside(1, bspx, esil); },
                              {"fBSP.fCoordinates.fX", "ESil"})};
        std::cout << "Saving inelastic tree" << '\n';
        gated.Snapshot("Pipe1_Tree", "./Outputs/Pipe1/inelastic.root");
    }


    // Plotting
    auto* c0 {new TCanvas {"c10", "Pipe 1 canvas 0"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hESilR->DrawClone("colz");
    cuts.DrawAll();
    c0->cd(2);
    hEBeamRP->DrawClone();
    c0->cd(3);
    hEx->DrawClone();
    c0->cd(4);
    hESilEx->DrawClone("colz");
    c0->cd(5);
    hEBeamX->DrawClone("colz");
}

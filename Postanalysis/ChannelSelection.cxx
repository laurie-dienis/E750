#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "ActKinematics.h"
#include "TROOT.h"
#include "TString.h"

#include "/home/laurie/E750/HistConfig.h"

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

    // Read cuts
    ActRoot::CutsManager<int> cuts;
    cuts.ReadCut(0, TString::Format("/home/laurie/E750/Postanalysis/Cuts/elastic.root").Data());
    cuts.ReadCut(1, TString::Format("/home/laurie/E750/Postanalysis/Cuts/inelastic.root").Data());
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
    hESilR->DrawClone("colz");
    cuts.DrawAll();
}

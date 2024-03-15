#include "ActDataManager.h"
#include "ActTPCData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <fstream>

void PlotRP()
{
    // ROOT::EnableImplicitMT();
    // Get data
    ActRoot::DataManager datman {"../configs/data.conf", ActRoot::ModeType::EFilter};
    auto chain {datman.GetJoinedData()};

    // DF
    ROOT::RDataFrame df {*chain};

    // Filter events with RP
    auto gated {df.Filter([](const ActRoot::TPCData& d) { return d.fRPs.size() > 0; }, {"TPCData"})};
    gated = gated.Define("RPx", "fRPs.front().X()").Define("RPy", "fRPs.front().Y()").Define("RPz", "fRPs.front().Z()");

    // Plot
    auto hRPxy {gated.Histo2D({"hRPxy", "RP;X [pad];Y [pad]", 200, 0, 128, 200, 0, 128}, "RPx", "RPy")};
    auto hRPxz {gated.Histo2D({"hRPxz", "RP;X [pad];Z [tb]", 200, 0, 128, 200, 0, 128}, "RPx", "RPz")};
    auto hRPyz {gated.Histo2D({"hRPyz", "RP;Y [pad];z [tb]", 200, 0, 128, 200, 0, 128}, "RPy", "RPz")};

    // Visual debug of RP with low Z
    auto visual {gated.Filter("10 <= RPz && RPz <= 25")};
    std::ofstream streamer {"manual_RPz_low.dat"};
    int entry {};
    df.Foreach(
        [&](const ActRoot::TPCData& d)
        {
            if(d.fRPs.size() > 0)
            {
                auto z {d.fRPs.front().Z()};
                if(10 <= z && z <= 25)
                {
                    streamer << 84 << " " << entry << '\n';
                }
            }
            entry++;
        },
        {"TPCData"});
    streamer.close();


    // Plotting
    auto* c0 {new TCanvas {"c0", "RP canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hRPxy->DrawClone("colz");
    c0->cd(2);
    hRPxz->DrawClone("colz");
    c0->cd(3);
    hRPyz->DrawClone("colz");
}

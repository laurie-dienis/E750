#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"

#include <vector>
void PlotSP()
{
    ActRoot::DataManager datman {"../configs/data.conf", ActRoot::ModeType::EMerge};
    auto chain {datman.GetJoinedData()};
    ROOT::RDataFrame df {*chain};

    auto gated {df.Filter("fSilLayers.size() == 1")};
    //gated.Describe().Print();

    auto hSP {gated.Histo2D({"hSP", "SP;Y [pad];Z [tb]", 200, 0, 280, 200, 100, 450}, "fSP.fCoordinates.fY",
                            "fSP.fCoordinates.fZ")};
    // SP per silicon
    int nsil {20};
    std::vector<TH1D*> h1ds;
    std::vector<TH2D*> hs, hsq;
    for(int s = 0; s < nsil; s++)
    {
        h1ds.push_back(new TH1D {TString::Format("h1D%d", s), TString::Format("Raw SilE for %d", s), 8192, 0, 20000});
        hs.push_back(new TH2D {TString::Format("hSP%d", s), TString::Format("SP for %d", s), 200, 0, 280, 200, 100, 450});
        hsq.push_back( new TH2D {TString::Format("hQ%d", s), TString::Format("Q vs ESil for %d", s), 500, 0, 15000, 1000, 0, 3000});
    }
    TH2D * h6 = (new TH2D {"h6", "h6", 200, 0, 128, 200, 0, 300});
    gated.Foreach(
        [&](const ActRoot::MergerData& d)
        {
            if(d.fSilLayers.size() == 1)
            {
                h1ds[d.fSilNs.front()]->Fill(d.fSilEs.front());
                hs[d.fSilNs.front()]->Fill(d.fSP.Y(), d.fSP.Z());
                if (d.fSilNs.front()==6){
                h6->Fill(d.fSP.Y(), d.fSP.Z());
                }
                hsq[d.fSilNs.front()]->Fill(d.fSilEs.front(), d.fQave);
            }
        },
        {"MergerData"});

    // Plot
    auto* c0 {new TCanvas {"c0", "SP canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hSP->DrawClone("colz");
    c0->cd(2);
    std::vector<int> colors {kYellow,   kOrange, kRed,  kMagenta - 10, kMagenta, kViolet,
                             kBlue + 2, kAzure,  kCyan, kTeal,         kSpring,  kGreen + 2, 
                             kYellow,   kOrange, kRed,  kMagenta - 10, kMagenta, kViolet,
                             kBlue + 2, kAzure};
    for(int i = 0; i < 20; i++)
    {
        hs[i]->SetMarkerStyle(6);
        hs[i]->SetMarkerColor(colors[i]);
        if(i == 0)
            hs[i]->Draw("scat");
        else
            hs[i]->Draw("scat same");
    }

    // // Plotting
    auto* c1 {new TCanvas {"c1", "Raw silE canvas"}};
    c1->DivideSquare(nsil);
    std::vector<int> placement {16,17,18,19,20,11,12,13,14,15,6,7,8,9,10,1,2,3,4,5};
    for(int i = 0; i < nsil; i++)
    {
        c1->cd(placement[i]);
        h1ds[i]->Draw();
    }
    
    auto* c2 {new TCanvas {"c2", "Q vs E sil canvas"}};
    c2->DivideSquare(nsil);
    //for(int i = 0; i < nsil; i++)
    //{
     //   c2->cd(placement[i]);
       // hsq[i]->Draw();
    //}
    // Stack
    auto* qstack {new THStack};
    for(auto& h : hsq)
        qstack->Add(h, "colz");

    qstack->Draw("pads");
}

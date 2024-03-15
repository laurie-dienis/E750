#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"

#include <vector>
void Ex()
{
    double m1 = 1.;
    double m2 = 1.;
    double m3 = 1.;
    double m4 = 1.;
    double Ebeam = 1.; 
    double Ex = 0.;
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
        hsq.push_back( new TH2D {TString::Format("hQ%d", s), TString::Format("Q vs ESil for %d", s), 250, 0, 15, 100, 0, 2000});
    }
    TH2D * hbsp = (new TH2D {"hbsp", "hbsp", 200, 10, 30, 200, 0, 12});
    //Kinematics(m1,m2,m3,m4,Ebeam,Ex);
    gated.Foreach(
        [&](const ActRoot::MergerData& d)
        {
            if(d.fSilLayers.size() == 1)
            {
                h1ds[d.fSilNs.front()]->Fill(d.fSilEs.front());
                hs[d.fSilNs.front()]->Fill(d.fSP.Y(), d.fSP.Z());
                hsq[d.fSilNs.front()]->Fill(d.fSilEs.front()*(9.5/5860.), d.fQave);
                if (d.fSilNs.front()==8){
                hbsp->Fill(d.fBSP.x()/10., d.fSilEs.front()*(9.5/5860.));
                }
            }
        },
        {"MergerData"});

    // Plot
    auto* c0 {new TCanvas {"c0", "Q vs E sil canvas"}};
    hsq[8]->Draw("colz");
    hsq[8]->SetNameTitle("c0","""PID;E_{Si}(MeV);dE/dx (arb. units)");
    auto* c1 {new TCanvas {"c1", "E_Si vs beam stopping point"}};
    hbsp->Draw("colz");
    hbsp->SetNameTitle("c1","""beam stopping point;beam stopping point (cm);E_{Si} (MeV)");
}

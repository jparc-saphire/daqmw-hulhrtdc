
#include <iostream>
#include <algorithm>

#include <TLine.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TVirtualPad.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>
#include <THttpServer.h>
#include <TMemFile.h>
#include <TDirectory.h>
#include <TSystem.h>

#include "cpp11_make_unique.hh"
#include "MznHRTdcData.hh"

#include "MznHRTdcDraw.hh"

//______________________________________________________________________________
const double kTimeUnit = 1.0/520e6/20148 * 1e9; // nsec
const unsigned int kDefaultNbins = 150000;
const double kDefaultXmin =0;
const double kDefaultXmax = 100;

//______________________________________________________________________________
std::vector<TCanvas*> cLE(4);
std::vector<TCanvas*> cTE(4);
std::vector<TCanvas*> cWidth(4);
//std::vector<TCanvas*> cLE(2);
//std::vector<TCanvas*> cTE(2);
//std::vector<TCanvas*> cWidth(2);

TH1* hStatus=nullptr;
TH1* hNhit=nullptr;
TH1* htNhit=nullptr;
TH1* hNhitCh=nullptr;
TH1* htNhitCh=nullptr;
TH1* htrigtag=nullptr;
std::vector<TH1*> hLE(MZN_HRTDC::NofCh);
std::vector<TH1*> hTE(MZN_HRTDC::NofCh);
std::vector<TH1*> hWidth(MZN_HRTDC::NofCh);
//std::vector<TH1*> hLE(MZN_HRTDC::NofCh/2);   
//std::vector<TH1*> hTE(MZN_HRTDC::NofCh/2);   
//std::vector<TH1*> hWidth(MZN_HRTDC::NofCh/2);
//______________________________________________________________________________
MznHRTdcDraw::MznHRTdcDraw(const char* name)
  : fName(name)
{
}

//______________________________________________________________________________
void MznHRTdcDraw::Fill(const MZN_HRTDC::Data_t& cont,bool isupdate)
{
  if (hStatus) {
    if (cont.StopOut[0]>0) hStatus->Fill(0., cont.StopOut[0]);
    if (cont.StopOut[1]>0) hStatus->Fill(1., cont.StopOut[1]);
    if (cont.Through[0]>0) hStatus->Fill(2., cont.Through[0]);
    if (cont.Through[1]>0) hStatus->Fill(3., cont.Through[1]);
    if (cont.OverFlow[0]>0)  hStatus->Fill(4., cont.OverFlow[0]);
    if (cont.OverFlow[1]>0)  hStatus->Fill(5., cont.OverFlow[1]);
  }

  // Nhit, tNhit
  if (hNhit && (cont.Nhit>0))   hNhit->Fill(cont.Nhit);
  if (htNhit && (cont.tNhit>0)) htNhit->Fill(cont.tNhit);

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  for (auto ich=0; ich<MZN_HRTDC::NofCh; ++ich) {
    //  for (auto ich=0; ich<MZN_HRTDC::NofCh/2; ++ich) {
    auto nhitLE = 0;
    if (hNhitCh) {
      nhitLE = cont.NhitCh[ich];
      if (nhitLE>0) hNhitCh->Fill(ich, nhitLE);
      
    }
    
    auto nhitTE = 0;
    if (htNhitCh) {
      nhitTE = cont.tNhitCh[ich];
      if (nhitTE>0) htNhitCh->Fill(ich, nhitTE);
    }
  }
  //+++++++++++++++++++++++++++++++++++++++
  // tdc 
  for (auto ih=0; ih<cont.Nhit; ++ih){
    auto hitch = cont.Ch[ih];
    if (hLE[hitch]) hLE[hitch]->Fill(cont.tdc_leading[ih] * kTimeUnit);
  }     
  if(isupdate){
    for(auto ich=0;ich<MZN_HRTDC::NofCh;++ich){ 
    //  for(auto ich=0;ich<MZN_HRTDC::NofCh/2;++ich){ 
      cLE[ich/16]->cd(ich%16+1); hLE[ich]->Draw("hist");
    }
  }
 
  // bool foundBeamtag=false;
  // bool foundScattag=false;
  for (auto ih=0;ih<cont.tNhit;++ih){  
    auto hitch = cont.tCh[ih]; 
    if (hTE[hitch]) hTE[hitch]->Fill(cont.tdc_trailing[ih] * kTimeUnit);
    
  //   if (hitch== MZN_HRTDC::trigTagBeam){
  //     if (!foundBeamtag){
  //       foundBeamtag=true;
  //     }
  //   }
  //   if (hitch== MZN_HRTDC::trigTagScat){
  //     if (!foundScattag){
  //       foundScattag=true;
  //     }
  //   }
  }
  
  // if(foundBeamtag) htrigtag->Fill(0);
  // if(foundScattag) htrigtag->Fill(1);
  // if(foundBeamtag && foundScattag) htrigtag->Fill(2);
  // if( (!foundBeamtag) && (!foundScattag)) htrigtag->Fill(3);

  if(isupdate){
    for (auto ich=0; ich<MZN_HRTDC::NofCh; ++ich){
    //for (auto ich=0; ich<MZN_HRTDC::NofCh/2; ++ich){
      cTE[ich/16]->cd(ich%16+1); hTE[ich]->Draw("hist");
    }
  }

  auto nhitW = std::min(cont.Nhit , cont.tNhit);
  for (auto ih=0; ih<nhitW; ++ih){
    auto hitch = cont.Ch[ih];
    auto thitch = cont.tCh[ih];
    if (hWidth[hitch] && (hitch==thitch)) {
        hWidth[hitch]->Fill( (cont.tdc_leading[ih] - cont.tdc_trailing[ih]) * kTimeUnit);
    }
  }
  if(isupdate){
    for(auto ich=0;ich<MZN_HRTDC::NofCh;++ich){      
      //for(auto ich=0;ich<MZN_HRTDC::NofCh/2;++ich){      
      cWidth[ich/16]->cd(ich%16+1); hWidth[ich]->Draw("hist");
    }
  }
}

//______________________________________________________________________________
void MznHRTdcDraw::Initialize(THttpServer* server, 
			 const std::string& name)
{
  for (auto ic=0; ic<4; ++ic) {
    //for (auto ic=0; ic<2; ++ic) {
    if (!cLE[ic]) {
      auto cname = Form("%scLE%d", name.c_str(), ic);
      auto c = new TCanvas(cname, "leading edge");
      c->Divide(4, 4);
      cLE[ic] = c;
    }
  }

  for (auto ic=0; ic<4; ++ic) {
    //for (auto ic=0; ic<2; ++ic) {
    if (!cTE[ic]) {
      auto cname = Form("%scTE%d", name.c_str(), ic);
      auto c = new TCanvas(cname, "trailing edge");
      c->Divide(4, 4);
      cTE[ic] = c;
    }
  }
  
  for (auto ic=0; ic<4; ++ic) {
    //for (auto ic=0; ic<2; ++ic) {
    if (!cWidth[ic]) {
      auto cname = Form("%scWidth%d", name.c_str(), ic);
      auto c = new TCanvas(cname, "width");
      c->Divide(4, 4);
      cWidth[ic] = c;
    }
  }

  // status
  if (!hStatus) {
    TH1* h = new TH1F(Form("h%sstatus", name.c_str()), 
		      Form("h%sstatus", name.c_str()), 
		      6, 0, 6);
    h->SetDirectory(0);
    h->SetXTitle("stateus");
    h->SetLineColor(kBlack);
    h->SetOption("hist");
    hStatus = h;
    server->Register(Form("/mzntdc/%s", name.c_str()), h);
  }

  if (!htrigtag){
    // TH1* h = new TH1I(Form("hTriggerTag"),
    //      Form("Trigger Tag"),
    //      4,0,4);
    // h->SetDirectory(0);
    // const char *label[4]={"beam","Scatter","both","NoTag"};
    // h->SetXTitle("trigger ID");
    // h->GetXaxis()->SetBinLabel(1,label[0]);
    // h->GetXaxis()->SetBinLabel(2,label[1]);
    // h->GetXaxis()->SetBinLabel(3,label[2]);
    // h->GetXaxis()->SetBinLabel(4,label[3]);
    // h->SetOption("hist");
    // htrigtag = h;
    // //server->Register(Form("/mzntdc/%s", name.c_str()), h);
    // server->Register(Form("/"), h);
  }
  
  // nhit leading edge
  if (!hNhit) {
    TH1* h = new TH1F(Form("h%sNhit", name.c_str()), 
		      Form("%s Nhit by leading edge", name.c_str()),  
		      MZN_HRTDC::MaxHit, 0, MZN_HRTDC::MaxHit);
    h->SetDirectory(0);
    h->SetXTitle("Nhit");
    h->SetLineColor(kBlack);
    h->SetOption("hist");
    hNhit = h;
    server->Register(Form("/mzntdc/%s", name.c_str()), h);

  }

  // nhit trailing edge
  if (!htNhit) {
    TH1* h = new TH1F(Form("h%stNhit", name.c_str()), 
		      Form("%s Nhit by trailing edge", name.c_str()),  
		      MZN_HRTDC::MaxHit, 0, MZN_HRTDC::MaxHit);
    h->SetDirectory(0);
    h->SetXTitle("Nhit");
    h->SetLineColor(kBlack);
    h->SetOption("hist");
    htNhit = h;
    server->Register(Form("/mzntdc/%s", name.c_str()), h);
  }

  // nhit ch leading edge
  if (!hNhitCh) {
    TH1* h = new TH1F(Form("h%sNhitCh", name.c_str()), 
		      Form("h%sNhitCh", name.c_str()), 
		      64, 0, 64);
    h->SetDirectory(0);
    h->SetXTitle("Ch#");
    h->SetLineColor(kBlack);
    hNhitCh = h;
    server->Register(Form("/mzntdc/%s", name.c_str()), h);
  }

  // nhit ch trailing edge
  if (!htNhitCh) {
    TH1* h = new TH1F(Form("h%stNhitCh", name.c_str()), 
		      Form("h%stNhitCh", name.c_str()), 
		      64, 0, 64);
    h->SetDirectory(0);
    h->SetXTitle("Ch#");
    h->SetLineColor(kBlack);
    htNhitCh = h;
    server->Register(Form("/mzntdc/%s", name.c_str()), h);
  }



  // leading edge
  for (int ich=0; ich<MZN_HRTDC::NofCh; ++ich) {
    //for (int ich=0; ich<MZN_HRTDC::NofCh/2; ++ich) {
    if (hLE[ich]) continue;

    TH1* h = new TH1F(Form("h%sLE%03d", name.c_str(), ich),
		      Form("h%sLE%03d", name.c_str(), ich),
		      kDefaultNbins, kDefaultXmin, kDefaultXmax);
    h->SetDirectory(0);
    h->SetXTitle("TDC leading edge [ns]");
    h->SetLineColor(kBlack);
    cLE[ich/16]->cd(ich%16+1); h->Draw("hist");
    gPad->SetLogy();
    hLE[ich] = h;
    server->Register(Form("/mzntdc/%s/leading", name.c_str()), h);
  }


  // trailing edge
  for (int ich=0; ich<MZN_HRTDC::NofCh; ++ich) {
    //for (int ich=0; ich<MZN_HRTDC::NofCh/2; ++ich) {
    if (hTE[ich]) continue;

    TH1* h = new TH1F(Form("h%sTE%03d", name.c_str(), ich),
		      Form("h%sTE%03d", name.c_str(), ich),
		      kDefaultNbins, kDefaultXmin, kDefaultXmax);
    h->SetDirectory(0);
    h->SetXTitle("TDC trailing edge [ns]");
    h->SetLineColor(kBlack);
    cTE[ich/16]->cd(ich%16+1); h->Draw("hist");
    gPad->SetLogy();
    hTE[ich] = h;
    server->Register(Form("/mzntdc/%s/trailing", name.c_str()), h);
  }

  // width
  for (int ich=0; ich<MZN_HRTDC::NofCh; ++ich) {
    //  for (int ich=0; ich<MZN_HRTDC::NofCh/2; ++ich) {
    if (hWidth[ich]) continue;

    TH1* h = new TH1F(Form("h%sWidth%03d", name.c_str(), ich),
		      Form("h%sWidth%03d", name.c_str(), ich),
		      kDefaultNbins, kDefaultXmin, kDefaultXmax);
    h->SetDirectory(0);
    h->SetXTitle("TDC width [ns]");
    h->SetLineColor(kBlack);
    cWidth[ich/16]->cd(ich%16+1); h->Draw("hist");
    gPad->SetLogy();
    hWidth[ich] = h;
    server->Register(Form("/mzntdc/%s/width", name.c_str()), h);
  }

}

//______________________________________________________________________________
void MznHRTdcDraw::Reset()
{
  hStatus->Reset();
  //htrigtag->Reset();
  hNhit->Reset();
  htNhit->Reset();
  hNhitCh->Reset();
  htNhitCh->Reset();
  for (auto&& p: hLE)    if (p) p->Reset(); 
  for (auto&& p: hTE)    if (p) p->Reset(); 
  for (auto&& p: hWidth) if (p) p->Reset(); 
}

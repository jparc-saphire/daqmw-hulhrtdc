#define  check_hrtdc_cc
#include "check_hrtdc.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TCut.h>
#include <TMath.h>
#include <TProfile.h>
#include <TString.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>

double check_hrtdc::call(vector<vector<double> > *vec, Int_t arg1, Int_t arg2){

    double val;
    try{
        val = vec -> at(arg1).at(arg2);
    }
    catch(...){
        val = -999;
    }

    return val;
}

void check_hrtdc::check(Int_t run){
  const Bool_t detail = false;
    gStyle->SetOptFit(1111);
    gStyle->SetPadTopMargin(0.12);
    gStyle->SetPadBottomMargin(0.12);
    gStyle->SetPadLeftMargin(0.12);
    gStyle->SetPadRightMargin(0.12);

    //RPC
    const Double_t t_min = 20;
    const Double_t t_max = 120;
    //TSC
    const Double_t t_min_TSC = 0;
    const Double_t t_max_tsc = 80;

    //def
    //    TCanvas *canvas_leading[4];
    //    TCanvas *canvas_trailing[4];

    TCanvas *c1 = new TCanvas("c1", "c1", 800, 800);
    c1->Divide(4,4);

    /*
    for(Int_t i = 0; i < 4; i++){
        canvas_leading[i] = new TCanvas(Form("canvas_leading%d", i), Form("leading(ch%d - ch%d)", i*16, i*16+15), 800, 800);
        canvas_leading[i] -> Divide(4,4);
        canvas_trailing[i] = new TCanvas(Form("canvas_trailing%d", i), Form("trailing(ch%d - ch%d)", i*16, i*16+15), 800, 800);
        canvas_trailing[i] -> Divide(4,4);
    }
    */

    TH1D *h_le1[64];
    TH1D *h_le2[64];
    TH1D *h_le3[64];
    TH1D *h_tr1[64];
    TH1D *h_tr2[64];
    TH1D *h_tr3[64];
    TH1D *h_wi1[64];
    TH1D *h_wi2[64];
    TH1D *h_wi3[64];

    TH2D *h_lw1[64];
    TH2D *h_lw2[64];
    TH2D *h_lw3[64];

    TH2D *h_lt1[64];
    TH2D *h_lt2[64];
    TH2D *h_lt3[64];

    TH2D *h_l1_12[16];

    TH2D *h_nh[64];
    //sako 2020/5/18
    TH2D *h_rpc_xy[2];
    TH2D *h_rpc_xy_tsc[2];
    TH2D *h_rpc_xy_hsc[2];
    TH2D *h_rpc_xy_tsc_hsc[2];
    TH2D *h_tsc_xy;
    TH2D *h_tsc_xy_hsc;

    TH2D *h_lcor[4];
    TH2D *h_tcor[4];
    TH1D *h_multil[4];//multiplicity for each plane (4)
    TH1D *h_multit[4];//multiplicity for each plane (4)
	
    TH2D *h_lcor12;
    //    TH2D *h_tcor12;
    TH2D *h_lcor12_good;
    //    TH2D *h_tcor12_good;
    TH2D *h_lcor_good[4];//good data
    TH2D *h_tcor_good[4];//good data
    TH2D *h_lcor_same[4];
    TH2D *h_tcor_same[4];
    TH1D *h_dlt_same[4];
    TH1D *h_dlt[4];
    TH1D *h_dtt_same[4];
    TH1D *h_dtt[4];
    TH1D *h_lch[4];//channel dist
    TH1D *h_tch[4];//channel dist

    h_lcor12 = new TH2D(Form("h_lcor12"), "channel cor Plane 1 and 2 (leading)", 16,-0.5,15.5,16,-0.5,15.5);
    h_lcor12_good = new TH2D(Form("h_lcor12_good"), "channel cor Plane 1 and 2 (leading)", 16,-0.5,15.5,16,-0.5,15.5);

    //sako 2020/5/18
    for (Int_t irpc=0;irpc<2;irpc++) {
      h_rpc_xy[irpc] = new TH2D(Form("h_rpc_xy_%d", (irpc+1)), "RPC x y dist", 8,-0.5,7.5,100,-15.,15.);
      h_rpc_xy_tsc[irpc] = new TH2D(Form("h_rpc_xy_tsc_%d", (irpc+1)), "RPC x y dist", 8,-0.5,7.5,100,-15.,15.);
      h_rpc_xy_hsc[irpc] = new TH2D(Form("h_rpc_xy_hsc_%d", (irpc+1)), "RPC x y dist", 8,-0.5,7.5,100,-15.,15.);
      h_rpc_xy_tsc_hsc[irpc] = new TH2D(Form("h_rpc_xy_tsc_hsc_%d", (irpc+1)), "RPC x y dist", 8,-0.5,7.5,100,-15.,15.);
    }
    h_tsc_xy = new TH2D(Form("h_tsc_xy"), "TSC x y dist", 8,-0.5,7.5,100,-15.,15.);
    h_tsc_xy_hsc = new TH2D(Form("h_tsc_xy_hsc"), "TSC x y dist", 8,-0.5,7.5,100,-15.,15.);
    //ESC time difference among PMTs
    const Int_t npmt = 16;
    TH1D *h_esc_tdif1[npmt][npmt]={0};
    TH1D *h_esc_tdif[npmt][npmt]={0};
    TH1D *h_esc_tdifg[npmt][npmt]={0};
    TH1D *h_esc_tsc_tdif = new TH1D(Form("h_esc_tsc_tdif"), "t diff (ESC-TSC)", 2400,-120,120);
    for (Int_t ipmt=0;ipmt<npmt;ipmt++) {
      for (Int_t jpmt=ipmt+1;jpmt<npmt;jpmt++) {
	h_esc_tdif1[ipmt][jpmt] = new TH1D(Form("h_esc_tdif1_%d_%d",ipmt,jpmt), "t diff first hit (diff ch)", 1200,-12,12);
	h_esc_tdif[ipmt][jpmt] = new TH1D(Form("h_esc_tdif_%d_%d",ipmt,jpmt), "t diff (diff ch)", 1200,-12,12);
	h_esc_tdifg[ipmt][jpmt] = new TH1D(Form("h_esc_tdifg_%d_%d",ipmt,jpmt), "t diff good (diff ch)", 1200,-12,12);
      }
    }


    for(Int_t ch = 0; ch < 64; ch++){
      if (ch%16==0) {
	h_multil[ch/16] = new TH1D(Form("h_multil_%d", ch/16), "no. of leading hits per plane (16ch)", 17,-0.5,16.5);
	h_multit[ch/16] = new TH1D(Form("h_multit_%d", ch/16), "no. of trailing hits per plane (16ch)", 17,-0.5,16.5);
	h_lcor[ch/16] = new TH2D(Form("h_lcor_%d", ch/16), "channel cor (leading)", 16,-0.5,15.5,16,-0.5,15.5);
	h_tcor[ch/16] = new TH2D(Form("h_tcor_%d", ch/16), "channel cor (trailing)", 16,-0.5,15.5,16,-0.5,15.5);
	h_lcor_good[ch/16] = new TH2D(Form("h_lcor_good_%d", ch/16), "channel cor (leading)", 16,-0.5,15.5,16,-0.5,15.5);
	h_tcor_good[ch/16] = new TH2D(Form("h_tcor_good_%d", ch/16), "channel cor (trailing)", 16,-0.5,15.5,16,-0.5,15.5);
	h_lcor_same[ch/16] = new TH2D(Form("h_lcor_same_%d", ch/16), "channel cor (leading, same ch)", 16,-0.5,15.5,16,-0.5,15.5);
	h_tcor_same[ch/16] = new TH2D(Form("h_tcor_same_%d", ch/16), "channel cor (trailing, same ch)", 16,-0.5,15.5,16,-0.5,15.5);
	h_dlt_same[ch/16] = new TH1D(Form("h_dlt_same_%d", ch/16), "t diff (leading, same ch)", 201,-100.5,100.5);
	h_dlt[ch/16] = new TH1D(Form("h_dlt_%d", ch/16), "t diff (leading, different ch)", 201,-100.5,100.5);
	h_dtt_same[ch/16] = new TH1D(Form("h_dtt_same_%d", ch/16), "t diff (trailing, same ch)", 201,-100.5,100.5);
	h_dtt[ch/16] = new TH1D(Form("h_dtt_%d", ch/16), "t diff (trailing different ch)", 201,-100.5,100.5);
	h_lch[ch/16] = new TH1D(Form("h_lch_%d", ch/16), "ch dist (leading)", 16,-0.5,15.5);
	h_tch[ch/16] = new TH1D(Form("h_tch_%d", ch/16), "ch dist (trailing)", 16,-0.5,15.5);
      }
      /*
        h_le1[ch] = new TH1D(Form("h_le1_%d", ch), "leading;(ns)", 1000, -500, 500);
        h_le2[ch] = new TH1D(Form("h_le2_%d", ch), "leading;(ns)", 1000, -500, 500);
        h_le3[ch] = new TH1D(Form("h_le3_%d", ch), "leading;(ns)", 1000, -500, 500);
        h_tr1[ch] = new TH1D(Form("h_tr1_%d", ch), "trailing;(ns)", 1000, -500, 500);
        h_tr2[ch] = new TH1D(Form("h_tr2_%d", ch), "trailing;(ns)", 1000, -500, 500);
        h_tr3[ch] = new TH1D(Form("h_tr3_%d", ch), "trailing;(ns)", 1000, -500, 500);
*/
	//def
      /*
        h_le1[ch] = new TH1D(Form("h_le1_%d", ch), "leading;(ns)", 200, -0.5, 199.5);
        h_le2[ch] = new TH1D(Form("h_le2_%d", ch), "leading;(ns)", 200, -0.5, 199.5);
        h_le3[ch] = new TH1D(Form("h_le3_%d", ch), "leading;(ns)", 200, -0.5, 199.5);
        h_tr1[ch] = new TH1D(Form("h_tr1_%d", ch), "trailing;(ns)", 200, -0.5, 199.5);
        h_tr2[ch] = new TH1D(Form("h_tr2_%d", ch), "trailing;(ns)", 200, -0.5, 199.5);
        h_tr3[ch] = new TH1D(Form("h_tr3_%d", ch), "trailing;(ns)", 200, -0.5, 199.5);
      */

        h_le1[ch] = new TH1D(Form("h_le1_%d", ch), "leading;(ns)",  500, -0.5, 499.5);
        h_le2[ch] = new TH1D(Form("h_le2_%d", ch), "leading;(ns)",  500, -0.5, 499.5);
        h_le3[ch] = new TH1D(Form("h_le3_%d", ch), "leading;(ns)",  500, -0.5, 499.5);
        h_tr1[ch] = new TH1D(Form("h_tr1_%d", ch), "trailing;(ns)", 500, -0.5, 499.5);
        h_tr2[ch] = new TH1D(Form("h_tr2_%d", ch), "trailing;(ns)", 500, -0.5, 499.5);
        h_tr3[ch] = new TH1D(Form("h_tr3_%d", ch), "trailing;(ns)", 500, -0.5, 499.5);

	//        h_wi1[ch] = new TH1D(Form("h_wi1_%d", ch), "width;(ns)", 1000, 0, 500);
	//        h_wi2[ch] = new TH1D(Form("h_wi2_%d", ch), "width;(ns)", 1000, 0, 500);
	//        h_wi3[ch] = new TH1D(Form("h_wi3_%d", ch), "width;(ns)", 1000, 0, 500);

        h_wi1[ch] = new TH1D(Form("h_wi1_%d", ch), "width;(ns)", 3200, -400., 400.);
        h_wi2[ch] = new TH1D(Form("h_wi2_%d", ch), "width;(ns)", 3200, -400., 400.);
        h_wi3[ch] = new TH1D(Form("h_wi3_%d", ch), "width;(ns)", 3200, -400., 400.);

        h_lw1[ch] = new TH2D(Form("h_lw1_%d", ch), "leading vs width;(ns)", 800, -100., 100.,600,0.,150.);
        h_lw2[ch] = new TH2D(Form("h_lw2_%d", ch), "leading vs width;(ns)", 800, -100., 100.,600,0.,150.);
        h_lw3[ch] = new TH2D(Form("h_lw3_%d", ch), "leading vs width;(ns)", 800, -100., 100.,600,0.,150.);


        h_lt1[ch] = new TH2D(Form("h_lt1_%d", ch), "leading vs trailing(ns)", 600, 0., 150.,600,0.,150.);
        h_lt2[ch] = new TH2D(Form("h_lt2_%d", ch), "leading vs trailing(ns)", 600, 0., 150.,600,0.,150.);
        h_lt3[ch] = new TH2D(Form("h_lt3_%d", ch), "leading vs trailing(ns)", 600, 0., 150.,600,0.,150.);

	if ((ch>=0)&&(ch<16)) {
	  h_l1_12[ch] = new TH2D(Form("h_l1_12_%d", ch), "leading Plane 1 2 (same ch)(ns)", 600, 0., 150.,600,0.,150.);
	}

        h_nh[ch] = new TH2D(Form("h_nh_%d", ch), "leading vs trailing no. of hits", 20, -0.5,19.5,20,-0.5,19.5);

        h_le1[ch] -> SetLineColor(kRed);
        h_le2[ch] -> SetLineColor(kBlue);
        h_le3[ch] -> SetLineColor(kBlack);
        h_tr1[ch] -> SetLineColor(kRed);
        h_tr2[ch] -> SetLineColor(kBlue);
        h_tr3[ch] -> SetLineColor(kBlack);
        h_wi1[ch] -> SetLineColor(kRed);
        h_wi2[ch] -> SetLineColor(kBlue);
        h_wi3[ch] -> SetLineColor(kBlack);

        h_lw1[ch] -> SetMarkerColor(kRed);
        h_lw2[ch] -> SetMarkerColor(kBlue);
        h_lw3[ch] -> SetMarkerColor(kBlack);

        h_lt1[ch] -> SetMarkerColor(kRed);
        h_lt2[ch] -> SetMarkerColor(kBlue);
        h_lt3[ch] -> SetMarkerColor(kBlack);
    }

    h_lcor[0] -> SetMarkerColor(kBlack);
    h_lcor[1] -> SetMarkerColor(kBlack);
    h_lcor[2] -> SetMarkerColor(kBlack);
    h_lcor[3] -> SetMarkerColor(kBlack);
    h_tcor[0] -> SetMarkerColor(kBlack);
    h_tcor[1] -> SetMarkerColor(kBlack);
    h_tcor[2] -> SetMarkerColor(kBlack);
    h_tcor[3] -> SetMarkerColor(kBlack);
    h_dlt_same[0] -> SetLineColor(kBlack);
    h_dlt_same[1] -> SetLineColor(kBlack);
    h_dlt_same[2] -> SetLineColor(kBlack);
    h_dlt_same[3] -> SetLineColor(kBlack);
    h_dlt[0] -> SetLineColor(kBlack);
    h_dlt[1] -> SetLineColor(kBlack);
    h_dlt[2] -> SetLineColor(kBlack);
    h_dlt[3] -> SetLineColor(kBlack);

    TFile *fout = new TFile(Form("check_hrtdc_run%d.root", run),"RECREATE");
    Int_t hev=0;
  const Int_t max_nh = 20;
  const Int_t nch = 16;
  TTree *htree = new TTree("htree","all hits");
  Int_t hnh_rpc1[nch];
  Int_t hnh_rpc2[nch];
  Int_t hnh_tsc[nch];
  Int_t hnh_esc[nch];
  Double_t hlt_rpc1[nch][max_nh];
  Double_t htt_rpc1[nch][max_nh];
  Double_t hlt_rpc2[nch][max_nh];
  Double_t htt_rpc2[nch][max_nh];
  Double_t hlt_tsc[nch][max_nh];
  Double_t htt_tsc[nch][max_nh];
  Double_t hlt_esc[nch][max_nh];
  Double_t htt_esc[nch][max_nh];

  htree->Branch("ievt",&hev, "ev/I");
  htree->Branch("nh_rpc1",hnh_rpc1,"nh_rpc1[16]/I");
  htree->Branch("nh_rpc2",hnh_rpc2,"nh_rpc2[16]/I");
  htree->Branch("nh_tsc",hnh_tsc,"nh_tsc[16]/I");
  htree->Branch("nh_esc",hnh_esc,"nh_esc[16]/I");

  htree->Branch("lt_rpc1",hlt_rpc1,"lt_rpc1[16][20]/D");
  htree->Branch("lt_rpc2",hlt_rpc2,"lt_rpc2[16][20]/D");
  htree->Branch("lt_tsc",hlt_tsc,"lt_tsc[16][20]/D");
  htree->Branch("lt_esc",hlt_tsc,"lt_esc[16][20]/D");

  htree->Branch("tt_rpc1",htt_rpc1,"tt_rpc1[16][20]/D");
  htree->Branch("tt_rpc2",htt_rpc2,"tt_rpc2[16][20]/D");
  htree->Branch("tt_tsc",htt_tsc,"tt_tsc[16][20]/D");
  htree->Branch("tt_esc",htt_tsc,"tt_esc[16][20]/D");

  Int_t nentries = tree0 -> GetEntriesFast();

    cout << "Event : " << nentries << endl;

    for(Int_t event = 0; event < nentries; event++){
      if ((event%10000) == 0) {
	cout << "event : " << event << endl;
      }
        Int_t ientry = tree0 -> LoadTree(event);
        tree0 -> GetEntry(ientry);

	Bool_t RPC1_flag = false;
	Bool_t RPC2_flag = false;
	Bool_t TSC_flag = false;
	Bool_t HSC_flag = false;


	//	cout << "going to count " << endl;
	//        count(nentries, event);
	//	cout << "end count " << endl;

	vector <Double_t> ltdcs;
	vector <Double_t> ttdcs;
	vector <Int_t> lchs;
	vector <Int_t> tchs;
	vector <Int_t> leles;
	vector <Int_t> teles;

        for(Int_t ich = 0; ich < 16; ich++){
	  hnh_rpc1[ich] = 0;
	  hnh_rpc2[ich] = 0;
	  hnh_tsc[ich] = 0;
	  hnh_esc[ich] = 0;
	  for (Int_t ih=0;ih<max_nh;ih++) {
	    hlt_rpc1[ich][ih]=-1;
	    hlt_rpc2[ich][ih]=-1;
	    hlt_tsc[ich][ih]=-1;
	    hlt_esc[ich][ih]=-1;
	    htt_rpc1[ich][ih]=-1;
	    htt_rpc2[ich][ih]=-1;
	    htt_tsc[ich][ih]=-1;
	    htt_esc[ich][ih]=-1;
	  }
	}

	Int_t nhitl_channel[64]={0};//leading multiplicity per plane
	Int_t nhitt_channel[64]={0};//trailing multiplicity per plane

        for(Int_t ch = 0; ch < 64; ch++){
	  //	  cout << "ch = " << ch << endl;
            //----------leading time----------
	  Int_t plane = ch/16;
	  Int_t channel = ch%16;
	  Int_t nhitl = ltdc->at(ch).size();
	  Int_t nhitt = ttdc->at(ch).size();
	  nhitl_channel[ch] = nhitl;
	  nhitt_channel[ch] = nhitt;
	  Int_t nhit = nhitl;
	  if ((channel<0)||(channel>=16)||(plane<0)||(plane>=4)) {
	    if (detail) cerr << " Strange channel, plane = " << channel << " " << plane << endl;
	    return;
	  }
	  if (nhitl != nhitt) {
	    if (detail) cerr << "Inconsistent nhits> plane channel nhitl nhitt " << plane << " " << channel << " " << nhitl << " " << nhitt << endl;
	    //	    return;
	    if (nhitl<=nhitt) {
	      nhit = nhitl;
	    } else {
	      nhit = nhitt;
	    }
	  } else {
	    //	    cout << "consistent nhitl nhitt" << endl;
	  }
	
	  h_nh[ch]->Fill(nhitl,nhitt);
	  
	  if (plane==0) {
	    hnh_rpc1[channel] = nhit;
	    if (nhit>0) {
	      RPC1_flag = true;
	    }
	  } else if (plane==1) {
	    hnh_rpc2[channel] = nhit;
	    if (nhit>0) {
	      RPC2_flag = true;
	    }
	  } else if (plane==2) {
	    hnh_tsc[channel] = nhit;
	    if (nhit>0) {
	      TSC_flag = true;
	    }
	  } else if (plane==3) {
	    hnh_esc[channel] = nhit;
	    if (channel == 0) {
	      if (nhit>0) {
		HSC_flag = true;
	      }
	    }
	  }
		
	  for(Int_t element = 0; element < ltdc -> at(ch).size(); element++){
	    Double_t lt = ltdc->at(ch).at(element);
	    lchs.push_back(ch);
	    //	    cout << "+ch, element, lt = "<< ch << " " << element << " " << lt << endl;
	    leles.push_back(element);
	    ltdcs.push_back(lt);
	    if(element == 0) h_le1[ch] -> Fill(lt);
	    else if(element == 1) h_le2[ch] -> Fill(lt);
	    else h_le3[ch] -> Fill(lt);

	    if ((element<nhit)&&(element<max_nh)) {
	      if (plane==0) {
		hlt_rpc1[channel][element]=lt;
		//		cout << "hlt_rpc1["<<channel<<"]["<<element<<"]="<<hlt_rpc1[channel][element]<< endl;
	      } else if (plane==1) {
		hlt_rpc2[channel][element]=lt;
	      } else if (plane==2) {
		hlt_tsc[channel][element]=lt;
	      } else if (plane==3) {
		hlt_esc[channel][element]=lt;
	      }
	    } else {
	      if (detail) cerr <<"Strange element, nhit = " << element <<" " << nhit << endl;
	      //	      return;
	    }
	  }
	
	  //----------trailing time----------
	  for(Int_t element = 0; element < ttdc -> at(ch).size(); element++){
	    Double_t tt = ttdc->at(ch).at(element);
	    tchs.push_back(ch);
	    //	    cout << "+ch, element, tt = "<< ch << " " << element << " " << tt << endl;
	    teles.push_back(element);
	    ttdcs.push_back(tt);
	    
	    if(element == 0) h_tr1[ch] -> Fill(tt);
	    else if(element == 1) h_tr2[ch] -> Fill(tt);
	    else h_tr3[ch] -> Fill(tt);
	    
	    if ((element<nhit)&&(element<max_nh)) {
	      if (plane==0) {
		htt_rpc1[channel][element]=tt;
	      } else if (plane==1) {
		htt_rpc2[channel][element]=tt;
	      } else if (plane==2) {
		htt_tsc[channel][element]=tt;
	      } else if (plane==3) {
		htt_esc[channel][element]=tt;
	      }
	    }
	    
	  }
	
	
	  // width (sako)
            for(Int_t element = 0; element < ltdc -> at(ch).size(); element++){
	      Double_t lt = ltdc->at(ch).at(element);
	      Double_t tt = -1;
	      if (element < ttdc->at(ch).size()) {
		tt = ttdc->at(ch).at(element);
	      }
	      //	      cout << "width: element, ch = " << element << " " << ch << endl;
	      if(element == 0)      {
		if (element < ttdc->at(ch).size()) {
		  h_wi1[ch] -> Fill(lt-tt);
		  h_lw1[ch] -> Fill(lt-tt,lt);
		  h_lt1[ch] -> Fill(lt,tt);
		}
	      }
	      else if(element == 1) {
		if (element < ttdc->at(ch).size()) {
		  h_wi2[ch] -> Fill(lt-tt);
		  h_lw2[ch] -> Fill(lt-tt,lt);
		  h_lt2[ch] -> Fill(lt,tt);
		}
	      }
	      else {
		//		cout << "filling h_wi3 " << endl;
		if (element < ttdc->at(ch).size()) {
		  h_wi3[ch] -> Fill(lt-tt);
		  h_lw3[ch] -> Fill(lt-tt,lt);
		  h_lt3[ch] -> Fill(lt,tt);
		}
	      }
            }
	    //new correct	
	} //ch loop
    
	//calc multiplicity
	Int_t multil[4] = {0,0,0,0};
	Int_t multit[4] = {0,0,0,0};
	for (Int_t ch=0;ch<64;ch++) {
	  Int_t plane = ch/16;
	  if (nhitl_channel[ch]>0) {
	    multil[plane]++;
	  }
	  if (nhitt_channel[ch]>0) {
	    multit[plane]++;
	  }
	}
	for (Int_t iplane=0;iplane<4;iplane++) {
	  h_multil[iplane]->Fill(multil[iplane]);
	  h_multit[iplane]->Fill(multit[iplane]);
	}
	
	htree->Fill();

	//ch correlations
	   for (Int_t ich=0;ich<lchs.size();ich++) {
	      Int_t ichan=lchs[ich];
	      Int_t iele=leles[ich];
	      Double_t ilt=ltdcs[ich];
	      h_lch[ichan/16] -> Fill(ichan-(ichan/16)*16);

	      for (Int_t jch=ich+1;jch<lchs.size();jch++) {//always ich<jch
		Int_t jchan=lchs[jch];
		Int_t jele=leles[jch];
		Double_t jlt=ltdcs[jch];

		if ((0<=ichan)&&(ichan<16)&&(0<=jchan)&&(jchan<16)) {
		  if (ichan==jchan) {
		    h_lcor_same[0]->Fill(ichan,jchan);
		  } else {
		    h_lcor[0]->Fill(ichan,jchan);
		    if ((iele==0)&&(jele==0)&&(lchs.size()==tchs.size())&&(t_min<=ilt)&&(ilt<=t_max)&&(t_min<=jlt)&&(jlt<=t_max)) {
		      h_lcor_good[0]->Fill(ichan,jchan);
		    }
		  }
		  if (ichan == jchan) {
		    h_dlt_same[0]->Fill(ilt-jlt);
		  } else {
		    h_dlt[0]->Fill(ilt-jlt);
		  }
		} else if ((16<=ichan)&&(ichan<32)&&(16<=jchan)&&(jchan<32)) {
		  if (ichan==jchan) {
		    h_lcor_same[1]->Fill(ichan-16,jchan-16);
		  } else {
		    h_lcor[1]->Fill(ichan-16,jchan-16);
		    if ((iele==0)&&(jele==0)&&(lchs.size()==tchs.size())&&(t_min<=ilt)&&(ilt<=t_max)&&(t_min<=jlt)&&(jlt<=t_max)) {
		      h_lcor_good[1]->Fill(ichan-16,jchan-16);
		    }
		  }
		  if (ichan == jchan) {
		    h_dlt_same[1]->Fill(ilt-jlt);
		  } else {
		    h_dlt[1]->Fill(ilt-jlt);
		  }
		} else if ((32<=ichan)&&(ichan<48)&&(32<=jchan)&&(jchan<48)) {
		  if (ichan==jchan) {
		    h_lcor_same[2]->Fill(ichan-32,jchan-32);
		  } else {
		    h_lcor[2]->Fill(ichan-32,jchan-32);
		    if ((iele==0)&&(jele==0)&&(lchs.size()==tchs.size())&&(t_min<=ilt)&&(ilt<=t_max)&&(t_min<=jlt)&&(jlt<=t_max)) {
		      h_lcor_good[2]->Fill(ichan-32,jchan-32);
		    }
		  }
		  if (ichan == jchan) {
		    h_dlt_same[2]->Fill(ilt-jlt);
		  } else {
		    h_dlt[2]->Fill(ilt-jlt);
		  }
		} else if ((ichan>=48)&&(jchan>=48)) {
		  if (ichan==jchan) {
		    h_lcor_same[3]->Fill(ichan-48,jchan-48);
		  } else {
		    h_lcor[3]->Fill(ichan-48,jchan-48);
		    if ((iele==0)&&(jele==0)&&(lchs.size()==tchs.size())&&(t_min<=ilt)&&(ilt<=t_max)&&(t_min<=jlt)&&(jlt<=t_max)) {
		      h_lcor_good[3]->Fill(ichan-48,jchan-48);
		    }
		  }
		  if (ichan == jchan) {
		    h_dlt_same[3]->Fill(ilt-jlt);
		  } else {
		    h_dlt[3]->Fill(ilt-jlt);
		  }
		}
	      }
	   }


	    for (Int_t ich=0;ich<tchs.size();ich++) {
	      Int_t ichan=tchs[ich];
	      Int_t iele=teles[ich];
	      Double_t ilt=ttdcs[ich];
	      h_tch[ichan/16] -> Fill(ichan-(ichan/16)*16);
	      for (Int_t jch=ich+1;jch<tchs.size();jch++) {//always ich<jch
		Int_t jchan=tchs[jch];
		Int_t jele=teles[jch];
		Double_t jlt=ttdcs[jch];
		if ((0<=ichan)&&(ichan<16)&&(0<=jchan)&&(jchan<16)) {
		  if (ichan==jchan) {
		    h_tcor_same[0]->Fill(ichan,jchan);
		  } else {
		    h_tcor[0]->Fill(ichan,jchan);
		    if ((iele==0)&&(jele==0)&&(lchs.size()==tchs.size())&&(t_min<=ilt)&&(ilt<=t_max)&&(t_min<=jlt)&&(jlt<=t_max)) {
		      h_tcor_good[0]->Fill(ichan,jchan);
		    }
		  }
		  if (ichan == jchan) {
		    h_dtt_same[0]->Fill(ilt-jlt);
		  } else {
		    h_dtt[0]->Fill(ilt-jlt);
		  }
		} else if ((16<=ichan)&&(ichan<32)&&(16<=jchan)&&(jchan<32)) {
		  if (ichan==jchan) {
		    h_tcor_same[1]->Fill(ichan-16,jchan-16);
		  } else {
		    h_tcor[1]->Fill(ichan-16,jchan-16);
		    if ((iele==0)&&(jele==0)&&(lchs.size()==tchs.size())&&(t_min<=ilt)&&(ilt<=t_max)&&(t_min<=jlt)&&(jlt<=t_max)) {
		      h_tcor_good[1]->Fill(ichan-16,jchan-16);
		    }
		  }
		  if (ichan == jchan) {
		    h_dtt_same[1]->Fill(ilt-jlt);
		  } else {
		    h_dtt[1]->Fill(ilt-jlt);
		  }
		} else if ((32<=ichan)&&(ichan<48)&&(32<=jchan)&&(jchan<48)) {
		  if (ichan==jchan) {
		    h_tcor_same[2]->Fill(ichan-32,jchan-32);
		  } else {
		    h_tcor[2]->Fill(ichan-32,jchan-32);
		    if ((iele==0)&&(jele==0)&&(lchs.size()==tchs.size())&&(t_min<=ilt)&&(ilt<=t_max)&&(t_min<=jlt)&&(jlt<=t_max)) {
		      h_tcor_good[2]->Fill(ichan-32,jchan-32);
		    }
		  }
		  if (ichan == jchan) {
		    h_dtt_same[2]->Fill(ilt-jlt);
		  } else {
		    h_dtt[2]->Fill(ilt-jlt);
		  }
		} else if ((ichan>=48)&&(jchan>=48)) {
		  if (ichan==jchan) {
		    h_tcor_same[3]->Fill(ichan-48,jchan-48);
		  } else {
		    h_tcor[3]->Fill(ichan-48,jchan-48);
		    if ((iele==0)&&(jele==0)&&(lchs.size()==tchs.size())&&(t_min<=ilt)&&(ilt<=t_max)&&(t_min<=jlt)&&(jlt<=t_max)) {
		      h_tcor_good[3]->Fill(ichan-48,jchan-48);
		    }
		  }
		  if (ichan == jchan) {
		    h_dtt_same[3]->Fill(ilt-jlt);
		  } else {
		    h_dtt[3]->Fill(ilt-jlt);
		  }
		}
	      
	      }
	    }
	    //sako 2020/5/30
	    //ESC channel correlation
	    //crash tempory comment out

	    for (Int_t ich=0;ich<lchs.size();ich++) {
	      Int_t ichan=lchs[ich];
	      Int_t istrip = ichan-48;
	      Int_t iele=leles[ich];
	      Double_t ilt=ltdcs[ich];
	      if (ich>=tchs.size()) {
		continue;
	      }
	      Double_t itt=ttdcs[ich];
	      Int_t iplane = ichan/16;
	      if (iplane!=3) continue;

	      for (Int_t jch=ich+1;jch<lchs.size();jch++) {//always ich<jch
		Int_t jchan=lchs[jch];
		Int_t jstrip = jchan-48;
		Int_t jele=leles[jch];
		Double_t jlt=ltdcs[jch];
		if (jch>=tchs.size()) {
		  continue;
		}

		Double_t jtt=ttdcs[jch];
		Int_t jplane = jchan/16;

		if (jplane!=3) continue;

		if (ichan==jchan) continue;//different channel
		
		
		//		if ((iele==0)&&(jele==0)&&(1.<(tt_top-lt_top))&&((lt_bot<tt_bot)<50)) {
		if ((iele==0)&&(jele==0)) {
		  h_esc_tdif1[istrip][jstrip]->Fill(jlt-ilt);
		}
		if ((1.<(itt-ilt))&&((itt-ilt)<50.)
		    &&(1.<(jtt-jlt))&&((jtt-jlt)<50.)) {
		  h_esc_tdif[istrip][jstrip]->Fill(jlt-ilt);
		  if ((1.<(itt-ilt))&&((itt-ilt)<6.)
		      &&(1.<(jtt-jlt))&&((jtt-jlt)<6.)) {
		    h_esc_tdifg[istrip][jstrip]->Fill(jlt-ilt);
		  }
		}
	      }
	    }

	    //sako 2020/5/30
	    //ESC-TSC correlation

	    for (Int_t ich=0;ich<lchs.size();ich++) {
	      Int_t ichan=lchs[ich];
	      Int_t istrip = ichan-48;
	      Int_t iele=leles[ich];
	      Double_t ilt=ltdcs[ich];
	      if (ich>=tchs.size()) {
		continue;
	      }
	      Double_t itt=ttdcs[ich];

	      Int_t iplane = ichan/16;
	      if ((!((iplane==2)&&(ichan%16>=8)))&&(iplane!=3)) continue;

	      for (Int_t jch=ich+1;jch<lchs.size();jch++) {//always ich<jch
		Int_t jchan=lchs[jch];
		Int_t jstrip = jchan-48;
		Int_t jele=leles[jch];
		Double_t jlt=ltdcs[jch];
		if (jch>=tchs.size()) {
		  continue;
		}
		Double_t jtt=ttdcs[jch];
		Int_t jplane = jchan/16;

		if ((!((jplane==2)&&(jchan%16>=8)))&&(jplane!=3)) continue;
		if (iplane == jplane) continue;
		if (ichan==jchan) continue;//different channel

		if ((iele==0)&&(jele==0)) {
		  h_esc_tsc_tdif->Fill(jlt-ilt);
		}
	      }
	    }


	    //sako 2020/5/18
	    //TSC up and down correlation
	    //	    Bool_t TSC_flag = false;
	    for (Int_t ich=0;ich<lchs.size();ich++) {
	      Int_t ichan=lchs[ich];
	      Int_t iele=leles[ich];
	      Double_t ilt=ltdcs[ich];
	      Int_t iplane = ichan/16;
	      if (iplane!=2) continue;
	      //	      TSC_flag = true;
	      for (Int_t jch=ich+1;jch<lchs.size();jch++) {//always ich<jch
		Int_t jchan=lchs[jch];
		Int_t jele=leles[jch];
		Double_t jlt=ltdcs[jch];
		Int_t jplane = jchan/16;
		if (jplane!=2) continue;
		if (fabs(ichan-jchan)!=1) continue;
		Double_t lt_top=0;
		Double_t lt_bot=0;
		Double_t strip = (ichan-32)/2;
		if (ichan>jchan) {
		  if (jchan%2 != 0) continue;
		  lt_top = jlt;
		  lt_bot = ilt;
		} else {
		  if (ichan%2 != 0) continue;
		  lt_top = ilt;
		  lt_bot = jlt;
		}
		//		if ((iele==0)&&(jele==0)) { //remove reflected signals
		  h_tsc_xy->Fill(strip,lt_top-lt_bot);
		  if (HSC_flag) {
		    h_tsc_xy_hsc->Fill(strip,lt_top-lt_bot);
		  }
		  //		}
	      }
	    }

	    //RPC up and down difference
	    for (Int_t ich=0;ich<lchs.size();ich++) {
	      Int_t ichan=lchs[ich];
	      Int_t iele=leles[ich];
	      Double_t ilt=ltdcs[ich];
	      Int_t iplane = ichan/16;
	      Int_t ichannel = ichan%16;
	      Int_t irpc = ichannel/8;
	      Int_t strip = ichannel%8;

	      if ((iplane!=0)&&(iplane!=1)) continue;
	      //	      TSC_flag = true;
	      for (Int_t jch=ich+1;jch<lchs.size();jch++) {//always ich<jch
		Int_t jchan=lchs[jch];
		Int_t jele=leles[jch];
		Double_t jlt=ltdcs[jch];
		Int_t jplane = jchan/16;
		Int_t jchannel = jchan%16;
		if (iplane == jplane) continue; //always different plane
		if (ichannel!=jchannel) continue;
		if ((jplane!=0)&&(jplane!=1)) continue;
		Double_t lt_top=0;
		Double_t lt_bot=0;
		if (ichan>jchan) {
		  lt_top = jlt;
		  lt_bot = ilt;
		} else {
		  lt_top = ilt;
		  lt_bot = jlt;
		}
		if ((iele==0)&&(jele==0)&&
		    (t_min<=lt_top)&&(lt_top<=t_max)&&
		    (t_min<=lt_bot)&&(lt_bot<=t_max))
		  { //remove reflected signals
		  h_rpc_xy[irpc]->Fill(strip,lt_top-lt_bot);
		  if (TSC_flag) {
		    h_rpc_xy_tsc[irpc]->Fill(strip,lt_top-lt_bot);
		    if (HSC_flag) {
		      h_rpc_xy_tsc_hsc[irpc]->Fill(strip,lt_top-lt_bot);
		    }
		  }

		  if (HSC_flag) {
		    h_rpc_xy_hsc[irpc]->Fill(strip,lt_top-lt_bot);
		  }
		}
	      }
	    }

	   //channel correlation between RPC1 and RPC2
	   for (Int_t ich=0;ich<lchs.size();ich++) {
	      Int_t ichan=lchs[ich];
	      Int_t iele=leles[ich];
	      Double_t ilt=ltdcs[ich];
	      Int_t iplane = ichan/16;
	      if ((iplane!=0)&&(iplane!=1)) continue;

	      for (Int_t jch=ich+1;jch<lchs.size();jch++) {//always ich<jch
		Int_t jchan=lchs[jch];
		Int_t jele=leles[jch];
		Double_t jlt=ltdcs[jch];
		Int_t jplane = jchan/16;
		if ((jplane!=0)&&(jplane!=1)) continue;
		if (jplane==iplane) continue;//different plane

		h_lcor12->Fill(ichan,jchan-16);
		if ((ichan==jchan-16)&&(ichan>=0)&&(ichan<16)&&(iele==0)&&(jele==0)&&(lchs.size()==tchs.size())) {
		  h_l1_12[ichan]->Fill(ilt,jlt);
		}
		if ((iele==0)&&(jele==0)&&(lchs.size()==tchs.size())&&(t_min<=ilt)&&(ilt<=t_max)&&(t_min<=jlt)&&(jlt<=t_max)) {
		  h_lcor12_good->Fill(ichan,jchan-16);
		}
	      }
	   }

	    //def wrong	} ch loopw

    }

    //def
    /*
    for(Int_t ch = 0; ch < 64; ch++){
      //def        canvas_leading[ch/16] -> cd(ch%16+1);
      c1->cd(ch%16+1);
        h_le1[ch] -> Draw();
        h_le2[ch] -> Draw("same");
        h_le3[ch] -> Draw("same");
	c1->Update();
	c1->SaveAs(Form("leading%d.gif",ch/16));
	//def        canvas_trailing[ch/16] -> cd(ch%16+1);
      c1 -> cd(ch%16+1);
        h_tr1[ch] -> Draw();
        h_tr2[ch] -> Draw("same");
        h_tr3[ch] -> Draw("same");
	c1->Update();
	c1->SaveAs(Form("trailing%d.gif",ch/16));

      c1 -> cd(ch%16+1);
        h_tr1[ch] -> Draw();
        h_tr2[ch] -> Draw("same");
        h_tr3[ch] -> Draw("same");
	c1->Update();
	c1->SaveAs(Form("trailing%d.gif",ch/16));

    }
    */

    for(Int_t ch = 0; ch < 64; ch++){
      //      cout <<"ch = " << ch << endl;
      c1->cd(ch%16+1);
      h_le1[ch] -> Draw();
      h_le2[ch] -> Draw("same");
      h_le3[ch] -> Draw("same");
      if ((ch%16+1)==16) {
	c1->Update();
	c1->SaveAs(Form("leading%d_run%d.gif",ch/16,run));
      }
    }

    for(Int_t ch = 0; ch < 64; ch++){
      c1 -> cd(ch%16+1);
      h_tr1[ch] -> Draw();
      h_tr2[ch] -> Draw("same");
      h_tr3[ch] -> Draw("same");
      if ((ch%16+1)==16) {
	c1->Update();
	c1->SaveAs(Form("trailing%d_run%d.gif",ch/16,run));
      }
    }

    for(Int_t ch = 0; ch < 64; ch++){
      c1 -> cd(ch%16+1);
      h_wi1[ch] -> Draw();
      h_wi2[ch] -> Draw("same");
      h_wi3[ch] -> Draw("same");
      if ((ch%16+1)==16) {
	c1->Update();
	c1->SaveAs(Form("width%d_run%d.gif",ch/16,run));
      }
    }

    for(Int_t ch = 0; ch < 64; ch++){
      c1 -> cd(ch%16+1);
      h_lw1[ch] -> Draw();
      h_lw2[ch] -> Draw("same");
      h_lw3[ch] -> Draw("same");
      if ((ch%16+1)==16) {
	c1->Update();
	c1->SaveAs(Form("width-leading%d_run%d.gif",ch/16,run));
      }

    }

    TCanvas *c2 = new TCanvas("c2", "c2", 800, 800);
      gStyle->SetOptStat(0);
      if (detail) {
	for(Int_t ich = 0; ich < 32; ich++){
	  h_nh[ich]->SetMaximum(1000);
	  h_nh[ich]->Draw("colz");
	  c2->Update();
	  c2->SaveAs(Form("nh%d_run%d.gif",ich,run));
	}
	for(Int_t ich = 0; ich < 16; ich++){
	  h_l1_12[ich]->Draw("colz");
	  c2->Update();
	  c2->SaveAs(Form("l1_12_%d_run%d.gif",ich,run));
	}
      }
      gStyle->SetOptStat(1);

    for(Int_t ich = 0; ich < 4; ich++){
      h_multil[ich]->Draw();
      c2->Update();
      c2->SaveAs(Form("multil_%d_run%d.gif",ich,run));
      h_multit[ich]->Draw();
      c2->Update();
      c2->SaveAs(Form("multit_%d_run%d.gif",ich,run));

      //      if (ich==0) {
      //	c2->SetLogz(1);
      //      }
      h_lcor[ich]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("lcor%d_run%d.gif",ich,run));

      //      if (ich==0) {
      //	c2->SetLogz(0);
      //      }

      //      h_tcor[ich]->Draw("colz");
      //      c2->Update();
      //      c2->SaveAs(Form("tcor%d_run%d.gif",ich,run));

      h_lcor_good[ich]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("lcor_good%d_run%d.gif",ich,run));
      //      h_tcor_good[ich]->Draw("colz");
      //      c2->Update();
      //      c2->SaveAs(Form("tcor_good%d_run%d.gif",ich,run));

      h_lcor_same[ich]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("lcor_same%d_run%d.gif",ich,run));
      //      h_tcor_same[ich]->Draw("colz");
      //      c2->Update();
      //      c2->SaveAs(Form("tcor_same%d_run%d.gif",ich,run));

      h_lcor12_good->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("lcor12_good_run%d.gif",run));

      gStyle->SetOptStat(1);
      //      c2->SetLogz(0);

      h_dlt_same[ich]->Draw();
      c2->Update();
      c2->SaveAs(Form("dlt_same%d_run%d.gif",ich,run));
      h_dlt[ich]->Draw();
      c2->Update();
      c2->SaveAs(Form("dlt%d_run%d.gif",ich,run));

      h_lch[ich]->Draw();
      c2->Update();
      c2->SaveAs(Form("lch%d_run%d.gif",ich,run));
      h_tch[ich]->Draw();
      c2->Update();
      c2->SaveAs(Form("tch%d_run%d.gif",ich,run));
      //    }

      gStyle->SetOptStat(0);
      h_rpc_xy[0]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("rpc%d_xy_run%d.gif",1,run));

      h_rpc_xy[1]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("rpc%d_xy_run%d.gif",2,run));

      h_rpc_xy_tsc[0]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("rpc%d_xy_tsc_run%d.gif",1,run));

      h_rpc_xy_tsc[1]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("rpc%d_xy_tsc_run%d.gif",2,run));

      h_rpc_xy_hsc[0]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("rpc%d_xy_hsc_run%d.gif",1,run));

      h_rpc_xy_hsc[1]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("rpc%d_xy_hsc_run%d.gif",2,run));

      h_rpc_xy_tsc_hsc[0]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("rpc%d_xy_tsc_hsc_run%d.gif",1,run));

      h_rpc_xy_tsc_hsc[1]->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("rpc%d_xy_tsc_hsc_run%d.gif",2,run));

      h_tsc_xy->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("tsc_xy_run%d.gif",run));

      h_tsc_xy_hsc->Draw("colz");
      c2->Update();
      c2->SaveAs(Form("tsc_xy_hsc_run%d.gif",run));
    }
      //      h_tcor12_good->Draw("colz");
      //      c2->Update();
      //      c2->SaveAs(Form("tcor12_good_run%d.gif",ich));


    //    TFile *fout = new TFile(Form("check_hrtdc_run%d.root", run),"RECREATE");
    //sako 2020/5/18
    for (Int_t irpc=0;irpc<2;irpc++) {
      h_rpc_xy[irpc]->Write();
      h_rpc_xy_tsc[irpc]->Write();
      h_rpc_xy_hsc[irpc]->Write();
      h_rpc_xy_tsc_hsc[irpc]->Write();
    }
    h_tsc_xy->Write();
    h_tsc_xy_hsc->Write();

    for (Int_t ch=0;ch<64;ch++) {
      h_le1[ch]->Write();
      h_le2[ch]->Write();
      h_le3[ch]->Write();
      h_tr1[ch]->Write();
      h_tr2[ch]->Write();
      h_tr3[ch]->Write();
      h_wi1[ch]->Write();
      h_wi2[ch]->Write();
      h_wi3[ch]->Write();
      h_lw1[ch]->Write();
      h_lw2[ch]->Write();
      h_lw3[ch]->Write();
      h_lt1[ch]->Write();
      h_lt2[ch]->Write();
      h_lt3[ch]->Write();
      h_nh[ch]->Write();

      if (ch<16) h_l1_12[ch]->Write();

      h_lcor12->Write();
      h_lcor12_good->Write();
      
      if (ch%16==0) {
	h_multil[ch/16]->Write();
	h_multit[ch/16]->Write();
	h_lcor[ch/16]->Write();
	h_tcor[ch/16]->Write();
	h_lcor_good[ch/16]->Write();
	h_tcor_good[ch/16]->Write();
	h_lcor_same[ch/16]->Write();
	h_tcor_same[ch/16]->Write();
	h_dlt_same[ch/16]->Write();
	h_dtt_same[ch/16]->Write();
	h_dlt[ch/16]->Write();
	h_dtt[ch/16]->Write();
	h_lch[ch/16]->Write();
	h_tch[ch/16]->Write();
      }
    }
    for (Int_t ipmt=0;ipmt<16;ipmt++) {
      for (Int_t jpmt=ipmt+1;jpmt<16;jpmt++) {
	h_esc_tdif1[ipmt][jpmt]->Write();
	h_esc_tdif[ipmt][jpmt]->Write();
	h_esc_tdifg[ipmt][jpmt]->Write();
      }
    }
    h_esc_tsc_tdif->Write();

    htree->Write();
    fout->Close();
    
}

check_hrtdc::check_hrtdc(Int_t run, TString basedir) {
  base = basedir;

  cout << "base = " << base << endl;
  fileopen0(run);

  check(run);

    //    return 0;
}

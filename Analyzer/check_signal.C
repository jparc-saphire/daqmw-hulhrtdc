#define  check_signal_cc
#include "check_signal.h"
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

double check_signal::call(vector<vector<double>> *vec, Int_t arg1, Int_t arg2){

    double val;
    try{
        val = vec -> at(arg1).at(arg2);
    }
    catch(...){
        val = -999;
    }

    return val;
}

void check_signal::check(Int_t run){
    gStyle->SetOptFit(1111);

    TCanvas *canvas_leading[4];
    TCanvas *canvas_trailing[4];

    for(Int_t i = 0; i < 4; i++){
        canvas_leading[i] = new TCanvas(Form("canvas_leading%d", i), Form("leading(ch%d - ch%d)", i*16, i*16+15), 2000, 2000);
        canvas_leading[i] -> Divide(4,4);
        canvas_trailing[i] = new TCanvas(Form("canvas_trailing%d", i), Form("trailing(ch%d - ch%d)", i*16, i*16+15), 2000, 2000);
        canvas_trailing[i] -> Divide(4,4);
    }

    TH1D *h_le[64];
    TH1D *h_tr[64];
    TH1D *h_wi[64];

    for(Int_t ch = 0; ch < 64; ch++){
      //        h_le[ch] = new TH1D(Form("h_le%d", ch), "leading;(ns)", 6000, 0, 150);
              h_le[ch] = new TH1D(Form("h_le%d", ch), "leading;(ns)", 12000
, 0, 300);
	      //        h_tr[ch] = new TH1D(Form("h_tr%d", ch), "trailing;(ns)", 6000, 0, 150);
	              h_tr[ch] = new TH1D(Form("h_tr%d", ch), "trailing;(ns)", 12000, 0, 300);
        h_wi[ch] = new TH1D(Form("h_wi%d", ch), "width;(ns)", 6000, 0, 2300);
    }

    Int_t nentries = tree0 -> GetEntriesFast();

    cout << "Event : " << nentries << endl;

    for(Int_t event = 0; event < nentries; event++){
        Int_t ientry = tree0 -> LoadTree(event);
        tree0 -> GetEntry(ientry);

        count(nentries, event);

        for(Int_t ch = 0; ch < 64; ch++){
            //----------leading time----------
            for(Int_t element = 0; element < ltdc -> at(ch).size(); element++){
                h_le[ch] -> Fill(ltdc -> at(ch).at(element));
            }
            //----------trailing time----------
            for(Int_t element = 0; element < ttdc -> at(ch).size(); element++){
                h_tr[ch] -> Fill(ttdc -> at(ch).at(element));
            }
        }   
    }

    for(Int_t ch = 0; ch < 64; ch++){
        canvas_leading[ch/16] -> cd(ch%16+1);
        h_le[ch] -> Draw();
        canvas_trailing[ch/16] -> cd(ch%16+1);
        h_tr[ch] -> Draw();
    }
}

check_signal::check_signal(Int_t run){
    fileopen0(run);

    check(run);

    return 0;
}

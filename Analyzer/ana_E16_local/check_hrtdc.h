#ifndef check_hrtdc_h
#define check_hrtdc_h

#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <vector>

#include <iostream>

using namespace std;

class check_hrtdc{
    public :

    void     fileopen0(Int_t run);
    void     Init0(TTree *tree);

    double   call(vector<vector<double> > *vec, Int_t arg1, Int_t arg2);

    void     check(Int_t run);

    void     count(Int_t Nevent, Int_t eventid);
    Int_t    percent_tmp;

    //-----object-----
    TFile   *file0;
    TFile   *file1;
    TTree   *tree0;
    TTree   *tree1;

    // List of branches
    TBranch        *b_nh_ltdc;   //!
    TBranch        *b_nh_ttdc;   //!
    TBranch        *b_ltdc;   //!
    TBranch        *b_ttdc;   //!
    TBranch        *b_width;   //!

    //-----raw data-----
    Int_t   nh_ltdc[64];
    Int_t   nh_ttdc[64];

    vector<vector<double> > *ltdc = 0;
    vector<vector<double> > *ttdc = 0;
    vector<vector<double> > *width = 0;

    //-----flag-----
    // Bool_t f_size[64][]

    //    check_hrtdc(Int_t run, TString basedir = "/disk3/sako/E16-MRPC/data/TSC-20200319");
    //    check_hrtdc(Int_t run, TString basedir = "/disk3/sako/E16-MRPC/data/dat_20200402");
        check_hrtdc(Int_t run, TString basedir = "/disk1/mrpcdaq/data/dat_E16local_20200526");

 private:
    TString base;
};
#endif

#ifdef check_hrtdc_cc

void check_hrtdc::fileopen0(Int_t run)
{
  //    file0 = new TFile(Form("./data/dat_20200317/run%06d_0.root", run), "read");
  //  file0 = new TFile(Form("/disk3/sako/E16-MRPC/data/dat_20200402/run%06d_0.root", run), "read");
  //  file0 = new TFile(Form("/disk3/sako/E16-MRPC/data/TSC-20200319/run%06d_0.root", run), "read");

  file0 = new TFile(Form("%s/run%06d_0.root", base.Data(), run), "read");

    if(!file0){
      cout << Form("Can't open run%06d_0.root", run) << endl;;
        return;
    }
    tree0 = (TTree*)file0 -> Get("tree");
    Init0(tree0);
}

void check_hrtdc::Init0(TTree *tree){
    // tree -> SetBranchAddress("nh_ltdc", nh_ltdc, &b_nh_ltdc);
    // tree -> SetBranchAddress("nh_ttdc", nh_ttdc, &b_nh_ttdc);
    tree -> SetBranchAddress("ltdc", &ltdc, &b_ltdc);
    tree -> SetBranchAddress("ttdc", &ttdc, &b_ttdc);
    tree -> SetBranchAddress("width", &width, &b_width);
}

void check_hrtdc::count(Int_t Nevent, Int_t eventid){

  cout << "eventid, Nevent = " << eventid << " " << Nevent << endl;
    Int_t percent = 100*eventid/Nevent;
    if(eventid==Nevent-1) cout << "\r" << "[##########] "   << 100 << "%" << string(10, ' ') << endl;
    if(percent == percent_tmp) return;

    if(percent<10)        cout << "\r" << "[__________]   " << percent << "%" << string(5, ' ') << flush;
    else if(percent<20)   cout << "\r" << "[#_________]  "  << percent << "%" << string(5, ' ') << flush;
    else if(percent<30)   cout << "\r" << "[##________]  "  << percent << "%" << string(5, ' ') << flush;
    else if(percent<40)   cout << "\r" << "[###_______]  "  << percent << "%" << string(5, ' ') << flush;
    else if(percent<50)   cout << "\r" << "[####______]  "  << percent << "%" << string(5, ' ') << flush;
    else if(percent<60)   cout << "\r" << "[#####_____]  "  << percent << "%" << string(5, ' ') << flush;
    else if(percent<70)   cout << "\r" << "[######____]  "  << percent << "%" << string(5, ' ') << flush;
    else if(percent<80)   cout << "\r" << "[#######___]  "  << percent << "%" << string(5, ' ') << flush;
    else if(percent<90)   cout << "\r" << "[########__]  "  << percent << "%" << string(5, ' ') << flush;
    else if(percent<100)  cout << "\r" << "[#########_]  "  << percent << "%" << string(5, ' ') << flush;

    percent_tmp = percent;
    // if(eventid == Nevent-1) cout << endl;
}

#endif

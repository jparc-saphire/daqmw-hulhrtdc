#include<iostream>
#include<bitset>

#include"TH1.h"
#include"TFile.h"
#include"TTree.h"
#include"TROOT.h"

static const int MaxHit = 1024;
static const int NofCh  = 32;

int Nhit;
int NhitCh[NofCh];
int Ch[MaxHit];
int FineCount[MaxHit];
int SemiCC[MaxHit];
int CoarseCount[MaxHit];
int c_tdc[MaxHit];
int s_tdc[MaxHit];
int f_tdc[MaxHit];

enum index_argv{
  i_bin, i_RunNo, size_index
};

int main(int argc, char* argv[])
{
  // File open
  std::string run_number   = argv[i_RunNo];
  std::string infile_path  = "rootfile/run" + run_number + ".root";
  std::string outfile_path = "lut/run" + run_number + "_lut.root";

  new TFile(infile_path.c_str());
  std::cout << "#D: start make lut table" << std::endl;

  TTree *tree = (TTree*)gFile->Get("tree");
  tree->SetBranchAddress("Nhit",     &Nhit);
  tree->SetBranchAddress("NhitCh",    NhitCh);
  tree->SetBranchAddress("Ch",        Ch);
  tree->SetBranchAddress("FineCount", FineCount);

  
  TFile *fout = new TFile(outfile_path.c_str(), "recreate");
  TH1F *hlut[10][NofCh];
  for(int i = 0; i<10; ++i){
    for(int j = 0; j<NofCh; ++j){
      hlut[i][j] = new TH1F(Form("hlut%d_%d", i, j),
			   Form("hlut%d_%d", i, j),
			   64, 0, 64);
    }// for(j)
  }// for(i)

  int n_event = tree->GetEntries();
  for(int i = 0; i<n_event; ++i){
    tree->GetEntry(i);
    
    std::bitset<NofCh> first_flag;
    first_flag.set();
    
    for(int j = 0; j<Nhit; ++j){
      if(first_flag[Ch[j]]){
	hlut[0][Ch[j]]->Fill(FineCount[j]);
	if(i<500000) hlut[1][Ch[j]]->Fill(FineCount[j]);
	if(i<400000) hlut[2][Ch[j]]->Fill(FineCount[j]);
	if(i<300000) hlut[3][Ch[j]]->Fill(FineCount[j]);
	if(i<200000) hlut[4][Ch[j]]->Fill(FineCount[j]);
	if(i<100000) hlut[5][Ch[j]]->Fill(FineCount[j]);
	if(i<75000)  hlut[6][Ch[j]]->Fill(FineCount[j]);
	if(i<50000)  hlut[7][Ch[j]]->Fill(FineCount[j]);
	if(i<25000)  hlut[8][Ch[j]]->Fill(FineCount[j]);
	if(i<10000)  hlut[9][Ch[j]]->Fill(FineCount[j]);
      }

      first_flag.reset(Ch[j]);
    }// for(i)

  }// for(i)
  
  fout->Write();
  fout->Close();

}//main

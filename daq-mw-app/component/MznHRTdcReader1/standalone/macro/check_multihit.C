const int NofBin = 64;
const int NofCh  = 32;
const int MaxHit = 1024;

const double time_semi = 1.92077;

void check_multihit(char* runno)
{
  gROOT->SetStyle("MyStyle");

  // data file
  std::string run_number = runno;
  std::string data_path = "rootfile/run" + run_number + ".root";
  new TFile(data_path.c_str());

  unsigned int Nhit;
  unsigned int NhitCh[NofCh];
  unsigned int Ch[MaxHit];
  unsigned int LutCount[MaxHit];
  unsigned int s_tdc[MaxHit];

  unsigned int tNhit;
  unsigned int tNhitCh[NofCh];
  unsigned int tCh[MaxHit];
  unsigned int tLutCount[MaxHit];
  unsigned int ts_tdc[MaxHit];
  
  TTree *tree = (TTree*)gFile->Get("tree");
  tree->SetBranchAddress("Nhit",     &Nhit);
  tree->SetBranchAddress("NhitCh",    NhitCh);
  tree->SetBranchAddress("Ch",        Ch);
  tree->SetBranchAddress("LutCount", LutCount);
  tree->SetBranchAddress("s_tdc",     s_tdc);

  tree->SetBranchAddress("tNhit",     &tNhit);
  tree->SetBranchAddress("tNhitCh",    tNhitCh);
  tree->SetBranchAddress("tCh",        tCh);
  tree->SetBranchAddress("tLutCount", tLutCount);
  tree->SetBranchAddress("ts_tdc",     ts_tdc);

  TH1F *ctof[NofCh];
  TH1F *htof[NofCh];
  for(int i = 0; i<NofCh; ++i){
    ctof[i] = new TH1F(Form("ctof%d", i),
		       Form("ctof%d", i),
		       //		       16000, -20, 20
		       20000, 50, 150
		       );

    htof[i] = new TH1F(Form("htof%d", i),
		       Form("htof%d", i),
		       20000, 50, 150
		       );

    SetTitle(htof[i], "Time difference [ns]", "Counts");
  }// for(i)

  std::cout << "Start analyze" << std::endl;

  int NofEntry = 0x7ffff;
  NofEntry = NofEntry >> 8;
  int n_event = tree->GetEntries();
  for(int i = 0; i<n_event; ++i){
    tree->GetEntry(i);
    
    //    if(Nhit != 4) continue;
    
    double time[NofCh][16];
    double ttime[NofCh][16];
    int index[NofCh] = {0};
    int tindex[NofCh] = {0};
    for(int j = 0; j<Nhit; ++j){
      double count = (double)LutCount[j];
      double tcount = (double)tLutCount[j];
      
      time[Ch[j]][index[Ch[j]]++] = s_tdc[j]*time_semi + count/NofEntry*time_semi;
      ttime[tCh[j]][tindex[tCh[j]]++] = ts_tdc[j]*time_semi + tcount/NofEntry*time_semi;
    }// for(j)
    
    for(int j = 0; j<NofCh; ++j){
      for(int k = 0; k<NhitCh[j]; ++k){
	ctof[j]->Fill(time[j][k]);
	htof[j]->Fill(ttime[j][k]);
	//	if(j != 0 ) htof[j]->Fill(time[j][k]-time[0][0]);
      }// for(k)
    }// for(j)
  }// for(i)

  TF1 *fgaus = new TF1("fgaus", "gaus");

  TCanvas *c1[2];
  TLatex *tex[32];

  for(int i = 0; i<1; ++i){
    c1[i] = new TCanvas();
    c1[i]->Divide(4,4);    

    for(int j = 0; j<16; ++j){
      c1[i]->cd(j+1);      
      ctof[16*i+j]->Draw();
      gPad->SetLogy();
    }

  }

  for(int i = 0; i<1; ++i){
    c1[i+1] = new TCanvas();
    c1[i+1]->Divide(4,4);    

    for(int j = 0; j<16; ++j){
      c1[i+1]->cd(j+1);      
      htof[16*i+j]->Draw();
      gPad->SetLogy();
    }

  }
}

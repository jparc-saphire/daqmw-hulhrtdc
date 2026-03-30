const int NofBin = 64;
const int NofCh  = 64;
const int MaxHit = 1024;

const double time_semi = 1.92077;

void check_reso(char* runno)
{
  gROOT->SetStyle("MyStyle");

  // data file
  std::string run_number = runno;
  std::string data_path = "rootfile/run" + run_number + ".root";
  new TFile(data_path.c_str());

  int Nhit;
  int NhitCh[NofCh];
  int Ch[MaxHit];
  int LutCount[MaxHit];
  int s_tdc[MaxHit];
  TTree *tree = (TTree*)gFile->Get("tree");
  tree->SetBranchAddress("Nhit",     &Nhit);
  tree->SetBranchAddress("NhitCh",    NhitCh);
  tree->SetBranchAddress("Ch",        Ch);
  tree->SetBranchAddress("LutCount", LutCount);
  tree->SetBranchAddress("CoarseCount",     s_tdc);

  std::string name[2] = {"hch0_ch1", "hch2_ch3"};
  TH1F *htof[2];
  for(int i = 0; i<2; ++i){
    htof[i] = new TH1F(Form("%s", name[i].c_str()),
		       Form("%s", name[i].c_str()),
		       //		       16000, -20, 20
		       8000, 210, 250
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
    
    double t0, t1, t2, t3;
    for(int j = 0; j<Nhit; ++j){
      double count = (double)LutCount[j];

      if(Ch[j] == 34) t0= s_tdc[j]*time_semi + count/NofEntry*time_semi;
      if(Ch[j] == 1) t1= s_tdc[j]*time_semi + count/NofEntry*time_semi;
      if(Ch[j] == 4) t2= s_tdc[j]*time_semi + count/NofEntry*time_semi;
      if(Ch[j] == 5) t3= s_tdc[j]*time_semi + count/NofEntry*time_semi;
    }// for(j)
    
    htof[0]->Fill(t0);
    htof[1]->Fill(t0-t1);
  }// for(i)

  TF1 *fgaus = new TF1("fgaus", "gaus");

  TCanvas *c1;
  c1 = new TCanvas();
  c1->Divide(2,1);

  TLatex *tex[2];
  for(int i = 0; i<2; ++i){
    c1->cd(i+1);
    htof[i]->Draw();
    htof[i]->Fit("fgaus","L");
    std::cout << "RMS : " << htof[i]->GetRMS() << std::endl;

    double sigma = fgaus->GetParameter("Sigma")*1000.;
    tex[i] = new TLatex(-9.5, htof[i]->GetMaximum()*0.8, Form("#sigma=%.2f ps", sigma));
    tex[i]->Draw();
  }

}

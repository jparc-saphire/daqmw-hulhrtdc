const int NofBin = 64;
const int NofCh  = 32;
const int MaxHit = 1024;

const double time_semi = 1.92077;

void check_reso_ch0_all(char* runno)
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
  tree->SetBranchAddress("s_tdc",     s_tdc);

  TH1F *htof[NofCh];
  for(int i = 0; i<NofCh; ++i){
    htof[i] = new TH1F(Form("hch0_ch%d", i),
		       Form("hch0_ch%d", i),
		       //		       16000, -20, 20
		       8000, -20, 20
		       );

    SetTitle(htof[i], "Time difference [ns]", "Counts");
  }// for(i)

  std::cout << "Start analyze" << std::endl;

  int NofEntry = 0x7ffff;
  NofEntry = NofEntry >> 8;
  int n_event = tree->GetEntries();
  for(int i = 0; i<n_event; ++i){
    tree->GetEntry(i);
    
    if(Nhit != 2) continue;
    
    double time[NofCh];
    for(int j = Nhit-1; j!=-1; --j){
      double count = (double)LutCount[j];
      time[Ch[j]] = s_tdc[j]*time_semi + count/NofEntry*time_semi;
    }// for(j)

    for(int j = Nhit-1; j!=-1; --j){
      htof[Ch[j]]->Fill(time[Ch[j]]-time[0]);
    }// for(j)
  }// for(i)

  TF1 *fgaus = new TF1("fgaus", "gaus");

  TCanvas *c1[2];
  TLatex *tex[32];

  for(int i = 0; i<2; ++i){
    c1[i] = new TCanvas();
    c1[i]->Divide(4,4);    

    for(int j = 0; j<16; ++j){
      c1[i]->cd(j+1);      
      htof[16*i+j]->Draw();

      if(htof[16*i +j]->Integral(1, 16000) == 0) continue;
      htof[16*i+j]->Fit("fgaus","QL");

      double sigma  = fgaus->GetParameter("Sigma")*1000.;
      double esigma = fgaus->GetParError(2)*1000.;
      tex[16*i+j] = new TLatex(-9.5, htof[16*i+j]->GetMaximum()*0.8, Form("#sigma=%.2f ps", sigma));
      tex[16*i+j]->Draw();
      std::string dch = Form("ch0-ch%d",16*i+j);
      std::cout << dch << " " << sigma << " " << esigma << std::endl;
    }

  }

}

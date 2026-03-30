const int NofBin = 64;
const int NofCh  = 32;
const int MaxHit = 1024;

const double time_semi = 1.92077;

void check_reso_2dai_Wcstop()
{
  gROOT->SetStyle("MyStyle");

  TFile *f1 = new TFile("./rootfile/run190.root");
  TTree* tree1 = (TTree*)f1->Get("tree");

  int Nhit1;
  int NhitCh1[NofCh];
  int Ch1[MaxHit];
  int LutCount1[MaxHit];
  int s_tdc1[MaxHit];
  tree1->SetBranchAddress("Nhit",     &Nhit1);
  tree1->SetBranchAddress("NhitCh",    NhitCh1);
  tree1->SetBranchAddress("Ch",        Ch1);
  tree1->SetBranchAddress("LutCount", LutCount1);
  tree1->SetBranchAddress("s_tdc",     s_tdc1);

  TFile *f2 = new TFile("./rootfile/run191.root");
  TTree* tree2 = (TTree*)f2->Get("tree");

  int Nhit2;
  int NhitCh2[NofCh];
  int Ch2[MaxHit];
  int LutCount2[MaxHit];
  int s_tdc2[MaxHit];
  tree2->SetBranchAddress("Nhit",     &Nhit2);
  tree2->SetBranchAddress("NhitCh",    NhitCh2);
  tree2->SetBranchAddress("Ch",        Ch2);
  tree2->SetBranchAddress("LutCount", LutCount2);
  tree2->SetBranchAddress("s_tdc",     s_tdc2);


  std::string name[2] = {"hch0_ch1", "hch2_ch3"};
  TH1F *htof[2];
  for(int i = 0; i<2; ++i){
    htof[i] = new TH1F(Form("%s", name[i].c_str()),
		       Form("%s", name[i].c_str()),
		       //		       16000, -20, 20
		       8000, -20, 20
		       );

    SetTitle(htof[i], "Time difference [ns]", "Counts");
  }// for(i)

  std::cout << "Start analyze" << std::endl;

  int NofEntry = 0x7ffff;
  NofEntry = NofEntry >> 8;
  int n_event = tree1->GetEntries();
  for(int i = 0; i<n_event; ++i){
    tree1->GetEntry(i);
    tree2->GetEntry(i);
    
    //    if(Nhit != 4) continue;
    
    double t0, t1;
    double count1 = (double)LutCount1[0];
    double count2 = (double)LutCount2[0];

    t0= s_tdc1[0]*time_semi + count1/NofEntry*time_semi;
    t1= s_tdc2[0]*time_semi + count2/NofEntry*time_semi;

    htof[0]->Fill(t0-t1);
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

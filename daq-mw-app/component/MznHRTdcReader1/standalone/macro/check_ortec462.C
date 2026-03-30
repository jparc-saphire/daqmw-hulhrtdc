const int NofBin = 64;
const int NofCh  = 32;
const int MaxHit = 1024;

const double time_semi = 1.92077;

#include<algorithm>

void check_ortec462(char* runno)
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

  std::string name[2] = {"hch0_ch1", "hch2_ch3"};
  TH1F *htof[2];
  for(int i = 0; i<2; ++i){
    htof[i] = new TH1F(Form("%s", name[i].c_str()),
		       Form("%s", name[i].c_str()),
		       //		       16000, -20, 20
		       //		       (10000-130)*100, 130, 10000
		       (1000-130)*100, 130, 1000
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

      if(Ch[j] == 0) t0= s_tdc[j]*time_semi + count/NofEntry*time_semi;
      if(Ch[j] == 1) t1= s_tdc[j]*time_semi + count/NofEntry*time_semi;
      if(Ch[j] == 4) t2= s_tdc[j]*time_semi + count/NofEntry*time_semi;
      if(Ch[j] == 5) t3= s_tdc[j]*time_semi + count/NofEntry*time_semi;
    }// for(j)
    
    //    htof[0]->Fill(t2); // RUN121-135
    htof[0]->Fill(t0); // RUN181-
    //    htof[1]->Fill(0);
  }// for(i)

  TF1 *fgaus = new TF1("fgaus", "gaus");
  double x[1000];
  double y[1000];

  const int n_peak = 1000;

  TCanvas *c1;
  c1 = new TCanvas();
  htof[0]->Draw();

  TSpectrum spec(n_peak);
  spec.Search(htof[0], 3, "", 0.01);
  //  spec.Search(htof[0]);
  float *xpos = spec.GetPositionX();
  int n = spec.GetNPeaks();
  std::sort(xpos, xpos+n);
  for(int i = 0; i<n; ++i){
    x[i] = i*10;
    double peakpos = *(xpos+i);
    //    std::cout << peakpos << std::endl;
    htof[0]->Fit("fgaus", "QNO", "", peakpos-1, peakpos+1);
    y[i] = fgaus->GetParameter("Mean");

  }

  TGraph *graph = new TGraph(n, x, y);
  graph->Draw("ap");
  graph->Fit("pol1");

  htof[0]->Fit("fgaus", "", "", 133, 136);
}

const int NofCh = 32;

const double time_semi = 1.92077;

void draw_lutdist()
{
  gROOT->SetStyle("MyStyle");

  TChain *tree = new TChain("tree", "tree");
  tree->Add("rootfile/run1.root");
  tree->Add("rootfile/run2.root");
  
  int Nhit;
  int Ch[NofCh];
  int FineCount[NofCh];
  tree->SetBranchAddress("Nhit", &Nhit);
  tree->SetBranchAddress("Ch", Ch);
  tree->SetBranchAddress("FineCount", FineCount);
  
  
  TH1F* hlut[NofCh];
  for(int i = 0; i<NofCh; ++i){
    hlut[i] = new TH1F(Form("hlut%d", i), "hlut", 64, 0, 64);
  }

  TH1F* hbin_width = new TH1F("hbin_width", "hbin_width", 300, 0, 150);

  int n_event = tree->GetEntries();
  for(int i = 0; i<n_event ; ++i){
    tree->GetEntry(i);

    if(Nhit != 16) continue;
    for(int j = 0; j<Nhit; ++j){
      hlut[Ch[j]]->Fill(FineCount[j]);
    }// for(j)
    
  }// for(i)


  TCanvas *c1 = new TCanvas();
  c1->Divide(4,4);
  for(int i = 0; i<16; ++i){
    c1->cd(1+i);
    hlut[i]->Draw();
  }

  TCanvas *c2 = new TCanvas();
  c2->Divide(4,4);
  for(int i = 0; i<16; ++i){
    c2->cd(1+i);
    hlut[i+16]->Draw();
  }
  
  for(int i = 0; i<NofCh; ++i){

    int NofEntry = hlut[i]->GetEntries();
    for(int j = 0; j<54; ++j){
      
      double width = (double)hlut[i]->GetBinContent(1+j)/NofEntry*time_semi*1000;

      hbin_width->Fill(width);
    }
  }

  new TCanvas();
  SetTitle(hbin_width, "dT [ps]", "Entry");
  TLatex *rms = new TLatex(50, hbin_width->GetMaximum()*0.5, Form("rms = %.2f ps", hbin_width->GetRMS()));
  hbin_width->Draw();
  //  rms->Draw();

  new TCanvas();
  SetTitle(hlut[0], "Tap number", "Counts");
  hlut[0]->Draw();
}

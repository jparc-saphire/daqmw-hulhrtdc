#include<iostream>
#include<fstream>
#include<string>
#include<ios>

#include"TH1.h"
#include"TFile.h"
#include"TTree.h"
#include"TROOT.h"

static const int MaxHit = 64*16;
static const int NofCh  = 64;

// 0-31ch (Slot-U), 32-63ch (Slot-D)
enum slot{i_up, i_down, sizeSlot};
int StopOut[sizeSlot];
int Through[sizeSlot];
int OverFlow[sizeSlot];

// Common stop
int StopFineCount;
int StopEstimator;
int StopCoarseCount;
int common_stop;

// Leading
int Nhit; // total # of multi-hit in HUL
int NhitCh[NofCh]; // # of multi-hit in Ch
int Ch[MaxHit];
int FineCount[MaxHit];
int Estimator[MaxHit];
int CoarseCount[MaxHit];
int tdc_leading[MaxHit];

// Trailing
int tNhit;
int tNhitCh[NofCh];
int tCh[MaxHit];
int tFineCount[MaxHit];
int tEstimator[MaxHit];
int tCoarseCount[MaxHit];
int tdc_trailing[MaxHit];

int raw_tdc[MaxHit];

enum index_argv{
  i_bin, i_RunNo, size_index
};

int main(int argc, char* argv[])
{
  // File open
  std::string run_number   = argv[i_RunNo];
  std::string infile_path  = "data/run" + run_number + ".dat.gz";
  std::string outfile_path = "rootfile/run" + run_number + ".root";

  std::ifstream ifs(infile_path.c_str());
  if(!ifs.is_open()){
    std::cerr << "#E: no such file " << infile_path << std::endl;
    return -1;
  }

  std::cout << "#D: Start decode, Run No. " << run_number << std::endl;

  // ROOT file create
  TFile fout(outfile_path.c_str(), "recreate");
  TTree tree("tree", "tree");
  tree.Branch("Through",        Through,       "Through[2]/I");
  tree.Branch("StopOut",        StopOut,       "StopOut[2]/I");
  tree.Branch("OverFlow",       OverFlow,      "OverFlow[2]/I");

  tree.Branch("StopFineCount",   &StopFineCount, "StopFineCount/I");
  tree.Branch("StopEstimator",   &StopEstimator,  "StopEstimator/I");
  tree.Branch("StopCoarseCount", &StopEstimator,  "StopCoarseCount/I");
  tree.Branch("common_stop",     &common_stop,   "common_stop/I");

  tree.Branch("Nhit",          &Nhit,          "Nhit/I");
  tree.Branch("NhitCh",         NhitCh,        "NhitCh[64]/I");
  tree.Branch("Ch",             Ch,            "Ch[Nhit]/I");
  tree.Branch("FineCount",      FineCount,     "FineCount[Nhit]/I");
  tree.Branch("Estimator",      Estimator,      "Estimator[Nhit]/I");
  tree.Branch("CoarseCount",    CoarseCount,   "CoarseCount[Nhit]/I");
  tree.Branch("tdc_leading",    tdc_leading,   "tdc_leading[Nhit]/I");

  tree.Branch("tNhit",         &tNhit,         "tNhit/I");
  tree.Branch("tNhitCh",        tNhitCh,       "tNhitCh[32]/I");
  tree.Branch("tCh",            tCh,           "tCh[tNhit]/I");
  tree.Branch("tFineCount",     tFineCount,    "tFineCount[tNhit]/I");
  tree.Branch("tEstimator",     tEstimator,     "tEstimator[tNhit]/I");
  tree.Branch("tCoarseCount",   tCoarseCount,  "tCoarseCount[tNhit]/I");
  tree.Branch("tdc_trailing",   tdc_trailing,  "tdc_trailing[Nhit]/I");

  // decode
  static const unsigned int magic           = 0xffff800b;
  static const unsigned int magic_slot_u    = 0xfa000000;
  static const unsigned int magic_slot_d    = 0xfb000000;
  static const unsigned int magic_leading   = 0x6;
  static const unsigned int magic_trailing  = 0x5;
  static const unsigned int magic_stop      = 0x4;

  static const unsigned int mask_slot       = 0xff000000;

  static const int shift_data_head = 29;
  static const int mask_data_head  = 7;

  static const int shift_through   = 11;
  static const int shift_stop_dout = 12;
  static const int shift_overflow  = 13;
  static const int mask_nword      = 0x7ff;

  static const int shift_ch = 24;
  static const int mask_ch  = 0x1f;

  static const int shift_cc = 11;
  static const int mask_cc  = 0x1fff;

  static const int mask_fc  = 0x7ff;

  static const int mask_tdc = 0xffffff;

  unsigned int buf[2048];
  unsigned int header1;
  unsigned int header2;
  unsigned int header3;
  int          n_of_word;
  int          index_slot=0;
  
  while(!ifs.eof()){
    ifs.read((char*)&header1, sizeof(unsigned int ));
    
    if(header1 == magic){
      Nhit       = 0;
      tNhit      = 0;
      index_slot = 0;

      // header
      ifs.read((char*)&header2, sizeof(unsigned int ));
      ifs.read((char*)&header3, sizeof(unsigned int ));
      n_of_word = header2 & mask_nword;

      // data body
      ifs.read((char*)buf, n_of_word*sizeof(unsigned int ));
      int n_read_word = ifs.gcount()/(int)sizeof(unsigned int);
      //      std::cout << "read " << n_read_word << std::endl;
      //      std::cout << "head " << n_of_word << std::endl;
      if(n_read_word != n_of_word){
	if(!ifs.eof()){
	  // not eof
	  std::cerr << "#E: read count mis-match" << std::endl;
	}else{
	  break;
	}

      }// error

      for(int i = 0; i<NofCh; ++i){
	NhitCh[i] = 0;
	tNhitCh[i] = 0;
      }// for(i)

      for(int i = 0; i<n_of_word; ++i){
	// slot identification
	unsigned int reg_slot = (buf[i] & mask_slot);
	if(magic_slot_u == reg_slot || magic_slot_d == reg_slot){
	  index_slot = magic_slot_u == reg_slot ? 0 : 1;
	  OverFlow[index_slot] = (buf[i] >> shift_overflow) & 0x1;
	  StopOut[index_slot]  = (buf[i] >> shift_stop_dout) & 0x1;
	  Through[index_slot]  = (buf[i] >> shift_through) & 0x1;
	}

	// Data body
	unsigned int data_header = (buf[i] >> shift_data_head) & mask_data_head;
	if(data_header == magic_leading){
	  tdc_leading[Nhit] = buf[i] & mask_tdc;

	  Ch[Nhit]   nterpret_cast<unsigned char*>(&(m_header[0])),e      = ((buf[i] >> shift_ch)  & mask_ch) + index_slot*32;
	  unsigned int fc   = (buf[i])              & mask_fc;
	  FineCount[Nhit]   = Through[index_slot] == 1 ? fc : -1;
	  Estimator[Nhit]   = Through[index_slot] == 0 ? fc : -1;
	  CoarseCount[Nhit] = (buf[i] >> shift_cc)  & mask_cc;

	  NhitCh[Ch[Nhit]]++;
	  Nhit++;
	}// leading data

	if(data_header == magic_trailing){
	  tdc_trailing[tNhit] = buf[i] & mask_tdc;

	  tCh[tNhit]         = ((buf[i] >> shift_ch)  & mask_ch) + index_slot*32;
	  unsigned int fc    = (buf[i])              & mask_fc;
	  tFineCount[Nhit]   = Through[index_slot] == 1 ? fc : -1;
	  tEstimator[Nhit]   = Through[index_slot] == 0 ? fc : -1;
	  tCoarseCount[tNhit]= (buf[i] >> shift_cc)  & mask_cc;

	  tNhitCh[tCh[tNhit]]++;
	  tNhit++;
	}// trailing data

	if(data_header == magic_stop){
	  common_stop    = buf[i] & mask_tdc;

	  unsigned int fc= (buf[i])              & mask_fc;
	  StopFineCount  = Through[index_slot] == 1 ? fc : -1;
	  StopEstimator  = Through[index_slot] == 0 ? fc : -1;
	  StopCoarseCount= (buf[i] >> shift_cc)  & mask_cc;
	}// common stop data

      }// for(i)

      tree.Fill();
    }// decode one event
  }// while

  fout.Write();
  fout.Close();

  std::cout << "#D: End of decode" << std::endl;

  return 0;
}

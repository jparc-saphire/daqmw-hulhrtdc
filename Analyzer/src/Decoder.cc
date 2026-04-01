#include<ios>
#include<iomanip>
#include<iostream>
#include<cstdlib>
#include<cmath>
#include<algorithm>
#include<functional>
#include<arpa/inet.h>
#include <cstdint>

#include"Decoder.hh"

#include "ConfMan.hh"

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file.hpp>

#define BLCOR 0
#define VER2  0

static const std::string MyName = "Decoder";

const double kTimeUnit      = 1.0/960e6/1e-9; // nsec
const double kAmplitudeUnit = 0.445; // mV
const double kChargeUnit    = kTimeUnit*kAmplitudeUnit/50; // pC
const double kTDCTimeUnit   = 1.0;

// Open ----------------------------------------------------------------------
bool
Decoder::Open(const char* name)
{
  static const std::string MyFunc = "::Open ";

  {// local scope
    std::ifstream ifs_org(name, std::ios::binary);

    if(!ifs_org.is_open()){
      std::cerr << MyName << MyFunc
                << "File open error " << name << std::endl;
      return false;
    }
  }

  // add gzip compressor to filter chain
 // m_ifs.push(boost::iostreams::gzip_decompressor());
  m_ifs.push(boost::iostreams::file_source(name, std::ios::binary));
  m_ifs.auto_close();

  return true;
}

// getNextEvent --------------------------------------------------------------
bool
Decoder::getNextEvent()
{
  static const std::string MyFunc = "::getNextEvent ";
  // EOF ?
  if(m_ifs.eof()){
    std::cout << "#D : " << MyName << MyFunc
	      << "End Of File" << std::endl;
    return false;
  }

  const unsigned int DAQMW_HEADERSIZE = 8 ; //bytes
  const unsigned int DAQMW_FOOTERSIZE = 8 ;
  char buf[DAQMW_HEADERSIZE];
  m_ifs.read(buf,DAQMW_HEADERSIZE);

  //DAQMW header check 
  if( ((buf[0] & 0x000000ff)  != 0xe7 ) && ((buf[1] & 0x000000ff)  != 0xe7 )){
    std::cerr << "Invalid DAQMW header !"  << std::endl;
    for(unsigned int i=0;i<DAQMW_HEADERSIZE; i++){
      std::cout << std::setfill('0') << std::hex << std::setw(8) << (int) buf[i] << " ";
    }
    std::cerr << " " << std::endl;
      
    return false;
  } else {//sako
    //    std::cerr << "Good DAQMW header !"  << std::endl;
    //    for(unsigned int i=0;i<DAQMW_HEADERSIZE; i++){
    //      std::cout << std::setfill('0') << std::hex << std::setw(8) << (int) buf[i] << " ";
    //    }
    //    std::cerr << " " << std::endl;

  }
  char bufComp[2*sizeof(uint32_t)];
  m_ifs.read(bufComp,2*sizeof(uint32_t));

  //MznHRTdc
  char mzncompheader[2*sizeof(uint32_t)];
  m_ifs.read(mzncompheader,2*sizeof(uint32_t));

  unsigned int mznheader1=0;
  unsigned int mznheader2=0;
  unsigned int mznheader3=0;
  int          mznnword=0;
  int          index_slot=0;

  sMZNData.clear();
  
  m_ifs.read((char*)&mznheader1, sizeof(unsigned int));
  if(mznheader1 !=MZN_HRTDC::magic) {
    std::cerr << "Not MZN_HRTDC magic word " << std::hex << std::setw(8) << mznheader1 << std::endl;
    return false;  
  }

  m_ifs.read((char*)&mznheader2, sizeof(unsigned int));
  mznnword = mznheader2 & MZN_HRTDC::mask_nword;
  m_ifs.read((char*)&mznheader3, sizeof(unsigned int));
  
  for(int i=0;i<mznnword;i++){
    unsigned int rdata =0;
    m_ifs.read((char*)&rdata, sizeof(unsigned int));
    unsigned int reg_slot = rdata & MZN_HRTDC::mask_slot;
  
    if(MZN_HRTDC::magic_slot_u == reg_slot || MZN_HRTDC::magic_slot_d == reg_slot){
      // slot identification
      index_slot = MZN_HRTDC::magic_slot_u == reg_slot ? 0 : 1;
      sMZNData.OverFlow[index_slot] = (rdata >> MZN_HRTDC::shift_overflow) & 0x1;
      sMZNData.StopOut[index_slot]  = (rdata >> MZN_HRTDC::shift_stop_dout) & 0x1;
      sMZNData.Through[index_slot]  = (rdata >> MZN_HRTDC::shift_through) & 0x1;
    }
    // Data body
    unsigned int data_header = (rdata >> MZN_HRTDC::shift_data_head) & MZN_HRTDC::mask_data_head;
    // std::cout << "#D data type = " << data_header << std::endl;
    if(data_header == MZN_HRTDC::magic_leading){
      sMZNData.tdc_leading[sMZNData.Nhit] = rdata & MZN_HRTDC::mask_tdc;

      sMZNData.Ch[sMZNData.Nhit]          = ((rdata >> MZN_HRTDC::shift_ch)  & MZN_HRTDC::mask_ch) + index_slot*32;
      unsigned int fc               = (rdata) & MZN_HRTDC::mask_fc;
      sMZNData.FineCount[sMZNData.Nhit]   = sMZNData.Through[index_slot] == 1 ? fc : -1;
      sMZNData.Estimator[sMZNData.Nhit]   = sMZNData.Through[index_slot] == 0 ? fc : -1;
      sMZNData.CoarseCount[sMZNData.Nhit] = (rdata >> MZN_HRTDC::shift_cc)  &  MZN_HRTDC::mask_cc;

      sMZNData.NhitCh[sMZNData.Ch[sMZNData.Nhit]]++;
      sMZNData.Nhit++;
    }// leading data
    
    if(data_header ==  MZN_HRTDC::magic_trailing){
      sMZNData.tdc_trailing[sMZNData.tNhit] = rdata & MZN_HRTDC::mask_tdc;

      sMZNData.tCh[sMZNData.tNhit]          = ((rdata >> MZN_HRTDC::shift_ch)  & MZN_HRTDC::mask_ch) + index_slot*32;
      unsigned int fc                 = (rdata)              & MZN_HRTDC::mask_fc;
      sMZNData.tFineCount[sMZNData.tNhit]   = sMZNData.Through[index_slot] == 1 ? fc : -1;
      sMZNData.tEstimator[sMZNData.tNhit]   = sMZNData.Through[index_slot] == 0 ? fc : -1;
      sMZNData.tCoarseCount[sMZNData.tNhit] = (rdata >> MZN_HRTDC::shift_cc)  & MZN_HRTDC::mask_cc;

      sMZNData.tNhitCh[sMZNData.tCh[sMZNData.tNhit]]++;
      sMZNData.tNhit++;
    }// trailing data

    if(data_header == MZN_HRTDC::magic_stop){
      sMZNData.common_stop     = rdata & MZN_HRTDC::mask_tdc;

      unsigned int fc       = (rdata) & MZN_HRTDC::mask_fc;
      sMZNData.StopFineCount   = sMZNData.Through[index_slot] == 1 ? fc : -1;
      sMZNData.StopEstimator   = sMZNData.Through[index_slot] == 0 ? fc : -1;
      sMZNData.StopCoarseCount = (rdata >> MZN_HRTDC::shift_cc)  & MZN_HRTDC::mask_cc;
    }// common stop data
  }//for mznnword

  char DAQMWfooter[DAQMW_FOOTERSIZE];
  m_ifs.read(DAQMWfooter,DAQMW_FOOTERSIZE);
  //DAQMW footer check 
  if( ((DAQMWfooter[0] & 0x000000ff)  != 0xcc ) && ((DAQMWfooter[1] & 0x000000ff)  != 0xcc )){
    std::cerr << "Invalid DAQMW footer !"  << std::endl;
    for(unsigned int i=0;i<DAQMW_FOOTERSIZE; i++){
      std::cout << std::setfill('0') << std::hex << std::setw(8) << (int) DAQMWfooter[i] << " " ;
    }
    std::cout << std::dec  << std::endl;
  } else {
    //    std::cerr << "Good DAQMW footer !"  << std::endl;
    //    for(unsigned int i=0;i<DAQMW_FOOTERSIZE; i++){
    //      std::cout << std::setfill('0') << std::hex << std::setw(8) << (int) DAQMWfooter[i] << " " ;
    //    }
    //    std::cout << std::dec  << std::endl;
  }
  
  return true;
}
// Constructor ---------------------------------------------------------------
Decoder::Decoder()
{
}

// Destructor ----------------------------------------------------------------
Decoder::~Decoder()
{
  if (!m_ifs.empty()) m_ifs.reset();
}


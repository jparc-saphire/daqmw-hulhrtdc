// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include <THttpServer.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TStyle.h>
#include <cstdlib>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdint>

#include "Monitor.h"
#include "MznHRTdcData.hh"
#include "MznHRTdcDraw.hh"

#include "getaddr.h"
#include "HexDump.hh"

//______________________________________________________________________________
using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::INPORT_ERROR;
using DAQMW::FatalType::HEADER_DATA_MISMATCH;
using DAQMW::FatalType::FOOTER_DATA_MISMATCH;
using DAQMW::FatalType::USER_DEFINED_ERROR1;

MznHRTdcDraw mzntdc("mzntdc0");

struct MZN_HRTDC::Data_t sMZNData;

//______________________________________________________________________________
// Module specification
// Change following items to suit your component's spec.
static const char* monitor_spec[] =
  {
    "implementation_id", "Monitor",
    "type_name",         "Monitor",
    "description",       "Monitor component",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
  };

//______________________________________________________________________________
Monitor::Monitor(RTC::Manager* manager)
  : DAQMW::DaqComponentBase(manager),
    m_InPort("in0",   m_in_data)
{
  // Registration: InPort/OutPort/Service

  // Set InPort buffers
  registerInPort ("in0",  m_InPort);

  init_command_port();
  init_state_table();
  set_comp_name("MONITOR");

  gROOT->SetStyle("Plain");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetFrameFillColor(0);
  //  gStyle->SetCanvasColor(0);
  gStyle->SetPadBorderSize(0);
  gStyle->SetTitleAlign(22);
  gStyle->SetTitleX(0.5);
  gStyle->SetTitleY(0.95);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  Int_t fontid = 132;

  gStyle->SetStatFont(fontid);
  gStyle->SetLabelFont(fontid, "XYZ");
  gStyle->SetLabelFont(fontid, "");
  gStyle->SetTitleFont(fontid, "XYZ");
  gStyle->SetTitleFont(fontid, "");
  gStyle->SetTitleOffset(1.2, "XYZ");
  gStyle->SetTextFont(fontid);
  gStyle->SetFuncWidth(2);
  gStyle->SetLegendBorderSize(0);

  gStyle->SetOptStat("ouirmen");
  gStyle->SetOptFit(true);
  
  gStyle->SetPadGridX(true);
  gStyle->SetPadGridY(true);

}

//______________________________________________________________________________
Monitor::~Monitor()
{
}

//______________________________________________________________________________
RTC::ReturnCode_t Monitor::onInitialize()
{
  if (m_Verbosity) {
    std::cerr << "Monitor::onInitialize()" << std::endl;
  }

  return RTC::RTC_OK;
}

//______________________________________________________________________________
RTC::ReturnCode_t Monitor::onExecute(RTC::UniqueId ec_id)
{
  daq_do();

  return RTC::RTC_OK;
}

//______________________________________________________________________________
int Monitor::daq_dummy()
{
  update();
  //gSystem->Sleep(10);
  
  return 0;
}

//______________________________________________________________________________
int Monitor::daq_configure()
{
  std::cerr << "*** Monitor::configure" << std::endl;

  ::NVList* paramList;
  paramList = m_daq_service0.getCompParams();
  parse_params(paramList);


  if (!m_server) {
    m_server = new THttpServer(m_serverName.c_str());
  }
  m_server->SetReadOnly(kFALSE);
  
  //MZN-MHTDC
  update();
  mzntdc.Initialize(m_server);
  if(m_Verbosity){
    std::cout << "init MZN-HRTDC" << std::endl;
  }

  update();
  std::cout << "***************************" << std::endl;
  std::cout << "Monitor Initialized" << std::endl;
  std::cout << "***************************" << std::endl;

  return 0;
}

//______________________________________________________________________________
int Monitor::parse_params(::NVList* list)
{

  std::cerr << "param list length:" << (*list).length() << std::endl;

  int len = (*list).length();
  for (int i = 0; i < len; i+=2) {
    std::string sname  = (std::string)(*list)[i].value;
    std::string svalue = (std::string)(*list)[i+1].value;

    std::cerr << "sname: " << sname << "  ";
    std::cerr << "value: " << svalue << std::endl;
    if( sname=="verbosity"){
       m_Verbosity = std::stoul(svalue);
    }
    if (sname=="server") {
      m_serverName = svalue;
    }
    if (sname=="update_cycle") {
      m_update_cycle = std::stoul(svalue);
    }
    if (sname=="canupdate_cycle"){
      m_canupdate_cycle = std::stoul(svalue);
    }
  }


  return 0;
}

//______________________________________________________________________________
int Monitor::daq_unconfigure()
{
  std::cerr << "*** Monitor::unconfigure" << std::endl;

  update();
  
  return 0;
}

//______________________________________________________________________________
int Monitor::daq_start()
{
  std::cerr << "*** Monitor::start" << std::endl;

  m_in_status  = BUF_SUCCESS;
  mzntdc.Reset();
  update();

  return 0;
}

//______________________________________________________________________________
int Monitor::daq_stop()
{
  std::cout<< "Total Event: " << EventNum_ << std::endl;
  EventNum_=0;

  std::cerr << "*** Monitor::stop" << std::endl;
  update();
    
  reset_InPort();


  return 0;
}

//______________________________________________________________________________
int Monitor::daq_pause()
{
  std::cerr << "*** Monitor::pause" << std::endl;
  update();
    
  return 0;
}

//______________________________________________________________________________
int Monitor::daq_resume()
{
  std::cerr << "*** Monitor::resume" << std::endl;
  update();
    
  return 0;
}

//______________________________________________________________________________
int Monitor::reset_InPort()
{
  int ret = true;
  while(ret == true) {
    ret = m_InPort.read();
  }

  return 0;
}

//______________________________________________________________________________
unsigned int Monitor::read_InPort()
{
  if (check_trans_lock()) set_trans_unlock();

    
  /////////////// read data from InPort Buffer ///////////////
  unsigned int recv_byte_size = 0;
  bool ret = m_InPort.read();

  //////////////////// check read status /////////////////////
  if (ret == false) { // false: TIMEOUT or FATAL
    m_in_status = check_inPort_status(m_InPort);
    if (m_in_status == BUF_TIMEOUT) { // Buffer empty.
      if (check_trans_lock()) {     // Check if stop command has come.
        set_trans_unlock();       // Transit to CONFIGURE state.
      }
    }
    else if (m_in_status == BUF_FATAL) { // Fatal error
      fatal_error_report(INPORT_ERROR);
    }
  }
  else {
    recv_byte_size = m_in_data.data.length();
  }

  if (m_Verbosity) {
    std::cerr << "m_in_data.data.length():" << recv_byte_size
              << std::endl;
  }

  return recv_byte_size;
}

//______________________________________________________________________________
int Monitor::daq_run()
{
  if (m_Verbosity) {
    std::cerr << "*** Monitor::run" << std::endl;
  }

  unsigned int recv_byte_size = read_InPort();
  if (recv_byte_size == 0) { // Timeout
    return 0;
  }

  // temporarily comment out to avoid footer mismatch
  //  check_header_footer(m_in_data, recv_byte_size); // check header and footer
   
  //get data size after subtracting header and footer
  unsigned int event_byte_size = get_event_size(recv_byte_size);
  if(m_Verbosity){
    std::cerr << event_byte_size << std::endl;
  }
 
  /////////////  Write component main logic here. /////////////
  //H. Asano memo
  //HEADER_BYTE_SIZE is 8 bytes, defined in /user/include/DaqComponentBase.h
  //after the header defined by DAQMW, the data sent by readerComp will be started.
  //data structure
  //1. DAQ-MW header (8 bytes, fixed length) 
  //2. Merger comp header
  //3. comp header + MZN-HRTDC (variable length)
  //4. DAQ-MW footer (8 bytes,fixed length)
  
  std::vector <unsigned char> recv_data;
  recv_data.resize(event_byte_size);
  
  //cut off DAQ-MW's header and Merger's header
  //Note "event_byte_size" does not include DAQ-MW's header and footer size
  auto wp = HEADER_BYTE_SIZE;//DAQ-MW's header
  wp += 2*sizeof(uint32_t);//Merger's header
  memcpy(&recv_data[0], &m_in_data.data[wp], event_byte_size);

  wp+=0;
  std::vector<unsigned int> recv_data2((event_byte_size-2*sizeof(uint32_t))/sizeof(unsigned int));
  memcpy(reinterpret_cast<unsigned char*>(&recv_data2[0]),
         &m_in_data.data[wp],
         (event_byte_size-2*sizeof(uint32_t)));
  
  auto dataitr = recv_data2.begin();

  //MZN-HR TDC
  dataitr += 2;//Comp header
  
  unsigned int mznheader1=0;
  unsigned int mznheader2=0;
  unsigned int mznheader3=0;
  int          mznnword=0;
  int          index_slot=0;

  sMZNData.clear();

  mznheader1  = *dataitr;
  if(mznheader1 !=MZN_HRTDC::magic) {
    std::cerr << "Not MZN_HRTDC magic word " << std::hex << std::setw(8) << mznheader1 << std::endl;
  }

  mznheader2 =  *(dataitr+1) ;    
  mznnword = mznheader2 & MZN_HRTDC::mask_nword;
  mznheader3 =  *(dataitr+2) ; //thrown away
  
  for(int i=0;i<mznnword;i++){
    const auto rdata = *(dataitr+i+3);
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
  
  FillHistos();
  inc_sequence_num();                       // increase sequence num.
  inc_total_data_size(event_byte_size);     // increase total data byte size


  EventNum_++;
  if(EventNum_%1000 == 0){
    std::cout<< "Event: " << EventNum_ << std::endl;
  }

  return 0;
}

//______________________________________________________________________________
void
Monitor::parse_file(const std::string& fileName)
{
  std::cout << "#D parse() file = " << fileName << std::endl;
  if (fileName.empty()) return;
  
  std::ifstream ifile(fileName.c_str());
  if (ifile.fail()) {
    std::cout << " error file open " << fileName << std::endl;
    return;
  }
  
  while (ifile) {
    std::string line;
    std::getline(ifile, line);
    if (line.empty()) continue;
    if (line.find("#")!=std::string::npos) continue;
    std::istringstream iss(line);
    Int_t ch;
    std::string name;
    std::string title;
    Int_t start;
    Int_t end;
    Int_t threshold;
    iss >> ch >> name >> title >> start >> end >> threshold;

    std::cout << ch << " "
              << start << " "
              << end << " "
              << threshold
              << std::endl;
  }
}


//______________________________________________________________________________
void
Monitor::update()
{
  gSystem->ProcessEvents();
}

bool Monitor::FillHistos()
{
  bool IsCanvasUpdate = false;
  if ((get_sequence_num() % m_canupdate_cycle == 0)) IsCanvasUpdate = true;

  //Fill
  mzntdc.Fill(sMZNData,IsCanvasUpdate); 

  update();

  return true;
}


//______________________________________________________________________________
extern "C"
{
  void MonitorInit(RTC::Manager* manager)
  {
    RTC::Properties profile(monitor_spec);
    manager->registerFactory(profile,
                             RTC::Create<Monitor>,
                             RTC::Delete<Monitor>);
  }
};



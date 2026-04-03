// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <algorithm>
#include <functional>
#include <sstream>
#include <thread> // for sleep
#include <memory>

#include "UDPRBCP.hh"
#include "RegisterMap.hh"
#include "BctBusBridgeFunc.hh"
#include "daq_funcs.hh"

#include "MznHRTdcReader1.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::OUTPORT_ERROR;
using DAQMW::FatalType::USER_DEFINED_ERROR1;
using DAQMW::FatalType::USER_DEFINED_ERROR2;

// Module specification
// Change following items to suit your component's spec.
//______________________________________________________________________________
static const char* mzn_hrtdc_reader_spec[] =
  {
    "implementation_id", "MznHRTdcReader1",
    "type_name",         "MznHRTdcReader1",
    "description",       "MznHRTdcReader1 component",
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
MznHRTdcReader1::MznHRTdcReader1(RTC::Manager* manager)
  : DAQMW::DaqComponentBase(manager),
    m_OutPort("out1", m_out_data),
    m_data(kNofData)
{
  // Registration: InPort/OutPort/Service

  // Set OutPort buffers
  registerOutPort("out1", m_OutPort);

  init_command_port();
  init_state_table();
  set_comp_name("MznHRTdcReader1");
}

//______________________________________________________________________________
RTC::ReturnCode_t MznHRTdcReader1::onInitialize()
{
  if (m_debug) {
    std::cerr << "MznHRTdcReader1::onInitialize()" << std::endl;
  }

  return RTC::RTC_OK;
}

//______________________________________________________________________________
RTC::ReturnCode_t MznHRTdcReader1::onExecute(RTC::UniqueId ec_id)
{
  daq_do();

  return RTC::RTC_OK;
}

//______________________________________________________________________________
int MznHRTdcReader1::daq_dummy()
{
  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::daq_configure()
{
  std::cerr << "*** MznHRTdcReader1::configure" << std::endl;

  ::NVList* paramList;
  paramList = m_daq_service0.getCompParams();
  parse_params(paramList);

  using namespace HUL;
  using namespace HUL::HRTDC_BASE;

  if (!m_udpRbcp || !m_fModule) {
    std::cout << " create module " << std::endl;
    m_udpRbcp = std::make_unique<RBCP::UDPRBCP>(
        m_srcAddr, static_cast<uint32_t>(m_udpPort), RBCP::UDPRBCP::kNoDisp);
    m_fModule = std::make_unique<HUL::FPGAModule>(*m_udpRbcp);
  }

  m_fModule->WriteModule(HUL::BCT::kAddrReset, 0);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  if (m_enUp) {
    m_fModule->WriteModule(DCT::kAddrCtrlReg, 1);
    m_fModule->WriteModule(DCT::kAddrCtrlReg, DCT::kRegFRstU);
    m_fModule->WriteModule(DCT::kAddrCtrlReg, 0);
  }
  if (m_enDown) {
    m_fModule->WriteModule(DCT::kAddrCtrlReg, 1);
    m_fModule->WriteModule(DCT::kAddrCtrlReg, DCT::kRegFRstD);
    m_fModule->WriteModule(DCT::kAddrCtrlReg, 0);
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "set registers " << std::endl;

  {
    auto fwBase = m_fModule->ReadModule(HUL::BCT::kAddrVersion, 4);
    std::cout << "#D Firmware (BASE): " << std::hex << fwBase << std::dec << std::endl;

    if (m_enUp) {
      auto fwMU = ReadModuleIn2ndryFPGA(*m_fModule, BBP::kUpper,
                                        HRTDC_MZN::BCT::kAddrVersion, 4);
      std::cout << "#D Firmware (Mezzanine U): " << std::hex << fwMU << std::dec
                << std::endl;
    } else {
      std::cout << "No Mezzanine U" << std::endl;
    }

    if (m_enDown) {
      auto fwMD = ReadModuleIn2ndryFPGA(*m_fModule, BBP::kLower,
                                        HRTDC_MZN::BCT::kAddrVersion, 4);
      std::cout << "#D Firmware (Mezzanine D): " << std::hex << fwMD << std::dec
                << std::endl;
    } else {
      std::cout << "No Mezzanine D" << std::endl;
    }
  }

  std::cout << "set registers sel trig" << std::endl;
  uint32_t sel_trig = TRM::kRegL1Ext;
  m_fModule->WriteModule(TRM::kAddrSelectTrigger, sel_trig, 2);

  std::cout << "set registers (leading/trailing blocks, search windows)" << std::endl;
  uint32_t en_blocks =
      HRTDC_MZN::DCT::kEnLeading | HRTDC_MZN::DCT::kEnTrailing;
  if (m_enUp) {
    WriteModuleIn2ndryFPGA(*m_fModule, BBP::kUpper, HRTDC_MZN::DCT::kAddrEnBlocks,
                           en_blocks, 1);
  }
  if (m_enDown) {
    WriteModuleIn2ndryFPGA(*m_fModule, BBP::kLower, HRTDC_MZN::DCT::kAddrEnBlocks,
                           en_blocks, 1);
  }
  if (m_enUp) {
    SetTdcWindow(m_windowMax, m_windowMin, *m_fModule, BBP::kUpper);
  }
  if (m_enDown) {
    SetTdcWindow(m_windowMax, m_windowMin, *m_fModule, BBP::kLower);
  }

  m_fModule->WriteModule(IOM::kAddrExtL1, IOM::kReg_i_Nimin1);

  if (m_sock == NULL) {
    try {
      m_sock = new DAQMW::Sock();
      std::cout << "conecting socket ... " << std::endl;
      m_sock->connect(m_srcAddr, m_srcPort);
    } catch (DAQMW::SockException& e) {
      std::cerr << "Sock Fatal Error : " << e.what() << std::endl;
      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    } catch (...) {
      std::cerr << "Sock Fatal Error : Unknown" << std::endl;
      fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }
  }
  std::cout << "socket connected" << std::endl;

  std::cout << "set receive/read timeout " << m_recvtimeout << " sec" << std::endl;
  m_sock->setOptRecvTimeOut(m_recvtimeout);

  m_fModule->WriteModule(DCT::kAddrResetEvb, 0);

  std::cout << __FILE__ << " L." << __LINE__ << ": set registers tdc window "
            << "mezzanine (U, D) = (" << m_enUp << ", " << m_enDown << ")"
            << " (min, max) = (" << m_windowMin << ", " << m_windowMax << ")"
            << std::endl;

  DdrInitialize(*m_fModule, m_enUp, m_enDown);

  if (m_enUp) {
    CalibLUT(*m_fModule, BBP::kUpper);
  }
  if (m_enDown) {
    CalibLUT(*m_fModule, BBP::kLower);
  }

  uint32_t tdc_ctrl = 0;
  if (m_enUp) {
    std::cout << "en up" << std::endl;
    WriteModuleIn2ndryFPGA(*m_fModule, BBP::kUpper, HRTDC_MZN::TDC::kAddrControll,
                           tdc_ctrl, 1);
    WriteModuleIn2ndryFPGA(*m_fModule, BBP::kUpper, HRTDC_MZN::DCT::kAddrGate, 1,
                           1);
  }
  if (m_enDown) {
    std::cout << "en down" << std::endl;
    WriteModuleIn2ndryFPGA(*m_fModule, BBP::kLower, HRTDC_MZN::TDC::kAddrControll,
                           tdc_ctrl, 1);
    WriteModuleIn2ndryFPGA(*m_fModule, BBP::kLower, HRTDC_MZN::DCT::kAddrGate, 1,
                           1);
  }
  m_fModule->WriteModule(DCT::kAddrDaqGate, 1);
  m_fModule->WriteModule(DCT::kAddrDaqGate, 1);

  std::cout << "***************************" << std::endl;
  std::cout << "MznHRTdcReader1 Initialized" << std::endl;
  std::cout << "***************************" << std::endl;

  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::parse_params(::NVList* list)
{
  std::cerr << "param list length:" << (*list).length() << std::endl;

  int len = (*list).length();
  for (int i = 0; i < len; i+=2) {
    std::string sname  = (std::string)(*list)[i].value;
    std::string svalue = (std::string)(*list)[i+1].value;

    std::cerr << "sname: " << sname << "  ";
    std::cerr << "value: " << svalue << std::endl;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (sname == "debug") {
      if (svalue=="true" || svalue=="TRUE" || 
	  svalue=="yes" || svalue=="YES" || svalue=="y" || svalue=="Y" || svalue=="1") m_debug=true;
      else m_debug = false;
    }

    if (sname == "timeout"){
      m_recvtimeout = std::stoul(svalue);
      std::cout << "recv. time out (sec.) " << m_recvtimeout << std::endl;
    }


    if (sname == "srcAddr") {
      std::stringstream ss(svalue);
      ss >> m_srcAddr;
      if (m_debug) {
        std::cout << "#D source address = " << m_srcAddr << std::endl;
      }
      
      m_srcAddr         = getAddrIPv4_str(m_srcAddr.c_str());
      m_compHeader.m_id = getAddrIPv4(m_srcAddr.c_str());
      std::cout << "#D board ID = " << std::hex << m_compHeader.m_id << std::dec << std::endl;
    }
    
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (sname == "srcPort") {
      m_srcPort = std::stoul(svalue);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // if (sname == "boardId") {
    //   m_compHeader.m_id = std::strtoul(svalue.c_str(), 0, 16);
    // }
    
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (sname == "udpPort") {
      m_udpPort = std::stoul(svalue);
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (sname == "window_max") {
      m_windowMax = std::stoul(svalue);
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (sname == "window_min") {
      m_windowMin = std::stoul(svalue);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (sname == "en_up") {
      if (svalue=="yes" || svalue=="YES" || svalue=="y" || svalue=="Y" ||
	  svalue=="true" || svalue=="TRUE" || svalue=="1") m_enUp = true;
      else  m_enUp = false;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (sname == "en_down") {
      if (svalue=="yes" || svalue=="YES" || svalue=="y" || svalue=="Y" ||
	  svalue=="true" || svalue=="TRUE" || svalue=="1")  m_enDown = true;
      else  m_enDown = false;
    }

  }

  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::daq_unconfigure()
{
  std::cerr << "*** MznHRTdcReader1::unconfigure" << std::endl;

  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::daq_start()
{
  std::cerr << "*** MznHRTdcReader1::start" << std::endl;
  reset_sequence_num();
  reset_total_data_size();
  m_out_status = BUF_SUCCESS;

  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::daq_stop()
{
  std::cerr << "*** MznHRTdcReader1::stop" << std::endl;

  m_sock->setOptRecvTimeOut(2);
  m_fModule->WriteModule(HUL::HRTDC_BASE::DCT::kAddrDaqGate, 0);

  if (m_enUp) {
    WriteModuleIn2ndryFPGA(*m_fModule, HUL::HRTDC_BASE::BBP::kUpper,
                           HUL::HRTDC_MZN::DCT::kAddrGate, 0, 1);
  }

  if (m_enDown) {
    WriteModuleIn2ndryFPGA(*m_fModule, HUL::HRTDC_BASE::BBP::kLower,
                           HUL::HRTDC_MZN::DCT::kAddrGate, 0, 1);
  }

  size_t thrownSize=0;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  //while(-1 != Event_Cycle(m_sock, &m_data[0]));
  unsigned char rs[1]={0};
  while( (m_sock->read(rs,1) !=DAQMW::Sock::ERROR_TIMEOUT)){
    thrownSize++;
  }
  std::cout << std::dec <<  "ThrownDataSize: " << thrownSize << std::endl;

  //  shutdown(m_sock, SHUT_RDWR);
  //close(m_sock);
  if (m_sock) {
      m_sock->disconnect();
      delete m_sock;
      m_sock = NULL;
  }
  std::cerr << std::dec << "total sequence # " << get_sequence_num() << std::endl;
  

  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::daq_pause()
{
  std::cerr << "*** MznHRTdcReader1::pause" << std::endl;

  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::daq_resume()
{
  std::cerr << "*** MznHRTdcReader1::resume" << std::endl;

  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::read_data_from_detectors()
{
  if (m_debug) {
    std::cout << "#D read_data_from_detectors()" << std::endl;
  }
  int received_data_size = 0;
  /// write your logic here

  //functions if you read using standalone codes
  //auto n_word = 0; 
  //while (-1 == (n_word = Event_Cycle(m_sock, &m_data[0])));
  //received_data_size = n_word * sizeof(uint32_t);
  
  unsigned int sizeofHeader = kNofHead*sizeof(unsigned int);
  int status = m_sock->readAll(reinterpret_cast<unsigned char*>(&(m_data[0])), sizeofHeader);

  if (status == DAQMW::Sock::ERROR_FATAL) {
    std::cerr << "### ERROR: m_sock->readAll header" << std::endl;
    fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
  } else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
    std::cerr << "### Timeout: m_sock->readAll header" << std::endl;
    return 0;
  } else {
    received_data_size += sizeofHeader;
  }

  unsigned int n_word_data = m_data[1] & 0x7ff;
  unsigned int sizeData    = n_word_data*sizeof(unsigned int);
  if(n_word_data == 0) return received_data_size;
   
  /*
  unsigned int revdSize=0;
  //int status=0;
  unsigned char tmpdata[sizeData];
  while(revdSize < sizeData){
    status = m_sock->read(&(tmpdata[sizeofHeader+revdSize]),sizeData - revdSize);
    if(status == 0) break;
    if (status == DAQMW::Sock::ERROR_FATAL) {
       std::cerr << "### ERROR: m_sock->readAll header" << std::endl;
       fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
    }else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
      std::cerr << "### Timeout: m_sock->readAll header" << std::endl;
      fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
    }else {
      revdSize += status;
    }
  }
  received_data_size += revdSize;
  memcpy(reinterpret_cast<unsigned char*>(&m_data[kNofHead]),&tmpdata[0],sizeData); 
  */
  
  //read data body
  status = m_sock->readAll(reinterpret_cast<unsigned char*>(&m_data[kNofHead]),sizeData);

  if (status == DAQMW::Sock::ERROR_FATAL) {
    std::cerr << "### ERROR: m_sock->readAll body" << std::endl;
    fatal_error_report(USER_DEFINED_ERROR1, "SOCKET FATAL ERROR");
  }else if (status == DAQMW::Sock::ERROR_TIMEOUT) {
    std::cerr << "### Timeout: m_sock->readAll body" << std::endl;
    fatal_error_report(USER_DEFINED_ERROR2, "SOCKET TIMEOUT");
  }else {
    received_data_size += sizeData;
  }
  
  //std::cout << " recv data size = " << received_data_size << std::endl;
  return received_data_size;
}

//______________________________________________________________________________
//asano memo:
//data stuctre
//1.DAQMW's header 8byte
//2.TNT defined comp header including IP address 
//3.Hul's header and data
//4.DAQMW's footer 8byte

int MznHRTdcReader1::set_data(unsigned int data_byte_size)
{
  unsigned char header[8];
  unsigned char footer[8];
  
  //asano memo: here data_byte_size does not include header size
  set_header(&header[0], data_byte_size + kCompHeaderSize);
  set_footer(&footer[0]);

  ///set OutPort buffer length
  m_out_data.data.length(data_byte_size + kCompHeaderSize + HEADER_BYTE_SIZE + FOOTER_BYTE_SIZE);
  memcpy(&(m_out_data.data[0]), &header[0], HEADER_BYTE_SIZE);

  auto wp = HEADER_BYTE_SIZE;
  memcpy(&(m_out_data.data[wp]), reinterpret_cast<uint8_t*>(&m_compHeader), kCompHeaderSize);

  wp += kCompHeaderSize;
  memcpy(&(m_out_data.data[wp]), reinterpret_cast<uint8_t*>(&m_data[0]), data_byte_size);

  wp += data_byte_size;
  memcpy(&(m_out_data.data[wp]), &footer[0], FOOTER_BYTE_SIZE);

  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::write_OutPort()
{
  ////////////////// send data from OutPort  //////////////////
  bool ret = m_OutPort.write();

  //////////////////// check write status /////////////////////
  if (ret == false) {  // TIMEOUT or FATAL
    m_out_status  = check_outPort_status(m_OutPort);
    if (m_out_status == BUF_FATAL) {   // Fatal error
      fatal_error_report(OUTPORT_ERROR);
    }
    if (m_out_status == BUF_TIMEOUT) { // Timeout
      return -1;
    }
  }
  else {
    m_out_status = BUF_SUCCESS; // successfully done
  }

  return 0;
}

//______________________________________________________________________________
int MznHRTdcReader1::daq_run()
{
  if (m_debug) {
    std::cerr << "*** MznHRTdcReader1::run" << std::endl;
  }

  if (check_trans_lock()) {  // check if stop command has come
    set_trans_unlock();    // transit to CONFIGURED state
    return 0;
  }

  if (m_out_status == BUF_SUCCESS) {   // previous OutPort.write() successfully done
    m_recv_byte_size = read_data_from_detectors();
    if (m_recv_byte_size > 0) {
      set_data(m_recv_byte_size); // set data to OutPort Buffer
      if (write_OutPort() < 0) {
        ;     // Timeout. do nothing.
      } else {
        inc_sequence_num();
        inc_total_data_size(m_recv_byte_size);
      }
    }
  }

  return 0;
}

//______________________________________________________________________________
extern "C"
{
  void MznHRTdcReader1Init(RTC::Manager* manager)
  {
    RTC::Properties profile(mzn_hrtdc_reader_spec);
    manager->registerFactory(profile,
                             RTC::Create<MznHRTdcReader1>,
                             RTC::Delete<MznHRTdcReader1>);
  }
};

unsigned int
getAddrIPv4(const char* hostname)
{
  unsigned int ret = 0xffffffff;
  
  addrinfo hints;
  addrinfo* res;
  sockaddr_in* addr_in;
  int s;

  // initialize hints
  memset(&hints, 0, sizeof(addrinfo));
  //  hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = 0;
  hints.ai_protocol = 0; // any protocol

  // resolve name
  s = getaddrinfo(hostname, 0, &hints, &res);

  if (s!=0) {
    std::string errormess = gai_strerror(s);
    std::cerr << "getaddrinfo(): " << errormess << std::endl;
  }

  // std::cout << "ai_family: " << res->ai_family << std::endl;
  // std::cout << "ai_socktype: " << res->ai_socktype << std::endl;
  // std::cout << "ai_protocol: " << res->ai_protocol << std::endl;
  // std::cout << "ai_addrlen: " << res->ai_addrlen << std::endl;
  //  std::cout << "ai_canonname: " << res->ai_canonname << std::endl;

  addr_in = reinterpret_cast<sockaddr_in*>(res->ai_addr);
  //  std::cout << std::hex << addr_in->sin_addr.s_addr << std::dec << std::endl;
  ret = addr_in->sin_addr.s_addr;
  
  freeaddrinfo(res);
  return ret;
}

std::string
getAddrIPv4_str(const char* hostname)
{
  unsigned int ipv4_i = getAddrIPv4(hostname);
  //  std::cout << " ipv4 = " << std::hex << ipv4_i << std::dec << std::endl;
  unsigned int addr[4];
  addr[0] = ipv4_i & 0xff;
  addr[1] = (ipv4_i>>8) & 0xff;
  addr[2] = (ipv4_i>>16) & 0xff;
  addr[3] = (ipv4_i>>24) & 0xff;
  //  std::cout << std::hex << addr[0] << " " << addr[1] << " " << addr[2] << " " << addr[3] << std::dec << std::endl;
  std::ostringstream ret;
  ret << addr[0] << "."
      << addr[1] << "."
      << addr[2] << "."
      << addr[3];
  // std::cout << ret.str() << std::endl;
  return ret.str();
}



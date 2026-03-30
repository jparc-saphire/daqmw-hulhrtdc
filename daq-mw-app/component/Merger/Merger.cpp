// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */


//data structure
//
//1. DAQ-MW header (8 bytes, fixed length) 
//2. Comp header + NIMEASIROC1 (variable length)
//3. Comp header + NIMEASIROC2 (variable length)
//4. Drs4QDC1 (variable length)
//5. Drs4QDC2 (variable length)
//6. Comp header Hul-Scaler compheader +(12 (header) + 256 (body) = 268 bytes, fixed length) 
//7. Comp header MZN-HRTDC  compheader +(variable length)
//8. Comp header Hul-MHTDC  compheader +(variable length)
//8. DAQ-MW footer (8 bytes,fixed length)
//

#include <string>

//#include "HexDump.hh"
#include "Merger.h"

constexpr int kNInPort = 1;

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::INPORT_ERROR;
using DAQMW::FatalType::OUTPORT_ERROR;
using DAQMW::FatalType::USER_DEFINED_ERROR1;

//______________________________________________________________________________
// Module specification
// Change following items to suit your component's spec.
static const char* merger_spec[] =
  {
    "implementation_id", "Merger",
    "type_name",         "Merger",
    "description",       "Merger component",
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
Merger::Merger(RTC::Manager* manager)
  : DAQMW::DaqComponentBase(manager),
    m_in_data(kNInPort), 
    m_InPort(kNInPort),
    m_OutPort("out", m_out_data),
    m_recv_byte_size(kNInPort, 0),
    m_header(),
    m_in_status(kNInPort, BUF_SUCCESS)
{
  // Registration: InPort/OutPort/Service
  // Set InPort buffers
  for (auto i=0; i<kNInPort; ++i) {
    std::string name = "in" + std::to_string(i);
    InPort<TimedOctetSeq>* p = new InPort<TimedOctetSeq>(name.c_str(), m_in_data[i]);
    m_InPort[i] = p;
    registerInPort(name.c_str(), *p);
  }

  // Set OutPort buffers
  registerOutPort("out", m_OutPort);

  init_command_port();
  init_state_table();
  set_comp_name("MERGER");

  m_header.fMagic = kMagic;
}

//______________________________________________________________________________
RTC::ReturnCode_t Merger::onInitialize()
{
  if (m_debug) {
    std::cerr << "Merger::onInitialize()" << std::endl;
  }

  return RTC::RTC_OK;
}

//______________________________________________________________________________
RTC::ReturnCode_t Merger::onExecute(RTC::UniqueId ec_id)
{
  daq_do();

  return RTC::RTC_OK;
}

//______________________________________________________________________________
int Merger::daq_dummy()
{
  return 0;
}

//______________________________________________________________________________
int Merger::daq_configure()
{
  std::cerr << "*** Merger::configure" << std::endl;
  std::cout << "# of InPorts :" << kNInPort << std::endl;

  ::NVList* paramList;
  paramList = m_daq_service0.getCompParams();
  parse_params(paramList);

  std::cerr << "***********************************" << std::endl;
  std::cerr << "MERGER Initialized " << std::endl;
  std::cerr << "***********************************" << std::endl;

  return 0;
}

//______________________________________________________________________________
int Merger::parse_params(::NVList* list)
{
  std::cerr << "param list length:" << (*list).length() << std::endl;

  int len = (*list).length();
  for (int i = 0; i < len; i+=2) {
    std::string sname  = (std::string)(*list)[i].value;
    std::string svalue = (std::string)(*list)[i+1].value;

    std::cerr << "sname: " << sname << "  ";
    std::cerr << "value: " << svalue << std::endl;

    if (sname == "debug") {
      if (svalue=="yes" || svalue=="YES" || svalue=="y" || svalue=="Y" || svalue=="1" ||
	  svalue=="true" || svalue=="TRUE") m_debug=true;
      else m_debug=false;
    }
  }

  return 0;
}

//______________________________________________________________________________
int Merger::daq_unconfigure()
{
  std::cerr << "*** Merger::unconfigure" << std::endl;

  return 0;
}

//______________________________________________________________________________
int Merger::daq_start()
{
  std::cerr << "*** Merger::start" << std::endl;

  reset_sequence_num();
  reset_total_data_size();
  for (auto& e: m_in_status) e = BUF_SUCCESS;
  m_out_status = BUF_SUCCESS;

  for (auto& e: m_recv_byte_size) e = 0;
  m_idx = 0;

  return 0;
}

//______________________________________________________________________________
int Merger::daq_stop()
{
  std::cerr << "*** Merger::stop" << std::endl;
  reset_InPort();
  return 0;
}

//______________________________________________________________________________
int Merger::daq_pause()
{
  std::cerr << "*** Merger::pause" << std::endl;

  return 0;
}

//______________________________________________________________________________
int Merger::daq_resume()
{
  std::cerr << "*** Merger::resume" << std::endl;

  return 0;
}

//______________________________________________________________________________
unsigned int Merger::read_InPort() 
{
  //std::cout << " idx = " << m_idx << std::endl;
  if (check_trans_lock()) set_trans_unlock();
  // check read status 
  auto& inPort = *(m_InPort[m_idx]);

  bool status = inPort.read();
  if (status == false) { // timeout or fatal error
    m_in_status[m_idx] = check_inPort_status(inPort);
    if (m_in_status[m_idx] == BUF_TIMEOUT) {
      if (check_trans_lock()) set_trans_unlock();
    }
    else if (m_in_status[m_idx] == BUF_FATAL) fatal_error_report(INPORT_ERROR);
  }
  else {
    m_recv_byte_size[m_idx] = m_in_data[m_idx].data.length();
    m_in_status[m_idx]      = BUF_SUCCESS;
    
    // for (auto i=0; i<m_recv_byte_size[m_idx]; ++i) {
    //   std::cout << std::hex << (int)m_in_data[m_idx].data[i] << " ";
    // }
    // std::cout << std::endl;
  }


  if (m_debug) {
    std::cerr << " InPort[" << m_idx << "]: data.length() " << m_recv_byte_size[m_idx]
              << std::endl;
  }    

  return m_recv_byte_size[m_idx];
}

//______________________________________________________________________________
void Merger::reset_InPort()
{
  for (auto& p: m_InPort) {
    while (p->read()); // empty loop
  }
}

//______________________________________________________________________________
int Merger::set_data()
{
  //Merger'sheader size
  auto event_size = kHeaderSize;
  //event size = datasize - (DAQMW's header and footer size)
  for (auto e: m_recv_byte_size) event_size += get_event_size(e);
  
  uint8_t header[8];
  uint8_t footer[8];
  set_header(&header[0], event_size);
  set_footer(&footer[0]);
  
  m_out_data.data.length(event_size + HEADER_BYTE_SIZE + FOOTER_BYTE_SIZE);
  memcpy(&(m_out_data.data[0]), &header[0], HEADER_BYTE_SIZE);
  auto wp = HEADER_BYTE_SIZE;
  
  //# of words (uint32_t unit) is stored in the data
  m_header.fSize = event_size/sizeof(uint32_t);
  memcpy(&(m_out_data.data[wp]), reinterpret_cast<uint8_t*>(&m_header), kHeaderSize);
  wp += kHeaderSize;

  for (auto i=0; i<kNInPort; ++i) {
    auto fragment_size = get_event_size(m_recv_byte_size[i]);
    // std::cout << "src = " << i << std::endl;
    // std::for_each(reinterpret_cast<uint32_t*>(&(m_in_data[i].data[HEADER_BYTE_SIZE])), 
    // 		  reinterpret_cast<uint32_t*>(&(m_in_data[i].data[HEADER_BYTE_SIZE+fragment_size])), 
    // 		  hddaq::HexDump());
    memcpy(&(m_out_data.data[wp]), &(m_in_data[i].data[HEADER_BYTE_SIZE]), fragment_size);
    wp += fragment_size;
  }
  memcpy(&(m_out_data.data[wp]), &footer[0], FOOTER_BYTE_SIZE);

  return 0;
}

//______________________________________________________________________________
int Merger::write_OutPort()
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
//asano memo:
//daq_run() reads one InPort and the InPort is specified by m_idx, which is incremented if read_InPort is done successfully.
//This means that daq_run() functions runs N times (N=NInPort) in every one event.
int Merger::daq_run()
{
  if (m_debug) {
    std::cerr << "*** Merger::run" << std::endl;
  }

  if (check_trans_lock()) {  // check if stop command has come
    set_trans_unlock();    // transit to CONFIGURED state
    return 0;
  }

  bool in_status = (m_idx == kNInPort);
  if (!in_status && (m_out_status != BUF_TIMEOUT)) {   // previous OutPort.write() successfully done
  //if (m_out_status == BUF_SUCCESS) {   // previous OutPort.write() successfully done
    if (m_recv_byte_size[m_idx]==0) m_recv_byte_size[m_idx] = read_InPort();

    if (m_recv_byte_size[m_idx]>0) { // not timeout
      //std::cout << "check header footer" << std::endl;
      check_header_footer(m_in_data[m_idx], m_recv_byte_size[m_idx]);
    }
    if (m_in_status[m_idx] != BUF_TIMEOUT) {
      //std::cout << " increment idx " << std::endl;
      if (m_idx<kNInPort) ++m_idx;
    }
  }
  

  if (in_status) set_data();
  else return 0;

  if (write_OutPort() < 0) { //timeout
    if(m_debug){
      std::cerr << "write Outport timeout !" << std::endl;
    }
    return 0;
  } else {
    if(m_debug){
      std::cerr << "write Outport done. !" << std::endl;
    }
    //std::cout << "end of event " << std::endl;
    // OutPort write successfully done
    inc_sequence_num();                     // increase sequence num.

    auto sum = 0;
    for (auto v: m_recv_byte_size) sum += v;
    auto event_data_size = get_event_size(sum);
    inc_total_data_size(event_data_size);
    for (auto& v: m_recv_byte_size) v = 0;
    m_idx = 0;
  }

  return 0;
}

//______________________________________________________________________________
extern "C"
{
  void MergerInit(RTC::Manager* manager)
  {
    RTC::Properties profile(merger_spec);
    manager->registerFactory(profile,
                             RTC::Create<Merger>,
                             RTC::Delete<Merger>);
  }
};

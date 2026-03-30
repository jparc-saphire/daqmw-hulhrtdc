// -*- C++ -*-
/*!
 * @file BestEffortDispatcher.h
 * @brief This class works as merger, event builder and dispatcher
 * @date
 * @author Hidemitsu Asano
 * @email hidemitsu.asano@riken.jp 
 *
 */

#include <string>

#include "BestEffortDispatcher.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::INPORT_ERROR;
using DAQMW::FatalType::OUTPORT_ERROR;
using DAQMW::FatalType::USER_DEFINED_ERROR1;

// Module specification
// Change following items to suit your component's spec.
static const char* besteffortdispatcher_spec[] =
  {
    "implementation_id", "BestEffortDispatcher",
    "type_name",         "BestEffortDispatcher",
    "description",       "Dispatcher component with best-effort OutPort",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    ""
  };

//______________________________________________________________________________
BestEffortDispatcher::BestEffortDispatcher(RTC::Manager* manager)
  : DAQMW::DaqComponentBase(manager),
    m_InPort("in", m_in_data),
    m_OutPort("out", m_out_data),//for DAQlogger, standard type
    m_BestEffort_OutPort("best_effort_out", m_best_effort_out_data)//for Monitor, best effort type
    // m_v1718(0),
    // m_Handle(0)
{
  // Registration: InPort/OutPort/Service

  registerInPort("in", m_InPort);
  registerOutPort("out", m_OutPort);
  registerOutPort("best_effort_out", m_BestEffort_OutPort);

  init_command_port();
  init_state_table();
  set_comp_name("BestEffortDispatcher");
}

//______________________________________________________________________________
BestEffortDispatcher::~BestEffortDispatcher()
{
}

//______________________________________________________________________________
RTC::ReturnCode_t BestEffortDispatcher::onInitialize()
{
  if (m_debug) {
    std::cerr << "BestEffortDispatcher::onInitialize()" << std::endl;
  }

  return RTC::RTC_OK;
}

//______________________________________________________________________________
RTC::ReturnCode_t BestEffortDispatcher::onExecute(RTC::UniqueId ec_id)
{
  daq_do();

  return RTC::RTC_OK;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_dummy()
{
  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_configure()
{
  std::cerr << "*** BestEffortDispatcher::configure" << std::endl;
  
  reset_InPort();
  ::NVList* paramList;
  paramList = m_daq_service0.getCompParams();
  parse_params(paramList);
  // Create VME anad //
  //base address of v1718 is hard-coded here. if you change base address via
  //rotary switch of v1718, modify it.
  //BdType , Link, base address (0x00), pointer for handling 
  // if(UseCAENv1718){
  //   if(!m_v1718){
  //     m_v1718 = new CAENv1718(cvV1718,0,0,&m_Handle);
  //     m_v1718->Initialize();
  //   }
  //   m_v1718->SystemReset();

  //   //clear output register 1
  //   m_v1718->SetBusy(0);
  //   //one pulse from ch.0
  //   m_v1718->OnePulse();
  // }
  //std::cerr << "CAEN v1718 is initialized" << std::endl;
  
  std::cout << "********************************" << std::endl;
  std::cout << "BestEffortDispatcher Initialized " << std::endl;
  std::cout << "********************************" << std::endl;

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::parse_params(::NVList* list)
{
  std::cerr << "param list length:" << (*list).length() << std::endl;

  int len = (*list).length();
  for (int i = 0; i < len; i+=2) {
    std::string sname  = (std::string)(*list)[i].value;
    std::string svalue = (std::string)(*list)[i+1].value;

    std::cerr << "sname: " << sname << "  ";
    std::cerr << "value: " << svalue << std::endl;
    
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (sname=="debug") {
      if (svalue=="true" || svalue=="TRUE" || 
	  svalue=="yes" || svalue=="YES" || svalue=="y" || svalue=="Y" || svalue=="1") m_debug=true;
      else m_debug = false;
    }
    // if(sname=="UseCAENv1718"){
    //   if(svalue=="false"){ 
    //     UseCAENv1718=false;
    //     std::cout <<  "L." << __LINE__ << "  CAENv1718 is not to be used " << std::endl;
    //   }
    // }
  }

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_unconfigure()
{
  std::cerr << "*** BestEffortDispatcher::unconfigure" << std::endl;
  
  //m_v1718->OnePulse();
  //m_v1718->SystemReset();
  //delete m_v1718;
  //m_v1718 = 0;
  //m_v1718->SetBusy(0);
  //m_v1718->SetBusy2(1);

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_start()
{
  std::cerr << "*** BestEffortDispatcher::start" << std::endl;
  reset_sequence_num();
  reset_total_data_size();
  m_in_status  = BUF_SUCCESS;
  m_out_status = BUF_SUCCESS;
  m_besteffort_out_status = BUF_SUCCESS;

  //set output register ch 1
  // if(UseCAENv1718)m_v1718->SetBusy(1);
  // std::cerr << "CAEN v1718 clear latch from output 1" << std::endl;
  //m_v1718->SetBusy2(0);
  //clear trigger veto
  //m_v1718->OnePulse2();
  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_stop()
{
  //generate latch veto from output 1
  //m_v1718->SetBusy(1);
  //stop spill gate/
  // if(UseCAENv1718 && m_v1718){
  //   m_v1718->SetBusy(0);//clear output reg. ch.1
  //   std::cerr << "CAEN v1718 set latch from output 1" << std::endl;
  //   m_v1718->OnePulse();//one pulse from ch.0
  // }
  std::cerr << "*** BestEffortDispatcher::stop" << std::endl;
  reset_InPort();
  std::cerr << "*** reset InPorts.. " << std::endl;
  std::cout << "BestEffortDispather::total sequence # " << get_sequence_num() << std::endl;

  //m_v1718->SetBusy(0);
  //m_v1718->SetBusy2(1);
  std::cout << "Please Wait....(1 minute) " << std::endl;
  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_pause()
{
  std::cerr << "*** BestEffortDispatcher::pause" << std::endl;

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_resume()
{
  std::cerr << "*** BestEffortDispatcher::resume" << std::endl;

  return 0;
}

//______________________________________________________________________________
int BestEffortDispatcher::read_data_from_detectors()
{
  int received_data_size = 0;
  /// write your logic here
  return received_data_size;
}

//______________________________________________________________________________
int BestEffortDispatcher::set_data_OutPort(unsigned int data_byte_size)
{
  ///set OutPort buffer length
  //  std::cerr << "#D set_data_OutPort()" << std::endl;
  
  m_out_data.data.length(data_byte_size);
  memcpy(&(m_out_data.data[0]), &m_in_data.data[0], data_byte_size);
  return 0;
  
}

//______________________________________________________________________________
int BestEffortDispatcher::set_data_BestEffort_OutPort(unsigned int data_byte_size)
{
  ///set OutPort buffer length
  //  std::cerr << "#D set_data_BestEffort_OutPort()" << std::endl;

  // modify footer
  // unsigned char footer[8];
  // set_footer(&footer[8]);

  //  std::cout << "#D footer " << " " << get_sequence_num() << " " << m_loop_out1 << std::endl;
  // footer[4] = (m_loop_out1 & 0xff000000) >> 24;
  // footer[5] = (m_loop_out1 & 0x00ff0000) >> 16;
  // footer[6] = (m_loop_out1 & 0x0000ff00) >>  8;
  // footer[7] = (m_loop_out1 & 0x000000ff);

  m_best_effort_out_data.data.length(data_byte_size);
  // memcpy(&(m_out1_data.data[0]), &(m_in_data.data[0]), data_byte_size-FOOTER_BYTE_SIZE);
  // memcpy(&(m_out1_data.data[data_byte_size-FOOTER_BYTE_SIZE]), &footer[0], FOOTER_BYTE_SIZE);
  memcpy(&(m_best_effort_out_data.data[0]), &(m_in_data.data[0]), data_byte_size);

  return 0;
}

//______________________________________________________________________________
unsigned int BestEffortDispatcher::read_InPort()
{
  if (check_trans_lock()) set_trans_unlock();
  /////////////// read data from InPort Buffer ///////////////
  unsigned int recv_byte_size = 0;
  bool ret = m_InPort.read();

  //////////////////// check read status /////////////////////
  if (ret == false) { // false: TIMEOUT or FATAL
    m_in_status = check_inPort_status(m_InPort);
    if (m_in_status == BUF_TIMEOUT) { // Buffer empty.
      m_in_tout_counts++;
      if (check_trans_lock()) {     // Check if stop command has come.
        set_trans_unlock();       // Transit to CONFIGURE state.
      }
    }
    else if (m_in_status == BUF_FATAL) { // Fatal error
      fatal_error_report(INPORT_ERROR);
    }
  }
  else {
    m_in_tout_counts = 0;
    recv_byte_size = m_in_data.data.length();
    m_in_status = BUF_SUCCESS;
  }
  if (m_debug) {
    std::cerr << "m_in_data.data.length():" << recv_byte_size
              << std::endl;
  }

  return recv_byte_size;
}

//______________________________________________________________________________
int BestEffortDispatcher::reset_InPort()
{
  //asano memo, why empty loop
  while (m_InPort.read()) ; // empty loop 

  return 0;
}



//______________________________________________________________________________
//send data  to outport 0 (=logger component)
int BestEffortDispatcher::write_OutPort()
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
      m_out_tout_counts++;
      return -1;
    }
  }
  m_out_tout_counts = 0;
  return 0; // successfully done
}


//______________________________________________________________________________
//out port for monitor with best effort type
int BestEffortDispatcher::write_BestEffort_OutPort()
{
  ////////////////// send data from OutPort  //////////////////
  bool ret = m_BestEffort_OutPort.write();

  //////////////////// check write status /////////////////////
  if (ret == false) {  // TIMEOUT or FATAL
    m_besteffort_out_status  = check_outPort_status(m_BestEffort_OutPort);
    if (m_besteffort_out_status == BUF_FATAL) {   // Fatal error
      fatal_error_report(OUTPORT_ERROR);
    }
    if (m_besteffort_out_status == BUF_TIMEOUT) { // Timeout
      m_besteffort_out_tout_counts++;
      return -1;
    }
  }
  m_besteffort_out_tout_counts = 0;
  return 0; // successfully done
}

//______________________________________________________________________________
int BestEffortDispatcher::daq_run()
{
  if (m_debug) {
    std::cerr << "*** BestEffortDispatcher::run" << std::endl;
  }

  // std::cerr << "out1_tout_counts:" << m_out1_tout_counts << "  bestEff_tout_counts:" << m_out1_tout_counts << std::endl;

  //if ((m_out_status != BUF_TIMEOUT) && (m_besteffort_out_status != BUF_TIMEOUT)) {
  if (m_out_status != BUF_TIMEOUT) {
    m_inport_recv_data_size = read_InPort();

    if (m_inport_recv_data_size == 0) { // TIMEOUT
      return 0;
    }
    else {
      check_header_footer(m_in_data, m_inport_recv_data_size);
      set_data_OutPort(m_inport_recv_data_size);
      set_data_BestEffort_OutPort(m_inport_recv_data_size);
    }
  }

  if (m_in_status != BUF_TIMEOUT) {
    if (write_OutPort() < 0) { // TIMEOUT
      ; // do nothing
    }
    else {
      m_out_status = BUF_SUCCESS;
    }
  }

  if ((m_in_status != BUF_TIMEOUT) && (m_out_status != BUF_TIMEOUT)) {
    if (write_BestEffort_OutPort() < 0) { // TIMEOUT
      ; // do nothing
    }
    else {
      m_besteffort_out_status = BUF_SUCCESS;
    }
  }

  if ((m_in_status  != BUF_TIMEOUT) &&
      (m_out_status != BUF_TIMEOUT)) {

      
    inc_sequence_num();                    // increase sequence num.
    unsigned int event_data_size = get_event_size(m_inport_recv_data_size);
    inc_total_data_size(event_data_size);  // increase total data byte size

    // std::cout << get_sequence_num() << " events processed. " << std::endl;
    int seqnum = get_sequence_num();
    if ((seqnum%1000 == 0) && m_debug)
      std::cout << seqnum << " events processed. " << std::endl;
  }

  return 0;

}



//______________________________________________________________________________
extern "C"
{
  void BestEffortDispatcherInit(RTC::Manager* manager)
  {
    RTC::Properties profile(besteffortdispatcher_spec);
    manager->registerFactory(profile,
                             RTC::Create<BestEffortDispatcher>,
                             RTC::Delete<BestEffortDispatcher>);
  }
};

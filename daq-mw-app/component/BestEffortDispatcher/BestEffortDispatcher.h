// -*- C++ -*-
/*!
 * @file BestEffortDispatcher.h
 * @brief
 * @date Oct.5th, 2017
 * @author Hidemitsu Asano
 * @email hidemitsu.asnao@riken.jp
 *
 */

#ifndef BESTEFFORTDISPATCHER_H
#define BESTEFFORTDISPATCHER_H

#include "DaqComponentBase.h"
// #include "CAENVMEtypes.h"
// #include "CAENv1718.h"
// #include "CAENModule.h"

using namespace RTC;

class BestEffortDispatcher
  : public DAQMW::DaqComponentBase
{
public:
  BestEffortDispatcher(RTC::Manager* manager);
  ~BestEffortDispatcher();
 
  // The initialize action (on CREATED->ALIVE transition)
  // former rtc_init_entry()
  virtual RTC::ReturnCode_t onInitialize();

  // The execution action that is invoked periodically
  // former rtc_active_do()
  virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);
  
private:
  //inport from merger
  TimedOctetSeq          m_in_data;
  InPort<TimedOctetSeq>  m_InPort;

  //outport for logger
  TimedOctetSeq          m_out_data;
  OutPort<TimedOctetSeq> m_OutPort;
  
  //outport for online monitor
  TimedOctetSeq          m_best_effort_out_data;
  OutPort<TimedOctetSeq> m_BestEffort_OutPort;
   

private:
  int daq_dummy();
  int daq_configure();
  int daq_unconfigure();
  int daq_start();
  int daq_run();
  int daq_stop();
  int daq_pause();
  int daq_resume();

  int parse_params(::NVList* list);
  int read_data_from_detectors();
  //set data for logger 
  int set_data_OutPort(unsigned int data_byte_size);
  //set data for Monitor
  int set_data_BestEffort_OutPort(unsigned int data_byte_size);
  int reset_InPort();
  unsigned int read_InPort();
  int write_OutPort();//DAQLogger 
  int write_BestEffort_OutPort();//Monitor

  BufferStatus m_in_status{BUF_SUCCESS};
  BufferStatus m_out_status{BUF_SUCCESS};
  BufferStatus m_besteffort_out_status{BUF_SUCCESS};

  
  unsigned int m_in_tout_counts{0};
  unsigned int m_out_tout_counts{0};
  unsigned int m_besteffort_out_tout_counts{0};

  unsigned int m_inport_recv_data_size{0};
  bool m_debug{false};
  

  // //CAEN bus controller
  // CAENv1718* m_v1718;
  // int32_t m_Handle;
  // bool UseCAENv1718{true};
  
  int m_Verbosity{0};
  
};


extern "C"
{
  void BestEffortDispatcherInit(RTC::Manager* manager);
};

#endif // BESTEFFORTDISPATCHER_H

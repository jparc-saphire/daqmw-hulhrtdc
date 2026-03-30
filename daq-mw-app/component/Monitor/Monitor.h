// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef MONITOR_H
#define MONITOR_H

#include <string>
#include <vector>

#include "DaqComponentBase.h"

////////// ROOT Include files //////////
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TFile.h>
#include <TApplication.h>

using namespace RTC;


class THttpServer;

class Monitor
  : public DAQMW::DaqComponentBase
{
public:
  Monitor(RTC::Manager* manager);
  ~Monitor();

  // The initialize action (on CREATED->ALIVE transition)
  // former rtc_init_entry()
  virtual RTC::ReturnCode_t onInitialize();

  // The execution action that is invoked periodically
  // former rtc_active_do()
  virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
  TimedOctetSeq          m_in_data;
  InPort<TimedOctetSeq>  m_InPort;

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
  int reset_InPort();

  unsigned int read_InPort();
  bool FillHistos();

  int EventNum_;

  void parse_file(const std::string& fileName="");
  void update();

  BufferStatus m_in_status{BUF_SUCCESS};
  int m_Verbosity{0};

  //web base monitoring
  std::string m_serverName{"http:8888?thrds=4"};
  THttpServer* m_server{nullptr};
  unsigned int m_update_cycle{10};
  unsigned int m_canupdate_cycle{10};
};


extern "C"
{
  void MonitorInit(RTC::Manager* manager);
};

#endif // MONITOR_H

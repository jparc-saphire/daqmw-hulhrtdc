// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef MERGER_H
#define MERGER_H

#include <cstdint>
#include <vector>
#include <memory>

#include "DaqComponentBase.h"

using namespace RTC;


struct merger_header_t {
  uint32_t fMagic;
  uint32_t fSize;
};

class Merger
  : public DAQMW::DaqComponentBase
{
public:
  static constexpr uint32_t kMagic      = 0x544e5645; // 'TNVE' ('EVNT')
  static constexpr uint32_t kHeaderSize = sizeof(merger_header_t);

public:
  Merger(RTC::Manager* manager);
  Merger(const Merger&)             = delete;
  Merger& operator=(const Merger&)  = delete;
  Merger(Merger&&)                  = delete;
  Merger& operator=(Merger&&)       = delete;
  virtual ~Merger()                 = default;

  // The initialize action (on CREATED->ALIVE transition)
  // former rtc_init_entry()
  virtual RTC::ReturnCode_t onInitialize();

  // The execution action that is invoked periodically
  // former rtc_active_do()
  virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
  std::vector<TimedOctetSeq>         m_in_data;
  std::vector<InPort<TimedOctetSeq>*> m_InPort;

  TimedOctetSeq          m_out_data;
  OutPort<TimedOctetSeq> m_OutPort;

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
  void reset_InPort();
  unsigned int read_InPort();
  int set_data();
  int write_OutPort();

  std::vector<unsigned int> m_recv_byte_size;
  int m_idx{0};
  merger_header_t m_header;

  std::vector<BufferStatus> m_in_status;
  BufferStatus m_out_status{BUF_SUCCESS};
  bool m_debug{false};
};


extern "C"
{
  void MergerInit(RTC::Manager* manager);
};

#endif // MERGER_H

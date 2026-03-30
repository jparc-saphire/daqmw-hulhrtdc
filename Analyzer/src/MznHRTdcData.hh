
namespace MZN_HRTDC{
  static const int MaxHit = 64*16;
  static const int NofCh = 64;

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

  // 0-31ch (Slot-U), 32-63ch (Slot-D)
  enum slot{i_up, i_down, sizeSlot};
  
  //event based structure 
  struct Data_t {
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
    
    Data_t(){
      for (auto& v: StopOut)  v = -0xffff;
      for (auto& v: Through)  v = -0xffff;
      for (auto& v: OverFlow) v = -0xffff;

      StopFineCount   = -0xffff;
      StopEstimator   = -0xffff;
      StopCoarseCount = -0xffff;
      common_stop     = -0xffff;

      Nhit   = 0;
      for (auto& v: NhitCh)       v = 0;
      for (auto& v: Ch)           v = -0xffff;
      for (auto& v: FineCount)    v = -0xffff;
      for (auto& v: Estimator)    v = -0xffff;
      for (auto& v: CoarseCount)  v = -0xffff;
      for (auto& v: tdc_leading)  v = -0xffff;

      tNhit   = 0;
      for (auto& v: tNhitCh)       v = 0;
      for (auto& v: tCh)           v = -0xffff;
      for (auto& v: tFineCount)    v = -0xffff;
      for (auto& v: tEstimator)    v = -0xffff;
      for (auto& v: tCoarseCount)  v = -0xffff;
      for (auto& v: tdc_trailing)  v = -0xffff;

      for (auto& v: raw_tdc)       v = -0xffff;
    }
    void clear(){
      for (auto& v: StopOut)  v = -0xffff;
      for (auto& v: Through)  v = -0xffff;
      for (auto& v: OverFlow) v = -0xffff;

      StopFineCount   = -0xffff;
      StopEstimator   = -0xffff;
      StopCoarseCount = -0xffff;
      common_stop     = -0xffff;

      Nhit   = 0;
      for (auto& v: NhitCh)       v = 0;
      for (auto& v: Ch)           v = -0xffff;
      for (auto& v: FineCount)    v = -0xffff;
      for (auto& v: Estimator)    v = -0xffff;
      for (auto& v: CoarseCount)  v = -0xffff;
      for (auto& v: tdc_leading)  v = -0xffff;

      tNhit   = 0;
      for (auto& v: tNhitCh)       v = 0;
      for (auto& v: tCh)           v = -0xffff;
      for (auto& v: tFineCount)    v = -0xffff;
      for (auto& v: tEstimator)    v = -0xffff;
      for (auto& v: tCoarseCount)  v = -0xffff;
      for (auto& v: tdc_trailing)  v = -0xffff;

      for (auto& v: raw_tdc)       v = -0xffff;
    }
  };

}

#include "daq_funcs.hh"

#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "BctBusBridgeFunc.hh"
#include "RegisterMap.hh"

using namespace HUL;
using namespace HUL::HRTDC_BASE;

void SetTdcWindow(uint32_t wmax, uint32_t wmin, HUL::FPGAModule& fmodule,
                  uint32_t base_addr)
{
  static const uint32_t kCounterMax = 2047;
  static const uint32_t kPtrDiffWr  = 2;

  uint32_t ptr_ofs = kCounterMax - wmax + kPtrDiffWr;

  WriteModuleIn2ndryFPGA(fmodule, base_addr, HUL::HRTDC_MZN::TDC::kAddrPtrOfs,
                         ptr_ofs, 2);
  WriteModuleIn2ndryFPGA(fmodule, base_addr, HUL::HRTDC_MZN::TDC::kAddrWinMax,
                         wmax, 2);
  WriteModuleIn2ndryFPGA(fmodule, base_addr, HUL::HRTDC_MZN::TDC::kAddrWinMin,
                         wmin, 2);
}

void DdrInitialize(HUL::FPGAModule& fmodule, bool enUp, bool enDown)
{
  uint32_t enable_up   = enUp ? DCT::kRegEnableU : 0;
  uint32_t enable_down = enDown ? DCT::kRegEnableD : 0;

  std::cout << "#D : Do DDR initialize" << std::endl;

  if (enUp) {
    WriteModuleIn2ndryFPGA(fmodule, BBP::kUpper,
                           HUL::HRTDC_MZN::DCT::kAddrTestMode, 1, 1);
  }

  if (enDown) {
    WriteModuleIn2ndryFPGA(fmodule, BBP::kLower,
                           HUL::HRTDC_MZN::DCT::kAddrTestMode, 1, 1);
  }

  uint32_t reg =
      enable_up | enable_down | DCT::kRegTestModeU | DCT::kRegTestModeD;

  fmodule.WriteModule(DCT::kAddrCtrlReg, reg);
  fmodule.WriteModule(DCT::kAddrInitDDR, 0);

  uint32_t ret = fmodule.ReadModule(DCT::kAddrRcvStatus, 1);

  if (enUp) {
    if (ret & DCT::kRegBitAlignedU) {
      std::cout << "#D : DDR initialize succeeded (MZN-U)" << std::endl;
    } else {
      std::cout << "#E : Failed (MZN-U)" << std::endl;
      std::exit(-1);
    }
  }

  if (enDown) {
    if (ret & DCT::kRegBitAlignedD) {
      std::cout << "#D : DDR initialize succeeded (MZN-D)" << std::endl;
    } else {
      std::cout << "#E : Failed (MZN-D)" << std::endl;
      std::exit(-1);
    }
  }

  if (enUp) {
    WriteModuleIn2ndryFPGA(fmodule, BBP::kUpper,
                           HUL::HRTDC_MZN::DCT::kAddrTestMode, 0, 1);
  }

  if (enDown) {
    WriteModuleIn2ndryFPGA(fmodule, BBP::kLower,
                           HUL::HRTDC_MZN::DCT::kAddrTestMode, 0, 1);
  }

  reg = enable_up | enable_down;
  fmodule.WriteModule(DCT::kAddrCtrlReg, reg);
}

void CalibLUT(HUL::FPGAModule& fmodule, uint32_t base_addr)
{
  WriteModuleIn2ndryFPGA(fmodule, base_addr, HUL::HRTDC_MZN::DCT::kAddrExtraPath,
                         1, 1);

  while (!(ReadModuleIn2ndryFPGA(fmodule, base_addr,
                                 HUL::HRTDC_MZN::TDC::kAddrStatus, 1) &
           HUL::HRTDC_MZN::TDC::kRegReadyLut)) {
    sleep(1);
    std::cout << "#D waiting LUT ready" << std::endl;
  }

  if ((int32_t)base_addr == (int32_t)BBP::kUpper) {
    std::cout << "#D LUT is ready! (MIF-U)" << std::endl;
  } else {
    std::cout << "#D LUT is ready! (MIF-D)" << std::endl;
  }

  WriteModuleIn2ndryFPGA(fmodule, base_addr, HUL::HRTDC_MZN::DCT::kAddrExtraPath,
                         0, 1);
  WriteModuleIn2ndryFPGA(fmodule, base_addr, HUL::HRTDC_MZN::TDC::kAddrReqSwitch,
                         1, 1);
}

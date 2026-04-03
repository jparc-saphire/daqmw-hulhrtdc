#ifndef DAQFUNCS_HH
#define DAQFUNCS_HH

#include <cstdint>

#include "FPGAModule.hh"

void SetTdcWindow(uint32_t wmax, uint32_t wmin, HUL::FPGAModule& fmodule,
                  uint32_t base_addr);
void DdrInitialize(HUL::FPGAModule& fmodule, bool enUp, bool enDown);
void CalibLUT(HUL::FPGAModule& fmodule, uint32_t base_addr);

#endif

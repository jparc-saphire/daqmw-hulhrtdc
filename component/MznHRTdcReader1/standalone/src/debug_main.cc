// Local smoke test: align with HULpackage HRTDC/bin/debug
#include <iostream>
#include <iomanip>
#include <ios>
#include <cstdio>

#include "RegisterMap.hh"
#include "FPGAModule.hh"
#include "UDPRBCP.hh"
#include "BctBusBridgeFunc.hh"
#include "rbcp.hh"

using namespace HUL;

int main(int argc, char* argv[])
{
  if (1 == argc) {
    std::cout << "Usage: debug [IP address]\n";
    return 0;
  }

  std::string board_ip = argv[1];

  RBCP::UDPRBCP udp_rbcp(board_ip, RBCP::gUdpPort, RBCP::UDPRBCP::kNoDisp);
  HUL::FPGAModule fpga_module(udp_rbcp);
  uint32_t bct_version = fpga_module.ReadModule(BCT::kAddrVersion, 4);
  uint32_t major_version = (bct_version >> 8) & 0xff;
  uint32_t minor_version = (bct_version) & 0xff;

  std::cout << "#D: HUL firmware" << std::endl;
  std::cout << std::hex;
  std::cout << std::setfill('0') << std::right << "FW ID      : 0x" << std::setw(4)
            << ((bct_version >> 16) & 0xffff) << std::endl;
  std::cout << std::setfill(' ') << std::dec;
  std::cout << "FW version : " << major_version << "." << minor_version << std::endl;

  std::cout << "\n#D: Mezzanine firmware" << std::endl;

  if (HRTDC_BASE::kEnSlotUp) {
    uint32_t mv = ReadModuleIn2ndryFPGA(fpga_module, HRTDC_BASE::BBP::kUpper,
                                        HRTDC_MZN::BCT::kAddrVersion, 4);
    major_version = (mv >> 8) & 0xff;
    minor_version = (mv) & 0xff;

    std::cout << std::hex;
    std::cout << std::setfill('0') << std::right << "FW ID (MZN-U)     : 0x"
              << std::setw(4) << ((mv >> 16) & 0xffff) << std::endl;
    std::cout << std::setfill(' ') << std::dec;
    std::cout << "FW version (MZN-U): " << major_version << "." << minor_version
              << std::endl;
  }

  if (HRTDC_BASE::kEnSlotDown) {
    uint32_t mv = ReadModuleIn2ndryFPGA(fpga_module, HRTDC_BASE::BBP::kLower,
                                        HRTDC_MZN::BCT::kAddrVersion, 4);
    major_version = (mv >> 8) & 0xff;
    minor_version = (mv) & 0xff;

    std::cout << std::hex;
    std::cout << std::setfill('0') << std::right << "FW ID (MZN-D)     : 0x"
              << std::setw(4) << ((mv >> 16) & 0xffff) << std::endl;
    std::cout << std::setfill(' ') << std::dec;
    std::cout << "FW version (MZN-D): " << major_version << "." << minor_version
              << std::endl;
  }

  return 0;
}

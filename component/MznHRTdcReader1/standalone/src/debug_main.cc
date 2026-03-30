#include <iostream>
#include <cstdio>

#include "RegisterMap.hh"
#include "network.hh"
#include "UDPRBCP.hh"
#include "CommandMan.hh"
#include "FPGAModule.hh"
#include "rbcp.h"
#include "errno.h"
#include "daq_funcs.hh"
#include "mif_func.hh"

#include "sitcp_controller.hh"

using namespace HRTDC_BASE;

void
ReadVersion(FPGAModule& fModule)
{
  std::cout << "Version (BASE) : 0x" << std::hex 
	    << fModule.ReadModule(HRTDC_BASE::BCT::mid,
				  HRTDC_BASE::BCT::laddr_Version, 4)
	    << std::endl;

  if(en_up){
    std::cout << "Version (MZN-U) : 0x" << std::hex 
	      << ReadMIFModule(fModule, MIFU::mid,
			       HRTDC_MZN::BCT::mid, HRTDC_MZN::BCT::laddr_Version, 4 )
	      << std::endl;
  }

  if(en_down){
    std::cout << "Version (MZN-D) : 0x" << std::hex 
	      << ReadMIFModule(fModule, MIFD::mid,
			       HRTDC_MZN::BCT::mid, HRTDC_MZN::BCT::laddr_Version, 4 )
	      << std::endl;
  }
}

int
main(int argc, char* argv[])
{
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "hul_main [IP address]" << std::endl;
    return 0;
  }// usage
  
  // body ------------------------------------------------------
  char* board_ip = argv[1];
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  FPGAModule fModule(board_ip, udp_port, &rbcpHeader, 0);
  // fModule.WriteModule(HRTDC_BASE::MIFU::mid, 
  // 		      HRTDC_BASE::MIF::laddr_frst, 
  // 		      0
  // 		      );
  // fModule.WriteModule(HRTDC_BASE::MIFD::mid, 
  // 		      HRTDC_BASE::MIF::laddr_frst, 
  // 		      0
  // 		      );

  //  Reset_SiTCP(board_ip, &rbcpHeader);
  
  ddr_initialize(fModule);
  ReadVersion(fModule);
  CalibLUT(fModule, HRTDC_BASE::MIFU::mid);
  CalibLUT(fModule, HRTDC_BASE::MIFD::mid);
  //  fModule.WriteModule(BCT::mid, BCT::laddr_Reset, 1);

  return 0;

}// main

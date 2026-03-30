#include <iostream>
#include <cstdio>

#include "RegisterMap.hh"
#include "network.hh"
#include "UDPRBCP.hh"
#include "CommandMan.hh"
#include "FPGAModule.hh"
#include "rbcp.h"
#include "daq_funcs.hh"
#include "mif_func.hh"

enum argIndex{i_bin, i_ip, i_runno, i_eventno};
using namespace HRTDC_BASE;

int main(int argc, char* argv[])
{
  if(1 == argc){
    std::cout << "Usage\n";
    std::cout << "hul_main [IP address] [run no.] [# of event]" << std::endl;
    return 0;
  }// usage
  
  // body ------------------------------------------------------
  char* board_ip = argv[i_ip];
  int runno      = atoi(argv[i_runno]);
  int eventno    = atoi(argv[i_eventno]);
  rbcp_header rbcpHeader;
  rbcpHeader.type = UDPRBCP::rbcp_ver_;
  rbcpHeader.id   = 0;

  FPGAModule fModule(board_ip, udp_port, &rbcpHeader);
  daq(board_ip, &rbcpHeader, runno, eventno);

  return 0;

}// main

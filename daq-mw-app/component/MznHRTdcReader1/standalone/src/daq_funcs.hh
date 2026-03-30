#ifndef DAQFUNCS_
#define DAQFUNCS_

#include"FPGAModule.hh"
#include"RegisterMap.hh"
#include"rbcp.h"
#include"network.hh"
#include"errno.h"

#include<iostream>
#include<cstdio>
#include<csignal>
#include<algorithm>
#include<functional>

void set_tdc_window(unsigned int wmax, unsigned int wmin, FPGAModule& fModule, unsigned int mif_id);

void ddr_initialize(FPGAModule& fModule, bool enUp=en_up, bool enDown=en_down);
void CalibLUT(FPGAModule& fModule, unsigned int mif_id);

void UserStop_FromCtrlC(int signal);
void daq(char* ip, rbcp_header *rbcpHeader, int runno, int eventno);
int  ConnectSocket(const char* ip);
int  Event_Cycle(int sock, unsigned int* buffer);

int  receive(int sock, char* data_buf, unsigned int length);

#endif

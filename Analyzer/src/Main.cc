/*
  Main.cc

  2019/08 K.Shirotori
*/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <string>
#include <vector>
#include <signal.h>

#include "ConfMan.hh"
#include "VEvent.hh"
#include "Decoder.hh"

#include <TROOT.h>
#include <TFile.h>

#include <Compression.h>

//if this is 0, all events will be analyzed.
unsigned int Nanalysis=0;

enum ArgName{
  exec, 
  conffile, 
  inputfile, 
  outputfile,
  nanalysis
};

bool find_opt(int argc, char* argv[], const std::string& item)
{
  bool ret=false;
  std::vector<std::string> v(argv, argv+argc);
  for (unsigned int i=0; i<v.size(); ++i) {
    if (v[i] == item) {
      ret = true;
      break;
    }
  }
  return ret;
}

const int MaxChar = 400;

void closeFile(int sig)
{
  if(gFile){
    gFile->Write();
    gFile->Close();
  }
}

TROOT theROOT("sample-ana", "test");

int main(int argc, char* argv[])
{
  if(argc<=3){
    std::cout <<" #D Usage: "
	      <<" [analyzer config file]"
	      <<" [data input file]"
	      <<" [output root file]"
	      <<" [number of analyzed events (optional)]"
	      << std::endl;
    return 0;
  } 

  std::vector<std::string> arg(argv, argv + argc);
  const std::string& confFile = arg[conffile];
  const std::string& dataFile = arg[inputfile];
  const std::string& rootFile = arg[outputfile];

  if(argc == 5){
    Nanalysis = atoi(argv[nanalysis]);
    std::cout << Nanalysis  << " events will be analyzed" << std::endl;
  }

  ConfMan* gconfManager =new ConfMan( confFile );

  signal(SIGINT,closeFile);

  TFile f(rootFile.c_str(), "RECREATE", "", 
	  ROOT::CompressionSettings(ROOT::kLZMA, 1));
  std::cout << "#D recreate root file : "
  	    << rootFile << std::endl;
  
  if (!gconfManager->Initialize())
    return 0;
  
  Decoder& gDec = Decoder::getInstance();
  gDec.Open(dataFile.c_str());

  int evNum=0;
  while(gDec.getNextEvent()){
    /////////////////////////////////////////////////////////
    ////////Event processing    
    VEvent* event = gconfManager->EventAllocator();
    if ( event->ProcessingNormal( gDec ) ){
      delete event;
    }
    else break;

    ++evNum;
    if( evNum%1000 == 0 ){
      std::cout << "EventNum=" << evNum << std::endl;
    }
    if(Nanalysis == evNum) break;
  }

  gFile->Write();
  gFile->Close();

  std::cout << "# of analyzed events: " << std::dec << evNum << std::endl;

  return 0;
}


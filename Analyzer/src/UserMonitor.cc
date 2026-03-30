/*
  UserMonitor.cc

  2019/08 K.Shirotori 
*/

#include "VEvent.hh"

#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include <iomanip>
#include <vector>

#include "ConfMan.hh"
#include "Decoder.hh"
#include "RootHelper.hh"
#include "DetectorID.hh"
#include "RawData.hh"
#include "HodoRawHit.hh"

#include "TFile.h"
#include "TTree.h"

#define check1 0

 VEvent::VEvent()
 {
 }

 VEvent::~VEvent()
 {
 }

class EventMonitor : public VEvent
{
public:
  EventMonitor();
  ~EventMonitor();

  bool ProcessingBegin();
  bool ProcessingEnd();
  bool ProcessingNormal( Decoder& gDec );
  void InitializeEvent();

private:
  RawData *rawData;

};

EventMonitor::EventMonitor()
  : VEvent(),
    rawData(0)
{
}

EventMonitor::~EventMonitor()
{
  if (rawData) delete rawData;
}

struct Event{
  //HR-TDC
  int nh_ltdc[NumOfHRTDC], nh_ttdc[NumOfHRTDC];

  std::vector<std::vector<double> > ltdc;
  std::vector<std::vector<double> > ttdc;
  std::vector<std::vector<double> > width;
};
static Event event;

bool EventMonitor::ProcessingBegin()
{
 return true;
}

bool EventMonitor::ProcessingNormal( Decoder& gDec )
{
  const std::string funcname = "ProcessingNormal";

  rawData = new RawData;
  if( !rawData->DecodeRawHits( gDec ) ) return false;
  //std::cout << "***" << std::endl;

  ConfMan *confMan = ConfMan::GetConfManager();

  //**************************************************************************
  //******************RawData
  TTree *tree = static_cast<TTree *>(gFile->Get("tree"));
  InitializeEvent();
  
  //**************************************************************************
  //HR-TDC
  {
#if check1
    std::cout << "HR-TDC***********************" << std::endl;
#endif
    event.ltdc.resize(NumOfHRTDC);
    event.ttdc.resize(NumOfHRTDC);
    event.width.resize(NumOfHRTDC);

    const HodoRHitContainer &cont =rawData->GetT0RHC();
    int nh=cont.size();
    for( int i=0; i<nh; ++i ){
      HodoRawHit *hit=cont[i];

      int layerId = hit->LayerId()+1;
      int segId = hit->SegmentId()+1;

      //TDC leading
      int sizelTdc = hit->GetSize_lTdc();
      event.nh_ltdc[segId-1] = sizelTdc;
      for( int it=0; it<sizelTdc; ++it ){
	int ltdc = (hit -> GetlTdc(it));
	event.ltdc[segId-1].push_back(double(ltdc)*Tdc2Time);
      }
      //TDC trailing
      int sizetTdc = hit->GetSize_tTdc();
      event.nh_ttdc[segId-1] = sizetTdc;
      for( int it=0; it<sizetTdc; ++it ){
	int ttdc = (hit -> GettTdc(it));
	event.ttdc[segId-1].push_back(double(ttdc)*Tdc2Time);
      }

      //ToT(width)
      // int sizeWidth = hit->GetSize_Width();
      // for( int it=0; it<sizeWidth; ++it ){
      // 	int twidth = (hit -> GetWidth(it));
      // 	event.width[segId-1].push_back(double(twidth)*Tdc2Time);
      // }

#if check1
      std::cout << "Layer = " << layerId 
      		<< " Seg = " << segId 
      		<< std::endl;
#endif

    }
  }

  tree->Fill();

  return true;
}

void EventMonitor::InitializeEvent( void )
{
  ConfMan *confMan = ConfMan::GetConfManager();

  for(int it=0; it<NumOfHRTDC; it++){
    event.nh_ltdc[it] = -1;
    event.nh_ttdc[it] = -1;
  }

  for(int it=0; it<event.ltdc.size(); it++){
    event.ltdc[it].clear();
  }
  for(int it=0; it<event.ttdc.size(); it++){
    event.ttdc[it].clear();
  }
}

bool EventMonitor::ProcessingEnd()
{
  // gFile->Write();
  // gFile->Close();

  return true;
}

VEvent *ConfMan::EventAllocator()
{
  return new EventMonitor;
}

bool ConfMan:: InitializeHistograms()
{  
  ConfMan *confMan = ConfMan::GetConfManager();

  HBTree("tree","tree of Spec");
  TTree *tree = dynamic_cast<TTree *>(gFile->Get("tree"));
  
  tree->Branch("nh_ltdc",  event.nh_ltdc, "nh_ltdc[64]/I");
  tree->Branch("nh_ttdc",  event.nh_ttdc, "nh_ttdc[64]/I");
  
  tree->Branch("ltdc", &event.ltdc);
  tree->Branch("ttdc", &event.ttdc);
  tree->Branch("width", &event.width);
  
  return true;
}

/*
  RawData.hh

  2018/10  K.Shirotori
*/

#ifndef RawData_h
#define RawData_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "DetectorID.hh"
#include <vector>

class Decoder;

class HodoRawHit;

typedef std::vector<HodoRawHit*> HodoRHitContainer;

typedef std::vector <double> DoubleVec;
typedef std::vector <unsigned int> uIntVec;
typedef std::vector <int> IntVec;

class RawData
{

private:
  HodoRHitContainer T0RHC;

public:
  RawData();
  ~RawData();

  void clearAll();
  bool DecodeRawHits( Decoder& gDec );

private:
  RawData(const RawData&);
  RawData& operator=(const RawData&);

  bool AddHodoRHit( HodoRHitContainer& cont,
		    int DetId, int Layer, int Seg, int UorD,
		    uIntVec lTdc, uIntVec tTdc, uIntVec Width );

public:
  const HodoRHitContainer& GetT0RHC() const;
};

#endif


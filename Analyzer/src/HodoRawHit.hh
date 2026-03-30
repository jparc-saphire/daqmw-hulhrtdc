/*
  HodoRawHit.hh
  
  2019/08  K.Shirotori
*/

#ifndef HodoRawHit_h 
#define HodoRawHit_h

#include <cstddef>
#include <vector>

typedef std::vector <double> DoubleVec;
typedef std::vector <unsigned int> uIntVec;

class HodoRawHit
{
private:
  int DetId_, LayerId_, SegId_, UorD_;
  uIntVec lTdc_, tTdc_, Width_;
  
public:
  HodoRawHit( int detid, int layerid, int segid )
    : DetId_(detid), LayerId_(layerid), SegId_(segid),
      UorD_(0), 
      lTdc_(0.0), tTdc_(0.0), Width_(0.0)
  {};
  ~HodoRawHit() {};

public:
  int DetectorId( void ) const { return DetId_; };
  int LayerId( void ) const { return LayerId_; };
  int SegmentId( void ) const { return SegId_; };

  void SetUorD( int uord ) { UorD_=uord; }
  void SetlTdc( uIntVec ltdc ) { lTdc_=ltdc; }
  void SettTdc( uIntVec ttdc ) { tTdc_=ttdc; }
  void SetWidth( uIntVec width ) { Width_=width; }

  int GetUorD( void ) const { return UorD_; }
  int GetlTdc( int nh ) const { return lTdc_[nh]; };
  int GettTdc( int nh ) const { return tTdc_[nh]; };
  int GetWidth( int nh ) const { return Width_[nh]; };

  int  GetSize_lTdc( void ) const { return lTdc_.size(); };
  int  GetSize_tTdc( void ) const { return tTdc_.size(); };
  int  GetSize_Width( void ) const { return Width_.size(); };
};
#endif

/*
  CMapMan.cc

  2018/10 K.Shirotori
*/

#include "CMapMan.hh"

#include <string>
#include <cstdio>
#include <iostream>

CMapMan::CMapMan()
{}

CMapMan::CMapMan( const std::string & filename )
  : mapfilename_(filename)
{}

CMapMan::~CMapMan()
{
  NContainer.clear();
  RContainer.clear();
}

const unsigned int KeyMask   = 0x000F;
const unsigned int AaddrMask = 0x007F; /* 0-127 */
const unsigned int NaddrMask = 0x001F; /* 0-31 */
const unsigned int CaddrMask = 0x000F; /* 0-15 */
const int AaddrShift =  4;
const int NaddrShift = 12;
const int CaddrShift = 20;
const unsigned int KeyFlag = 0x0003;

const unsigned int SegMask = 0x01FF; /* 0-511 */
const unsigned int PlMask  = 0x01FF; /* 0-511 */  
const unsigned int DetMask = 0x003F; /* 0- 63 */
const unsigned int UDMask  = 0x0001;
const unsigned int ATMask  = 0x0001;
const int SegShift =  4;
const int PlShift  = 13;
const int DetShift = 22;
const int UDShift  = 29;
const int ATShift  = 30;
const unsigned int RKeyFlag = 0x0004;

inline unsigned int Key( int c, int n, int a )
{
  return ( ((c&CaddrMask)<<CaddrShift) | ((n&NaddrMask)<<NaddrShift) |
	   ((a&AaddrMask)<<AaddrShift) | KeyFlag );
}

inline unsigned int RKey( int det, int pl, int seg, int ud  )
{
  return ( ((det&DetMask)<<DetShift) | ((pl&PlMask)<<PlShift) |
	   ((seg&SegMask)<<SegShift) | ((ud&UDMask)<<UDShift) | RKeyFlag );
}

const int BufSize = 144;

bool CMapMan::Initialize( void )
{
  static const std::string funcname = "[CMapMan::Initialize]";

  int c, n, a, det, pl, seg, ud;
  unsigned int key, rkey;
  int nd;
  char buf[BufSize];
  FILE *fp;

  if((fp=fopen(mapfilename_.c_str(),"r"))==0){
    std::cerr << funcname << ": file open fail."
	      << std::endl;
    exit(-1);
  }

  NContainer.clear();
  RContainer.clear();

  while( fgets(buf,BufSize,fp)!=0 ){
    if(buf[0]!='#'){
      if((nd=sscanf(buf,"%d %d %d %d %d %d %d",
		    &c, &n, &a, &det, &pl, &seg, &ud ))>=5 ){
	if(nd==6) ud=0;
	else if(nd==5) { ud=0; }
	key=Key(c,n,a); rkey=RKey(det,pl,seg,ud);
	NContainer[key]=rkey; RContainer[rkey]=key;
      }
      else{
	std::cerr << funcname << ": Invalid data format" << std::endl;
	std::cerr << std::string(buf) << std::endl;
      }
    }
  }
  fclose(fp);
  std::cout << funcname << ": Initialization finished"
	    << std::endl;
  
  return true;
}


bool CMapMan::GetLogical( int Caddr, int Naddr, int Aaddr,
			  int &DetId, int &PlId, int &SegId,
			  int &UorD ) const
{
  static const std::string funcname = "[CMapMap::GetLogical]";
  unsigned int rkey=NContainer[Key(Caddr,Naddr,Aaddr)];
  if( (rkey&KeyMask)==RKeyFlag ){
    DetId = (rkey>>DetShift)&DetMask;
    PlId  = (rkey>>PlShift) &PlMask;
    SegId = (rkey>>SegShift)&SegMask;
    UorD  = (rkey>>UDShift) &UDMask;
    return true;
  }
  else{
#ifdef WarnOut
    std::cerr << funcname << ": No address found ... C="
	      << Caddr << " N=" << Naddr << " A=" << Aaddr << std::endl;
#endif
    return false;
  }
}

bool CMapMan::GetGeoAddr( int DetId, int PlId, int SegId, 
			  int UorD,
			  int &Caddr, int &Naddr, int &Aaddr ) const
{
  static const std::string funcname = "[CMapMap::GetGeoAddr]";
  unsigned int key=RContainer[RKey(DetId,PlId,SegId,UorD)];
  if( (key&KeyMask)==KeyFlag ){
    Caddr = (key>>CaddrShift)&CaddrMask;
    Naddr = (key>>NaddrShift)&NaddrMask;
    Aaddr = (key>>AaddrShift)&AaddrMask;
    return true;
  }
  else{
#ifdef WarnOut
    std::cerr << funcname << ": No address found ... DetId="
	      << DetId << " PlId=" << PlId << " SegId=" << SegId
	      << " UorD=" << UorD << std::endl;
#endif
    return false;
  }
}

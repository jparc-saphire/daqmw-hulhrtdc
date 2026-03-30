/*
  CMapMan.hh

  2018/10 K.Shirotori
*/

#ifndef CMapMan_h
#define CMapMan_h 1

#include <map>
#include <string>

class CMapMan
{
private:
  std::string mapfilename_;
public:
  CMapMan();
  CMapMan( const std::string & filename );
  ~CMapMan();
private:
  CMapMan( const CMapMan & );
  CMapMan & operator = ( const CMapMan & );

public:
  void SetFileName( const std::string & filename )
  { mapfilename_=filename; }
  bool Initialize( void );

private:
  mutable std::map <unsigned int, unsigned int> NContainer, RContainer;

public:
  bool GetLogical( int Caddr, int Naddr, int Aaddr,
		   int &DetId, int &PlId, int &SegId, int &UorD ) const;

  bool GetGeoAddr( int DetId, int PlId, int SegId, int UorD, 
		   int &Caddr, int &Naddr, int &Aaddr ) const;
};
 
#endif

/*
  ConfMan.hh

  2018/10 K.Shirotori
*/

#ifndef Confman_h
#define Confman_h 1

#include <string>

class VEvent;
class CMapMan;

class ConfMan
{

private:
  //Conf
  std::string ConfFileName_;
  static ConfMan *confManager_;

  //Counter map
  std::string CMapFileName_;
  CMapMan *CMapManager_;

public:
  ConfMan( const std::string & filename );
  ~ConfMan();

  static ConfMan *GetConfManager( void ) { return confManager_; }
  bool Initialize( void );
  void SetFileName( const std::string & filename ) { ConfFileName_=filename; }
  VEvent* EventAllocator();

  //Counter map
  CMapMan *GetCMapManager( void )   { return CMapManager_; }

private:
  ConfMan( const ConfMan & );
  ConfMan & operator = ( const ConfMan & );

public:
  bool InitializeParameterFiles();
  bool InitializeHistograms();
};

#endif

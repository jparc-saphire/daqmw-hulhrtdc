/*
  ConfMan.cc

  2019/08 K.Shirotori
*/

#include "ConfMan.hh"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>

#include "CMapMan.hh"

const std::string defCMapFile="CMap.param";

ConfMan * ConfMan::confManager_ = 0;

ConfMan::ConfMan( const std::string & filename )
  : ConfFileName_(filename),
    CMapManager_(0) 
{
  static const std::string funcname="[ConfMan::ConfMan]";
  if( confManager_ ){
    std::cerr << funcname << ": constring twice" << std::endl;
    std::exit(-1);
  }
  confManager_=this;
}

ConfMan::~ConfMan( )
{
  confManager_=0;
}

const int BufSize=300;

bool ConfMan::Initialize()
{
  static const std::string funcname="[ConfMan::Initialize]";

  char buf[BufSize], buf1[BufSize];
  double val1, val2, val3;
  int id;

  FILE *fp;
  if((fp=fopen(ConfFileName_.c_str(),"r"))==0){
    std::cerr << funcname << ": file open fail" << std::endl;
    exit(-1);
  }

  while( fgets( buf, BufSize, fp ) != 0 ){
    if( buf[0]!='#' ){ 
      //Counter map
      if( sscanf(buf,"CMAP: %s",buf1)==1 ){
	CMapFileName_=buf1;
      }
      else {
	std::cout << funcname << ": un-recognized record\n"
		  << buf << std::endl;
      }
    }
  }

  fclose(fp);

  std::cout << "------------------" << ConfFileName_ << "------" << std::endl;
  std::cout << "Counter Map:      " << CMapFileName_ << std::endl;
  std::cout << "-----------------------------------------------" << std::endl;

  InitializeParameterFiles();
  InitializeHistograms();

  return true;
}

bool ConfMan::InitializeParameterFiles( void )
{
  if( CMapFileName_!="" )
    CMapManager_ = new CMapMan(CMapFileName_);
  if(CMapManager_) CMapManager_->Initialize();

  return true;
}

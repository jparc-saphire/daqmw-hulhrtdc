#ifndef DECODERH_
#define DECODERH_

#include<fstream>
#include<bitset>
#include<vector>
#include <boost/iostreams/filtering_stream.hpp>
#include<TGraphErrors.h>
#include"MznHRTdcData.hh"

class Decoder{

public:
  bool Open(const char* name);

  bool getNextEvent();
  
  MZN_HRTDC::Data_t GetMznHRTdcData(){return sMZNData;}

  static Decoder& getInstance();

private:
  boost::iostreams::filtering_istream m_ifs;

  Decoder();
  ~Decoder();
  Decoder(const Decoder& obj);
  Decoder& operator=(const Decoder& obj);
  
  MZN_HRTDC::Data_t sMZNData;

};

// getInstance --------------------------------------------------------------
inline
Decoder&
Decoder::getInstance()
{
  static Decoder obj;
  return obj;
}

#endif

/*
  VEvent.hh

  2019/08  K.Shirotori
*/

#ifndef VEVENT_H
#define VEVENT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class Decoder;

class VEvent
{

public:
  VEvent();
  virtual ~VEvent() = 0;

  virtual bool ProcessingBegin() = 0;
  virtual bool ProcessingEnd() = 0;
  virtual bool ProcessingNormal( Decoder& gDec ) = 0;
};

#endif

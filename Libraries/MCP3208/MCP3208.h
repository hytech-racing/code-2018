/*
  MCP3208.h - Library for communicating with MCP3208 Analog to digital converter.
  Created by Uros Petrevski, Nodesign.net 2013
  Released into the public domain.
  
  ported from Python code originaly written by Adafruit learning system for rPI :
  http://learn.adafruit.com/send-raspberry-pi-data-to-cosm/python-script
*/

#ifndef MCP3208_h
#define MCP3208_h

#include "Arduino.h"

class MCP3208
{
  public:
    MCP3208(int clockpin, int mosipin, int misopin, int cspin);
    int readADC(int adcnum);
  private:
      int _clockpin, _mosipin, _misopin, _cspin;
};


#endif

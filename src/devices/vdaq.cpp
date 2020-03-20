#pragma once

#include "vdaq.h"


vDAQ::vDAQ(int16_t deviceNum, bool designLoadAccess) :
  cRdiDeviceInterface(deviceNum, designLoadAccess),
  acqEngine(this, 0x400400),
  clkCfg(this, 0x440000),
  msadc(this, 0x460400),
  dataFifo(this, 0x480000)
{
  for (int ctr = 0; ctr < 5; ctr++)
    pWavegen[ctr] = new vDAQ_WaveformGen(this, 0x600000 + 0x10000*ctr);
}


vDAQ::~vDAQ() {
  for (int ctr = 0; ctr < 5; ctr++)
    delete pWavegen[ctr];
}



uint64_t vDAQ::getSystemClockT() {
  cacheSystemClockT();
  return getSystemClockTReg();
}







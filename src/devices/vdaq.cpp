#pragma once

#include "vdaq.h"


vDAQ::vDAQ(int16_t deviceNum, bool designLoadAccess) :
  cRdiDeviceInterface(deviceNum, designLoadAccess),
  acqEngine(this, 0x400400),
  clkCfg(this, 0x440000),
  msadc(this, 0x460400),
  dataFifo(this, 0x480000)
{
  pWavegenIp.resize(5);
  for (int ctr = 0; ctr < 5; ctr++)
    pWavegenIp[ctr] = new ddi::WaveformGenIp(this, 0x600000 + 0x10000*ctr);
}


vDAQ::~vDAQ() {
  while (!pWavegenIp.empty()) {
    auto pIp = pWavegenIp.begin();
    delete *pIp;
    pWavegenIp.erase(pIp);
  }
}


double vDAQ::getSampleClockRate() {
  uint32_t ticksPerT = getSampleClkTicksPerMeasInterval();
  if (ticksPerT == 8388607)
    return NAN;
  else
    return 200e6 * (((double)(2 << 18)) / ((double)ticksPerT));
}


uint64_t vDAQ::getSystemClockT() {
  cacheSystemClockT();
  return getSystemClockTReg();
}


bool vDAQ::verifyMsadcData() {
  int16_t vals[4];
  bool success = true;
  msadc.setUsrTestPatternReq(1);

  for (int i = 0; i < 100; i++) {
    acqEngine.getRawChannelVals(vals);

    for (int j = 0; j < 4; j++)
      if (vals[j] != 24160) {
        success = false;
        goto FINALIZE;
      }
  }

FINALIZE:
  msadc.setUsrTestPatternReq(0);
  return success;
}


bool vDAQ::initMsadc() {
  bool success = false;
  int tries = 1;

  setAfeSelection(1);

  do {
    msadc.doReset();
    success = verifyMsadcData();
  } while (!success && (tries++ < 100));

  return success;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////                      Acq Engine                         ///////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void vDAQ_AcquisitionEngine::resetAcqPlan() {
  setAcqPlanNumSteps(0);
  m_acqPlanWriteIdx = 0;
}

void vDAQ_AcquisitionEngine::addAcqPlanStep(bool frameClockState, uint32_t numPeriods) {
  setAcqPlanNumSteps(getAcqPlanNumSteps() + 1);

  numPeriods >>= 1;

  if (numPeriods > 127) {
    writeAcqPlanStep(m_acqPlanWriteIdx++, true, frameClockState, numPeriods - (numPeriods >> 7));
    writeAcqPlanStep(m_acqPlanWriteIdx++, false, frameClockState, numPeriods >> 7);
  }
  else {
    writeAcqPlanStep(m_acqPlanWriteIdx++, true, frameClockState, numPeriods);
  }
  writeAcqPlanStep(m_acqPlanWriteIdx,true);
}

void vDAQ_AcquisitionEngine::writeAcqPlanStep(uint32_t idx, bool newEntry, bool frameClockState, uint32_t numPeriods) {
  if (newEntry)
    writeAcqPlanStepReg((idx << 9) + (1<<8) + (numPeriods & ((1<<7)-1)) + (((uint32_t)frameClockState)<<7));
  else
    writeAcqPlanStepReg((idx << 9) + (numPeriods & ((1<<8)-1)));
}

void vDAQ_AcquisitionEngine::writeMaskTableEntry(uint32_t index, uint32_t val) {
  writeMaskTableInt((val&((1<<12)-1)) + (index<<12));
}


void vDAQ_AcquisitionEngine::getRawChannelVals(int16_t *vals) {
  *((uint32_t*)(&vals[0])) = getAcqStatusRawChannelDataReg1();
  *((uint32_t*)(&vals[2])) = getAcqStatusRawChannelDataReg2();
}


void vDAQ_AcquisitionEngine::getRawChannelOffsets(int16_t *vals) {
  *((uint32_t*)(&vals[0])) = getAcqParamChannelOffsetsReg1();
  *((uint32_t*)(&vals[2])) = getAcqParamChannelOffsetsReg2();
}


void vDAQ_AcquisitionEngine::setRawChannelOffsets(int16_t *vals) {
  setAcqParamChannelOffsetsReg1(*((uint32_t*)(&vals[0])));
  setAcqParamChannelOffsetsReg2(*((uint32_t*)(&vals[2])));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////                     Clock Config                        ///////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void vDAQ_ClockCfg::issueDeviceCommand(uint8_t address, uint8_t sizeBits, bool read) {
  writeDeviceCommandReg(((uint32_t)address) + (((uint32_t)sizeBits) << 8) + (((uint32_t)read) << 16));
}


void vDAQ_ClockCfg::writeAllDeviceSettings() {
  issueDeviceCommand(2, 136, false);
  // a complete write of device settings takes about 15us. Ensure it is complete
  Sleep(1);
}


void vDAQ_ClockCfg::readAllDeviceSettings() {
  issueDeviceCommand(0, 160, true);
  // a complete read of device settings takes about 16.8us. Ensure it is complete
  Sleep(1);
}


bool vDAQ_ClockCfg::checkIpId() {
  return getIpId() == 0xBC1DC10C;
}


bool vDAQ_ClockCfg::setupMsadcSampleClock() {
  uint32_t dat;

  dat = getOutputBufWord2Reg();
  CLEAR_BIT(dat, 24); // unmute clock 0
  SET_BIT(dat, 25); // mute clock 1
  SET_BIT(dat, 26); // mute clock 2
  SET_BIT(dat, 27); // mute clock 3
  SET_BIT(dat, 28); // mute clock 4
  setOutputBufWord2Reg(dat);
  writeAllDeviceSettings();

  return lockPll();
}


bool vDAQ_ClockCfg::lockPll() {
  uint32_t dat;

  dat = getOutputBufWord1Reg();
  SET_BIT(dat, 7); // CPMID
  SET_BIT(dat, 5); // CPRST
  setOutputBufWord1Reg(dat);
  writeAllDeviceSettings();

  CLEAR_BIT(dat, 7); // CPMID
  CLEAR_BIT(dat, 5); // CPRST
  setOutputBufWord1Reg(dat);
  writeAllDeviceSettings();

  return checkPll();
}


bool vDAQ_ClockCfg::checkPll() {
  readAllDeviceSettings();
  return (getReadbackBufWord0Reg() & (1 << 26)) > 0;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////                         MSADC                           ///////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool vDAQ_Msadc::calInProgress() {
  return (getCalDataRaw() & 1<<20) > 0;
}


void vDAQ_Msadc::doReset() {
  setResetReg(1);
  setResetReg(0);

  int w = 1;
  do {
    Sleep(20);
  } while (calInProgress() && (w++ < 10));
}




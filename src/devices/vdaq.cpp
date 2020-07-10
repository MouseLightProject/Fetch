#pragma once

#include "vdaq.h"


vdaq::Device::Device(int16_t deviceNum, bool designLoadAccess) :
  rdi::Device(deviceNum, designLoadAccess),
  acqEngine(this, 0x400400),
  clkCfg(this, 0x440000),
  msadc(this, 0x460400)
{
  pWavegenIp.resize(5);
  for (int ctr = 0; ctr < 5; ctr++)
    pWavegenIp[ctr] = new ddi::WaveformGenIp(this, 0x600000 + 0x10000 * ctr);

  pChannelFifos.resize(4);
  for (int ctr = 0; ctr < 4; ctr++)
    pChannelFifos[ctr] = new rdi::Fifo(this, 0x470000 + 0x1000 * ctr);

  DWORD p = pChannelFifos[0]->getProtocolVersion();
  p = 2;
}


vdaq::Device::~Device() {
  auto pIp = pWavegenIp.begin();
  while (pIp != pWavegenIp.end())
    delete *pIp++;

  auto pFifo = pChannelFifos.begin();
  while (pFifo != pChannelFifos.end())
    delete *pFifo++;
}


double vdaq::Device::getSampleClockRate() {
  uint32_t ticksPerT = getSampleClkTicksPerMeasInterval();
  if (ticksPerT == 8388607)
    return NAN;
  else
    return 200e6 * (((double)(2 << 18)) / ((double)ticksPerT));
}


uint64_t vdaq::Device::getSystemClockT() {
  cacheSystemClockT();
  return getSystemClockTReg();
}


bool vdaq::Device::verifyMsadcData() {
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


bool vdaq::Device::initMsadc() {
  bool success = false;
  int tries = 1;

  setAfeSelection(1);

  do {
    msadc.doReset();
    success = verifyMsadcData();
  } while (!success && (tries++ < 100));

  return success;
}



int16_t vdaq::Device::getDioIndex(const char *channelName, bool mustBeOutput) {
  if (strlen(channelName) != 4) throw "Invalid channel name.";
  if (channelName[0] != 'D') throw "Invalid channel name.";
  if (channelName[2] != '.') throw "Invalid channel name.";

  char prtNm[2] = { channelName[1], 0 };
  int port = atoi(prtNm);

  prtNm[0] = channelName[3];
  int line = atoi(prtNm);

  if (mustBeOutput && (port == 1)) throw "Selected port does not support output.";
  if ((port < 0) || (port > 2)) throw "Invalid port selection.";
  if ((line < 0) || (line > 7)) throw "Invalid line selection.";

  return port * 8 + line;
}


int16_t vdaq::Device::getDioOutputIndex(const char *channelName) {
  return getDioIndex(channelName, true);
}


void vdaq::Device::setDioOuputLevel(const char *channelName, bool level) {
  int16_t dioIdx = getDioOutputIndex(channelName);
  setDioOuputLevel(dioIdx, level);
}


void vdaq::Device::setDioOuputLevel(int16_t channelId, bool level) {
  writeRegU32(0x400000 + 200 + 4 * channelId, ((int)level) + 1);
}


void vdaq::Device::setDioOuputSignal(const char *channelName, uint32_t signal) {
  int16_t dioIdx = getDioOutputIndex(channelName);
  setDioOuputSignal(dioIdx, signal);
}


void vdaq::Device::setDioOuputSignal(int16_t channelId, uint32_t signal) {
  writeRegU32(0x400000 + 200 + 4 * channelId, signal);
}


void  vdaq::Device::setDioOuputTristate(const char *channelName) {
  int16_t dioIdx = getDioOutputIndex(channelName);
}


void  vdaq::Device::setDioOuputTristate(int16_t channelId) {
  if (channelId > 7) throw "Selected digital line cannot be tristated.";
  writeRegU32(0x400000 + 200 + 4 * channelId, 0);
}


bool vdaq::Device::getDioInputLevel(const char *channelName) {
  return getDioInputLevel(getDioIndex(channelName));
}


bool vdaq::Device::getDioInputLevel(int16_t channelId) {
  return ((bool)(getDioInputVals() & (1 << channelId)));
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////                      Acq Engine                         ///////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void vdaq::AcquisitionEngine::resetAcqPlan() {
  setAcqPlanNumSteps(0);
  m_acqPlanWriteIdx = 0;
}

void vdaq::AcquisitionEngine::addAcqPlanStep(bool frameClockState, uint32_t numPeriods) {
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

void vdaq::AcquisitionEngine::writeAcqPlanStep(uint32_t idx, bool newEntry, bool frameClockState, uint32_t numPeriods) {
  if (newEntry)
    writeAcqPlanStepReg((idx << 9) + (1<<8) + (numPeriods & ((1<<7)-1)) + (((uint32_t)frameClockState)<<7));
  else
    writeAcqPlanStepReg((idx << 9) + (numPeriods & ((1<<8)-1)));
}

void vdaq::AcquisitionEngine::writeMaskTableEntry(uint32_t index, uint32_t val) {
  writeMaskTableInt((val&((1<<12)-1)) + (index<<12));
}


void vdaq::AcquisitionEngine::getRawChannelVals(int16_t *vals) {
  *((uint32_t*)(&vals[0])) = getAcqStatusRawChannelDataReg1();
  *((uint32_t*)(&vals[2])) = getAcqStatusRawChannelDataReg2();
}


void vdaq::AcquisitionEngine::getRawChannelOffsets(int16_t *vals) {
  *((uint32_t*)(&vals[0])) = getAcqParamChannelOffsetsReg1();
  *((uint32_t*)(&vals[2])) = getAcqParamChannelOffsetsReg2();
}


void vdaq::AcquisitionEngine::setRawChannelOffsets(int16_t *vals) {
  setAcqParamChannelOffsetsReg1(*((uint32_t*)(&vals[0])));
  setAcqParamChannelOffsetsReg2(*((uint32_t*)(&vals[2])));
}


int16_t vdaq::AcquisitionEngine::getAnalogResonantPhaseThreshold() {
  return (int16_t)getAnalogResonantPhaseThresholdReg();
}


void vdaq::AcquisitionEngine::setAnalogResonantPhaseThreshold(int16_t threshold) {
  setAnalogResonantPhaseThresholdReg(threshold);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////                     Clock Config                        ///////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void vdaq::ClockCfg::issueDeviceCommand(uint8_t address, uint8_t sizeBits, bool read) {
  writeDeviceCommandReg(((uint32_t)address) + (((uint32_t)sizeBits) << 8) + (((uint32_t)read) << 16));
}


void vdaq::ClockCfg::writeAllDeviceSettings() {
  issueDeviceCommand(2, 136, false);
  // a complete write of device settings takes about 15us. Ensure it is complete
  Sleep(1);
}


void vdaq::ClockCfg::readAllDeviceSettings() {
  issueDeviceCommand(0, 160, true);
  // a complete read of device settings takes about 16.8us. Ensure it is complete
  Sleep(1);
}


bool vdaq::ClockCfg::checkIpId() {
  return getIpId() == 0xBC1DC10C;
}


bool vdaq::ClockCfg::setupMsadcSampleClock() {
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


bool vdaq::ClockCfg::lockPll() {
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


bool vdaq::ClockCfg::checkPll() {
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


bool vdaq::Msadc::calInProgress() {
  return (getCalDataRaw() & 1<<20) > 0;
}


void vdaq::Msadc::doReset() {
  setResetReg(1);
  setResetReg(0);

  int w = 1;
  do {
    Sleep(20);
  } while (calInProgress() && (w++ < 10));
}




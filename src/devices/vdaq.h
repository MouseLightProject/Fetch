#pragma once

#include "rdiLib.h"



class vDAQ_AcquisitionEngine : public cRdiDeviceRegisterMap
{
public:
  vDAQ_AcquisitionEngine(cRdiDeviceRegisterMap *pParent, uint32_t baseAddr) : cRdiDeviceRegisterMap(pParent, baseAddr) {};

  void resetStateMachine();
  void enableStateMachine();

  void softTrigger();

  void resetAcqPlan();
  void addAcqPlanStep(bool frameClockState, uint32_t numPeriods);

  void writeMaskTableEntry(uint32_t index, uint32_t val);

  void getRawChannelVals(int16_t *vals);

  void getRawChannelOffsets(int16_t *vals);
  void setRawChannelOffsets(int16_t *vals);

  // AE Regs
  REG_U32_RW(MaskTableSize, 116);

  REG_U32_RW(AcqParamPeriodTriggerChIdx, 120);
  REG_U32_RW(AcqParamStartTriggerChIdx, 124);
  REG_U32_RW(AcqParamNextTriggerChIdx, 128);
  REG_U32_RW(AcqParamStopTriggerChIdx, 132);
  REG_U32_RW(AcqParamStartTriggerInvert, 292);
  REG_U32_RW(AcqParamNextTriggerInvert, 296);
  REG_U32_RW(AcqParamStopTriggerInvert, 300);
  REG_U32_RW(AcqParamPhotonChIdx, 136);
  REG_U32_RW(AcqParamPeriodTriggerDebounce, 140);
  REG_U32_RW(AcqParamTriggerDebounce, 144);
  REG_U32_RW(AcqParamLiveHoldoffAdjustEnable, 148);
  REG_U32_RW(AcqParamLiveHoldoffAdjustPeriod, 152);
  REG_U32_RW(AcqParamTriggerHoldoff, 156);
  REG_U32_RW(AcqParamChannelsInvertReg, 160);
  REG_U32_RW(AcqParamSamplesPerLine, 168);
  REG_U32_RW(AcqParamVolumesPerAcq, 172);
  REG_U32_RW(AcqParamTotalAcqs, 176);
  REG_U32_RW(AcqParamBeamClockAdvance, 180);
  REG_U32_RW(AcqParamBeamClockDuration, 184);
  REG_U32_RW(AcqParamDummyVal, 188);
  REG_U32_RW(AcqParamDisableDivide, 192);
  REG_U32_RW(AcqParamScalePower, 196);
  REG_U32_RW(AcqParamEnableBidi, 200);
  REG_U32_RW(AcqParamPhotonPulseDebounce, 204);
  REG_U32_RW(AcqParamMaskLSBs, 208);
  REG_U32_RW(AcqParamEnableLineTag, 164);

  REG_U32_RW(AcqParamAuxTriggerEnable, 232);
  REG_U32_RW(AcqParamAuxTrig1TriggerChIdx, 212);
  REG_U32_RW(AcqParamAuxTrig2TriggerChIdx, 216);
  REG_U32_RW(AcqParamAuxTrig3TriggerChIdx, 220);
  REG_U32_RW(AcqParamAuxTrig4TriggerChIdx, 224);
  REG_U32_RW(AcqParamAuxTriggerDebounce, 228);
  REG_U32_RW(AcqParamAuxTriggerInvert, 288);

  REG_U32_RW(AcqParamPeriodTriggerMaxPeriod, 236);
  REG_U32_RW(AcqParamPeriodTriggerSettledThresh, 240);
  REG_U32_RW(AcqParamSimulatedResonantPeriod, 284);

  REG_U32_RW(AcqParamSampleClkPulsesPerPeriod, 324);
  REG_U32_RW(AcqParamLinearSampleClkPulseDuration, 328);

  REG_U32_RW(AcqParamLinearMode, 260);
  REG_U32_RW(AcqParamLinearFramesPerVolume, 264);
  REG_U32_RW(AcqParamLinearFrameClkHighTime, 268);
  REG_U32_RW(AcqParamLinearFrameClkLowTime, 272);
  REG_U32_RW(AcqParamUniformSampling, 276);
  REG_U32_RW(AcqParamUniformBinSize, 280);

  REG_U32_RW(I2cEnable, 348);
  REG_U32_RW(I2cDebounce, 352);
  REG_U32_RW(I2cAddress, 356);
  REG_U32_RW(I2cSdaPort, 360);
  REG_U32_RW(I2cSclPort, 364);

  REG_U32_RW(ScopeParamNumberOfSamples, 244);
  REG_U32_RW(ScopeParamDecimationLB2, 248);
  REG_U32_RW(ScopeParamTriggerId, 252);
  REG_U32_RW(ScopeParamTriggerHoldoff, 256);

  REG_U32_R(AcqStatusDataFifoOverflowCount, 404);
  REG_U32_R(AcqStatusAuxFifoOverflowCount, 408);
  REG_U32_R(AcqStatusStateMachineState, 412);
  REG_U32_R(AcqStatusVolumesDone, 416);
  REG_U32_R(ScopeStatusFifoOverflowCount, 420);
  REG_U32_R(ScopeStatusWrites, 424);

protected:
  REG_U32_Command(sendStateMachineCmd, 100);

  uint32_t m_acqPlanWriteIdx;
  REG_U32_RW(AcqPlanNumSteps, 108);

  REG_U32_Command(writeAcqPlanStepReg, 104);
  void writeAcqPlanStep(uint32_t idx, bool newEntry, bool frameClockState = 0, uint32_t numPeriods = 0);

  REG_U32_Command(writeMaskTableInt, 112);

  REG_U32_RW(AcqParamChannelOffsetsReg1, 340);
  REG_U32_RW(AcqParamChannelOffsetsReg2, 344);
  // todo: implement accessor methods

  REG_U32_R(AcqStatusPeriodTriggerInfoReg, 400);
  // todo: implement accessor methods

  REG_U32_R(AcqStatusRawChannelDataReg1, 500);
  REG_U32_R(AcqStatusRawChannelDataReg2, 504);
};


class vDAQ_ClockCfg : public cRdiDeviceRegisterMap
{
public:
  vDAQ_ClockCfg(cRdiDeviceRegisterMap *pParent, uint32_t baseAddr) : cRdiDeviceRegisterMap(pParent, baseAddr) {};

  bool checkIpId();

  void issueDeviceCommand(uint8_t address, uint8_t sizeBits, bool read);
  void writeAllDeviceSettings();
  void readAllDeviceSettings();

  bool setupMsadcSampleClock();

  bool lockPll();
  bool checkPll();

  REG_U32_Command(resetSettingWriteBuffer, 24);
  REG_U32_CommandV(hardSync, 64, 50000);

  REG_U32_RW(ClockSource, 40);

protected:

  REG_U32_R(IpId, 44);

  REG_U32_Command(writeDeviceCommandReg, 20);

  REG_U32_RW(OutputBufWord0Reg, 0);
  REG_U32_RW(OutputBufWord1Reg, 4);
  REG_U32_RW(OutputBufWord2Reg, 8);
  REG_U32_RW(OutputBufWord3Reg, 12);
  REG_U32_RW(OutputBufWord4Reg, 16);

  REG_U32_R(ReadbackBufWord0Reg, 20);
  REG_U32_R(ReadbackBufWord1Reg, 24);
  REG_U32_R(ReadbackBufWord2Reg, 28);
  REG_U32_R(ReadbackBufWord3Reg, 32);
  REG_U32_R(ReadbackBufWord4Reg, 36);
};


class vDAQ_Msadc : public cRdiDeviceRegisterMap
{
public:
  vDAQ_Msadc(cRdiDeviceRegisterMap *pParent, uint32_t baseAddr) : cRdiDeviceRegisterMap(pParent, baseAddr) {};

  bool calInProgress();
  void doReset();

  REG_U32_RW(UsrTestPatternReq, 56);

protected:
  REG_U32_RW(CalDataRaw, 60);
  REG_U32_RW(ResetReg, 64);
};


class vDAQ_WaveformGen : public cRdiDeviceRegisterMap
{
public:
  vDAQ_WaveformGen(cRdiDeviceRegisterMap *pParent, uint32_t baseAddr) : cRdiDeviceRegisterMap(pParent, baseAddr) {};
};


class vDAQ : public cRdiDeviceInterface
{
public:
  vDAQ(int16_t deviceNum, bool designLoadAccess = true);
  ~vDAQ();

  vDAQ_AcquisitionEngine acqEngine;
  vDAQ_ClockCfg clkCfg;
  vDAQ_Msadc msadc;

  vDAQ_WaveformGen *pWavegen[5];

  cRdiFifo dataFifo;

  double getSampleClockRate();
  uint64_t getSystemClockT();

  bool verifyMsadcData();
  bool initMsadc();

  // base regs
  REG_U32_R(PcieClkT, 0x400000 + 0);
  REG_U32_R(SampleClkTicksPerMeasInterval, 0x400000 + 4);

  REG_U32_RW(AfeSelection, 0x400000 + 64);

protected:
  REG_U32_Command(cacheSystemClockT, 8);
  REG_U64_R(SystemClockTReg, 8);
};



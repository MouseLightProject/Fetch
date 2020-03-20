#pragma once

#include "rdiLib.h"



class vDAQ_AcquisitionEngine : public cRdiDeviceRegisterMap
{
public:
  vDAQ_AcquisitionEngine(cRdiDeviceRegisterMap *pParent, uint32_t baseAddr) : cRdiDeviceRegisterMap(pParent, baseAddr) {};

  // AE Regs
  REG_U32_ProtectedCommand(sendStateMachineCmd, 100);
  // todo: implement accessor methods

  REG_U32_ProtectedCommand(writeAcqPlanStepInt, 104);
  // todo: implement write method
  REG_U32(AcqPlanNumSteps, 108);

  REG_U32_ProtectedCommand(writeMaskTableInt, 112);
  // todo: implement write method
  REG_U32(MaskTableSize, 116);

  REG_U32(AcqParamPeriodTriggerChIdx, 120);
  REG_U32(AcqParamStartTriggerChIdx, 124);
  REG_U32(AcqParamNextTriggerChIdx, 128);
  REG_U32(AcqParamStopTriggerChIdx, 132);
  REG_U32(AcqParamStartTriggerInvert, 292);
  REG_U32(AcqParamNextTriggerInvert, 296);
  REG_U32(AcqParamStopTriggerInvert, 300);
  REG_U32(AcqParamPhotonChIdx, 136);
  REG_U32(AcqParamPeriodTriggerDebounce, 140);
  REG_U32(AcqParamTriggerDebounce, 144);
  REG_U32(AcqParamLiveHoldoffAdjustEnable, 148);
  REG_U32(AcqParamLiveHoldoffAdjustPeriod, 152);
  REG_U32(AcqParamTriggerHoldoff, 156);
  REG_U32(AcqParamChannelsInvertReg, 160);
  REG_U32(AcqParamSamplesPerLine, 168);
  REG_U32(AcqParamVolumesPerAcq, 172);
  REG_U32(AcqParamTotalAcqs, 176);
  REG_U32(AcqParamBeamClockAdvance, 180);
  REG_U32(AcqParamBeamClockDuration, 184);
  REG_U32(AcqParamDummyVal, 188);
  REG_U32(AcqParamDisableDivide, 192);
  REG_U32(AcqParamScalePower, 196);
  REG_U32(AcqParamEnableBidi, 200);
  REG_U32(AcqParamPhotonPulseDebounce, 204);
  REG_U32(AcqParamMaskLSBs, 208);

  REG_U32(AcqParamAuxTriggerEnable, 232);
  REG_U32(AcqParamAuxTrig1TriggerChIdx, 212);
  REG_U32(AcqParamAuxTrig2TriggerChIdx, 216);
  REG_U32(AcqParamAuxTrig3TriggerChIdx, 220);
  REG_U32(AcqParamAuxTrig4TriggerChIdx, 224);
  REG_U32(AcqParamAuxTriggerDebounce, 228);
  REG_U32(AcqParamAuxTriggerInvert, 288);

  REG_U32(AcqParamPeriodTriggerMaxPeriod, 236);
  REG_U32(AcqParamPeriodTriggerSettledThresh, 240);
  REG_U32(AcqParamSimulatedResonantPeriod, 284);

  REG_U32(AcqParamSampleClkPulsesPerPeriod, 324);
  REG_U32(AcqParamLinearSampleClkPulseDuration, 328);

  REG_U32(AcqParamLinearMode, 260);
  REG_U32(AcqParamLinearFramesPerVolume, 264);
  REG_U32(AcqParamLinearFrameClkHighTime, 268);
  REG_U32(AcqParamLinearFrameClkLowTime, 272);
  REG_U32(AcqParamUniformSampling, 276);
  REG_U32(AcqParamUniformBinSize, 280);

  REG_U32_Protected(AcqParamChannelOffsetsReg1, 340);
  REG_U32_Protected(AcqParamChannelOffsetsReg2, 344);
  // todo: implement accessor methods

  REG_U32(I2cEnable, 348);
  REG_U32(I2cDebounce, 352);
  REG_U32(I2cAddress, 356);
  REG_U32(I2cSdaPort, 360);
  REG_U32(I2cSclPort, 364);

  REG_U32(ScopeParamNumberOfSamples, 244);
  REG_U32(ScopeParamDecimationLB2, 248);
  REG_U32(ScopeParamTriggerId, 252);
  REG_U32(ScopeParamTriggerHoldoff, 256);

  REG_U32_Protected(AcqStatusPeriodTriggerInfoReg, 400);
  // todo: implement accessor methods

  REG_U32(AcqStatusDataFifoOverflowCount, 404);
  REG_U32(AcqStatusAuxFifoOverflowCount, 408);
  REG_U32(AcqStatusStateMachineState, 412);
  REG_U32(AcqStatusVolumesDone, 416);
  REG_U32(ScopeStatusFifoOverflowCount, 420);
  REG_U32(ScopeStatusWrites, 424);

  REG_U32_Protected(AcqStatusRawChannelDataReg1, 500);
  REG_U32_Protected(AcqStatusRawChannelDataReg2, 504);
  // todo: implement accessor methods
};


class vDAQ_ClockCfg : public cRdiDeviceRegisterMap
{
public:
  vDAQ_ClockCfg(cRdiDeviceRegisterMap *pParent, uint32_t baseAddr) : cRdiDeviceRegisterMap(pParent, baseAddr) {};
};


class vDAQ_Msadc : public cRdiDeviceRegisterMap
{
public:
  vDAQ_Msadc(cRdiDeviceRegisterMap *pParent, uint32_t baseAddr) : cRdiDeviceRegisterMap(pParent, baseAddr) {};
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

  // base regs
  REG_U32(PcieClkT, 0x400000 + 0);
  REG_U32(SampleClkTicksPerMeasInterval, 0x400000 + 4);

  REG_U32_ProtectedCommand(cacheSystemClockT, 8);
  REG_U64_Protected(SystemClockTReg, 8);
  public: uint64_t getSystemClockT();
};



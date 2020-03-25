#pragma once

#include "rdiLib.h"
#include <list>

namespace ddi {

  class WaveformGenIp : public cRdiDeviceRegisterMap
  {
  public:
    WaveformGenIp(cRdiDeviceRegisterMap *pParent, uint32_t baseAddr) : cRdiDeviceRegisterMap(pParent, baseAddr) {};
  };


  typedef enum uint32_t{
    Finite,
    Continuous
  } SampleMode;

  class AnalogOutputTask
  {
  public:
    AnalogOutputTask(cRdiDeviceInterface *pDevice);

    ::int16_t addChannel(::uint32_t channelId);
    ::int16_t writeOutputBuffer();

    ::int16_t start();
    ::int16_t abort();

    SampleMode sampleMode;
    ::double sampleRate;
    ::uint64_t samplesPerTrigger;
    ::uint32_t startTriggerIndex;
    ::bool allowRetrigger;

  private:
    cRdiDeviceInterface *m_pDevice;
    cRdiDmaBuffer m_waveformDmaBuffer;
    list<WaveformGenIp*> m_pChanIp;
  };

}


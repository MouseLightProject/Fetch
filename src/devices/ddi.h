#pragma once

#include "rdiLib.h"
#include <list>


#define DDI_MAX(a,b) ((a > b) ? a : b)
#define DDI_MIN(a,b) ((a < b) ? a : b)

#define DDI_SAMPLE_CLK_TIMEBASE_HZ 200000000.0

namespace vdaq {
  class Device;
}

namespace ddi {

  typedef ::uint32_t uint32_t;

  class WaveformGenIp : public rdi::DeviceRegisterMap
  {
  public:
    WaveformGenIp(DeviceRegisterMap *pParent, ::uint32_t baseAddr);

    void createWaveBuffer(uint64_t nSamples);
    void freeWaveBuffer();

    void writeWaveBuffer(int16_t *data_counts, uint64_t nSamples, uint64_t sampleOffset = 0);
    void writeWaveBuffer(double *data_volts, uint64_t nSamples, uint64_t sampleOffset = 0);

    void writeChannelOutputValue(int16_t value_counts);
    void writeChannelOutputValue(double value_volts);

    REG_U32_Command(softTrigger, 52);
    REG_U32_CommandV(startBufferedOutput, 4, 1);
    REG_U32_Command(stopBufferedOutput, 4);
    REG_U32_CommandV(notifyBufferUpdate, 60, 1);

    REG_U32_R(ThisPeerTriggerId, 8);
    REG_U32_R(BufferedModeActive, 4);
    REG_U64_RW(WaveformLength, 64);
    REG_U32_RW(SamplePeriod, 28);
    REG_U64_RW(SamplesPerTrigger, 80);
    REG_U32_RW(PeerTriggerId, 40);
    REG_U32_RW(ExtTriggerId, 44);
    REG_U32_RW(AllowRetrigger, 48);
    REG_U32_R(BufferedOutputDone, 52);
    REG_U32_RW(TriggerInvert, 56);

    REG_U64_RW(OwnerId, 104);
    REG_U64_RW(BufferWriterId, 112);

    REG_U32_R(ErrorCount, 24);
    REG_U32_R(MaxDmaLatency, 96);

  private:
    REG_U32_R(IpIdentifier, 0);
    REG_U32_R(SgMaxNumPages, 204);
    REG_U32_RW(OutputValueReg, 20);

    rdi::DmaBuffer m_waveformDmaBuffer;
    uint64_t m_waveBufferSizeBytes;
    uint64_t m_waveBufferSizeSamples;
  };


  enum class SampleMode : uint32_t {
    Finite,
    Continuous
  };

  class AnalogOutputTask
  {
  public:
    AnalogOutputTask(vdaq::Device *pDevice);

    void addChannel(uint32_t channelId);

    void setOutputChannelValues(int16_t *data_counts);
    void setOutputChannelValues(double *data_volts);

    void setOutputBufferLength(uint64_t nSamplesPerChannel);
    void writeOutputBuffer(int16_t *data_counts, uint64_t nSamplesPerChannel = 0, uint64_t sampleOffset = 0);
    void writeOutputBuffer(double *data_volts, uint64_t nSamplesPerChannel = 0, uint64_t sampleOffset = 0);

    void start(bool triggerImmediately = false);
    void abort();

    bool isActive();
    bool isDone();

    SampleMode sampleMode;
    double sampleRate;
    uint64_t samplesPerTrigger;
    int32_t startTriggerIndex;
    bool allowRetrigger;

  private:
    bool checkIpOwners();
    bool verifyBuffers();

    void setOutputChannelValuesInt(int16_t *data_counts, double *data_volts = NULL);
    void writeOutputBufferInternal(int16_t *data_counts, uint64_t nSamplesPerChannel, uint64_t sampleOffset, double *data_volts = NULL);
    int16_t *convertSamples(double *data_volts, uint64_t nSamples);

    uint64_t m_outputBufferLength;
    bool m_bufferNeedsWrite;

    vdaq::Device *m_pDevice;

    list<uint16_t> m_channels;
    list<WaveformGenIp*> m_pChanIp;
  };

  int16_t vToCounts(double v);

}


#pragma once

#include "vdaq.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////                     Waveform Gen                        ///////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ddi::WaveformGenIp::WaveformGenIp(cRdiDeviceRegisterMap *pParent, ::uint32_t baseAddr)
  : cRdiDeviceRegisterMap(pParent, baseAddr)
  , m_waveformDmaBuffer(pParent)
  , m_waveBufferSizeBytes(0)
  , m_waveBufferSizeSamples(0)
{

}

void ddi::WaveformGenIp::createWaveBuffer(uint64_t nSamples) {
  freeWaveBuffer();

  uint64_t desiredBufferSize = nSamples * 2;
  m_waveBufferSizeBytes = m_waveformDmaBuffer.allocateBuffer(desiredBufferSize,getSgMaxNumPages());
  m_waveBufferSizeSamples = m_waveBufferSizeBytes / 2;

  if (m_waveBufferSizeBytes < desiredBufferSize)
    throw "Failed to allocate waveform buffer.";
}


void ddi::WaveformGenIp::freeWaveBuffer() {
  if (m_waveBufferSizeBytes)
    m_waveformDmaBuffer.freeBuffer();
  m_waveBufferSizeBytes = 0;
  m_waveBufferSizeSamples = 0;

  setBufferWriterId(0);
}


void ddi::WaveformGenIp::writeWaveBuffer(int16_t *data_counts, uint64_t nSamples, uint64_t sampleOffset) {
  if ((sampleOffset + nSamples) > m_waveBufferSizeSamples)
    throw "Requested write exceeds wave buffer size.";

  m_waveformDmaBuffer.writeToBuffer(sampleOffset * 2, data_counts, nSamples * 2);
}


void ddi::WaveformGenIp::writeWaveBuffer(double *data_volts, uint64_t nSamples, uint64_t sampleOffset) {
  int16_t *data_i = new int16_t[nSamples];

  for (uint64_t i = 0; i < nSamples; i++)
    data_i[i] = ddi::vToCounts(data_volts[i]);

  try {
    writeWaveBuffer(data_i, nSamples, sampleOffset);
    delete[] data_i;
  }
  catch (exception e) {
    delete[] data_i;
    throw;
  }
}


void ddi::WaveformGenIp::writeChannelOutputValue(int16_t value_counts) {
  setOutputValueReg((uint32_t)value_counts);
}


void ddi::WaveformGenIp::writeChannelOutputValue(double value_volts) {
  writeChannelOutputValue(ddi::vToCounts(value_volts));
}




int16_t ddi::vToCounts(double v) {
  return ((ddi_max(ddi_min(v,10.0),-10.0) + 10.0) / 20.0) * 65535.0;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////                  Analog Output Task                     ///////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ddi::AnalogOutputTask::AnalogOutputTask(vDAQ *pDevice) : m_pDevice(pDevice), m_waveformLength(0) {
  m_channels.clear();
  m_pChanIp.clear();
}



void ddi::AnalogOutputTask::addChannel(uint32_t channelId) {
  if (channelId >= m_pDevice->pWavegenIp.size())
    throw "Invalid channel ID requested.";

  m_channels.push_back(channelId);
  m_pChanIp.push_back(m_pDevice->pWavegenIp[channelId]);
}



bool ddi::AnalogOutputTask::checkIpOwners() {
  bool iCanOwn = true;
  auto i = m_pChanIp.begin();

  while (iCanOwn && (i != m_pChanIp.end())) {
    uint64_t owner = (*i)->getOwnerId();
    iCanOwn = iCanOwn && (!owner || (owner == ((uint64_t)this)));
  }

  return iCanOwn;
}



bool ddi::AnalogOutputTask::verifyBuffers() {
  bool iOwn = true;
  auto i = m_pChanIp.begin();

  while (iOwn && (i != m_pChanIp.end())) {
    uint64_t owner = (*i)->getBufferWriterId();
    iOwn = iOwn && (owner == ((uint64_t)this));
  }

  return iOwn;
}



void ddi::AnalogOutputTask::writeOutputBuffer(int16_t *data_counts, uint64_t nSamplesPerChannel, uint64_t sampleOffset) {
  if (!checkIpOwners())
    throw "Another task is using the requested channel.";

  auto i = m_pChanIp.begin();
  int16_t *dataCollumnI = data_counts;

  if (isActive()) {
    if ((nSamplesPerChannel + sampleOffset) > m_waveformLength)
      throw "Cannot increase buffer size while task is active.";

    while (i != m_pChanIp.end()) {
      (*i)->writeWaveBuffer(dataCollumnI, nSamplesPerChannel, sampleOffset);
      dataCollumnI += nSamplesPerChannel;
      (*i)->notifyBufferUpdate();
      (*i)->setBufferWriterId((uint64_t)this);
    }
  }
  else {
    if (sampleOffset)
      throw "Before starting task, only full buffer update is allowed.";
    m_waveformLength = nSamplesPerChannel;

    while (i != m_pChanIp.end()) {
      (*i)->createWaveBuffer(nSamplesPerChannel);
      (*i)->writeWaveBuffer(dataCollumnI, nSamplesPerChannel, 0);
      dataCollumnI += nSamplesPerChannel;
      (*i)->setBufferWriterId((uint64_t)this);
    }
  }
}



void ddi::AnalogOutputTask::writeOutputBuffer(double *data_volts, uint64_t nSamplesPerChannel, uint64_t sampleOffset) {
  uint64_t N = nSamplesPerChannel * m_pChanIp.size();
  int16_t *data_i = new int16_t[N];

  for (uint64_t i = 0; i < N; i++)
    data_i[i] = ddi::vToCounts(data_volts[i]);

  try {
    writeOutputBuffer(data_i, nSamplesPerChannel, sampleOffset);
    delete[] data_i;
  }
  catch (exception e) {
    delete[] data_i;
    throw;
  }
}



void ddi::AnalogOutputTask::start() {

}



void ddi::AnalogOutputTask::abort() {

}



bool ddi::AnalogOutputTask::isActive() {
  if (m_pChanIp.size()) {
    bool imActive = true;
    auto i = m_pChanIp.begin();

    while (imActive && (i != m_pChanIp.end())) {
      uint64_t owner = (*i)->getOwnerId();
      imActive = imActive && (owner == ((uint64_t)this)) && (*i)->getBufferedModeActive();
    }
    return imActive;
  }
  else
    return false;
}



bool ddi::AnalogOutputTask::isDone() {
  if (m_pChanIp.size()) {
    bool imDone = true;
    auto i = m_pChanIp.begin();

    while (imDone && (i != m_pChanIp.end())) {
      uint64_t owner = (*i)->getOwnerId();
      imDone = imDone && (owner == ((uint64_t)this)) && (*i)->getBufferedModeActive() && (*i)->getBufferedOutputDone();
    }
    return imDone;
  }
  else
    return false;
}









#pragma once

#include "vdaq.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////                     Waveform Gen                        ///////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ddi::WaveformGenIp::WaveformGenIp(DeviceRegisterMap *pParent, ::uint32_t baseAddr)
  : DeviceRegisterMap(pParent, baseAddr)
  , m_waveformDmaBuffer(pParent)
  , m_waveBufferSizeBytes(0)
  , m_waveBufferSizeSamples(0)
{

}

void ddi::WaveformGenIp::createWaveBuffer(uint64_t nSamples) {
  uint64_t desiredBufferSize = nSamples * 2;

  if (m_waveBufferSizeBytes < desiredBufferSize) {
    freeWaveBuffer();

    m_waveBufferSizeBytes = m_waveformDmaBuffer.allocateBuffer(desiredBufferSize, getSgMaxNumPages());
    m_waveBufferSizeSamples = m_waveBufferSizeBytes / 2;
  }

  if (m_waveBufferSizeBytes < desiredBufferSize)
    throw "Failed to allocate waveform buffer.";

  m_waveformDmaBuffer.writeBufferSGListToDeviceRegU32(m_baseAddr + 200);
  setWaveformLength(nSamples);
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


bool ddi::WaveformGenIp::initialReadIsPending() {
	return getInternalData1() & 0x2;
}




int16_t ddi::vToCounts(double v) {
  return (int16_t)(((DDI_MAX(DDI_MIN(v,10.0),-10.0) + 10.0) / 20.0) * 65535.0);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////                  Analog Output Task                     ///////////////////////////////////////
///////////////////////////////////////                                                         ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ddi::AnalogOutputTask::AnalogOutputTask(vdaq::Device *pDevice) : m_pDevice(pDevice), m_outputBufferLength(0)
  , sampleMode(SampleMode::Continuous), sampleRate(1e6), samplesPerTrigger(0), startTriggerIndex(-1), allowRetrigger(false)
{
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
    uint64_t owner = (*i++)->getOwnerId();
    iCanOwn = iCanOwn && (!owner || (owner == ((uint64_t)this)));
  }

  return iCanOwn;
}



bool ddi::AnalogOutputTask::verifyBuffers() {
  bool iOwn = true;
  auto i = m_pChanIp.begin();

  while (iOwn && (i != m_pChanIp.end())) {
    uint64_t owner = (*i++)->getBufferWriterId();
    iOwn = iOwn && (owner == ((uint64_t)this));
  }

  return iOwn;
}



void ddi::AnalogOutputTask::setOutputChannelValues(int16_t *data_counts) {
  setOutputChannelValuesInt(data_counts);
}



void ddi::AnalogOutputTask::setOutputChannelValues(double *data_volts) {
  setOutputChannelValuesInt(NULL, data_volts);
}



void ddi::AnalogOutputTask::setOutputChannelValuesInt(int16_t *data_counts, double *data_volts) {
  size_t nChans = m_channels.size();
  size_t i;

  if (!nChans)
    throw "Cannot write task without channels.";

  int16_t *data_actual = data_counts;

  if (!data_actual) {
    data_actual = new int16_t[nChans];
    for (i = 0; i < nChans; i++)
      data_actual[i] = vToCounts(data_volts[i]);
  }

  auto ipI = m_pChanIp.begin();

  try {
    i = 0;

    while (ipI != m_pChanIp.end())
      (*ipI++)->writeChannelOutputValue(data_actual[i++]);

    if (!data_counts)
      delete[] data_actual;
  }
  catch (exception e) {
    if (!data_counts)
      delete[] data_actual;

    throw;
  }
}



void ddi::AnalogOutputTask::setOutputBufferLength(uint64_t nSamplesPerChannel) {
  if (isActive())
    throw "Cannot change buffer size while task is active.";
  else {
    m_outputBufferLength = nSamplesPerChannel;
    m_bufferNeedsWrite = true;
  }
}



int16_t *ddi::AnalogOutputTask::convertSamples(double *data_volts, uint64_t nSamples) {
  int16_t *data_i = new int16_t[nSamples];

  for (uint64_t i = 0; i < nSamples; i++)
    data_i[i] = ddi::vToCounts(data_volts[i]);

  return data_i;
}



void ddi::AnalogOutputTask::writeOutputBuffer(double *data_volts, uint64_t nSamplesPerChannel, uint64_t sampleOffset) {
  writeOutputBufferInternal(NULL, nSamplesPerChannel, sampleOffset, data_volts);
}

void ddi::AnalogOutputTask::writeOutputBuffer(int16_t *data_counts, uint64_t nSamplesPerChannel, uint64_t sampleOffset) {
  writeOutputBufferInternal(data_counts, nSamplesPerChannel, sampleOffset);
}

void ddi::AnalogOutputTask::writeOutputBufferInternal(int16_t *data_counts, uint64_t nSamplesPerChannel, uint64_t sampleOffset, double *data_volts) {
  if (!m_channels.size())
    throw "Cannot write task without channels.";
  if (!checkIpOwners())
    throw "Another task is using the requested channel.";

  auto i = m_pChanIp.begin();
  int16_t *data_counts_actual;

  if (isActive()) {
    if (!nSamplesPerChannel && sampleOffset)
      throw "If number of samples is zero, offset must also be zero to imply a complete buffer write.";
    else if (!nSamplesPerChannel)
      nSamplesPerChannel = m_outputBufferLength;
    else if ((nSamplesPerChannel + sampleOffset) > m_outputBufferLength)
      throw "Cannot increase buffer size while task is active.";

    // we don't need to handle the case where both nSamplesPerChannel and m_outputBufferLength are zero,
    // because task is active. task would not start if m_outputBufferLength was zero

    if (data_counts)
      data_counts_actual = data_counts;
    else
      data_counts_actual = convertSamples(data_volts, nSamplesPerChannel * m_channels.size());

    int16_t *dataCollumnI = data_counts_actual;

    try {
      while (i != m_pChanIp.end()) {
        (*i)->writeWaveBuffer(dataCollumnI, nSamplesPerChannel, sampleOffset);
        (*i)->notifyBufferUpdate();
        (*i++)->setBufferWriterId((uint64_t)this);
        dataCollumnI += nSamplesPerChannel;
      }

      if (!data_counts)
        delete[] data_counts_actual;
    }
    catch (exception e) {
      if (!data_counts)
        delete[] data_counts_actual;

      throw;
    }
  }
  else {
    if (!nSamplesPerChannel && !m_outputBufferLength)
      throw "Waveform length must be set.";
    else if (sampleOffset && !nSamplesPerChannel)
      throw "If number of samples is zero, offset must also be zero to imply a complete buffer write.";
    else if (m_outputBufferLength && !nSamplesPerChannel)
      nSamplesPerChannel = m_outputBufferLength;
    else if (!m_outputBufferLength && sampleOffset)
      throw "If waveform length has not yet been set, write offset must be zero; waveform length will be implied from number of samples written.";
    else if (!m_outputBufferLength)
      m_outputBufferLength = nSamplesPerChannel;

    if (data_counts)
      data_counts_actual = data_counts;
    else
      data_counts_actual = convertSamples(data_volts, nSamplesPerChannel * m_channels.size());

    int16_t *dataCollumnI = data_counts_actual;

    if (!sampleOffset && (nSamplesPerChannel == m_outputBufferLength))
      m_bufferNeedsWrite = false;

    try {
      while (i != m_pChanIp.end()) {
        (*i)->createWaveBuffer(nSamplesPerChannel); // this only recreates the buffer if the existing buffer isn't large enough
        (*i)->writeWaveBuffer(dataCollumnI, nSamplesPerChannel, sampleOffset);
        (*i++)->setBufferWriterId((uint64_t)this);
        dataCollumnI += nSamplesPerChannel;
      }

      if (!data_counts)
        delete[] data_counts_actual;
    }
    catch (exception e) {
      if (!data_counts)
        delete[] data_counts_actual;

      throw;
    }
  }
}



void ddi::AnalogOutputTask::start(bool triggerImmediately) {
  if (!m_channels.size())
    throw "Cannot start task without channels.";
  if (!checkIpOwners())
    throw "Cannot start task. Some resources are already in use.";

  uint64_t samps;
  if (sampleMode == SampleMode::Finite) {
    if (!samplesPerTrigger)
      throw "Samples per trigger must be set for finite output task.";
    samps = samplesPerTrigger;
  }
  else
    samps = 0;

  if (m_bufferNeedsWrite)
    throw "Cannot start task; buffer was not written.";
  verifyBuffers();

  auto i = m_pChanIp.begin();
  WaveformGenIp *pMasterIp = *i;
  uint32_t peerTrig = pMasterIp->getThisPeerTriggerId();
  do {
    WaveformGenIp *pIp = *i;

    pIp->stopBufferedOutput();
    pIp->setSamplePeriod((uint32_t)(DDI_SAMPLE_CLK_TIMEBASE_HZ / sampleRate));
    pIp->setSamplesPerTrigger(samps);
    pIp->setAllowRetrigger(allowRetrigger);
    pIp->setPeerTriggerId(peerTrig);

    pIp->startBufferedOutput();
    pIp->setOwnerId((uint64_t)this);

  } while (++i != m_pChanIp.end());

  if (startTriggerIndex >= 0)
    pMasterIp->setExtTriggerId(startTriggerIndex);

  DWORD startT = GetTickCount();
  i = m_pChanIp.begin();
  do {
	  while ((*i)->initialReadIsPending()) {
		  Sleep(1);
		  if ((GetTickCount() - startT) > 100)
			  throw "Timeout while waiting for device to be ready.";
	  }
  } while (++i != m_pChanIp.end());

  if (triggerImmediately)
    pMasterIp->softTrigger();
}



void ddi::AnalogOutputTask::abort() {
  auto i = m_pChanIp.begin();
  do {
    WaveformGenIp *pIp = *i;
    uint64_t owner_i = pIp->getOwnerId();
    if (!owner_i || (owner_i == ((uint64_t)this))) {
      pIp->stopBufferedOutput();
      pIp->setOwnerId(0);
      pIp->setExtTriggerId(0xFF);
    }
  } while (++i != m_pChanIp.end());
}



bool ddi::AnalogOutputTask::isActive() {
  if (m_pChanIp.size()) {
    bool imActive = true;
    auto i = m_pChanIp.begin();

    while (imActive && (i != m_pChanIp.end())) {
      uint64_t owner = (*i)->getOwnerId();
      imActive = imActive && (owner == ((uint64_t)this)) && (*i++)->getBufferedModeActive();
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
      imDone = imDone && (owner == ((uint64_t)this)) && (*i)->getBufferedModeActive() && (*i++)->getBufferedOutputDone();
    }
    return imDone;
  }
  else
    return false;
}









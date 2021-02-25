/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "soapysdr_radio.hpp"

#include "ISource.hpp"
#include "qwidget.h"

#include <QDebug>
#include <QVersionNumber>

#include <memory>

SoapySdrRadio::SoapySdrRadio()
    : ndevices_(0), devicesFound_(nullptr), sdr_(nullptr), numChannels_(0), channel_(0),
      centreFrequency_(0), sampleRate_(0), bandwidth_(0), agcAvailable_(false),
      agc_(false), globalGain_(0), worker_(nullptr), running_(false),
      library_(SOAPY_LIBRARY_NAME), initialised_(initialiseLibrary()),
      widget_(std::unique_ptr<SoapySdrWidget>(nullptr))

{
    if (!initialised_)
    {
        return;
    }

    QString soapyVersion{SoapySDR_getAPIVersion()};
    qDebug() << "SoapySDR API version: " << soapyVersion;

    if (QVersionNumber::fromString(soapyVersion) < QVersionNumber{0, 8, 0})
    {
        qDebug() << " This module required SoapySDR API version > 0.8.0";
        return;
    }

    widget_ = std::make_unique<SoapySdrWidget>(this);

    enumerateDevices();

    nameDevices();
    makeDeviceStrings();

    widget_->deviceOpened();
};

SoapySdrRadio::~SoapySdrRadio()
{
    stop();

    if (devicesFound_)
    {
        SoapySDRKwargsList_clear(devicesFound_, ndevices_);
    }
}

void SoapySdrRadio::start()
{
    if (!initialised_)
    {
        return;
    }

    if (!makeDevice())
    {
        return;
    }

    getNumAvailableChannels();
    getAvailableAntennas();
    getAgcAvailability();

    setCentreFrequency(getCentreFrequency());

    getAvailableSampleRates();
    getAvailableBandwidths();

    getAvailableGlobalGainRange();
    getAvailableSeparateGainsRanges();

    setDeviceSampleRate();

    startWorker();

    widget_->deviceStarted();
}

void SoapySdrRadio::stop()
{
    if (!initialised_)
    {
        return;
    }

    if (!running_)
    {
        return;
    }

    stopWorker();
    unmakeDevice();

    widget_->deviceStopped();
}

double SoapySdrRadio::getCentreFrequency()
{
    return centreFrequency_;
}

void SoapySdrRadio::setCentreFrequency(double centreFrequency)
{
    if (!initialised_)
    {
        return;
    }

    centreFrequency_ = centreFrequency;

    if (sdr_)
    {
        auto res = SoapySDRDevice_setFrequency(sdr_, SOAPY_SDR_RX, channel_,
                                               centreFrequency_, nullptr);
        if (res != 0)
        {
            qDebug() << "SoapySDRDevice_setFrequency failed with error: "
                     << SoapySDRDevice_lastError();
        }
        // TODO: readback and check again it really set the frequency
    }

    for (const auto &listener : listeners_)
    {
        listener->setCentreFrequency(centreFrequency_);
    }
}

double SoapySdrRadio::getSampleRate()
{
    return sampleRate_;
}

QWidget *SoapySdrRadio::getWidget()
{
    return static_cast<QWidget *>(widget_.get());
}

std::vector<QString> SoapySdrRadio::getDeviceStrings()
{
    return deviceStrings_;
}

std::vector<QString> SoapySdrRadio::getDeviceNames()
{
    return deviceNames_;
}

QString SoapySdrRadio::getDeviceString()
{
    return deviceString_;
}

void SoapySdrRadio::setDeviceString(const QString &sourceString)
{
    deviceString_ = sourceString;

    resetParameters();
}

size_t SoapySdrRadio::getChannelCount()
{
    return numChannels_;
}

size_t SoapySdrRadio::getChannel()
{
    return channel_;
}

void SoapySdrRadio::setChannel(size_t channel)
{
    if (!initialised_)
    {
        return;
    }

    if (channel > numChannels_ || channel == channel_)
    {
        return;
    }

    channel_ = channel;
}

std::vector<QString> SoapySdrRadio::getSupportedAntennas()
{
    return availableAntennas_;
}

QString SoapySdrRadio::getAntenna()
{
    return antenna_;
}

void SoapySdrRadio::setAntenna(QString antenna)
{
    if (!initialised_)
    {
        return;
    }
    if (std::find(availableAntennas_.begin(), availableAntennas_.end(), antenna) ==
        availableAntennas_.end())
    {
        qDebug() << "Didn't find the correct antenna: " << antenna;
        return;
    }

    antenna_ = antenna;

    if (sdr_)
    {
        auto res = SoapySDRDevice_setAntenna(sdr_, SOAPY_SDR_RX, channel_,
                                             antenna_.toLocal8Bit());
        if (res != 0)
        {
            qDebug() << "SoapySDRDevice_setAntenna failed with error: "
                     << SoapySDRDevice_lastError();
        }
    }
}

std::vector<double> SoapySdrRadio::getSupportedSampleRatesDiscrete()
{
    return supportedSampleRatesDiscrete_;
}

std::vector<SoapySDRRange> SoapySdrRadio::getSupportedSampleRatesRanges()
{
    return supportedBandwidthsRanges_;
}

void SoapySdrRadio::setSampleRate(double sampleRate)
{
    if (!initialised_)
    {
        return;
    }

    if (!validateSampleRate(sampleRate))
    {
        qDebug() << "Failed to set sample rate because of invalid input.";
        return;
    }

    sampleRate_ = sampleRate;

    for (const auto &listener : listeners_)
    {
        listener->setSampleRate(sampleRate_);
    }
}

std::vector<double> SoapySdrRadio::getSupportedBandwidthsDiscrete()
{
    return supportedBandwidthsDiscrete_;
}

std::vector<SoapySDRRange> SoapySdrRadio::getSupportedBandwidthsRanges()
{
    return supportedBandwidthsRanges_;
}

double SoapySdrRadio::getBandwidth()
{
    return bandwidth_;
}

void SoapySdrRadio::setBandwidth(double bandwidth)
{
    if (!initialised_)
    {
        return;
    }

    if (!validateBandwidth(bandwidth))
    {
        qDebug() << "Tried to set an invalid bandwidth: " << bandwidth;
        return;
    }

    bandwidth_ = bandwidth;

    if (sdr_)
    {
        auto res = SoapySDRDevice_setBandwidth(sdr_, SOAPY_SDR_RX, channel_, bandwidth_);
        if (res != 0)
        {
            qDebug() << "SoapySDRDevice_setBandwidth failed with error: "
                     << SoapySDRDevice_lastError();
        }
    }
}

bool SoapySdrRadio::hasAgc()
{
    return agcAvailable_;
}

bool SoapySdrRadio::getAgc()
{
    if (!initialised_)
    {
        return false;
    }

    if (sdr_)
    {
        agc_ = SoapySDRDevice_getGainMode(sdr_, SOAPY_SDR_RX, channel_);
    }

    return agc_;
}

void SoapySdrRadio::setAgc(bool state)
{
    if (!initialised_)
    {
        return;
    }

    if (!agcAvailable_)
    {
        return;
    }

    agc_ = state;

    if (sdr_)
    {
        auto res = SoapySDRDevice_setGainMode(sdr_, SOAPY_SDR_RX, channel_, agc_);
        if (res != 0)
        {
            qDebug() << "SoapySDRDevice_setGainMode failed with error: "
                     << SoapySDRDevice_lastError();
        }
    }
}

SoapySDRRange SoapySdrRadio::getGlobalGainRange()
{
    return globalGainRange_;
}

double SoapySdrRadio::getGlobalGain()
{
    return globalGain_;
}

void SoapySdrRadio::setGlobalGain(double gain)
{
    if (!initialised_)
    {
        return;
    }

    if (gain > globalGainRange_.maximum || gain < globalGainRange_.minimum)
    {
        // TODO: also check the step...
        return;
    }

    globalGain_ = gain;

    if (sdr_)
    {
        auto res = SoapySDRDevice_setGain(sdr_, SOAPY_SDR_RX, channel_, globalGain_);
        if (res != 0)
        {
            qDebug() << "SoapySDRDevice_setGain failed with error: "
                     << SoapySDRDevice_lastError();
        }
        for (auto i = 0u; i < specificGainNames_.size(); i++)
        {
            auto name = specificGainNames_[i].toLocal8Bit();
            specificGains_[i] =
                SoapySDRDevice_getGainElement(sdr_, SOAPY_SDR_RX, channel_, name);
        }
    }
}

std::vector<QString> SoapySdrRadio::getSpecificGainNames()
{
    return specificGainNames_;
}

std::vector<SoapySDRRange> SoapySdrRadio::getSpecificGainRanges()
{
    return specificGainRanges_;
}

std::vector<double> SoapySdrRadio::getSpecificGains()
{
    return specificGains_;
}

void SoapySdrRadio::setSpecificGain(QString name, double value)
{
    if (!initialised_)
    {
        return;
    }

    auto gainElem = std::find(specificGainNames_.begin(), specificGainNames_.end(), name);
    if (gainElem == specificGainNames_.end())
    {
        qDebug() << "Didn't find the correct gain element: " << name;
        return;
    }

    auto gainElemIndex = std::distance(specificGainNames_.begin(), gainElem);

    if (value > specificGainRanges_[gainElemIndex].maximum ||
        value < specificGainRanges_[gainElemIndex].minimum)
    {
        // TODO: also check the step..
        return;
    }

    specificGains_[gainElemIndex] = value;

    if (sdr_)
    {
        SoapySDRDevice_setGainElement(sdr_, SOAPY_SDR_RX, channel_, name.toLocal8Bit(),
                                      value);

        globalGain_ = SoapySDRDevice_getGain(sdr_, SOAPY_SDR_RX, channel_);
    }
}

void SoapySdrRadio::resetParameters()
{
    centreFrequency_ = SOAPY_INITIAL_CENTRE_FREQUENCY;
    sampleRate_ = SOAPY_INITIAL_SAMPLE_RATE;
}

void SoapySdrRadio::enumerateDevices()
{
    if (!initialised_)
    {
        return;
    }

    devicesFound_ = SoapySDRDevice_enumerate(nullptr, &ndevices_);
}

void SoapySdrRadio::nameDevices()
{
    deviceNames_.clear();

    if (ndevices_ < 1 || !devicesFound_)
    {
        return;
    }

    deviceNames_.reserve(ndevices_);
    for (size_t i = 0; i < ndevices_; i++)
    {
        deviceNames_.push_back(
            QString::fromLocal8Bit(SoapySDRKwargs_get(&devicesFound_[i], "label")));
    }
}

void SoapySdrRadio::makeDeviceStrings()
{
    deviceStrings_.clear();

    if (ndevices_ < 1 || !devicesFound_)
    {
        return;
    }

    deviceStrings_.reserve(ndevices_);
    for (size_t i = 0; i < ndevices_; i++)
    {
        deviceStrings_.push_back(
            QString::fromLocal8Bit(SoapySDRKwargs_toString(&devicesFound_[i])));
    }
}

bool SoapySdrRadio::makeDevice()
{
    if (!initialised_)
    {
        return false;
    }

    if (sdr_)
    {
        qDebug() << "SDR not previously closed, can't start...";
        return false;
    }

    sdr_ = SoapySDRDevice_makeStrArgs(deviceString_.toLocal8Bit());
    if (!sdr_)
    {
        qDebug() << "SoapySDRDevice_make failed: " << SoapySDRDevice_lastError();
        return false;
    }

    return true;
}

bool SoapySdrRadio::unmakeDevice()
{
    if (!initialised_)
    {
        return false;
    }

    if (sdr_)
    {
        auto err = SoapySDRDevice_unmake(sdr_);
        if (err != 0)
        {
            qDebug() << "SoapySDRDevice_unmake fail: " << SoapySDRDevice_lastError();
            return false;
        }

        sdr_ = nullptr;
    }

    return true;
}

void SoapySdrRadio::getNumAvailableChannels()
{
    if (!initialised_)
    {
        return;
    }

    if (!sdr_)
    {
        return;
    }

    numChannels_ = SoapySDRDevice_getNumChannels(sdr_, SOAPY_SDR_RX);
}

void SoapySdrRadio::getAvailableAntennas()
{
    if (!initialised_)
    {
        return;
    }

    if (!sdr_)
    {
        return;
    }

    availableAntennas_.clear();
    size_t nantennas;
    auto antennaCharList =
        SoapySDRDevice_listAntennas(sdr_, SOAPY_SDR_RX, channel_, &nantennas);
    for (auto j = 0u; j < nantennas; j++)
    {
        availableAntennas_.push_back(antennaCharList[j]);
    }

    antenna_ = QString{SoapySDRDevice_getAntenna(sdr_, SOAPY_SDR_RX, channel_)};
}

void SoapySdrRadio::initCentreFrequency()
{
    setCentreFrequency(SOAPY_INITIAL_CENTRE_FREQUENCY);
}

bool SoapySdrRadio::validateSampleRate(double sampleRate)
{
    if (std::find(supportedSampleRatesDiscrete_.begin(),
                  supportedSampleRatesDiscrete_.end(),
                  sampleRate) != supportedSampleRatesDiscrete_.end())
    {
        return true;
    }
    else
    {
        for (auto &currRange : supportedSampleRatesRanges_)
        {
            if (sampleRate <= currRange.maximum && sampleRate >= currRange.minimum)
            {
                if (currRange.step == 0.0)
                {
                    return true;
                }
                // TODO: account for double precision...
                else if (std::remainder(sampleRate - currRange.minimum, currRange.step) ==
                         0)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void SoapySdrRadio::getAvailableSampleRates()
{
    if (!initialised_)
    {
        return;
    }

    if (!sdr_)
    {
        return;
    }

    supportedSampleRatesDiscrete_.clear();
    supportedSampleRatesRanges_.clear();

    size_t fsLen;
    auto supFs = SoapySDRDevice_getSampleRateRange(sdr_, SOAPY_SDR_RX, channel_, &fsLen);

    for (auto j = 0u; j < fsLen; j++)
    {
        if (supFs[j].minimum == supFs[j].maximum)
        {
            supportedSampleRatesDiscrete_.push_back(supFs[j].minimum);
        }
        else
        {
            supportedSampleRatesRanges_.push_back(supFs[j]);
        }
    }

    if (validateSampleRate(sampleRate_))
    {
        return;
    }

    if (supportedSampleRatesDiscrete_.size() > 0)
    {
        auto idx =
            static_cast<int>(std::round((supportedSampleRatesDiscrete_.size() - 1) / 2));
        sampleRate_ = supportedSampleRatesDiscrete_[idx];
    }
    else if (supportedSampleRatesRanges_.size() > 0)
    {
        sampleRate_ = supportedSampleRatesRanges_[0].minimum;
    }
}

void SoapySdrRadio::setDeviceSampleRate()
{
    if (!initialised_)
    {
        return;
    }

    if (!sdr_)
    {
        return;
    }

    auto res = SoapySDRDevice_setSampleRate(sdr_, SOAPY_SDR_RX, channel_, sampleRate_);
    if (res != 0)
    {
        qDebug() << "SoapySDRDevice_setSampleRate failed with error: "
                 << SoapySDRDevice_lastError();
    }

    recomputeBandwidth();
}

bool SoapySdrRadio::validateBandwidth(double bandwidth)
{
    if (std::find(supportedBandwidthsDiscrete_.begin(),
                  supportedBandwidthsDiscrete_.end(),
                  bandwidth) != supportedBandwidthsDiscrete_.end())
    {
        return true;
    }
    else
    {
        for (auto i = 0u; i < supportedBandwidthsRanges_.size(); i++)
        {
            // TODO: account for the step
            if (supportedBandwidthsRanges_[i].minimum < bandwidth &&
                supportedBandwidthsRanges_[i].maximum > bandwidth)
            {
                return true;
            }
        }
    }

    return false;
}

void SoapySdrRadio::getAvailableBandwidths()
{
    if (!initialised_)
    {
        return;
    }

    if (!sdr_)
    {
        return;
    }

    supportedBandwidthsDiscrete_.clear();
    supportedBandwidthsRanges_.clear();

    size_t bwLen;
    auto supBws = SoapySDRDevice_getBandwidthRange(sdr_, SOAPY_SDR_RX, channel_, &bwLen);

    for (auto j = 0u; j < bwLen; j++)
    {
        if (supBws[j].minimum == supBws[j].maximum)
        {
            supportedBandwidthsDiscrete_.push_back(supBws[j].minimum);
        }
        else
        {
            supportedBandwidthsRanges_.push_back(supBws[j]);
        }
    }

    bandwidth_ = SoapySDRDevice_getBandwidth(sdr_, SOAPY_SDR_RX, channel_);
}

void SoapySdrRadio::recomputeBandwidth()
{
    auto currSampleRate = getSampleRate();

    if (getSupportedBandwidthsRanges().size() > 0)
    {
        for (auto &currBwRange : getSupportedBandwidthsRanges())
        {
            if (currSampleRate >= currBwRange.minimum &&
                currSampleRate <= currBwRange.maximum)
            {
                setBandwidth(currSampleRate);
                break;
            }
            else if (currSampleRate > currBwRange.maximum)
            {
                setBandwidth(currBwRange.maximum);
            }
            else if (currSampleRate < currBwRange.minimum)
            {
                setBandwidth(currBwRange.minimum + 1);
            }
        }
    }
    if (getSupportedBandwidthsDiscrete().size() > 0)
    {
        for (auto i = 0u; i < getSupportedBandwidthsDiscrete().size(); i++)
        {
            if (i + 1 == getSupportedBandwidthsDiscrete().size() ||
                getSupportedBandwidthsDiscrete()[i + 1] > currSampleRate)
            {
                setBandwidth(getSupportedBandwidthsDiscrete()[i]);
                break;
            }
        }
    }
}

void SoapySdrRadio::getAgcAvailability()
{
    if (!initialised_)
    {
        return;
    }

    if (!sdr_)
    {
        return;
    }

    agcAvailable_ = SoapySDRDevice_hasGainMode(sdr_, SOAPY_SDR_RX, channel_);

    agc_ = SoapySDRDevice_getGainMode(sdr_, SOAPY_SDR_RX, channel_);
}

void SoapySdrRadio::getAvailableGlobalGainRange()
{
    if (!initialised_)
    {
        return;
    }

    if (!sdr_)
    {
        return;
    }

    globalGainRange_ = SoapySDRDevice_getGainRange(sdr_, SOAPY_SDR_RX, channel_);

    globalGain_ = SoapySDRDevice_getGain(sdr_, SOAPY_SDR_RX, channel_);
}

void SoapySdrRadio::getAvailableSeparateGainsRanges()
{
    if (!initialised_)
    {
        return;
    }

    if (!sdr_)
    {
        return;
    }

    specificGainNames_.clear();
    size_t nstages;
    auto gainNames = SoapySDRDevice_listGains(sdr_, SOAPY_SDR_RX, channel_, &nstages);
    for (auto j = 0u; j < nstages; j++)
    {
        specificGainNames_.push_back(gainNames[j]);
    }

    specificGainRanges_.clear();
    for (auto j = 0u; j < specificGainNames_.size(); j++)
    {
        specificGainRanges_.push_back(SoapySDRDevice_getGainElementRange(
            sdr_, SOAPY_SDR_RX, channel_, specificGainNames_[j].toLocal8Bit()));
    }

    specificGains_.clear();
    for (auto j = 0u; j < specificGainNames_.size(); j++)
    {
        specificGains_.push_back(SoapySDRDevice_getGainElement(
            sdr_, SOAPY_SDR_RX, channel_, specificGainNames_[j].toLocal8Bit()));
    }
}

void SoapySdrRadio::worker()
{
    size_t channelList[1] = {channel_};
    auto rxStream = SoapySDRDevice_setupStream(sdr_, SOAPY_SDR_RX, SOAPY_SDR_CF32,
                                               channelList, 1, nullptr);
    if (!rxStream)
    {
        qDebug() << "SoapySDRDevice_setupStream failed with error: "
                 << SoapySDRDevice_lastError();
    }

    auto bufferSize = SoapySDRDevice_getStreamMTU(sdr_, rxStream);

    auto err = SoapySDRDevice_activateStream(sdr_, rxStream, 0, 0, 0);
    if (err != 0)
    {
        qDebug() << "SoapySDRDevice_activateStream failed with error: "
                 << SoapySDRDevice_lastError();
    }

    if (sampleBuffer_.size() < bufferSize)
    {
        sampleBuffer_.resize(bufferSize);
    }

    std::complex<float> *buffs[1] = {sampleBuffer_.data()};

    int flags;
    long long timeNs;
    int samplesWrittenOrError;

    while (running_)
    {
        samplesWrittenOrError =
            SoapySDRDevice_readStream(sdr_, rxStream, (void **)buffs, bufferSize, &flags,
                                      &timeNs, SOAPY_FRAME_TIMEOUT);

        if (samplesWrittenOrError < 0)
        {
            qDebug() << "SoapySDRDevice_readStream failed with error: "
                     << SoapySDR_errToStr(samplesWrittenOrError);
            continue;
        }

        if (!running_)
        {
            break;
        }

        for (const auto &listener : listeners_)
        {
            // I don't have to check for the buffer size, fortunately
            listener->receiveSamples(sampleBuffer_);
        }
    }

    err = SoapySDRDevice_deactivateStream(sdr_, rxStream, 0, 0);
    if (err != 0)
    {
        qDebug() << "SoapySDRDevice_deactivateStream failed with error: "
                 << SoapySDRDevice_lastError();
    }

    err = SoapySDRDevice_closeStream(sdr_, rxStream);
    if (err != 0)
    {
        qDebug() << "SoapySDRDevice_closeStream failed with error: "
                 << SoapySDRDevice_lastError();
    }
}

void SoapySdrRadio::startWorker()
{
    running_ = true;

    worker_ = new std::thread(&SoapySdrRadio::worker, this);
}

void SoapySdrRadio::stopWorker()
{
    running_ = false;

    if (worker_)
    {
        worker_->join();
        delete worker_;
    }

    worker_ = nullptr;
}

bool SoapySdrRadio::initialiseLibrary()
{
    SoapySDR_getAPIVersion =
        (SoapySDR_getAPIVersion_t)library_.resolve("SoapySDR_getAPIVersion");
    if (!SoapySDR_getAPIVersion)
    {
        qDebug() << "Could not get: "
                 << "SoapySDR_getAPIVersion";
        return false;
    }

    SoapySDRDevice_enumerate =
        (SoapySDRDevice_enumerate_t)library_.resolve("SoapySDRDevice_enumerate");
    if (!SoapySDRDevice_enumerate)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_enumerate";
        return false;
    }

    SoapySDRKwargsList_clear =
        (SoapySDRKwargsList_clear_t)library_.resolve("SoapySDRKwargsList_clear");
    if (!SoapySDRDevice_enumerate)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRKwargsList_clear";
        return false;
    }

    SoapySDRKwargs_toString =
        (SoapySDRKwargs_toString_t)library_.resolve("SoapySDRKwargs_toString");
    if (!SoapySDRKwargs_toString)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRKwargs_toString";
        return false;
    }

    SoapySDRKwargs_get = (SoapySDRKwargs_get_t)library_.resolve("SoapySDRKwargs_get");
    if (!SoapySDRKwargs_get)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRKwargs_get";
        return false;
    }

    SoapySDRKwargs_fromString =
        (SoapySDRKwargs_fromString_t)library_.resolve("SoapySDRKwargs_fromString");
    if (!SoapySDRKwargs_fromString)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRKwargs_fromString";
        return false;
    }

    SoapySDRDevice_make = (SoapySDRDevice_make_t)library_.resolve("SoapySDRDevice_make");
    if (!SoapySDRDevice_make)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_make";
        return false;
    }

    SoapySDRDevice_makeStrArgs =
        (SoapySDRDevice_makeStrArgs_t)library_.resolve("SoapySDRDevice_makeStrArgs");
    if (!SoapySDRDevice_makeStrArgs)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_makeStrArgs";
        return false;
    }

    SoapySDRDevice_lastError =
        (SoapySDRDevice_lastError_t)library_.resolve("SoapySDRDevice_lastError");
    if (!SoapySDRDevice_lastError)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_lastError";
        return false;
    }

    SoapySDRDevice_setFrequency =
        (SoapySDRDevice_setFrequency_t)library_.resolve("SoapySDRDevice_setFrequency");
    if (!SoapySDRDevice_setFrequency)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_setFrequency";
        return false;
    }

    SoapySDRDevice_getSampleRate =
        (SoapySDRDevice_getSampleRate_t)library_.resolve("SoapySDRDevice_getSampleRate");
    if (!SoapySDRDevice_getSampleRate)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getSampleRate";
        return false;
    }

    SoapySDRDevice_unmake =
        (SoapySDRDevice_unmake_t)library_.resolve("SoapySDRDevice_unmake");
    if (!SoapySDRDevice_unmake)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_unmake";
        return false;
    }

    SoapySDRDevice_getSampleRateRange =
        (SoapySDRDevice_getSampleRateRange_t)library_.resolve(
            "SoapySDRDevice_getSampleRateRange");
    if (!SoapySDRDevice_getSampleRateRange)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getSampleRateRange";
        return false;
    }

    SoapySDRDevice_setSampleRate =
        (SoapySDRDevice_setSampleRate_t)library_.resolve("SoapySDRDevice_setSampleRate");
    if (!SoapySDRDevice_setSampleRate)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_setSampleRate";
        return false;
    }

    SoapySDRDevice_listSampleRates = (SoapySDRDevice_listSampleRates_t)library_.resolve(
        "SoapySDRDevice_listSampleRates");
    if (!SoapySDRDevice_listSampleRates)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_listSampleRates";
        return false;
    }

    SoapySDRDevice_setBandwidth =
        (SoapySDRDevice_setBandwidth_t)library_.resolve("SoapySDRDevice_setBandwidth");
    if (!SoapySDRDevice_setBandwidth)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_setBandwidth";
        return false;
    }

    SoapySDRDevice_getBandwidth =
        (SoapySDRDevice_getBandwidth_t)library_.resolve("SoapySDRDevice_getBandwidth");
    if (!SoapySDRDevice_getBandwidth)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getBandwidth";
        return false;
    }

    SoapySDRDevice_listBandwidths = (SoapySDRDevice_listBandwidths_t)library_.resolve(
        "SoapySDRDevice_listBandwidths");
    if (!SoapySDRDevice_listBandwidths)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_listBandwidths";
        return false;
    }

    SoapySDRDevice_setupStream =
        (SoapySDRDevice_setupStream_t)library_.resolve("SoapySDRDevice_setupStream");
    if (!SoapySDRDevice_setupStream)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_setupStream";
        return false;
    }

    SoapySDRDevice_activateStream = (SoapySDRDevice_activateStream_t)library_.resolve(
        "SoapySDRDevice_activateStream");
    if (!SoapySDRDevice_activateStream)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_activateStream";
        return false;
    }

    SoapySDRDevice_deactivateStream = (SoapySDRDevice_deactivateStream_t)library_.resolve(
        "SoapySDRDevice_deactivateStream");
    if (!SoapySDRDevice_deactivateStream)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_deactivateStream";
        return false;
    }

    SoapySDRDevice_closeStream =
        (SoapySDRDevice_closeStream_t)library_.resolve("SoapySDRDevice_closeStream");
    if (!SoapySDRDevice_closeStream)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_closeStream";
        return false;
    }

    SoapySDRDevice_readStream =
        (SoapySDRDevice_readStream_t)library_.resolve("SoapySDRDevice_readStream");
    if (!SoapySDRDevice_readStream)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_readStream";
        return false;
    }

    SoapySDRDevice_getNumChannels = (SoapySDRDevice_getNumChannels_t)library_.resolve(
        "SoapySDRDevice_getNumChannels");
    if (!SoapySDRDevice_getNumChannels)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getNumChannels";
        return false;
    }

    SoapySDRDevice_getChannelInfo = (SoapySDRDevice_getChannelInfo_t)library_.resolve(
        "SoapySDRDevice_getChannelInfo");
    if (!SoapySDRDevice_getChannelInfo)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getChannelInfo";
        return false;
    }

    SoapySDRDevice_listAntennas =
        (SoapySDRDevice_listAntennas_t)library_.resolve("SoapySDRDevice_listAntennas");
    if (!SoapySDRDevice_listAntennas)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_listAntennas";
        return false;
    }

    SoapySDRDevice_hasGainMode =
        (SoapySDRDevice_hasGainMode_t)library_.resolve("SoapySDRDevice_hasGainMode");
    if (!SoapySDRDevice_hasGainMode)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_hasGainMode";
        return false;
    }

    SoapySDRDevice_setGainMode =
        (SoapySDRDevice_setGainMode_t)library_.resolve("SoapySDRDevice_setGainMode");
    if (!SoapySDRDevice_setGainMode)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_setGainMode";
        return false;
    }

    SoapySDRDevice_getGainMode =
        (SoapySDRDevice_getGainMode_t)library_.resolve("SoapySDRDevice_getGainMode");
    if (!SoapySDRDevice_getGainMode)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getGainMode";
        return false;
    }

    SoapySDRDevice_setGain =
        (SoapySDRDevice_setGain_t)library_.resolve("SoapySDRDevice_setGain");
    if (!SoapySDRDevice_setGain)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_setGain";
        return false;
    }

    SoapySDRDevice_setGainElement = (SoapySDRDevice_setGainElement_t)library_.resolve(
        "SoapySDRDevice_setGainElement");
    if (!SoapySDRDevice_setGainElement)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_setGainElement";
        return false;
    }

    SoapySDRDevice_getGain =
        (SoapySDRDevice_getGain_t)library_.resolve("SoapySDRDevice_getGain");
    if (!SoapySDRDevice_getGain)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getGain";
        return false;
    }

    SoapySDRDevice_getGainElement = (SoapySDRDevice_getGainElement_t)library_.resolve(
        "SoapySDRDevice_getGainElement");
    if (!SoapySDRDevice_getGainElement)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getGainElement";
        return false;
    }

    SoapySDRDevice_getGainRange =
        (SoapySDRDevice_getGainRange_t)library_.resolve("SoapySDRDevice_getGainRange");
    if (!SoapySDRDevice_getGainRange)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getGainRange";
        return false;
    }

    SoapySDRDevice_getGainElementRange =
        (SoapySDRDevice_getGainElementRange_t)library_.resolve(
            "SoapySDRDevice_getGainElementRange");
    if (!SoapySDRDevice_getGainElementRange)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getGainElementRange";
        return false;
    }

    SoapySDRDevice_getAntenna =
        (SoapySDRDevice_getAntenna_t)library_.resolve("SoapySDRDevice_getAntenna");
    if (!SoapySDRDevice_getAntenna)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getAntenna";
        return false;
    }

    SoapySDRDevice_listGains =
        (SoapySDRDevice_listGains_t)library_.resolve("SoapySDRDevice_listGains");
    if (!SoapySDRDevice_listGains)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_listGains";
        return false;
    }

    SoapySDRDevice_setAntenna =
        (SoapySDRDevice_setAntenna_t)library_.resolve("SoapySDRDevice_setAntenna");
    if (!SoapySDRDevice_setAntenna)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_setAntenna";
        return false;
    }

    SoapySDRDevice_getBandwidthRange =
        (SoapySDRDevice_getBandwidthRange_t)library_.resolve(
            "SoapySDRDevice_getBandwidthRange");
    if (!SoapySDRDevice_getBandwidthRange)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getBandwidthRange";
        return false;
    }

    SoapySDRDevice_getStreamMTU =
        (SoapySDRDevice_getStreamMTU_t)library_.resolve("SoapySDRDevice_getStreamMTU");
    if (!SoapySDRDevice_getStreamMTU)
    {
        qDebug() << "Could not get: "
                 << "SoapySDRDevice_getStreamMTU";
        return false;
    }

    SoapySDR_errToStr = (SoapySDR_errToStr_t)library_.resolve("SoapySDR_errToStr");
    if (!SoapySDR_errToStr)
    {
        qDebug() << "Could not get: "
                 << "SoapySDR_errToStr";
        return false;
    }

    return true;
}

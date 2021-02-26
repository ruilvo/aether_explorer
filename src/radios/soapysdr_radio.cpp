/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "soapysdr_radio.hpp"

#include "qdebug.h"
#include "soapysdr_widget.hpp"

#include <QDebug>
#include <QVersionNumber>

#include <algorithm>
#include <memory>

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
SoapySdrRadio::SoapySdrRadio()
    : sdr_(nullptr), worker_(nullptr), running_(false), channelCount_(0), channel_(0),
      centreFrequency_(SOAPY_INITIAL_CENTRE_FREQUENCY),
      sampleRate_(SOAPY_INITIAL_SAMPLE_RATE), bandwidth_(SOAPY_INITIAL_SAMPLE_RATE),
      agcAvailable_(false), agc_(false), globalGainRange_{0, 0, 0}, globalGain_(0),
      widget_(nullptr), library_(SOAPY_LIBRARY_NAME), initialised_(initialiseLibrary())
{
    if (!initialised_)
    {
        qDebug() << "Couldn't load the runtime library!";
        return;
    }

    QString soapyVersion{SoapySDR_getAPIVersion()};
    qDebug() << "SoapySDR API version: " << soapyVersion << ".";

    // NOLINTNEXTLINE(readability-magic-numbers)
    if (QVersionNumber::fromString(soapyVersion) < QVersionNumber{0, 8, 0})
    {
        qDebug() << " This module requires SoapySDR API version > 0.8.0.";
        initialised_ = false;
        return;
    }

    widget_ = std::make_unique<SoapySdrWidget>(this);

    discoverDevices();
}

SoapySdrRadio::~SoapySdrRadio()
{
    stop();
}

void SoapySdrRadio::discoverDevices()
{
    if (!initialised_)
    {
        qDebug() << "Function `discoverDevices` not called for failing to load DLL.";
        return;
    }

    if (running_)
    {
        qDebug() << "Can't discover devices while running.";
        return;
    }

    sdr_ = nullptr;

    size_t ndevs(0);
    auto *devs_found = SoapySDRDevice_enumerate(nullptr, &ndevs);

    devicesFound_.clear();
    devicesFound_.assign(devs_found, devs_found + ndevs);

    SoapySDRKwargsList_clear(devs_found, ndevs);

    widget_->devicesDiscovered();
}

void SoapySdrRadio::readDevice()
{
    if (!initialised_)
    {
        qDebug() << "Function `readDevice` not called for failing to load DLL.";
        return;
    }

    if (sdr_ == nullptr)
    {
        qDebug() << "No device to read.";
        return;
    }

    if (running_)
    {
        qDebug() << "Can't read devices while running.";
        return;
    }

    // Channels
    channelCount_ = SoapySDRDevice_getNumChannels(sdr_, SOAPY_SDR_RX);
    if (channel_ > channelCount_)
    {
        channel_ = 0;
    }

    // Antennas
    availableAntennas_.clear();
    size_t nant(0);
    auto *antennaCharList =
        SoapySDRDevice_listAntennas(sdr_, SOAPY_SDR_RX, channel_, &nant);
    for (auto j = 0U; j < nant; j++)
    {
        availableAntennas_.emplace_back(antennaCharList[j]);
    }

    antenna_ = QString{SoapySDRDevice_getAntenna(sdr_, SOAPY_SDR_RX, channel_)};

    // Frequency
    auto res = SoapySDRDevice_setFrequency(sdr_, SOAPY_SDR_RX, channel_, centreFrequency_,
                                           nullptr);
    if (res != 0)
    {
        qDebug() << "SoapySDRDevice_setFrequency failed with error: "
                 << SoapySDRDevice_lastError();
    }

    // Sample rate
    supportedSampleRatesDiscrete_.clear();
    supportedSampleRatesRanges_.clear();

    size_t nfss(0);
    auto *fsranges =
        SoapySDRDevice_getSampleRateRange(sdr_, SOAPY_SDR_RX, channel_, &nfss);
    for (auto j = 0U; j < nfss; j++)
    {
        if (fsranges[j].minimum == fsranges[j].maximum)
        {
            supportedSampleRatesDiscrete_.push_back(fsranges[j].minimum);
        }
        else
        {
            supportedSampleRatesRanges_.push_back(fsranges[j]);
        }
    }

    if (!validateSampleRate(sampleRate_))
    {
        if (!supportedSampleRatesDiscrete_.empty())
        {
            auto idx = static_cast<int>(
                std::round((supportedSampleRatesDiscrete_.size() - 1) / 2));
            sampleRate_ = supportedSampleRatesDiscrete_[idx];
        }
        else if (!supportedSampleRatesRanges_.empty())
        {
            sampleRate_ = supportedSampleRatesRanges_[0].minimum;
        }
    }

    for (const auto &listener : listeners_)
    {
        listener->setSampleRate(sampleRate_);
    }

    // Bandwidth
    supportedBandwidthsDiscrete_.clear();
    supportedBandwidthsRanges_.clear();

    size_t nbws(0);
    auto *bwranges =
        SoapySDRDevice_getBandwidthRange(sdr_, SOAPY_SDR_RX, channel_, &nbws);
    for (auto j = 0U; j < nbws; j++)
    {
        if (bwranges[j].minimum == bwranges[j].maximum)
        {
            supportedBandwidthsDiscrete_.push_back(bwranges[j].minimum);
        }
        else
        {
            supportedBandwidthsRanges_.push_back(bwranges[j]);
        }
    }

    readjustBandwidth();

    // AGC
    agcAvailable_ = SoapySDRDevice_hasGainMode(sdr_, SOAPY_SDR_RX, channel_);
    agc_ = SoapySDRDevice_getGainMode(sdr_, SOAPY_SDR_RX, channel_);

    // Global gain
    globalGainRange_ = SoapySDRDevice_getGainRange(sdr_, SOAPY_SDR_RX, channel_);
    globalGain_ = SoapySDRDevice_getGain(sdr_, SOAPY_SDR_RX, channel_);

    // Specific gains
    specificGainsRanges_.clear();
    specificGains_.clear();
    size_t ngainstages(0);
    auto *gainNames =
        SoapySDRDevice_listGains(sdr_, SOAPY_SDR_RX, channel_, &ngainstages);
    for (auto j = 0U; j < ngainstages; j++)
    {
        specificGainsRanges_[gainNames[j]] = SoapySDRDevice_getGainElementRange(
            sdr_, SOAPY_SDR_RX, channel_, gainNames[j]);
        specificGains_[gainNames[j]] =
            SoapySDRDevice_getGainElement(sdr_, SOAPY_SDR_RX, channel_, gainNames[j]);
    }

    widget_->deviceRead();
}

void SoapySdrRadio::makeDevice(const QString &sourceString)
{
    if (!initialised_)
    {
        qDebug() << "Function `makeDevice` not called for failing to load DLL.";
        return;
    }

    if (running_)
    {
        qDebug() << "Can't change the SDR while running!";
        return;
    }

    if (sdr_ != nullptr)
    {
        qDebug() << "Deleting previous SDR";
        unmakeDevice();
    }

    // Try to make the device. This can fail because I'll allow the widget to give invalid
    // input.
    sdr_ = SoapySDRDevice_makeStrArgs(sourceString.toLocal8Bit());
    if (sdr_ == nullptr)
    {
        qDebug() << "SoapySDRDevice_makeStrArgs failed: " << SoapySDRDevice_lastError();
        return;
    }

    readDevice();
}

void SoapySdrRadio::unmakeDevice()
{
    if (!initialised_)
    {
        qDebug() << "Function `unmakeDevice` not called for failing to load DLL.";
        return;
    }

    if (running_)
    {
        qDebug() << "Can't delete the SDR while running!";
        return;
    }

    if (sdr_ == nullptr)
    {
        qDebug() << "No SDR to destroy!";
        return;
    }

    auto err = SoapySDRDevice_unmake(sdr_);
    if (err != 0)
    {
        qDebug() << "SoapySDRDevice_unmake fail: " << SoapySDRDevice_lastError();
    }

    sdr_ = nullptr;

    widget_->deviceDestroyed();
}

void SoapySdrRadio::worker()
{
    // NOLINTNEXTLINE: Avoid C-arrays and etc, but I *need* them.
    size_t channelList[1] = {channel_};

    auto *rxStream = SoapySDRDevice_setupStream(sdr_, SOAPY_SDR_RX, SOAPY_SDR_CF32,
                                                // NOLINTNEXTLINE: Cast pointer to array
                                                channelList, 1, nullptr);
    if (rxStream == nullptr)
    {
        qDebug() << "SoapySDRDevice_setupStream failed with error: "
                 << SoapySDRDevice_lastError();
        return;
    }

    auto err = SoapySDRDevice_activateStream(sdr_, rxStream, 0, 0, 0);
    if (err != 0)
    {
        qDebug() << "SoapySDRDevice_activateStream failed with error: "
                 << SoapySDRDevice_lastError();
    }

    auto bufferSize = SoapySDRDevice_getStreamMTU(sdr_, rxStream);
    std::vector<std::complex<float>> sampleBuffer(bufferSize);

    // NOLINTNEXTLINE: Avoid C-arrays and etc, but I *need* them.
    std::complex<float> *buffer_data[1]{sampleBuffer.data()};

    int flags = 0;
    long long timeNs = 0;
    int samplesWrittenOrError = 0;

    // Sync up the listeners
    for (const auto &listener : listeners_)
    {
        listener->setSampleRate(sampleRate_);
        listener->setCentreFrequency(centreFrequency_);
    }

    while (running_)
    {
        samplesWrittenOrError = SoapySDRDevice_readStream(
            sdr_, rxStream, reinterpret_cast<void **>(buffer_data), bufferSize, &flags,
            &timeNs, SOAPY_FRAME_TIMEOUT);

        if (samplesWrittenOrError < 0)
        {
            qDebug() << "SoapySDRDevice_readStream failed with error: "
                     << SoapySDR_errToStr(samplesWrittenOrError);
            continue;
        }

        for (const auto &listener : listeners_)
        {
            listener->receiveSamples(sampleBuffer);
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

void SoapySdrRadio::start()
{
    if (!initialised_)
    {
        qDebug() << "Function `start` not called for failing to load DLL.";
        return;
    }

    if (running_)
    {
        qDebug() << "Already running!";
        return;
    }

    if (sdr_ == nullptr)
    {
        qDebug() << "No SDR to start.";
        return;
    }

    running_ = true;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    worker_ = new std::thread(&SoapySdrRadio::worker, this);

    widget_->deviceStarted();
}

void SoapySdrRadio::stop()
{
    if (!initialised_)
    {
        qDebug() << "Function `stop` not called for failing to load DLL.";
        return;
    }

    if (!running_)
    {
        qDebug() << "Already not running!";
        return;
    }

    if (sdr_ == nullptr)
    {
        qDebug() << "No SDR to stop!";
        return;
    }

    running_ = false;

    if (worker_ != nullptr)
    {
        worker_->join();
        delete worker_;
    }

    worker_ = nullptr;

    widget_->deviceStopped();
}

void SoapySdrRadio::setChannel(size_t channel)
{
    if (!initialised_)
    {
        qDebug() << "Function `setChannel` not called for failing to load DLL.";
        return;
    }

    if (running_)
    {
        qDebug() << "Can't change channel to a running device!";
        return;
    }

    if (channel >= channelCount_ || channel == channel_)
    {
        return;
    }

    channel_ = channel;
}

void SoapySdrRadio::setAntenna(QString &antenna)
{
    if (!initialised_)
    {
        qDebug() << "Function `setAntenna` not called for failing to load DLL.";
        return;
    }

    if (std::none_of(availableAntennas_.begin(), availableAntennas_.end(),
                     [&](const QString &ant) { return ant == antenna; }))
    {
        qDebug() << "Didn't find the correct antenna: " << antenna;
        return;
    }

    antenna_ = antenna;

    if (sdr_ != nullptr)
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

void SoapySdrRadio::setCentreFrequency(double centreFrequency)
{
    if (!initialised_)
    {
        qDebug() << "Function `setCentreFrequency` not called for failing to load DLL.";
        return;
    }

    centreFrequency_ = centreFrequency;

    if (sdr_ != nullptr)
    {
        auto res = SoapySDRDevice_setFrequency(
            sdr_, SOAPY_SDR_RX, channel_, static_cast<double>(centreFrequency_), nullptr);
        if (res != 0)
        {
            qDebug() << "SoapySDRDevice_setFrequency failed with error: "
                     << SoapySDRDevice_lastError();
        }
    }
}

bool SoapySdrRadio::validateSampleRate(double sampleRate)
{
    if (std::any_of(supportedSampleRatesDiscrete_.begin(),
                    supportedSampleRatesDiscrete_.end(),
                    [&](double sr) { return sr == sampleRate; }))
    {
        return true;
    }

    return std::any_of(
        supportedSampleRatesRanges_.begin(), supportedSampleRatesRanges_.end(),
        [&](const SoapySDRRange &currSrRange) {
            return currSrRange.minimum < sampleRate && currSrRange.maximum > sampleRate;
        });
}

void SoapySdrRadio::setSampleRate(double sampleRate)
{
    if (!initialised_)
    {
        qDebug() << "Function `setSampleRate` not called for failing to load DLL.";
        return;
    }

    if (running_)
    {
        qDebug() << "Can't set the sample rate to a running device";
        return;
    }

    if (sdr_ == nullptr)
    {
        qDebug() << "No radio to set sample rate to!";
        return;
    }

    if (!validateSampleRate(sampleRate))
    {
        qDebug() << "Failed to set sample rate because of invalid input.";
        return;
    }

    sampleRate_ = sampleRate;

    auto res = SoapySDRDevice_setSampleRate(sdr_, SOAPY_SDR_RX, channel_, sampleRate_);
    if (res != 0)
    {
        qDebug() << "SoapySDRDevice_setSampleRate failed with error: "
                 << SoapySDRDevice_lastError();
        sampleRate_ = SoapySDRDevice_getSampleRate(sdr_, SOAPY_SDR_RX, channel_);
        widget_->syncUi();
        return;
    }

    readjustBandwidth();
}

bool SoapySdrRadio::validateBandwidth(double bandwidth)
{
    if (std::any_of(supportedBandwidthsDiscrete_.begin(),
                    supportedBandwidthsDiscrete_.end(),
                    [&](double bw) { return bw == bandwidth; }))
    {
        return true;
    }

    return std::any_of(
        supportedBandwidthsRanges_.begin(), supportedBandwidthsRanges_.end(),
        [&](const SoapySDRRange &currBwRange) {
            return currBwRange.minimum < bandwidth && currBwRange.maximum > bandwidth;
        });
}

void SoapySdrRadio::readjustBandwidth()
{
    if (!supportedBandwidthsRanges_.empty())
    {
        for (const auto &currBwRange : supportedBandwidthsRanges_)
        {
            if (sampleRate_ >= currBwRange.minimum && sampleRate_ <= currBwRange.maximum)
            {
                setBandwidth(sampleRate_);
                break;
            }
            if (sampleRate_ > currBwRange.maximum)
            {
                setBandwidth(currBwRange.maximum);
            }
            else if (sampleRate_ < currBwRange.minimum)
            {
                setBandwidth(currBwRange.minimum + 1); // I forgot why this +1?
            }
        }
    }
    if (!supportedBandwidthsDiscrete_.empty())
    {
        for (auto i = 0U; i < getSupportedBandwidthsDiscrete().size(); i++)
        {
            if (i + 1 == getSupportedBandwidthsDiscrete().size() ||
                getSupportedBandwidthsDiscrete()[i + 1] > sampleRate_)
            {
                setBandwidth(getSupportedBandwidthsDiscrete()[i]);
                break;
            }
        }
    }
}

void SoapySdrRadio::setBandwidth(double bandwidth)
{
    if (!initialised_)
    {
        qDebug() << "Function `setBandwidth` not called for failing to load DLL.";
        return;
    }

    if (sdr_ == nullptr)
    {
        qDebug() << "No radio to set bandwidth to!";
        return;
    }

    if (!validateBandwidth(bandwidth))
    {
        qDebug() << "Failed to set bandwidth because of invalid input.";
        return;
    }

    bandwidth_ = bandwidth;

    auto res = SoapySDRDevice_setBandwidth(sdr_, SOAPY_SDR_RX, channel_, bandwidth_);
    if (res != 0)
    {
        qDebug() << "SoapySDRDevice_setBandwidth failed with error: "
                 << SoapySDRDevice_lastError();
        bandwidth_ = SoapySDRDevice_getBandwidth(sdr_, SOAPY_SDR_RX, channel_);
        widget_->syncUi();
        return;
    }
}

void SoapySdrRadio::setAgc(bool state)
{
    if (!initialised_)
    {
        qDebug() << "Function `setAgc` not called for failing to load DLL.";
        return;
    }

    if (sdr_ == nullptr)
    {
        qDebug() << "No radio to set AGC to!";
        return;
    }

    agc_ = state;
    auto res = SoapySDRDevice_setGainMode(sdr_, SOAPY_SDR_RX, channel_, agc_);
    if (res != 0)
    {
        qDebug() << "SoapySDRDevice_setGainMode failed with error: "
                 << SoapySDRDevice_lastError();
        agc_ = SoapySDRDevice_getGainMode(sdr_, SOAPY_SDR_RX, channel_);
        widget_->syncUi();
    }
}

void SoapySdrRadio::setGlobalGain(double gain)
{
    if (!initialised_)
    {
        qDebug() << "Function `setGlobalGain` not called for failing to load DLL.";
        return;
    }

    if (sdr_ == nullptr)
    {
        qDebug() << "No radio to set gain to!";
        return;
    }

    if (gain > globalGainRange_.maximum || gain < globalGainRange_.minimum)
    {
        qDebug() << "Gain out of range!";
        return;
    }

    globalGain_ = gain;

    auto res = SoapySDRDevice_setGain(sdr_, SOAPY_SDR_RX, channel_, globalGain_);
    if (res != 0)
    {
        qDebug() << "SoapySDRDevice_setGain failed with error: "
                 << SoapySDRDevice_lastError();
    }

    for (const auto &gainElem : specificGains_)
    {
        auto name = gainElem.first.toLocal8Bit();
        specificGains_[gainElem.first] =
            SoapySDRDevice_getGainElement(sdr_, SOAPY_SDR_RX, channel_, name);
    }
    widget_->syncUi();
}

void SoapySdrRadio::setSpecificGain(QString name, double value)
{
    if (!initialised_)
    {
        qDebug() << "Function `setSpecificGain` not called for failing to load DLL.";
        return;
    }

    if (sdr_ == nullptr)
    {
        qDebug() << "No radio to set gain to!";
        return;
    }

    // Check if gain is in the list:
    if (specificGainsRanges_.count(name) == 0)
    {
        qDebug() << "Gain with this name doesn't exist";
        return;
    }

    if (value > specificGainsRanges_[name].maximum ||
        value < specificGainsRanges_[name].minimum)
    {
        qDebug() << "Gain out of range!";
        return;
    }

    specificGains_[name] = value;

    SoapySDRDevice_setGainElement(sdr_, SOAPY_SDR_RX, channel_, name.toLocal8Bit(),
                                  value);

    globalGain_ = SoapySDRDevice_getGain(sdr_, SOAPY_SDR_RX, channel_);
    for (const auto &gainElem : specificGains_)
    {
        auto name = gainElem.first.toLocal8Bit();
        specificGains_[gainElem.first] =
            SoapySDRDevice_getGainElement(sdr_, SOAPY_SDR_RX, channel_, name);
    }
    widget_->syncUi();
}

#define SOAPY_LOAD_LIBRARY_FUNCION(funcname)                                      \
    {                                                                             \
        (funcname) = reinterpret_cast<funcname##_t>(library_.resolve(#funcname)); \
        if (!(funcname))                                                          \
        {                                                                         \
            qDebug() << "Could not get: " << #funcname;                           \
            return false;                                                         \
        }                                                                         \
    }

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool SoapySdrRadio::initialiseLibrary()
{
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDR_getAPIVersion);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDR_getAPIVersion);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_enumerate);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRKwargsList_clear);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRKwargs_toString);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRKwargs_get);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRKwargs_fromString);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_make);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_makeStrArgs);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_lastError);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_setFrequency);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getSampleRate);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_unmake);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getSampleRateRange);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_setSampleRate);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_listSampleRates);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_setBandwidth);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getBandwidth);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_listBandwidths);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_setupStream);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_activateStream);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_deactivateStream);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_closeStream);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_readStream);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getNumChannels);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getChannelInfo);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_listAntennas);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_hasGainMode);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_setGainMode);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getGainMode);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_setGain);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_setGainElement);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getGain);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getGainElement);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getGainRange);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getGainElementRange);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getAntenna);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_listGains);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_setAntenna);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getBandwidthRange);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDRDevice_getStreamMTU);
    SOAPY_LOAD_LIBRARY_FUNCION(SoapySDR_errToStr);
}

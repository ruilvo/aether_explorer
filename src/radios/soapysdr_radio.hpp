/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-only
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include "ISource.hpp"
#include "soapysdr_types.hpp"
#include "soapysdr_widget.hpp"

#include <QLibrary>
#include <QString>
#include <QWidget>

#include <complex>
#include <memory>
#include <thread>
#include <vector>

class SoapySdrRadio : public ISource
{
  public:
    SoapySdrRadio();
    ~SoapySdrRadio() override;
    SoapySdrRadio(const SoapySdrRadio &) = delete;
    SoapySdrRadio &operator=(const SoapySdrRadio &) = delete;

    // Device ----------------------------------------------------------------------------
  private:
    std::vector<SoapySDRKwargs> devicesFound_;
    std::map<QString, QString> deviceStrings_; // Label: kwargs
    void discoverDevices();

    SoapySDRDevice *sdr_;
    void readDevice();

  public:
    void makeDevice(const QString &sourceString);
    void unmakeDevice();
    [[nodiscard]] std::map<QString, QString> getDeviceStrings() const
    {
        return deviceStrings_;
    };

    // Acquisition -----------------------------------------------------------------------
  private:
    // Samples
    std::thread *worker_;
    bool running_;
    void worker();

  public:
    void start() override;
    void stop() override;

    // Channels --------------------------------------------------------------------------
  private:
    size_t channelCount_;
    size_t channel_;

  public:
    [[nodiscard]] size_t getChannelCount() const
    {
        return channelCount_;
    };
    [[nodiscard]] size_t getChannel() const
    {
        return channel_;
    };
    void setChannel(size_t channel);

    // Antennas --------------------------------------------------------------------------
  private:
    std::vector<QString> availableAntennas_;
    QString antenna_;

  public:
    [[nodiscard]] std::vector<QString> getSupportedAntennas() const
    {
        return availableAntennas_;
    };
    [[nodiscard]] QString getAntenna() const
    {
        return antenna_;
    };
    void setAntenna(const QString &antenna);

    // Frequency -------------------------------------------------------------------------
  private:
    double centreFrequency_;

  public:
    double getCentreFrequency() override
    {
        return centreFrequency_;
    };
    void setCentreFrequency(double centreFrequency) override;

    // Sample rate -----------------------------------------------------------------------
  private:
    std::vector<double> supportedSampleRatesDiscrete_;
    std::vector<SoapySDRRange> supportedSampleRatesRanges_;
    double sampleRate_;
    bool validateSampleRate(double sampleRate);

  public:
    [[nodiscard]] std::vector<double> getSupportedSampleRatesDiscrete() const
    {
        return supportedSampleRatesDiscrete_;
    };
    [[nodiscard]] std::vector<SoapySDRRange> getSupportedSampleRatesRanges() const
    {
        return supportedSampleRatesRanges_;
    };
    double getSampleRate() override
    {
        return sampleRate_;
    };
    void setSampleRate(double sampleRate);

    // Bandwidth -------------------------------------------------------------------------
  private:
    std::vector<double> supportedBandwidthsDiscrete_;
    std::vector<SoapySDRRange> supportedBandwidthsRanges_;
    double bandwidth_;
    bool validateBandwidth(double bandwidth);
    void readjustBandwidth();

  public:
    [[nodiscard]] std::vector<double> getSupportedBandwidthsDiscrete() const
    {
        return supportedBandwidthsDiscrete_;
    };
    [[nodiscard]] std::vector<SoapySDRRange> getSupportedBandwidthsRanges() const
    {
        return supportedBandwidthsRanges_;
    };
    [[nodiscard]] double getBandwidth() const
    {
        return bandwidth_;
    };
    void setBandwidth(double bandwidth);

    // AGC -------------------------------------------------------------------------------
  private:
    bool agcAvailable_;
    bool agc_;

  public:
    [[nodiscard]] bool hasAgc() const
    {
        return agcAvailable_;
    };
    [[nodiscard]] bool getAgc() const
    {
        return agc_;
    };
    void setAgc(bool state);

    // Global gain -----------------------------------------------------------------------
  private:
    SoapySDRRange globalGainRange_;
    double globalGain_;

  public:
    [[nodiscard]] SoapySDRRange getGlobalGainRange() const
    {
        return globalGainRange_;
    };
    [[nodiscard]] double getGlobalGain() const
    {
        return globalGain_;
    };
    void setGlobalGain(double gain);

    // Specific gains --------------------------------------------------------------------
  private:
    std::map<QString, SoapySDRRange> specificGainsRanges_;
    std::map<QString, double> specificGains_;

  public:
    [[nodiscard]] std::map<QString, SoapySDRRange> getSpecificGainRanges() const
    {
        return specificGainsRanges_;
    };
    [[nodiscard]] std::map<QString, double> getSpecificGains() const
    {
        return specificGains_;
    };
    void setSpecificGain(const QString &name, double value);

    // Widget ----------------------------------------------------------------------------
  private:
    std::unique_ptr<SoapySdrWidget> widget_; // Inherits from QWidget
  public:
    QWidget *getWidget() override
    {
        return static_cast<QWidget *>(widget_.get());
    };

    // Runtime library -------------------------------------------------------------------
  private:
    QLibrary library_;
    bool initialised_;
    bool initialiseLibrary();

    // Functions -------------------------------------------------------------------------
  private: // NOLINT(readability-redundant-access-specifiers)
    SoapySDR_getAPIVersion_t SoapySDR_getAPIVersion;
    SoapySDRDevice_enumerate_t SoapySDRDevice_enumerate;
    SoapySDRKwargsList_clear_t SoapySDRKwargsList_clear;
    SoapySDRKwargs_toString_t SoapySDRKwargs_toString;
    SoapySDRKwargs_get_t SoapySDRKwargs_get;
    SoapySDRKwargs_fromString_t SoapySDRKwargs_fromString;
    SoapySDRDevice_make_t SoapySDRDevice_make;
    SoapySDRDevice_makeStrArgs_t SoapySDRDevice_makeStrArgs;
    SoapySDRDevice_lastError_t SoapySDRDevice_lastError;
    SoapySDRDevice_setFrequency_t SoapySDRDevice_setFrequency;
    SoapySDRDevice_getSampleRate_t SoapySDRDevice_getSampleRate;
    SoapySDRDevice_unmake_t SoapySDRDevice_unmake;
    SoapySDRDevice_getSampleRateRange_t SoapySDRDevice_getSampleRateRange;
    SoapySDRDevice_setSampleRate_t SoapySDRDevice_setSampleRate;
    SoapySDRDevice_listSampleRates_t SoapySDRDevice_listSampleRates;
    SoapySDRDevice_setBandwidth_t SoapySDRDevice_setBandwidth;
    SoapySDRDevice_getBandwidth_t SoapySDRDevice_getBandwidth;
    SoapySDRDevice_listBandwidths_t SoapySDRDevice_listBandwidths;
    SoapySDRDevice_setupStream_t SoapySDRDevice_setupStream;
    SoapySDRDevice_activateStream_t SoapySDRDevice_activateStream;
    SoapySDRDevice_deactivateStream_t SoapySDRDevice_deactivateStream;
    SoapySDRDevice_closeStream_t SoapySDRDevice_closeStream;
    SoapySDRDevice_readStream_t SoapySDRDevice_readStream;
    SoapySDRDevice_getNumChannels_t SoapySDRDevice_getNumChannels;
    SoapySDRDevice_getChannelInfo_t SoapySDRDevice_getChannelInfo;
    SoapySDRDevice_listAntennas_t SoapySDRDevice_listAntennas;
    SoapySDRDevice_listGains_t SoapySDRDevice_listGains;
    SoapySDRDevice_hasGainMode_t SoapySDRDevice_hasGainMode;
    SoapySDRDevice_setGainMode_t SoapySDRDevice_setGainMode;
    SoapySDRDevice_getGainMode_t SoapySDRDevice_getGainMode;
    SoapySDRDevice_setGain_t SoapySDRDevice_setGain;
    SoapySDRDevice_setGainElement_t SoapySDRDevice_setGainElement;
    SoapySDRDevice_getGain_t SoapySDRDevice_getGain;
    SoapySDRDevice_getGainElement_t SoapySDRDevice_getGainElement;
    SoapySDRDevice_getGainRange_t SoapySDRDevice_getGainRange;
    SoapySDRDevice_getGainElementRange_t SoapySDRDevice_getGainElementRange;
    SoapySDRDevice_getAntenna_t SoapySDRDevice_getAntenna;
    SoapySDRDevice_setAntenna_t SoapySDRDevice_setAntenna;
    SoapySDRDevice_getBandwidthRange_t SoapySDRDevice_getBandwidthRange;
    SoapySDRDevice_getStreamMTU_t SoapySDRDevice_getStreamMTU;
    SoapySDR_errToStr_t SoapySDR_errToStr;
    SoapySDRDevice_getHardwareInfo_t SoapySDRDevice_getHardwareInfo;
};

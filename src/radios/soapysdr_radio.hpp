/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include "ISource.hpp"
#include "soapysdr_widget.hpp"

#include <QLibrary>
#include <QString>
#include <QWidget>

#include <complex>
#include <thread>
#include <vector>

#define SOAPY_LIBRARY_NAME "SoapySDR"
#define SOAPY_SDR_RX 1
#define SOAPY_SDR_CF32 "CF32"
#define SOAPY_INITIAL_CENTRE_FREQUENCY 100'000'000.0
#define SOAPY_INITIAL_SAMPLE_RATE 2'000'000.0
#define SOAPY_FRAME_TIMEOUT 500000

typedef struct
{
    size_t size;
    char **keys;
    char **vals;
} SoapySDRKwargs;

typedef struct
{
    double minimum;
    double maximum;
    double step;
} SoapySDRRange;

typedef struct SoapySDRDevice SoapySDRDevice;
typedef struct SoapySDRStream SoapySDRStream;

typedef const char *(*SoapySDR_getAPIVersion_t)(void);

typedef SoapySDRKwargs *(*SoapySDRDevice_enumerate_t)(const SoapySDRKwargs *args,
                                                      size_t *length);
typedef void (*SoapySDRKwargsList_clear_t)(SoapySDRKwargs *args, const size_t length);
typedef char *(*SoapySDRKwargs_toString_t)(const SoapySDRKwargs *args);
typedef const char *(*SoapySDRKwargs_get_t)(const SoapySDRKwargs *args, const char *key);
typedef SoapySDRKwargs (*SoapySDRKwargs_fromString_t)(const char *markup);
typedef SoapySDRDevice *(*SoapySDRDevice_make_t)(const SoapySDRKwargs *args);
typedef SoapySDRDevice *(*SoapySDRDevice_makeStrArgs_t)(const char *args);
typedef const char *(*SoapySDRDevice_lastError_t)(void);
typedef int (*SoapySDRDevice_unmake_t)(SoapySDRDevice *device);
typedef int (*SoapySDRDevice_setFrequency_t)(SoapySDRDevice *device, const int direction,
                                             const size_t channel, const double frequency,
                                             const SoapySDRKwargs *args);
typedef double (*SoapySDRDevice_getSampleRate_t)(const SoapySDRDevice *device,
                                                 const int direction,
                                                 const size_t channel);
typedef SoapySDRRange *(*SoapySDRDevice_getSampleRateRange_t)(
    const SoapySDRDevice *device, const int direction, const size_t channel,
    size_t *length);
typedef int (*SoapySDRDevice_setSampleRate_t)(SoapySDRDevice *device, const int direction,
                                              const size_t channel, const double rate);
typedef double *(*SoapySDRDevice_listSampleRates_t)(const SoapySDRDevice *device,
                                                    const int direction,
                                                    const size_t channel, size_t *length);
typedef int (*SoapySDRDevice_setBandwidth_t)(SoapySDRDevice *device, const int direction,
                                             const size_t channel, const double bw);
typedef double (*SoapySDRDevice_getBandwidth_t)(const SoapySDRDevice *device,
                                                const int direction,
                                                const size_t channel);
typedef double *(*SoapySDRDevice_listBandwidths_t)(const SoapySDRDevice *device,
                                                   const int direction,
                                                   const size_t channel, size_t *length);
typedef SoapySDRStream *(*SoapySDRDevice_setupStream_t)(
    SoapySDRDevice *device, const int direction, const char *format,
    const size_t *channels, const size_t numChans, const SoapySDRKwargs *args);
typedef int (*SoapySDRDevice_activateStream_t)(SoapySDRDevice *device,
                                               SoapySDRStream *stream, const int flags,
                                               const long long timeNs,
                                               const size_t numElems);
typedef int (*SoapySDRDevice_deactivateStream_t)(SoapySDRDevice *device,
                                                 SoapySDRStream *stream, const int flags,
                                                 const long long timeNs);
typedef int (*SoapySDRDevice_closeStream_t)(SoapySDRDevice *device,
                                            SoapySDRStream *stream);
typedef int (*SoapySDRDevice_readStream_t)(SoapySDRDevice *device, SoapySDRStream *stream,
                                           void *const *buffs, const size_t numElems,
                                           int *flags, long long *timeNs,
                                           const long timeoutUs);
typedef size_t (*SoapySDRDevice_getNumChannels_t)(const SoapySDRDevice *device,
                                                  const int direction);
typedef SoapySDRKwargs (*SoapySDRDevice_getChannelInfo_t)(const SoapySDRDevice *device,
                                                          const int direction,
                                                          const size_t channel);
typedef char **(*SoapySDRDevice_listAntennas_t)(const SoapySDRDevice *device,
                                                const int direction, const size_t channel,
                                                size_t *length);
typedef char **(*SoapySDRDevice_listGains_t)(const SoapySDRDevice *device,
                                             const int direction, const size_t channel,
                                             size_t *length);
typedef bool (*SoapySDRDevice_hasGainMode_t)(const SoapySDRDevice *device,
                                             const int direction, const size_t channel);
typedef int (*SoapySDRDevice_setGainMode_t)(SoapySDRDevice *device, const int direction,
                                            const size_t channel, const bool automatic);
typedef bool (*SoapySDRDevice_getGainMode_t)(const SoapySDRDevice *device,
                                             const int direction, const size_t channel);
typedef int (*SoapySDRDevice_setGain_t)(SoapySDRDevice *device, const int direction,
                                        const size_t channel, const double value);
typedef int (*SoapySDRDevice_setGainElement_t)(SoapySDRDevice *device,
                                               const int direction, const size_t channel,
                                               const char *name, const double value);
typedef double (*SoapySDRDevice_getGain_t)(const SoapySDRDevice *device,
                                           const int direction, const size_t channel);
typedef double (*SoapySDRDevice_getGainElement_t)(const SoapySDRDevice *device,
                                                  const int direction,
                                                  const size_t channel, const char *name);
typedef SoapySDRRange (*SoapySDRDevice_getGainRange_t)(const SoapySDRDevice *device,
                                                       const int direction,
                                                       const size_t channel);
typedef SoapySDRRange (*SoapySDRDevice_getGainElementRange_t)(
    const SoapySDRDevice *device, const int direction, const size_t channel,
    const char *name);
typedef char *(*SoapySDRDevice_getAntenna_t)(const SoapySDRDevice *device,
                                             const int direction, const size_t channel);
typedef int (*SoapySDRDevice_setAntenna_t)(SoapySDRDevice *device, const int direction,
                                           const size_t channel, const char *name);
typedef SoapySDRRange *(*SoapySDRDevice_getBandwidthRange_t)(const SoapySDRDevice *device,
                                                             const int direction,
                                                             const size_t channel,
                                                             size_t *length);
typedef size_t (*SoapySDRDevice_getStreamMTU_t)(const SoapySDRDevice *device,
                                                SoapySDRStream *stream);
typedef const char *(*SoapySDR_errToStr_t)(const int errorCode);

class SoapySdrRadio : public ISource
{
  public:
    SoapySdrRadio();
    ~SoapySdrRadio() override;
    SoapySdrRadio(const SoapySdrRadio &) = delete;
    SoapySdrRadio &operator=(const SoapySdrRadio &) = delete;

    // From ISource
    void start() override;
    void stop() override;

    double getCentreFrequency() override;
    void setCentreFrequency(double centreFrequency) override;
    double getSampleRate() override;

    QWidget *getWidget() override;
    // --------------

    // Devices
    std::vector<QString> getDeviceStrings();
    std::vector<QString> getDeviceNames();
    QString getDeviceString();
    void setDeviceString(const QString &sourceString);

    // Channels
    size_t getChannelCount();
    size_t getChannel();
    void setChannel(size_t channel);

    // Antennas
    std::vector<QString> getSupportedAntennas();
    QString getAntenna();
    void setAntenna(QString antenna);

    // Sample rate
    std::vector<double> getSupportedSampleRatesDiscrete();
    std::vector<SoapySDRRange> getSupportedSampleRatesRanges();
    void setSampleRate(double sampleRate);

    // Bandwidth
    std::vector<double> getSupportedBandwidthsDiscrete();
    std::vector<SoapySDRRange> getSupportedBandwidthsRanges();
    double getBandwidth();
    void setBandwidth(double bandwidth);

    // AGC
    bool hasAgc();
    bool getAgc();
    void setAgc(bool state);

    // Global gain
    SoapySDRRange getGlobalGainRange();
    double getGlobalGain();
    void setGlobalGain(double gain);

    // Specific gains
    std::vector<QString> getSpecificGainNames();
    std::vector<SoapySDRRange> getSpecificGainRanges();
    std::vector<double> getSpecificGains();
    void setSpecificGain(QString name, double value);

  private:
    void resetParameters();

    // Devices
    size_t ndevices_;
    SoapySDRKwargs *devicesFound_;
    void enumerateDevices();

    std::vector<QString> deviceStrings_;
    std::vector<QString> deviceNames_;
    void nameDevices();
    void makeDeviceStrings();

    QString deviceString_;
    SoapySDRDevice *sdr_;
    bool makeDevice();
    bool unmakeDevice();

    // Channels
    size_t numChannels_;
    size_t channel_;
    void getNumAvailableChannels();

    // Antennas
    std::vector<QString> availableAntennas_;
    QString antenna_;
    void getAvailableAntennas();

    // Centre frequency
    double centreFrequency_;
    void initCentreFrequency();

    // Sample rate
    std::vector<double> supportedSampleRatesDiscrete_;
    std::vector<SoapySDRRange> supportedSampleRatesRanges_;
    double sampleRate_;
    bool validateSampleRate(double sampleRate);
    void getAvailableSampleRates();
    void setDeviceSampleRate();

    std::vector<double> supportedBandwidthsDiscrete_;
    std::vector<SoapySDRRange> supportedBandwidthsRanges_;
    double bandwidth_;
    bool validateBandwidth(double bandwidth);
    void getAvailableBandwidths();
    void recomputeBandwidth();

    // AGC
    bool agcAvailable_;
    bool agc_;
    void getAgcAvailability();

    // Global gain
    SoapySDRRange globalGainRange_;
    double globalGain_;
    void getAvailableGlobalGainRange();

    // Specific gains
    std::vector<QString> specificGainNames_;
    std::vector<SoapySDRRange> specificGainRanges_;
    std::vector<double> specificGains_;
    void getAvailableSeparateGainsRanges();

    // Samples
    std::vector<std::complex<float>> sampleBuffer_;
    std::thread *worker_;
    void worker();
    void startWorker();
    void stopWorker();

    // Control variables
    bool running_;

    // Runtime library
    QLibrary library_;
    bool initialised_;
    bool initialiseLibrary();

    // Qt Widget
    std::unique_ptr<SoapySdrWidget> widget_;

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
};

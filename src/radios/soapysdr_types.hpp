/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#define SOAPY_LIBRARY_NAME "SoapySDR"
#define SOAPY_SDR_RX 1
#define SOAPY_SDR_CF32 "CF32"
#define SOAPY_INITIAL_CENTRE_FREQUENCY 100'000'000.0
#define SOAPY_INITIAL_SAMPLE_RATE 2'000'000.0
#define SOAPY_FRAME_TIMEOUT 500000

struct SoapySDRKwargs
{
    size_t size;
    char **keys;
    char **vals;
};

struct SoapySDRRange
{
    double minimum;
    double maximum;
    double step;
};

using SoapySDRDevice = struct SoapySDRDevice;
using SoapySDRStream = struct SoapySDRStream;

using SoapySDR_getAPIVersion_t = const char *(*)();
using SoapySDRDevice_enumerate_t = SoapySDRKwargs *(*)(const SoapySDRKwargs *, size_t *);
using SoapySDRKwargsList_clear_t = void (*)(SoapySDRKwargs *, const size_t);
using SoapySDRKwargs_toString_t = char *(*)(const SoapySDRKwargs *);
using SoapySDRKwargs_get_t = const char *(*)(const SoapySDRKwargs *, const char *);
using SoapySDRKwargs_fromString_t = SoapySDRKwargs (*)(const char *);
using SoapySDRDevice_make_t = SoapySDRDevice *(*)(const SoapySDRKwargs *);
using SoapySDRDevice_makeStrArgs_t = SoapySDRDevice *(*)(const char *);
using SoapySDRDevice_lastError_t = const char *(*)();
using SoapySDRDevice_unmake_t = int (*)(SoapySDRDevice *);
using SoapySDRDevice_setFrequency_t = int (*)(SoapySDRDevice *, const int, const size_t,
                                              const double, const SoapySDRKwargs *);
using SoapySDRDevice_getSampleRate_t = double (*)(const SoapySDRDevice *, const int,
                                                  const size_t);
using SoapySDRDevice_getSampleRateRange_t = SoapySDRRange *(*)(const SoapySDRDevice *,
                                                               const int, const size_t,
                                                               size_t *);
using SoapySDRDevice_setSampleRate_t = int (*)(SoapySDRDevice *, const int, const size_t,
                                               const double);
using SoapySDRDevice_listSampleRates_t = double *(*)(const SoapySDRDevice *, const int,
                                                     const size_t, size_t *);
using SoapySDRDevice_setBandwidth_t = int (*)(SoapySDRDevice *, const int, const size_t,
                                              const double);
using SoapySDRDevice_getBandwidth_t = double (*)(const SoapySDRDevice *, const int,
                                                 const size_t);
using SoapySDRDevice_listBandwidths_t = double *(*)(const SoapySDRDevice *, const int,
                                                    const size_t, size_t *);
using SoapySDRDevice_setupStream_t = SoapySDRStream *(*)(SoapySDRDevice *, const int,
                                                         const char *, const size_t *,
                                                         const size_t,
                                                         const SoapySDRKwargs *);
using SoapySDRDevice_activateStream_t = int (*)(SoapySDRDevice *, SoapySDRStream *,
                                                const int, const long long, const size_t);
using SoapySDRDevice_deactivateStream_t = int (*)(SoapySDRDevice *, SoapySDRStream *,
                                                  const int, const long long);
using SoapySDRDevice_closeStream_t = int (*)(SoapySDRDevice *, SoapySDRStream *);
using SoapySDRDevice_readStream_t = int (*)(SoapySDRDevice *, SoapySDRStream *,
                                            void *const *, const size_t, int *,
                                            long long *, const long);
using SoapySDRDevice_getNumChannels_t = size_t (*)(const SoapySDRDevice *, const int);
using SoapySDRDevice_getChannelInfo_t = SoapySDRKwargs (*)(const SoapySDRDevice *,
                                                           const int, const size_t);
using SoapySDRDevice_listAntennas_t = char **(*)(const SoapySDRDevice *, const int,
                                                 const size_t, size_t *);
using SoapySDRDevice_listGains_t = char **(*)(const SoapySDRDevice *, const int,
                                              const size_t, size_t *);
using SoapySDRDevice_hasGainMode_t = bool (*)(const SoapySDRDevice *, const int,
                                              const size_t);
using SoapySDRDevice_setGainMode_t = int (*)(SoapySDRDevice *, const int, const size_t,
                                             const bool);
using SoapySDRDevice_getGainMode_t = bool (*)(const SoapySDRDevice *, const int,
                                              const size_t);
using SoapySDRDevice_setGain_t = int (*)(SoapySDRDevice *, const int, const size_t,
                                         const double);
using SoapySDRDevice_setGainElement_t = int (*)(SoapySDRDevice *, const int, const size_t,
                                                const char *, const double);
using SoapySDRDevice_getGain_t = double (*)(const SoapySDRDevice *, const int,
                                            const size_t);
using SoapySDRDevice_getGainElement_t = double (*)(const SoapySDRDevice *, const int,
                                                   const size_t, const char *);
using SoapySDRDevice_getGainRange_t = SoapySDRRange (*)(const SoapySDRDevice *, const int,
                                                        const size_t);
using SoapySDRDevice_getGainElementRange_t = SoapySDRRange (*)(const SoapySDRDevice *,
                                                               const int, const size_t,
                                                               const char *);
using SoapySDRDevice_getAntenna_t = char *(*)(const SoapySDRDevice *, const int,
                                              const size_t);
using SoapySDRDevice_setAntenna_t = int (*)(SoapySDRDevice *, const int, const size_t,
                                            const char *);
using SoapySDRDevice_getBandwidthRange_t = SoapySDRRange *(*)(const SoapySDRDevice *,
                                                              const int, const size_t,
                                                              size_t *);
using SoapySDRDevice_getStreamMTU_t = size_t (*)(const SoapySDRDevice *,
                                                 SoapySDRStream *);
using SoapySDR_errToStr_t = const char *(*)(const int);

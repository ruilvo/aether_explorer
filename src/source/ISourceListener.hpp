/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-only
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include <complex>
#include <vector>

class ISourceListener
{
  public:
    ISourceListener() = default;
    virtual ~ISourceListener() = default;
    ISourceListener(const ISourceListener &) = delete;
    ISourceListener &operator=(ISourceListener const &) = delete;

    virtual void setSampleRate(double sampleRate) = 0;
    virtual void setCentreFrequency(double centreFrequency) = 0;
    virtual void receiveSamples(std::vector<std::complex<float>> &samples) = 0;
};

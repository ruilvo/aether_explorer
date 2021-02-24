/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include <complex>
#include <vector>

class ISourceListener
{
  public:
    virtual ~ISourceListener();
    ISourceListener(const ISourceListener &) = delete;
    ISourceListener &operator=(ISourceListener const &) = delete;
    virtual void setSampleRate(double sampleRate);
    virtual void setCentreFrequency(double centreFrequency);
    virtual void receiveSamples(std::vector<std::complex<float>> &samples);
};

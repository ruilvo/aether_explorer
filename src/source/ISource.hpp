/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-only
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include "ISourceListener.hpp"

#include <QWidget>

#include <vector>

class ISource
{
  public:
    ISource() = default;
    virtual ~ISource() = default;
    ISource(const ISource &) = delete;
    ISource &operator=(const ISource &) = delete;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual double getCentreFrequency() = 0;
    virtual void setCentreFrequency(double centreFrequency) = 0;
    virtual double getSampleRate() = 0;

    virtual QWidget *getWidget() = 0;

    void setListeners(std::vector<ISourceListener *> listeners);

  protected:
    // NOLINTNEXTLINE: protected members aren't that evil
    std::vector<ISourceListener *> listeners_;
};

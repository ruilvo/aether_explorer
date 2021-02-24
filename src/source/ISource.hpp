/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include "ISourceListener.hpp"

#include <QWidget>

#include <vector>

class ISource
{
  public:
    virtual ~ISource() = default;
    ISource(const ISource &) = delete;
    ISource &operator=(const ISource &) = delete;

    virtual void start();
    virtual void stop();

    virtual double getCentreFrequency();
    virtual void setCentreFrequency(double centreFrequency);
    virtual double getSampleRate();

    virtual QWidget *getWidget();

    void setListeners(std::vector<ISourceListener *> listeners);

  private:
    std::vector<ISourceListener *> listeners_;
};

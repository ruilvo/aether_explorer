/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include "ISourceListener.hpp"

#include <memory>
#include <vector>

class SourceListenersCollection
{
  public:
    SourceListenersCollection() = default;
    ~SourceListenersCollection() = default;
    SourceListenersCollection(const SourceListenersCollection &) = delete;
    SourceListenersCollection &operator=(SourceListenersCollection const &) = delete;

    void subscribe(ISourceListener &&listener);
    std::vector<ISourceListener *> getSubscribers();

  private:
    std::vector<std::unique_ptr<ISourceListener>> listeners_;
};

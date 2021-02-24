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
    SourceListenersCollection(SourceListenersCollection &&) = default;
    SourceListenersCollection &operator=(SourceListenersCollection &&) = default;

    void subscribe(std::shared_ptr<ISourceListener> listener);
    std::vector<ISourceListener *> getSubscribers();

  private:
    std::vector<std::shared_ptr<ISourceListener>> listeners_;
};

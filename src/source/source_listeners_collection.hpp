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

    void subscribe(ISourceListener &&listener);

  private:
    std::vector<std::unique_ptr<ISourceListener>> listeners_;
};

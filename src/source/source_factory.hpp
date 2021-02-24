/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include "ISource.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>

class SourceFactory
{
  public:
    SourceFactory() = default;
    ~SourceFactory() = default;

    std::unique_ptr<ISource> createSource(const std::string &sourceName);
    std::vector<std::string> getNames();
    void registerSouce(const std::string &sourceName,
                       std::function<std::unique_ptr<ISource>(void)> creator);

  private:
    std::map<std::string, std::function<std::unique_ptr<ISource>(void)>>
        sourcesAvailable_;
};

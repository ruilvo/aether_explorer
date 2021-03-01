/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-only
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
    SourceFactory(const SourceFactory &) = delete;
    SourceFactory &operator=(SourceFactory const &) = delete;
    SourceFactory(SourceFactory &&) = default;
    SourceFactory &operator=(SourceFactory &&) = default;

    std::unique_ptr<ISource> createSource(const std::string &sourceName);
    std::vector<std::string> getNames();
    void registerSource(const std::string &sourceName,
                       std::function<std::unique_ptr<ISource>(void)> creator);

  private:
    std::map<std::string, std::function<std::unique_ptr<ISource>(void)>>
        sourcesAvailable_;
};

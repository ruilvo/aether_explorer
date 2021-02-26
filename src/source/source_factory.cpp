/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "source_factory.hpp"

#include <memory>

std::unique_ptr<ISource> SourceFactory::createSource(const std::string &sourceName)
{
    auto source_pos = sourcesAvailable_.find(sourceName);
    if (source_pos != sourcesAvailable_.end())
    {
        return sourcesAvailable_[sourceName]();
    }
    return std::unique_ptr<ISource>(nullptr); // Todo: avoid this?!
};

std::vector<std::string> SourceFactory::getNames()
{
    std::vector<std::string> names;
    for (const auto &elem : sourcesAvailable_)
    {
        names.push_back(elem.first);
    }
    return names;
}

void SourceFactory::registerSource(const std::string &sourceName,
                                  std::function<std::unique_ptr<ISource>(void)> creator)
{
    sourcesAvailable_[sourceName] = std::move(creator);
}

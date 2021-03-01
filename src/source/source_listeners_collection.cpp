/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-only
 * Consult LICENSE.txt for detailed licensing information
 */

#include "source_listeners_collection.hpp"

void SourceListenersCollection::subscribe(std::shared_ptr<ISourceListener> listener)
{
    listeners_.push_back(std::move(listener));
};

std::vector<ISourceListener *> SourceListenersCollection::getSubscribers()
{
    std::vector<ISourceListener *> subs;
    for (const auto &listener : listeners_)
    {
        subs.push_back(listener.get());
    }
    return subs;
}

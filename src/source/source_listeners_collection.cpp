/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "source_listeners_collection.hpp"

void SourceListenersCollection::subscribe(ISourceListener &&listener)
{
    listeners_.push_back(std::make_unique<ISourceListener>(listener));
};

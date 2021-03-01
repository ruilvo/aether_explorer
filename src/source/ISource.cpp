/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-only
 * Consult LICENSE.txt for detailed licensing information
 */

#include "ISource.hpp"

void ISource::setListeners(std::vector<ISourceListener *> listeners)
{
    listeners_ = std::move(listeners);
};

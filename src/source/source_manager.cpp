/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "source_manager.hpp"

SourceManager::SourceManager(SourceFactory &&sourceFactory,
                             SourceListenersCollection &&listenersCollection)
    : currentSource_(std::unique_ptr<ISource>(nullptr)),
      widget_(std::make_unique<SourceManagerWidget>(this)),
      sourceFactory_(std::move(sourceFactory)),
      listenersCollection_(std::move(listenersCollection)){};

ISource *SourceManager::getSource()
{
    return currentSource_.get();
}

void SourceManager::setSource(const std::string &sourceName)
{
    currentSource_.reset();
    currentSource_ = sourceFactory_.createSource(sourceName);
}

QWidget *SourceManager::getWidget()
{
    return static_cast<QWidget *>(widget_.get());
}

SourceFactory *SourceManager::getSourceFactory()
{
    return &sourceFactory_;
}

std::vector<ISourceListener *> SourceManager::getSourceListeners()
{
    return listenersCollection_.getSubscribers();
}

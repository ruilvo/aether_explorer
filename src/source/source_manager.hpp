/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include "ISource.hpp"
#include "ISourceListener.hpp"
#include "source_factory.hpp"
#include "source_listeners_collection.hpp"
#include "source_manager_widget.hpp"

#include <QWidget>

#include <memory>
#include <vector>

class SourceManager
{
  public:
    SourceManager(SourceFactory &&sourceFactory,
                  SourceListenersCollection &&listenersCollection);
    SourceManager() = delete;
    ~SourceManager() = default;

    ISource *getSource();
    void setSource(const std::string &sourceName);
    QWidget *getWidget();
    SourceFactory *getSourceFactory();
    std::vector<ISourceListener *> getSourceListeners();

  private:
    std::unique_ptr<ISource> currentSource_;
    std::unique_ptr<SourceManagerWidget> widget_;
    SourceFactory sourceFactory_;
    SourceListenersCollection listenersCollection_;
};

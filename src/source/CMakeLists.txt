# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-only
# Consult LICENSE.txt for detailed licensing information

add_library(
  source STATIC
  "ISourceListener.hpp"
  "source_listeners_collection.hpp"
  "source_listeners_collection.cpp"
  "ISource.hpp"
  "ISource.cpp"
  "source_factory.hpp"
  "source_factory.cpp"
  "source_manager.hpp"
  "source_manager.cpp"
  "source_manager_widget.hpp"
  "source_manager_widget.cpp")
target_include_directories(source PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(source PUBLIC Qt::Core Qt::Widgets)

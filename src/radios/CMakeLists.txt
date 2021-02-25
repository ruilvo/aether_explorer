# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-or-later
# Consult LICENSE.txt for detailed licensing information

add_library(radios STATIC "soapysdr_source.hpp" "soapysdr_source.cpp"
                          "soapysdr_widget.hpp" "soapysdr_widget.cpp")
target_include_directories(radios PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(radios PUBLIC source Qt::Core Qt::Widgets)

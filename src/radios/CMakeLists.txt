# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-only
# Consult LICENSE.txt for detailed licensing information

add_library(radios STATIC "soapysdr_radio.hpp" "soapysdr_radio.cpp" "soapysdr_widget.hpp"
                          "soapysdr_widget.cpp" "soapysdr_types.hpp")
target_include_directories(radios PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(radios PUBLIC source Qt::Core Qt::Widgets)

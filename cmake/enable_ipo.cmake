# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-only
# Consult LICENSE.txt for detailed licensing information

# Check if interprocedural optimization (IPO/LTO) is supported
# https://stackoverflow.com/a/47370726/5168563
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT LTO_ERROR)
if(IPO_SUPPORTED)
  message(STATUS "IPO / LTO enabled")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(STATUS "IPO / LTO not supported: <${LTO_ERROR}>")
endif()

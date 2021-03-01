# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-only
# Consult LICENSE.txt for detailed licensing information

cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(aether_explorer)

# Options ---------------
option(AETHER_USE_SSE2 "Use SSE2 instructions." OFF)
option(AETHER_USE_AVX "Use AVX instructions." OFF)
option(AETHER_USE_AVX2 "Use AVX2 instructions." ON)
# -----------------------

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Enable C++ 17 globally
set(CMAKE_CXX_STANDARD 17)

# Enable fast math support
# MSVC at least already uses SSE2 if it can anyway
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast /openmp:experimental")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:fast /openmp:experimental")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -fopenmp")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math -fopenmp")
endif()

include(enablesimd)

# Up the warning levels
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

# Downloaded stuff goes here
set(AETHER_EXTDEP_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/extdep")

# Download miniaudio.h
include(download_miniaudio)

# Enable link-time optimization globally
include(enable_ipo)

# Run Conan
include(process_conan)

# Find packages provided by Conan
find_package(Boost REQUIRED COMPONENTS boost)
find_package(FFTW3f REQUIRED COMPONENTS fftw3f)

# Windeployqt macro
include(run_windeployqt)

# For Qt enable the meta-object compilers
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Make sure that Qt5 deprecated APIs are not used
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

# Find Qt
set(AETHER_QT_REQUIRED_COMPONENTS "Core;Widgets;OpenGLWidgets")
find_package(
  Qt6
  COMPONENTS ${AETHER_QT_REQUIRED_COMPONENTS}
  REQUIRED)
if(NOT Qt6_FOUND)
  find_package(
    Qt5 5.15
    COMPONENTS ${AETHER_QT_REQUIRED_COMPONENTS}
    REQUIRED)
endif()

# Actual source files
add_subdirectory("src")

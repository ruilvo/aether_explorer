# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-only
# Consult LICENSE.txt for detailed licensing information

include(CheckCXXCompilerFlag)

if(AETHER_USE_AVX2)
  foreach(FLAG "-mavx2" "/arch:AVX2")
    unset(HAVE_AVX2 CACHE)
    unset(HAVE_AVX2)
    check_cxx_compiler_flag(${FLAG} HAVE_AVX2)
    if(HAVE_AVX2)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    endif()
  endforeach()
elseif(AETHER_USE_AVX)
  foreach(FLAG "-mavx" "/arch:AVX")
    unset(HAVE_AVX CACHE)
    unset(HAVE_AVX)
    check_cxx_compiler_flag(${FLAG} HAVE_AVX)
    if(HAVE_AVX)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    endif()
  endforeach()
elseif(AETHER_USE_SSE2)
  foreach(FLAG "-msse2" "/arch:SSE2")
    unset(HAVE_SSE2 CACHE)
    unset(HAVE_SSE2)
    check_cxx_compiler_flag(${FLAG} HAVE_SSE2)
    if(HAVE_SSE2)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
    endif()
  endforeach()
endif()

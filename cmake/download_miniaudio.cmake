# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-or-later
# Consult LICENSE.txt for detailed licensing information

# Download miniaudio as required
macro(download_miniaudio)
  message(STATUS "Downloading miniaudio.h")
  file(
    DOWNLOAD "https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h"
    "${AETHER_EXTDEP_DOWNLOAD_DIR}/include/miniaudio/miniaudio.h"
    TLS_VERIFY ON
    SHOW_PROGRESS)
endmacro()

set(MINIAUDIO_H "${AETHER_EXTDEP_DOWNLOAD_DIR}/include/miniaudio/miniaudio.h")
# Update when miniaud.io updates
set(MINIAUDIO_H_MD5_GOOD "bec8ad66bb799b3305fa3a3427d917bb")

if(NOT EXISTS "${MINIAUDIO_H}")
  download_miniaudio()
else()
  file(MD5 "${MINIAUDIO_H}" MINIAUDIO_H_MD5)
  if("${MINIAUDIO_H_MD5}" STREQUAL "${MINIAUDIO_H_MD5_GOOD}")
    message(STATUS "Found miniaudio.h")
  else()
    message(STATUS "Bad miniaudio.h MD5. Downloading.")
    download_miniaudio()
  endif()
endif()

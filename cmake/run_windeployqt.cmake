# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-only
# Consult LICENSE.txt for detailed licensing information

macro(run_windeployqt deploy_target)
  # On Windows, run windeployqt after building
  if(WIN32)
    # Run winddeployqt if it can be found
    find_program(
      WINDEPLOYQT_EXECUTABLE
      NAMES windeployqt
      HINTS ${QTDIR} ENV QTDIR
      PATH_SUFFIXES bin)
    add_custom_command(
      TARGET ${deploy_target}
      POST_BUILD
      COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${deploy_target}>)
  endif()
endmacro()

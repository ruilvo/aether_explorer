# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-only
# Consult LICENSE.txt for detailed licensing information

# If Conan exists, try to use it
find_program(CONAN_BIN NAMES conan)
if(CONAN_BIN)
  message(STATUS "Conan found at ${CONAN_BIN}. Using.")

  # Download the integration from github
  if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    # Notice the 0.15 tag. Update as needed.
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
         "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
  endif()

  include(${CMAKE_BINARY_DIR}/conan.cmake)

  conan_cmake_run(CONANFILE conanfile.txt BASIC_SETUP BUILD missing)
else()
  message(WARNING "Conan not found, trying to proceed without it!")
endif()

# conanfile.txt is set to use Find<package_name> targets. If conan indeed ran,
# let's add the targets generated.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
  include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

# This file is part of Aether Explorer
#
# Copyright (c) 2021 Rui Oliveira
# SPDX-License-Identifier: GPL-3.0-only
# Consult LICENSE.txt for detailed licensing information

add_executable(app "main.cpp")
target_link_libraries(app PUBLIC Qt::Core Qt::Widgets source radios)
run_windeployqt(app)

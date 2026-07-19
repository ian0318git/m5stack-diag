# Top-level CMakeLists.txt for M5Stack CoreS3 Diagnostic System
#
# This is an ESP-IDF project.  Build with:
#   idf.py set-target esp32s3
#   idf.py build
#
# The diagnostic component lives under common/src/M5Stack_CoreS3/.
#
# Copyright (c) 2025 by M5Stack
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)

set(EXTRA_COMPONENT_DIRS
    common/src/M5Stack_CoreS3
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(m5s3_diag)

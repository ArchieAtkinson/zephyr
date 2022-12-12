# SPDX-License-Identifier: Apache-2.0
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_FLAGS -m32)

target_sources(app PRIVATE src/main.c)

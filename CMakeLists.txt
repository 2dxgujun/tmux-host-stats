# vim: tabstop=2 shiftwidth=2 expandtab textwidth=80 linebreak wrap
#
# Copyright 2012 Matthew McCormick
# Copyright 2015 Pawel 'l0ner' Soltys
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 2.6)

### General Package stuff
project(tmux-host-stats)
set(tmux-host-stats_VERSION_MAJOR 3)
set(tmux-host-stats_VERSION_MINOR 4)
set(tmux-host-stats_VERSION_PATCH 0)

#Compute full version string
set(tmux-host-stats_VERSION
  ${tmux-host-stats_VERSION_MAJOR}.${tmux-host-stats_VERSION_MINOR}.${tmux-host-stats_VERSION_PATCH})

# Check whether we have support for c++11 in compiler and fail if we don't
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(FATAL_ERROR
    "Compiler ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} has no C++11 support.")
endif()

# generate header file to handle version
configure_file(
  "${PROJECT_SOURCE_DIR}/src/version.h.in" "${PROJECT_BINARY_DIR}/version.h"
  )

# set build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# detect system type
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  message(STATUS "Linux detected")
  set(METER_SOURCES "src/stats.linux.cc")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  message(STATUS "Darwin detected")
  set(METER_SOURCES "src/stats.osx.cc")
else()
  message(FATAL_ERROR "Cannot be compiled on this system")
endif()

# set common source files
set(COMMON_SOURCES "src/main.cc")

# add binary tree so we find version.h
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src")

add_executable(tmux-host-stats ${COMMON_SOURCES} ${METER_SOURCES})
install(TARGETS tmux-host-stats RUNTIME DESTINATION bin)

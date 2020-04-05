#[===============================================================================[.rst
#
# The directory contains a demo demonstrating the usage of libStrategy in two ways:
#
# 1) C++ interface:
#
#    Following executables generated:
#
#      demo_stg : inherited from CStrategyProcess
#      single_demo : inherited from CStrategy
#
# 2) Python interface:
#
#    Following scripts provided:
#      demo.py
#      exec.py
#      check.py
#      gold_finger.py
#
#  Also, various kinds of configuration files are provided to drive the strategy
#
#    demo.json : comm based (CMDHelperComm)
#    faketd.json: fake td (CTDHelperFake)
#    pipe.json : pipe based (CMDHelperPipe, CTDHelperPipe)
#    replay.json: replay_mmap (CMDHelperReplayIO)
#    replay_dump.json: replay_ctpdump (CMDHelperReplayCtpDump)
#
#]===============================================================================]

# Setup minimum required version of cmake, policy
cmake_minimum_required(VERSION 3.15...3.17)

# Setup Project name and version
project(demo)

# find NatFramework
find_package(NatFramework)

# Target: single_demo ##############################################################
add_executable(single_demo)
target_sources(single_demo
  PRIVATE   "CDemo.cpp"
            "CDemo.h"
  )

target_include_directories(single_demo
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(single_demo
  PUBLIC  Strategy
          Utility
  )

# Target: demo_stg ##############################################################
add_executable(demo_stg)
target_sources(demo_stg
  PRIVATE   "CDemoStrategy.cpp"
            "CDemoStrategy.h"
  )

target_include_directories(demo_stg
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(demo_stg
  PUBLIC  Strategy
          Utility
  )

# Install the target ########################################
install(TARGETS single_demo demo_stg)

# Install: conf files ############################################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/conf/
  DESTINATION conf
  FILES_MATCHING PATTERN "*.json"
  )

# Install: python scripts ############################################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/
  DESTINATION scripts
  FILES_MATCHING PATTERN "*.py"
  PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
  )

# Test ########################################################################
# TODO: test

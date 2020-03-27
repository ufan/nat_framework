#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Nat::Strategy
#
# Notes:
#
#]===============================================================================]

# Define Static Target: Strategy ################################################
target_sources(Strategy
  PRIVATE  "ATSysUtils.cpp" # io, include, common, used in md, td
           "CGlobalLock.cpp" # common, used in io
           "CPipExecutor.cpp" # logger, common, used in md, td
           "CTradeBaseInfo.cpp" # include
           "CWareHouseReader.cpp" # include, common, io, used in md, pyext
           "utils.cpp" # include, logger
           # following : no external dependencies
           "CEncodeConv.cpp"
           "CTimer.cpp"
           "CityHash.cpp"
           "MurmurHash2.cpp"
           "MurmurHash3.cpp"
# PUBLIC
           "ATSysUtils.h"
           "CGlobalLock.h"
           "CPipExecutor.h"
           "CTradeBaseInfo.h"
           "CWareHouseReader.h"
           "utils.h"
           # following : no external dependencies
           "CEncodeConv.h"
           "CTimer.h"
           "CityHash.h"
           "MurmurHash2.h"
           "MurmurHash3.h"
           "CHashTable.h"
           "TimeMeasure.h"
           "compiler.h"
           "json.hpp"
  )

target_include_directories(Strategy
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

# Target: Logger ############################################################
target_sources(Logger
  PRIVATE  utils.cpp
           json.hpp
           utils.h
  )

target_include_directories(Logger
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Test ########################################################################
# TODO: test

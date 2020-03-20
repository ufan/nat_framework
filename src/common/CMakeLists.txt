#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Nat::CommonUtils
#
# Notes:
#
#]===============================================================================]

# Define Static Target: CommonUtils ################################################
add_library(CommonUtils OBJECT)

# alias name
add_library(Nat::CommonUtils ALIAS CommonUtils)

target_sources(CommonUtils
  PRIVATE  "ATSysUtils.cpp" # io, include, common, used in md, td
           "CGloablLock.cpp" # common, used in io
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
  PUBLIC   "ATSysUtils.h"
           "CGloablLock.h"
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

target_include_directories(CommonUtils
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(CommonUtils
  PRIVATE Nat::CommonHeader
          Nat::Logger
          Nat::IO
  )

add_library(Nat::CommonUtils ALIAS CommonUtils)

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# install(TARGETS CommonUtils EXPORT CommonUtilsExport // TODO
#   )

# install(EXPORT CommonUtilsExport
#         DESTINATION lib/cmake/NatFramework
#         NAMESPACE Nat::
#         FILE CommonUtilsTarget.cmake
#   )

# TODO: test

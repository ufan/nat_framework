#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Logger : OBJECT target
#
# Notes:
#
#]===============================================================================]

# For imported target : log4cplus::log4cplusS
find_package(${proj} REQUIRED)

# Target: Logger ##############################################################
add_library(Logger OBJECT)

# alias name
add_library(Nat::Logger ALIAS Logger)

target_sources(Logger
  PRIVATE   "HeavyLogger.cpp"
            "FastLogger.cpp"
            "CFastLog.cpp"
            "CLogPool.cpp"
  PUBLIC    "CFastLog.h"
            "CLogPool.h"
            "HeavyLogger.h"
            "FastLogger.h"
            "SimpleLogger.h"
  INTERFACE "Logger.h"
  )

target_include_directories(Logger
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Logger
  PRIVATE Nat::CommonHeader
          Nat::CommonUtils
  PUBLIC  log4cplus::log4cplusS # static library from log4cplus
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# TODO: install the headers from log4cplusS
# libstrategy does not need log4cplusS

# install(TARGETS Logger EXPORT LoggerExport // TODO
#   )

# install(EXPORT LoggerExport
#         DESTINATION lib/cmake/NatFramework
#         NAMESPACE Nat::
#         FILE LoggerTarget.cmake
#   )

# TODO: test


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

# Target: Logger ##############################################################
add_library(Logger OBJECT)
add_library(Nat::Logger ALIAS Logger)

target_sources(Logger
  PRIVATE   "HeavyLogger.cpp"
            "FastLogger.cpp"
            "CFastLog.cpp"
            "CLogPool.cpp"
            # PUBLIC
            "CFastLog.h"
            "CLogPool.h"
            "HeavyLogger.h"
            "FastLogger.h"
            "SimpleLogger.h"
            #INTERFACE
            "Logger.h"
  )

target_include_directories(Logger
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Logger
  PUBLIC  log4cplus::log4cplus
          Utility
          CommonHeader
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS Logger EXPORT Logger_export)

install(EXPORT Logger_export
  DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  NAMESPACE Nat::
  FILE LoggerTargets.cmake
  )

# Test ########################################################################
# TODO: test


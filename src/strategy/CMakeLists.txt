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

# Target: Strategy ##############################################################
add_library(Strategy STATIC)
add_library(Nat::Strategy ALIAS Strategy)

# change the output name of the library
set_target_properties(Strategy
  PROPERTIES OUTPUT_NAME strategy)

target_sources(Strategy
  PRIVATE   "CStrategy.cpp"
            "CStrategyProcess.cpp"
            "StrategyShared.cpp"
            "CStrategy.h"
            "CStrategyProcess.h"
            "StrategyShared.h"
  )

target_compile_definitions(Strategy
  PUBLIC FAST_LOGGER_
  )

target_include_directories(Strategy
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Strategy
  PUBLIC  CommonHeader
          CommonUtils
          Utility
          Logger
          IO
          Risk
          Bar
          Account
          # MDHelper
          # TDHelper
  PRIVATE Boost::headers
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  PATTERN "demo" EXCLUDE
  )

# Install the target ########################################
install(TARGETS Strategy EXPORT Strategy_export
  )

install(EXPORT Strategy_export
  DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  NAMESPACE Nat::
  FILE StrategyTargets.cmake
  )

# Install: demo directory ############################################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/demo
  DESTINATION share
  )

# Test ########################################################################
# TODO: test

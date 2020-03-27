#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Bar : OBJECT target
#
# Notes:
#
#]===============================================================================]

# Target: Bar ##############################################################
target_sources(Strategy
  PRIVATE   "Bar2.cpp"
            "BarHelper.cpp"
            "BarMaker.cpp"
            "Bar2.h"
            # PUBLIC
            "BarHelper.h"
            "BarMaker.h"
  )

target_include_directories(Strategy
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


#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Strategy : OBJECT target
#
# Notes:
#
#]===============================================================================]

# Target: Strategy ##############################################################
target_sources(Strategy
  PRIVATE   "PyExtExch.cpp"
            "PyExtension.cpp"
            # "PyUtils.cpp" # header only ?
            "PyWrapper.cpp"
            # PUBLIC
            "PyExtExch.h"
            "PyUtils.h"
            "PyWrapper.h"
  )

target_include_directories(Strategy
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Strategy
  PUBLIC Boost::python
         Python2::Module
         Python2::Python
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Test ########################################################################
# TODO: test


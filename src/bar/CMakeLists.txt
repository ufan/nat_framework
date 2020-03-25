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
add_library(Bar OBJECT)

target_sources(Bar
  PRIVATE   "Bar2.cpp"
            "BarHelper.cpp"
            "BarMaker.cpp"
            "Bar2.h"
  PUBLIC    "BarHelper.h"
            "BarMaker.h"
  )

target_include_directories(Bar
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Bar
  PUBLIC Logger
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS Bar EXPORT Bar_export)

# install(EXPORT Logger_export
#   DESTINATION ${NAT_CMAKE_INSTALL_DIR}
#   NAMESPACE Nat::
#   FILE LoggerTargets.cmake
#   )

# Test ########################################################################
# TODO: test


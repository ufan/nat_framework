#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   PyEmbed : OBJECT target
#
# Notes:
#
#]===============================================================================]


target_sources(Strategy
  PRIVATE   "PyExtension.cpp"
            "PyWrapper.cpp"
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
  )

# Target: PyEmbed ##############################################################
add_library(PyEmbed OBJECT)
add_library(Nat::PyEmbed ALIAS PyEmbed)

target_sources(PyEmbed
  PRIVATE   "PyExtExch.cpp"
            "PyExtExch.h"
            "PyUtils.h"
  )

target_include_directories(PyEmbed
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  )

target_link_libraries(PyEmbed
  PUBLIC CommonHeader
         Logger
         Boost::python
         Python2::Python
  )

# Install: the headers ############################################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS PyEmbed EXPORT PyEmbed_export)

install(EXPORT PyEmbed_export
  DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  NAMESPACE Nat::
  FILE PyEmbedTargets.cmake
  )

# Test ########################################################################
# TODO: test


#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   PyExt : OBJECT target
#
# Notes:
#
#]===============================================================================]

# Target: PyExt ##############################################################
add_library(PyExt STATIC)

target_sources(PyExt
  PRIVATE   "PyExtExch.cpp"
            "PyExtension.cpp"
            # "PyUtils.cpp" # header only ?
            "PyWrapper.cpp"
            # PUBLIC
            "PyExtExch.h"
            "PyUtils.h"
            "PyWrapper.h"
  )

target_include_directories(PyExt
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(PyExt
  PUBLIC Logger
         IO
         Strategy
         Risk
         MDHelper
         TDHelper
         Boost::python
         Python2::Module
         Python2::Python
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS PyExt EXPORT PyExt_export)

install(EXPORT PyExt_export
  DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  NAMESPACE Nat::
  FILE PyExtTargets.cmake
  )

# Test ########################################################################
# TODO: test


#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Nat::Utility
#
# Notes:
#
#]===============================================================================]

# Define Static Target: Utility ################################################
add_library(Utility OBJECT)

target_sources(Utility
  PRIVATE "CGlobalLock.cpp"
          "CEncodeConv.cpp"
          "CTimer.cpp"
          "CityHash.cpp"
          "MurmurHash2.cpp"
          "MurmurHash3.cpp"
          "utils.cpp"
          "CGlobalLock.h"
          "CEncodeConv.h"
          "CTimer.h"
          "CityHash.h"
          "MurmurHash2.h"
          "MurmurHash3.h"
          "CHashTable.h"
          "TimeMeasure.h"
          "utils.h"
  )

target_include_directories(Utility
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Utility
  PRIVATE CommonHeader
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS Utility EXPORT Utility_export)

install(EXPORT Utility_export
  DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  NAMESPACE Nat::
  FILE UtilityTargets.cmake
  )

# Test ########################################################################
# TODO: test

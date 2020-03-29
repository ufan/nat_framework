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

target_sources(CommonUtils
  PRIVATE  "ATSysUtils.cpp" # io, include, common, used in md, td
           "CPipExecutor.cpp" # logger, common, used in md, td
           "CTradeBaseInfo.cpp" # include
           "CWareHouseReader.cpp" # include, common, io, used in md, pyext
           "ATSysUtils.h"
           "CPipExecutor.h"
           "CTradeBaseInfo.h"
           "CWareHouseReader.h"
  )

target_include_directories(CommonUtils
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(CommonUtils
  PRIVATE CommonHeader
          Logger
          IO
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS CommonUtils EXPORT Common_export)

install(EXPORT Common_export
  DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  NAMESPACE Nat::
  FILE CommonTargets.cmake
  )

# Test ########################################################################
# TODO: test

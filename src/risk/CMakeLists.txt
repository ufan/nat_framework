#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Risk : OBJECT target
#
# Notes:
#
#]===============================================================================]

# Target: Risk ##############################################################
add_library(Risk STATIC)

target_sources(Risk
  PRIVATE   "RiskAcc.cpp"
            "RiskInstr.cpp"
            "RiskInstrStg.cpp"
            "RiskInstrTop.cpp"
            "RiskPrd.cpp"
            "RiskStg.cpp"
            "RiskTop.cpp"
            # "ConfUtil.cpp" # header only ?
            # PUBLIC
            "RiskAcc.h"
            "RiskInstr.h"
            "RiskInstrStg.h"
            "RiskInstrTop.h"
            "RiskPrd.h"
            "RiskStg.h"
            "RiskTop.h"
            "RiskError.h"
            "ConfUtil.h"
  )

target_include_directories(Risk
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Risk
  PUBLIC  CommonUtils
          CommonHeader
          Logger
          Account
          Boost::headers
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS Risk EXPORT Risk_export)

install(EXPORT Risk_export
  DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  NAMESPACE Nat::
  FILE RiskTargets.cmake
  )

# Test ########################################################################
# TODO: test


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

# Target: Strategy ##############################################################
target_sources(Strategy
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


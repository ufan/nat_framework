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
  PRIVATE   "AccBase.cpp"
            "ModAcc.cpp"
            "ModInstr.cpp"
            "ModPrd.cpp"
            "UnitAmt.cpp"
            "UnitPnl.cpp"
            "UnitPx.cpp"
            "UnitVol.cpp"
            "ModAcc.h"
            "ModInstr.h"
            "ModPrd.h"
            "UnitAmt.h"
            "UnitPnl.h"
            "UnitPx.h"
            "UnitVol.h"
            # PUBLIC
            "AccBase.h" # only used in risk
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


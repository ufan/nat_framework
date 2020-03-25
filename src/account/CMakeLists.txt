#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Account : OBJECT target
#
# Notes:
#
#]===============================================================================]

# Target: Account ##############################################################
add_library(Account OBJECT)

target_sources(Account
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
  PUBLIC    "AccBase.h" # only used in risk
  )

target_include_directories(Account
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Account
  PUBLIC Logger
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS Account EXPORT Account_export)

# install(EXPORT Logger_export
#   DESTINATION ${NAT_CMAKE_INSTALL_DIR}
#   NAMESPACE Nat::
#   FILE LoggerTargets.cmake
#   )

# Test ########################################################################
# TODO: test


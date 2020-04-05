#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Nat::ExecuteModel
#
# Notes:
#
#]===============================================================================]

# Target: ExecuteModel ##############################################################
add_library(ExecuteModel STATIC)
add_library(Nat::ExecuteModel ALIAS ExecuteModel)

# change the output name of the library
set_target_properties(ExecuteModel
  PROPERTIES OUTPUT_NAME execstg)

target_sources(ExecuteModel
  PRIVATE   "CExecuteStrategy.cpp"
            "CABM.cpp"
            "CAvgSpread.cpp"
            "CBaseSignal.cpp"
            "CDriftModel.cpp"
            "CLiquidityModel.cpp"
            "CLogitPModel.cpp"
            "CLossModel.cpp"
            "CMarketStruct.cpp"
            "CSigmaSignal.cpp"
            "CVolatilityModel.cpp"
            "PyExecStgExtension.cpp"
            "CExecuteStrategy.h"
            "CABM.h"
            "CAvgSpread.h"
            "CBaseSignal.h"
            "CDriftModel.h"
            "CLiquidityModel.h"
            "CLogitPModel.h"
            "CLossModel.h"
            "CMarketStruct.h"
            "CSigmaSignal.h"
            "CVolatilityModel.h"
            "PublicFun.h"
  )

target_compile_definitions(ExecuteModel
  PUBLIC FAST_LOGGER_
  )

target_include_directories(ExecuteModel
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(ExecuteModel
  PUBLIC  Strategy
          Boost::python
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS ExecuteModel EXPORT ExecuteModel_export
  )

install(EXPORT ExecuteModel_export
  DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  NAMESPACE Nat::
  FILE ExecuteModelTargets.cmake
  )

# Test ########################################################################
# TODO: test

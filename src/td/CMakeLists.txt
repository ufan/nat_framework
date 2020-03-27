#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Executable:
#     TDEngine
#     TDEngineEESV2
#     TDEngineEESV3
#     TDEngineXt
#
#   Object Target:
#     Strategy
#
# Notes:
#
#]===============================================================================]

# Executables: TDEngineXXX ##############################################################
set(TDEngine_SRC
  ITDEngine.cpp
  COrderTrackMmap.cpp
  )

set(TDEngine_Common_Deps
  Logger
  IO
  )

# EES major
add_executable(TDEngine)

target_sources(TDEngine
  PRIVATE ${TDEngine_SRC}
          TDEngine.cpp
          CTDEngineCtp.cpp
          CTDEngineEES.cpp
  )

target_include_directories(TDEngine
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(TDEngine
  PRIVATE ${TDEngine_Common_Deps}
          EES::Trader
          CTP::CTP
 )

# EES V2
add_executable(TDEngineEESV2)

target_sources(TDEngineEESV2
  PRIVATE ${TDEngine_SRC}
          TDEngineEESV2.cpp
          CTDEngineCtp.cpp
          CTDEngineEESV2.cpp
  )

target_include_directories(TDEngineEESV2
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(TDEngineEESV2
  PRIVATE ${TDEngine_Common_Deps}
          EESV2::Trader
          CTP::CTP
  )

# EES V3
add_executable(TDEngineEESV3)

target_sources(TDEngineEESV3
  PRIVATE ${TDEngine_SRC}
          TDEngine.cpp
          CTDEngineCtp.cpp
          CTDEngineEES.cpp
  )

target_include_directories(TDEngineEESV3
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(TDEngineEESV3
  PRIVATE ${TDEngine_Common_Deps}
  EESV3::Trader
  CTP::CTP
  )

# Xt
add_executable(TDEngineXt)

target_sources(TDEngineXt
  PRIVATE ${TDEngine_SRC}
          TDEngineXt.cpp
          CTDEngineXt.cpp
  )

target_include_directories(TDEngineXt
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(TDEngineXt
  PRIVATE ${TDEngine_Common_Deps}
          Xt::Xt
  )

# Target: Strategy ##############################################################
target_sources(Strategy
  PRIVATE   "ITDHelper.cpp"
            "CTDHelperFactory.cpp"
            "CTDHelperComm.cpp"
            "CTDHelperPipe.cpp"
            "CTDHelperPython.cpp"
            "CTDHelperFake.cpp"
            "ITDHelper.h"
            "CTDHelperPipe.h"
            "CTDHelperPython.h"
            # PUBLIC
            "CTDHelperFactory.h"
            "CTDHelperComm.h" # in used pyext
            "CTDHelperFake.h"
  )

target_include_directories(Strategy
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  PATTERN "trade_flow" EXCLUDE
  )

# Install: executables ########################################################
install(TARGETS TDEngine TDEngineEESV2 TDEngineEESV3 TDEngineXt
  DESTINATION TDEngine/bin
  )

# Test ########################################################################
# TODO: test

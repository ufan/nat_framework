#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Executable:
#     MDEngine
#     MDEngineEESV2
#
#   Object Target:
#     Strategy
#
# Notes:
#
#]===============================================================================]

# Executable: MDEngine, MDEngineEESV2 ##############################################################
set(MDEngine_SRC
  MDEngine.cpp
  IMDEngine.cpp
  CMDEngineCtp.cpp
  CMDEngineEES.cpp
  CEESOnload.cpp
  CMDEngineEESOnload.cpp
  CMDEngineCtp.h
  CMDEngineEES.h
  CEESOnload.h
  CMDEngineEESOnload.h
  IMDEngine.h
  )

set(MDEngine_Common_Deps
  Logger
  IO
  CTP::CTP # for CMDEngineCtp
  OpenOnload::OpenOnload # for CMDEngineEESOnload
  Slefh::Slefh # for CMDEngineEESOnload
  )

# EES major
add_executable(MDEngine)

target_sources(MDEngine
  PRIVATE ${MDEngine_SRC}
  )

target_include_directories(MDEngine
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(MDEngine
  PRIVATE ${MDEngine_Common_Deps}
          EES::Quote # for CMDEngineEES
 )

# EES V2
add_executable(MDEngineEESV2)

target_sources(MDEngineEESV2
  PRIVATE ${MDEngine_SRC}
  )

target_include_directories(MDEngineEESV2
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(MDEngineEESV2
  PRIVATE ${MDEngine_Common_Deps}
  EESV2::Quote # for CMDEngineEES
  )

# Target: Strategy ##############################################################

target_sources(Strategy
  PRIVATE   "IMDHelper.cpp"
            "CMDHelperFactory.cpp"
            "CMDHelperComm.cpp"
            "CMDHelperPipe.cpp"
            "CMDHelperPython.cpp"
            "CMDHelperReplayCtpDump.cpp"
            "CMDHelperReplayIO.cpp"
            "IMDHelper.h"
            "CMDHelperPipe.h"
            "CMDHelperPython.h"
            "CMDHelperReplayCtpDump.h"
            # PUBLIC
            "CMDHelperFactory.h"
            "CMDHelperReplayIO.h" # used in pyext
            "CMDHelperComm.h" # in used pyext
  )

target_include_directories(Strategy
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  PATTERN "con_dir" EXCLUDE
  )

# Install: executables ########################################################
install(TARGETS MDEngine MDEngineEESV2
  DESTINATION MDEngine/bin
  )

# Test ########################################################################
# TODO: test

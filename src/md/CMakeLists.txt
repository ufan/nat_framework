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
#     MDHelper
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
  CommonHeader
  CommonUtils
  Logger
  Utility
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

# Target: MDHelper ##############################################################

# add_library(MDHelper OBJECT)

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

target_link_libraries(Strategy
  PUBLIC Logger
         CommonUtils
         IO
         PyEmbed
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  # PATTERN "con_dir" EXCLUDE
  )

# Install: config files ############################################################
# configure_file(conf/tdengine.json.in conf/tdengine.json @ONLY)
# configure_file(conf/mdengine_test.json.in conf/mdengine_test.json @ONLY)
configure_file(conf/mdengine_offhour.json.in conf/mdengine_offhour.json @ONLY)
configure_file(conf/mdengine.json.in conf/mdengine.json @ONLY)
configure_file(conf/mdengine_test.json.in conf/mdengine_test.json @ONLY)
configure_file(conf/mdlogger.cnf.in conf/mdlogger.cnf @ONLY)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/conf
  DESTINATION MDEngine
  )

# make the log directory
install(DIRECTORY DESTINATION MDEngine/log)

# Install: scripts ############################################################
configure_file(scripts/md_svr.sh.in scripts/md_svr.sh @ONLY)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scripts
  DESTINATION MDEngine
  FILES_MATCHING PATTERN "*.sh"
  PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
  )

# Install: executables ########################################################
install(TARGETS MDEngine MDEngineEESV2
  DESTINATION MDEngine/bin
  )

# Install the target ########################################
# install(TARGETS MDHelper EXPORT MDHelper_export)

# install(EXPORT MDHelper_export
#   DESTINATION ${NAT_CMAKE_INSTALL_DIR}
#   NAMESPACE Nat::
#   FILE MDHelperTargets.cmake
#   )

# Test ########################################################################
# TODO: test

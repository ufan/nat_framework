#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Executable:
#     CTPDumper
#
#]===============================================================================]

# Executables: CTPDumper ##############################################################
add_executable(CTPDumper)

target_sources(CTPDumper
  PRIVATE CTPDumper.cpp
          CCTPTD.cpp
          CCTPMD.cpp
          FileMgr.cpp
          DailyInfoMgr.cpp
          CCTPTD.h
          CCTPMD.h
          FileMgr.h
          DailyInfoMgr.h
          Structure.h
  )

target_include_directories(CTPDumper
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(CTPDumper
  PRIVATE Logger
          Utility
          CTP::CTP
 )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install: config files ############################################################

configure_file(conf/dump.json.in conf/dump.json @ONLY)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/conf/
  DESTINATION dump/conf
  FILES_MATCHING PATTERN "*.json"
  )

# Install: executables ########################################################
install(TARGETS CTPDumper
  DESTINATION dump/bin
  )

# Test ########################################################################
# TODO: test

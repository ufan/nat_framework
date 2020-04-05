#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   ServerUtils : OBJECT target
#
# Notes:
#
#]===============================================================================]

# Target: ServerUtils ##############################################################
add_library(ServerUtils OBJECT)

target_sources(ServerUtils
  PRIVATE    "CBuffer.cpp"
             "CListener.cpp"
             "CService.cpp"
             "CSocket.cpp"
             "CBuffer.h"
             "CListener.h"
             "CService.h"
             "CSocket.h"
  )

target_include_directories(ServerUtils
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(ServerUtils
  PUBLIC Logger
         CommonHeader
         libev::libev
  )

# Test ########################################################################
# TODO: test


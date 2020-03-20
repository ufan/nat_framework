#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Nat::Strategy
#
# Notes:
#
#]===============================================================================]

# Target: Strategy ##############################################################
add_library(Strategy STATIC)

# alias name
add_library(Nat::Strategy ALIAS Strategy)

# target_sources(Strategy
#   PRIVATE   "CStrategy.cpp"
#             "CStrategyProcess.cpp"
#             "StrategyShared.cpp"
#   PUBLIC    "CStrategy.h"
#             "CStrategyProcess.h"
#             "StrategyShared.h"
#   INTERFACE "Logger.h"
#   )

# target_include_directories(Logger
#   PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#          $<INSTALL_INTERFACE:include>
#   )

target_link_libraries(Strategy
  PUBLIC  Nat::CommonHeader
          Nat::CommonUtils
          Nat::Logger
          Nat::IO
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

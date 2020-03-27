#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Crypt : STATIC target
#
# Notes:
#
#]===============================================================================]

# Target: Crypt ##############################################################
target_sources(Strategy
  PRIVATE   "CCrypt.cpp"
            "CCrypt.h"
  )

target_include_directories(Strategy
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Strategy
  PUBLIC  OpenSSL::SSL
          OpenSSL::Crypto
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Test ########################################################################
# TODO: test


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
add_library(Crypt STATIC)

target_sources(Crypt
  PRIVATE   "CCrypt.cpp"
  PUBLIC    "CCrypt.h"
  )

target_include_directories(Crypt
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(Crypt
  PUBLIC Logger
         OpenSSL::SSL
         OpenSSL::Crypto
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the target ########################################
install(TARGETS Crypt EXPORT Crypt_export)

# install(EXPORT Logger_export
#   DESTINATION ${NAT_CMAKE_INSTALL_DIR}
#   NAMESPACE Nat::
#   FILE LoggerTargets.cmake
#   )

# Test ########################################################################
# TODO: test


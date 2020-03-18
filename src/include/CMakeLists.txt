#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface targets::
#
#   Nat::CommonHeader
#
# Notes:
#
#]===============================================================================]

# Configure version number ####################################################
configure_file(VersionConfig.h.in VersionConfig.h)

# Define Interface target : CommonHeader #####################################################
set(Tgt CommonHeader)
add_library(${Tgt} INTERFACE)

# Add an alias
add_library(Nat::${Tgt} ALIAS CommonHeader)

target_include_directories(${Tgt} INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for VersionConfig.h
                           $<INSTALL_INTERFACE:include> # TODO: needed or not?
                           )

# Install the target ########################################
install(TARGETS ${Tgt} EXPORT ${Tgt}Export
      # INCLUDE_DIRECTORIES include # TODO: needed or not? overlap with target_link_directories
      )

# Install the related headers #########################################################
# TODO: SDK specific-target
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_BINARY_DIR}/
        TYPE INCLUDE
        FILES_MATCHING PATTERN "*.h"
        )

# Export the target to config script ##################################################
install(EXPORT ${Tgt}Export DESTINATION lib/cmake/NatFramework
        NAMESPACE Nat::
        FILE ${Tgt}Target.cmake
        )


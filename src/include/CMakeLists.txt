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

# Define Interface target : CommonHeader #####################################################
set(Tgt CommonHeader)

add_library(${Tgt} INTERFACE)
add_library(Nat::${Tgt} ALIAS CommonHeader)

# Configure version number ####################################################
configure_file(VersionConfig.h.in VersionConfig.h)

target_include_directories(${Tgt} INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for VersionConfig.h
                           $<INSTALL_INTERFACE:include>
                           )

# Alternative way to set up INTERFACE_INCLUDE_DIRECTORY
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
# target_include_directories(${Tgt} INTERFACE
#   $<INSTALL_INTERFACE:include>
#   )

# Install the related headers #########################################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_BINARY_DIR}/
        TYPE INCLUDE
        FILES_MATCHING PATTERN "*.h"
        )

# Install the target ########################################
install(TARGETS ${Tgt} EXPORT ${Tgt}Export)

install(EXPORT ${Tgt}Export DESTINATION lib/cmake/NatFramework
        NAMESPACE Nat::
        FILE ${Tgt}Target.cmake
        )


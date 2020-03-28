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
add_library(CommonHeader INTERFACE)
add_library(Nat::CommonHeader ALIAS CommonHeader)

# Configure version number ####################################################
configure_file(VersionConfig.h.in VersionConfig.h)

target_include_directories(CommonHeader INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for VersionConfig.h
                           $<INSTALL_INTERFACE:include>
                           )

# Alternative way to set up INTERFACE_INCLUDE_DIRECTORY
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
# target_include_directories(CommonHeader INTERFACE
#   $<INSTALL_INTERFACE:include>
#   )

# Install the related headers #########################################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_BINARY_DIR}/
        TYPE INCLUDE
        FILES_MATCHING PATTERN "*.h"
        )

# Install the target ########################################
install(TARGETS CommonHeader EXPORT CommonHeader_export)

install(EXPORT CommonHeader_export
        DESTINATION ${NAT_CMAKE_INSTALL_DIR}
        NAMESPACE Nat::
        FILE CommonHeaderTargets.cmake
        )

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
configure_file(VersionConfig.h.in VersionConfig.h @ONLY)
configure_file(SysConf.h.in SysConf.h @ONLY)

target_include_directories(CommonHeader INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           $<INSTALL_INTERFACE:include>
                           )

# Alternative way to set up INTERFACE_INCLUDE_DIRECTORY
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
# target_include_directories(CommonHeader INTERFACE
#   $<INSTALL_INTERFACE:include>
#   )

# make the global directory   #######################################################
# install(DIRECTORY DESTINATION global/io/td_engine)
# install(DIRECTORY DESTINATION global/io/md_engine)
# install(DIRECTORY DESTINATION global/io/system)
# install(DIRECTORY DESTINATION global/io/signal)
# install(DIRECTORY DESTINATION global/io/strategy)
# install(DIRECTORY DESTINATION global/logs/Strategy)
# install(DIRECTORY DESTINATION global/logs/order_track)
# install(DIRECTORY DESTINATION global/logs/account)

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

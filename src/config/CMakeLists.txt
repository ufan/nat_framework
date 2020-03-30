#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface targets::
#
#   Nat::Config
#
# Notes:
#
#]===============================================================================]

# Define Interface target : Config #####################################################
add_library(Config INTERFACE)
add_library(Nat::Config ALIAS Config)

target_include_directories(Config INTERFACE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include>
                           )

target_link_libraries(Config
  INTERFACE Boost::headers
  )

# Install the related headers #########################################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        TYPE INCLUDE
        FILES_MATCHING PATTERN "*.h"
        )

# Install the target ########################################
install(TARGETS Config EXPORT Config_export)

install(EXPORT Config_export
        DESTINATION ${NAT_CMAKE_INSTALL_DIR}
        NAMESPACE Nat::
        FILE ConfigHeaderTargets.cmake
        )

# Test ########################################################################
# TODO: test

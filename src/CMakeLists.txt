#[===============================================================================[.rst
#
# Project:
#
#   NAT Framework
#
# Description:
#
#
#]===============================================================================]

# Setup minimum required version of cmake, policy
cmake_minimum_required(VERSION 3.13...3.16)

# Setup Project name and version
project(NatFramework VERSION 1.0
  DESCRIPTION "NAT framework"
  LANGUAGES CXX
  )

#=================================================================================

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# DONE: folder grouping
# RELATED: target property: folder
option(CMakeTest_USE_FOLDERS "Enable folder grouping in IDEs." ON)
if(CMakeTest_USE_FOLDERS)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
  set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
endif()

# Always use '-fPIC' or '-fPIE' option
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#=================================================================================
# TODO:useful commands

# find_program(MAKE_EXE NAMES gamke namke make)
#=================================================================================
# External Dependency

# open source projects
add_subdirectory(external)

# binary distributed projects
add_subdirectory(ThirdParty)

#=================================================================================
# Global variables

# installation directory for headers, libraries, binary
set(NAT_INCLUDE_INSTALL_DIR include)
set(NAT_LIB_INSTALL_DIR lib)
set(NAT_CMAKE_INSTALL_DIR "lib/cmake/NatFramework-${CMAKE_PROJECT_VERSION}")

#=================================================================================
# Subdiretories

add_subdirectory(include)
# add_subdirectory(common)
# add_subdirectory(logger)
# add_subdirectory(io)
# add_subdirectory(strategy)

#=================================================================================
# TODO: find_package configuration

include(CMakePackageConfigHelpers)

# Package version file
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/NatFrameworkConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
  )

# Package config file
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/NatFrameworkConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/NatFrameworkConfig.cmake
  INSTALL_DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  PATH_VARS
    NAT_INCLUDE_INSTALL_DIR
    NAT_LIB_INSTALL_DIR
  )

# Install the config and version files
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/NatFrameworkConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/NatFrameworkConfigVersion.cmake
  DESTINATION ${NAT_CMAKE_INSTALL_DIR}
  )

#=================================================================================
# TODO: Test or GoogleTest or Catch

# if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
#   include(CTest)
# endif()

# if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
#   add_subdirectory(test)
# endif()

# TODO: or enable_testing()?

#=================================================================================
# Packaging

set(CPACK_PACKAGE_VENDOR "YZ")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NAT Framework")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
# set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
# set(CPACK_RESOURCE_FILE_READE "${CMAKE_CURRENT_SOURCE_DIR}/READE.md")

set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /.*build.*
    )

include(CPack)

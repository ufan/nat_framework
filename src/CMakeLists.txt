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


#############################################################
# Find external packages

# open source projects
add_subdirectory(external)

# binary distributed projects
add_subdirectory(ThirdParty)

#=================================================================================
# Subdiretories

add_subdirectory(include)
add_subdirectory(common)
add_subdirectory(logger)
add_subdirectory(io)
add_subdirectory(strategy)

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

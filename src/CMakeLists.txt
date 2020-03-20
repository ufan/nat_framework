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

#=================================================================================

# Setup minimum required version of cmake, policy
# This command should be the first command in the project
cmake_minimum_required(VERSION 3.12...3.16)

# Setup Project name and version
# CMakeTest_VERSION and PROJECT_VERSION variables are set
project(CMakeTest VERSION 1.0
  DESCRIPTION "Test Project of CMake Functions"
  LANGUAGES CXX
  )

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#=================================================================================
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

include(ExternalProject)
set(ThirdParty_DIR ${PROJECT_SOURCE_DIR}/ThirdParty)

# TODO: steps which are extracted as build targets
# set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure build test)

# TODO : EP_BASE or EP_PREFIX property?

## EP 1 : log4cplus
set(proj log4cplus)
ExternalProject_Add(${proj}
  ### Step 1 : setup directories ###
  # PREFIX         ${ThirdParty_DIR}/${proj}
  SOURCE_DIR       ${ThirdParty_DIR}/${proj}/src
  # BINARY_DIR     ${ThirdParty_DIR}/${proj}/build
  INSTALL_DIR      ${ThirdParty_DIR}/${proj}
  ### Step 2 : download external source ###
  GIT_REPOSITORY   "https://github.com/log4cplus/log4cplus"
  GIT_TAG          "tags/REL_2_0_0"
  GIT_PROGRESS     1
  ### Step 3 : build configuration (default is cmake-based configuration) ###
  CMAKE_ARGS       -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                   -DBUILD_SHARED_LIBS=FALSE
  ### Step 4 : installation ###
  ### Step 5 : test ###
  # TEST_BEFORE_INSTALL     1
  TEST_EXCLUDE_FROM_MAIN  1
  )
set_property(TARGET ${proj} PROPERTY FOLDER "${proj}")

ExternalProject_Get_Property(${proj} INSTALL_DIR)

# two ways of setting search loaction of Config-based package
set(log4cplus_ROOT ${INSTALL_DIR})

message("Install directory of log4cplus: ${INSTALL_DIR}")

## EP 2 : libev
# set(proj libev)
# ExternalProject_Add(${proj}
#   ### Step 1 : setup directories ###
#   # PREFIX         ${ThirdParty_DIR}/${proj}
#   SOURCE_DIR       ${ThirdParty_DIR}/${proj}/src
#   # BINARY_DIR     ${ThirdParty_DIR}/${proj}/build
#   INSTALL_DIR      ${ThirdParty_DIR}/${proj}
#   ### Step 2 : download external source ###
#   CVS_REPOSITORY   ":pserver:anonymous@cvs.schmorp.de/schmorpforge"
#   CVS_MODULE       "libev"
#   CVS_TAG          "-rrel-4_31"
#   ### Step 3 : build configuration (default is cmake-based configuration) ###
#   # configure command comes from libev/autogen.sh
#   CONFIGURE_COMMAND autoreconf --install --symlink --force <SOURCE_DIR>
#   COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
#   # BUILD_COMMAND     make
#   ### Step 4 : installation ###
#   ### Step 5 : test ###
#   # TEST_BEFORE_INSTALL     1
#   TEST_EXCLUDE_FROM_MAIN  1
#   )
# set_property(TARGET ${proj} PROPERTY FOLDER "${proj}")

# ExternalProject_Get_Property(${proj} INSTALL_DIR)

# set(libev_ROOT ${INSTALL_DIR})
# set(libev_LIBRARIES "-lev")
# set(libev_INCLUDE_DIRS "${libev_ROOT}/include")
# set(libev_DEFINITIONS "-static")
#TODO: extract version number from ev.h

# message("Install directory of libev: ${Libev_ROOT}")

# ## EP 3 : CMake
# set(Utility_DIR ${PROJECT_SOURCE_DIR}/Utility)
# set(proj cmake-latest)
# ExternalProject_Add(${proj}
#   SOURCE_DIR      ${Utility_DIR}/${proj}/src
#   INSTALL_DIR     ${Utility_DIR}/${proj}
#   URL https://github.com/Kitware/CMake/releases/download/v3.16.5/cmake-3.16.5.tar.gz
#   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
#              -DCMAKE_BUILD_TYPE=release
#              -DBUILD_TESTING=OFF
# #   BUILD_COMMAND ""
#   )

#############################################################
# Find external packages

# Config-file based packages
# list(APPEND CMAKE_PREFIX_PATH "${log}")

# Module-file based packages
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#TODO: find python

#TODO: find boost_python

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
#TODO: Packaging

# set(CPACK_PACKAGE_VENDOR "Yong")
# set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NAT Framework")
# set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
# set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
# set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
# # set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
# # set(CPACK_RESOURCE_FILE_READE "${CMAKE_CURRENT_SOURCE_DIR}/READE.md")

# set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
# set(CPACK_SOURCE_IGNORE_FILES
#     /.git
#     /.*build.*
#     /\\\\.DS_Store
#     )

# include(CPack)

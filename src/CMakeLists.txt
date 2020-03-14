cmake_minimum_required(VERSION 3.10)

# project name and version
project(NatFramework VERSION 0.1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

####################################################################
# external projects

# TODO : EP_BASE or EP_PREFIX property?
include(ExternalProject)
ExternalProject_Add(log4cplus
  ### Step 1 : setup directories ###
  # PREFIX         ${PROJECT_SOURCE_DIR}/ThirdParty/log4cplus
  # SOURCE_DIR       ${PROJECT_SOURCE_DIR}/ThirdParty/log4cplus/src
  # BINARY_DIR     ${PROJECT_SOURCE_DIR}/ThirdParty/log4cplus/build
  INSTALL_DIR     ${PROJECT_SOURCE_DIR}/ThirdParty/log4cplus
  ### Step 2 : download external source ###
  GIT_REPOSITORY   https://github.com/log4cplus/log4cplus
  GIT_TAG          tags/REL_2_0_0
  GIT_PROGRESS     true
  ### Step 3 : build configuration (default is cmake-based configuration) ###
  CMAKE_ARGS       -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/ThirdParty/log4cplus
                   -DBUILD_SHARED_LIBS=FALSE
  ### Step 4 : installation ###
  ### Step 5 : test ###
  TEST_BEFORE_INSTALL     TRUE
  TEST_EXCLUDE_FROM_MAIN  FALSE
  )

ExternalProject_Get_Property(log4cplus INSTALL_DIR)
set(LOG4CPLUS_DIR ${INSTALL_DIR})
# message("Install directory of log4cplus: ${LOG4CPLUS_DIR}")

# libev
ExternalProject_Add(libev
  ### Step 1 : setup directories ###
  # PREFIX         ${PROJECT_SOURCE_DIR}/ThirdParty/libev
  # SOURCE_DIR       ${PROJECT_SOURCE_DIR}/ThirdParty/libev/src
  # BINARY_DIR     ${PROJECT_SOURCE_DIR}/ThirdParty/libev/build
  INSTALL_DIR     ${PROJECT_SOURCE_DIR}/ThirdParty/libev
  ### Step 2 : download external source ###
  CVS_REPOSITORY   :pserver:anonymous@cvs.schmorp.de/schmorpforge
  CVS_MODULE       libev
  # CVS_TAG          "rel-4_24"
  ### Step 3 : build configuration (default is cmake-based configuration) ###
  CONFIGURE_COMMAND autoreconf --install --symlink --force <SOURCE_DIR>
  # COMMAND ./autogen.sh
  COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
  BUILD_COMMAND     make
  ### Step 4 : installation ###
  )

ExternalProject_Get_Property(libev INSTALL_DIR)
set(LIBEV_DIR ${INSTALL_DIR})
message("Install directory of libev: ${LIBEV_DIR}")

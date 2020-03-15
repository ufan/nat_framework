cmake_minimum_required(VERSION 3.10)

# project name and version
project(NatFramework VERSION 0.1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

####################################################################
# external projects

include(ExternalProject)

# TODO: steps which are extracted as build targets
# set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure build test)

# TODO : EP_BASE or EP_PREFIX property?

## EP 1 : log4cplus
set(proj log4cplus)
ExternalProject_Add(${proj}
  ### Step 1 : setup directories ###
  # PREFIX         ${PROJECT_SOURCE_DIR}/ThirdParty/${proj}
  SOURCE_DIR       ${PROJECT_SOURCE_DIR}/ThirdParty/${proj}/src
  # BINARY_DIR     ${PROJECT_SOURCE_DIR}/ThirdParty/${proj}/build
  INSTALL_DIR     ${PROJECT_SOURCE_DIR}/ThirdParty/${proj}
  ### Step 2 : download external source ###
  GIT_REPOSITORY   "https://github.com/log4cplus/log4cplus"
  GIT_TAG          "tags/REL_2_0_0"
  GIT_PROGRESS     true
  ### Step 3 : build configuration (default is cmake-based configuration) ###
  CMAKE_ARGS       -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/ThirdParty/${proj}
                   -DBUILD_SHARED_LIBS=FALSE
  ### Step 4 : installation ###
  ### Step 5 : test ###
  TEST_BEFORE_INSTALL     TRUE
  TEST_EXCLUDE_FROM_MAIN  FALSE
  )
set_property(TARGET ${proj} PROPERTY FOLDER "${proj}")

ExternalProject_Get_Property(${proj} INSTALL_DIR)
set(LOG4CPLUS_DIR ${INSTALL_DIR})
message("Install directory of log4cplus: ${LOG4CPLUS_DIR}")

## EP 2 : libev
set(proj libev)
ExternalProject_Add(${proj}
  ### Step 1 : setup directories ###
  # PREFIX         ${PROJECT_SOURCE_DIR}/ThirdParty/${proj}
  SOURCE_DIR       ${PROJECT_SOURCE_DIR}/ThirdParty/${proj}/src
  # BINARY_DIR     ${PROJECT_SOURCE_DIR}/ThirdParty/${proj}/build
  INSTALL_DIR      ${PROJECT_SOURCE_DIR}/ThirdParty/${proj}
  ### Step 2 : download external source ###
  CVS_REPOSITORY   ":pserver:anonymous@cvs.schmorp.de/schmorpforge"
  CVS_MODULE       "libev"
  CVS_TAG          "-rrel-4_31"
  ### Step 3 : build configuration (default is cmake-based configuration) ###
  # configure command comes from libev/autogen.sh
  CONFIGURE_COMMAND autoreconf --install --symlink --force <SOURCE_DIR>
  COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
  # BUILD_COMMAND     make
  ### Step 4 : installation ###
  )
set_property(TARGET ${proj} PROPERTY FOLDER "${proj}")

ExternalProject_Get_Property(${proj} INSTALL_DIR)
set(LIBEV_DIR ${INSTALL_DIR})
message("Install directory of libev: ${LIBEV_DIR}")

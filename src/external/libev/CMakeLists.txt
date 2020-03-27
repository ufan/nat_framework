#[===============================================================================[.rst
# ./external/libev/CMakeLists.txt -- NatFramework
#
# Download/Build libev library.
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# Imported targets in global scope:
#
#   libev::libev
#
#]===============================================================================]

# Setup minimum required version of cmake, policy
cmake_minimum_required(VERSION 3.15...3.17)

# Setup Project name and version
project(libev VERSION 1.0
  DESCRIPTION "Interface to libev library"
  LANGUAGES CXX
  )

#=================================================================================

include(FetchContent)

# Variables controlling FetchContent
set(FETCHCONTENT_QUIET OFF)
# set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)

# Declare the fetching information
# set(VERSION "-rrel-4_31")
set(VERSION "-rrel-4_24")
FetchContent_Declare(
  libev
  CVS_REPOSITORY   ":pserver:anonymous@cvs.schmorp.de/schmorpforge"
  CVS_MODULE       "libev"
  CVS_TAG          ${VERSION}
  SOURCE_DIR       ${CMAKE_CURRENT_LIST_DIR}/src
  )

FetchContent_GetProperties(libev)
if(NOT libev_POPULATED)
  # Download the src
  message(STATUS "Downloading libev")
  FetchContent_Populate(libev)

  # Configure, Build and Install
  option(ENABLE_LIBEV_BUILD "Enable reconfiguring and rerunning the libev build" ON)
  if(ENABLE_LIBEV_BUILD)

    message(STATUS "Configuring libev")
    execute_process(
      COMMAND chmod +x ./autogen.sh
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src
      )

    execute_process(
      COMMAND ./autogen.sh
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src
      )

    execute_process(
      COMMAND ./configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/install
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src
      )

    message(STATUS "Building libev")
    execute_process(
      COMMAND make install
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src
      )
  endif()

  # Export target : libev::libev
  add_library(libev INTERFACE)
  add_library(libev::libev ALIAS libev)

  set(libev_INCLUDE_INSTALL_DIR include/libev)
  set(libev_LIB_INSTALL_DIR lib/libev)
  set(libev_CMAKE_INSTALL_DIR lib/cmake/libev)

  target_include_directories(libev
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/install/include>
              $<INSTALL_INTERFACE:${libev_INCLUDE_INSTALL_DIR}>
    )

  target_link_libraries(libev
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/install/lib/libev.a>
              $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${libev_LIB_INSTALL_DIR}/libev.a>
    )

  # Install headers and libs
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/install/include/
    DESTINATION ${libev_INCLUDE_INSTALL_DIR}
    )

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/install/lib/
    DESTINATION ${libev_LIB_INSTALL_DIR}
    )

  # Install export targets
  install(TARGETS libev EXPORT libev_export)

  install(EXPORT libev_export
    NAMESPACE libev::
    FILE libevTargets.cmake
    DESTINATION ${libev_CMAKE_INSTALL_DIR}
    )

  # Install package config and version files

  include(CMakePackageConfigHelpers)

  # Package version file
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libevConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    )

  # Package config file
  configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/libevConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libevConfig.cmake
    INSTALL_DESTINATION ${libev_CMAKE_INSTALL_DIR}
    PATH_VARS
      libev_INCLUDE_INSTALL_DIR
      libev_LIB_INSTALL_DIR
    )

  # Install the config and version files
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libevConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libevConfigVersion.cmake
    DESTINATION ${libev_CMAKE_INSTALL_DIR}
    )

endif()



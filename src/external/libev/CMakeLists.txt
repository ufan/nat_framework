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

# Add to main project directly, useful for cmake project
# FetchContent_MakeAvailable(log4cplus) # generates log4cplus target without namespace "log4cplus"

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
      COMMAND ./configure --prefix=${CMAKE_CURRENT_LIST_DIR}/install
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src
      )

    message(STATUS "Building libev")
    execute_process(
      COMMAND make install
      WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/src
      )
  endif()

  # Export target : libev::libev
  add_library(libev INTERFACE
    IMPORTED GLOBAL
    )
  # add_library(libev::libev ALIAS libev)

  target_include_directories(libev INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/install/include
    )
  target_link_libraries(libev INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/install/lib/libev.a
    )
endif()



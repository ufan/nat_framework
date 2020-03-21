#[===============================================================================[.rst
# ./external/log4cplus/CMakeLists.txt -- NatFramework
#
# Download/Build log4cplus library.
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# Imported targets in global scope:
#
#   log4cplus::log4cplus
#
#]===============================================================================]


#=================================================================================
include(FetchContent)

# Variables controlling FetchContent
set(FETCHCONTENT_QUIET ON)

# Declare the fetching information
FetchContent_Declare(
  log4cplus
  GIT_REPOSITORY   "https://github.com/log4cplus/log4cplus"
  GIT_TAG          "tags/REL_2_0_5"
  # GIT_TAG          "tags/REL_2_0_0"
  SOURCE_DIR       ${CMAKE_CURRENT_LIST_DIR}/src
  )

# # Add to main project directly, useful for cmake project
# FetchContent_MakeAvailable(log4cplus) # generates log4cplus target without namespace "log4cplus"

FetchContent_GetProperties(log4cplus)
if(NOT log4cplus_POPULATED)
  message(STATUS "Downloading log4cplus")

  # Download the src
  FetchContent_Populate(log4cplus)

  # Add subdirectory and alias target
  if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
  endif()

  set(BUILD_SHARED_LIBS OFF)
  # set(ENABLE_SYMBOLS_VISIBILITY FALSE)
  set(LOG4CPLUS_ENABLE_DECORATED_LIBRARY_NAME OFF)
  add_subdirectory(${log4cplus_SOURCE_DIR} ${log4cplus_BINARY_DIR})
  # add_subdirectory(${log4cplus_SOURCE_DIR})
  # add_library(log4cplus::log4cplus ALIAS log4cplus)
endif()

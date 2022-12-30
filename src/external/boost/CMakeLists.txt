#[===============================================================================[.rst
# ./external/boost/CMakeLists.txt -- NatFramework
#
# Find Boost library.
# ^^^^^^^^^^^^^^^^^^^
#
# Imported targets in global scope:
#
#   Boost::headers
#   Boost::python
#
#]===============================================================================]

# TODO: how to integrate boost source base

# variables controlling the behavior of boost, referenced in BoostConfig.cmake
# set(Boost_DEBUG                TRUE) # verbose control
set(Boost_USE_RELEASE_LIBS     TRUE)
set(Boost_USE_STATIC_LIBS      TRUE)
set(Boost_USE_MULTITHREADED    TRUE)
# only used when boost is built with runtime-link=static
# set(Boost_USE_STATIC_RUNTIME   TRUE)

set(BOOST_VERSION 1.72.0)
find_package(Boost ${BOOST_VERSION}
  CONFIG
  REQUIRED COMPONENTS headers
                      python # header only libraries are found by default, in Boost::headers
  )

# Make found targets globally available, in case of list file in subdirectory
if (Boost_FOUND)
  set_target_properties(
    Boost::headers
    Boost::python
    PROPERTIES IMPORTED_GLOBAL TRUE)
endif ()

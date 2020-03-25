#[===============================================================================[.rst
# ./external/CMakeLists.txt -- NatFramework
#
# Collection of dependencies of open source projects
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
#]===============================================================================]

# Boost: user provided, access by find_package
add_subdirectory(boost)

# Python: user provided, access by find_package
add_subdirectory(python)

# log4cplus: downloaded and compiled, integrated as sub-project
add_subdirectory(log4cplus)

# libev: downloaded and compiled, integrated as imported target
add_subdirectory(libev)

# OpenSSL: user provided, access by find_package module
add_subdirectory(openssl)

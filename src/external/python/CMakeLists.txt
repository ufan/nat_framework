#[===============================================================================[.rst
# ./external/python/CMakeLists.txt -- NatFramework
#
# Find Python Development Environment
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# Imported targets in global scope:
#
#   Python::Python -- for embedding
#   Python::Module -- for extension
#
#]===============================================================================]

# Hint variable : Python2_ROOT_DIR
find_package(Python2 REQUIRED COMPONENTS
  Development
  )

# Make found targets globally available, in case of list file in subdirectory
if(Python2_FOUND AND Python2_Development_FOUND )
  set_target_properties(
    Python2::Python
    Python2::Module
    PROPERTIES IMPORTED_GLOBAL TRUE )
  # message("Python2 Found: ${Python2_EXECUTABLE} ${Python2_VERSION}")
  # message("Python2 Include Dir: ${Python2_INCLUDE_DIRS}")
  # message("Python2 Libraies: ${Python2_LIBRARY_DIRS} ${Python2_LIBRARIES}")
endif()

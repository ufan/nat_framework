#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Nat::Common
#
# Notes:
#
#]===============================================================================]

# Define Static Target: Common ################################################
set(Tgt Common)
add_library(${Tgt} STATIC)

target_sources(${Tgt}
  )

target_include_directories(${Tgt})

target_link_libraries(${Tgt})

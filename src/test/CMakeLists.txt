#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Executable:
#
#
# Notes:
#
#]===============================================================================]

# Executables:  ##############################################################
add_executable(ChangePwd)

target_sources(ChangePwd
  PRIVATE ChangePwd.cpp
  )

target_include_directories(ChangePwd
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(ChangePwd
  PRIVATE CTP::CTP
          Utility
 )

# Install: executables ########################################################
install(TARGETS ChangePwd
  DESTINATION test/bin
  )

# Test ########################################################################
# TODO: test

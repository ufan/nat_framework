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

# Executables:  ##############################################################
add_executable(TestTrader)

target_sources(TestTrader
  PRIVATE TestTrader.cpp
  )

target_include_directories(TestTrader
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(TestTrader
  PRIVATE CTP::CTP
  Utility
  )

# Executables:  ##############################################################
add_executable(TestMd)

target_sources(TestMd
  PRIVATE TestMd.cpp
  )

target_include_directories(TestMd
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(TestMd
  PRIVATE CTP::CTP
  Utility
  )

# Executables:  ##############################################################
add_executable(TestFlock)

target_sources(TestFlock
  PRIVATE TestFlock.cpp
  )

target_link_libraries(TestFlock
  PRIVATE Utility
          IO
  )

# Install: executables ########################################################
install(TARGETS ChangePwd TestTrader TestMd TestFlock
  DESTINATION test/bin
  )

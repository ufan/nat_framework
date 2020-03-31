#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Executable:
#     sshkeytransfer
#
#]===============================================================================]

# Executables: ##############################################################
add_executable(sshkeytransfer sshkeytransfer.cpp)
target_link_libraries(sshkeytransfer
  PRIVATE Crypt
  )

add_executable(fixmday fixmday.cpp)
target_link_libraries(fixmday
  PRIVATE IO
          Logger
          Utility
  )

add_executable(syscmd syscmd.cpp)
target_link_libraries(syscmd
  PRIVATE IO
  Logger
  Utility
  )

add_executable(ctpdump2mmap ctpdump2mmap.cpp)
target_link_libraries(ctpdump2mmap
  PRIVATE CommonHeader
          CommonUtils
          Utility
          Logger
          IO
  )

add_executable(dump2csv dump2csv.cpp)
target_link_libraries(dump2csv
  PRIVATE CommonHeader
          Utility
  )

add_executable(orderextract orderextract.cpp)
target_link_libraries(orderextract
  PRIVATE CommonHeader
          Utility
          IO
          Logger
  )

# Install: executables ########################################################
install(TARGETS
        sshkeytransfer
        fixmday
        syscmd
        ctpdump2mmap
        dump2csv
        orderextract
  DESTINATION tools/bin
  )

# Install: scripts ############################################################
configure_file(scripts/warehouse2mmap.sh.in scripts/warehouse2mmap.sh)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/ ${CMAKE_CURRENT_BINARY_DIR}/scripts/
  DESTINATION tools
  FILES_MATCHING PATTERN "*.sh"
  )

# Test ########################################################################
# TODO: test

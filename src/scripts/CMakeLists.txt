#[===============================================================================[.rst
#
# This will define the following variables::
#
#
# Notes:
#
#   Notes
#]===============================================================================]

# Install: config files ############################################################

configure_file(engine_svr.sh.in engine_svr.sh @ONLY)
configure_file(deploy.sh.in deploy.sh @ONLY)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_BINARY_DIR}/
  DESTINATION scripts
  FILES_MATCHING PATTERN "*.sh"
  )

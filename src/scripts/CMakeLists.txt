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
configure_file(dump_svr.sh.in dump_svr.sh @ONLY)
configure_file(tools.sh.in tools.sh @ONLY)
configure_file(alert_listener.py.in alert_listenser.py @ONLY)
configure_file(alert_check_alive.sh.in alert_check_alive.sh @ONLY)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_BINARY_DIR}/
  DESTINATION scripts
  FILES_MATCHING PATTERN "*.sh"
  PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
  )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_BINARY_DIR}/
  DESTINATION scripts
  FILES_MATCHING PATTERN "*.py"
  PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
  )

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

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION scripts
  FILES_MATCHING PATTERN "*.sh"
  )

#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Executable:
#     SignalAgent
#
#]===============================================================================]

# Executables: SignalAgent ##############################################################
add_executable(SignalAgent)

target_sources(SignalAgent
  PRIVATE main.cpp
          CSignalAgent.cpp
          CSignalRequester.cpp
          CSignalServer.cpp
          CSignalAgent.h
          CSignalRequester.h
          CSignalServer.h
          SignalAgentProtocol.h
  )

target_include_directories(SignalAgent
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(SignalAgent
  PRIVATE Logger
          CommonHeader
          Utility
          IO
          ServerUtils
 )

# Install: config files ############################################################
configure_file(conf/SignalAgent.json.in conf/SignalAgent.json)
configure_file(conf/salogger.cnf.in conf/salogger.cnf)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/conf
  DESTINATION SignalAgent
  )

# Install: scripts ############################################################
configure_file(scripts/sa_svr.sh.in scripts/sa_svr.sh)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scripts
  DESTINATION SignalAgent
  )

# Install: executables ########################################################
install(TARGETS SignalAgent
  DESTINATION SignalAgent/bin
  )

# Test ########################################################################
# TODO: test

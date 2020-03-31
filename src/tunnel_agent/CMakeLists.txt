#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Executable:
#     TunnelAgent
#     TunnelClient
#     LibLoader
#     PyLoader
#     StgManager
#
#]===============================================================================]

# Generate Public/Private key pair ############################################
add_custom_command(OUTPUT public_key.cpp private_key.cpp
  COMMAND sshkeytransfer
  )

# Executables: ##############################################################
add_executable(TunnelAgent)

target_sources(TunnelAgent
  PRIVATE main.cpp
          CTunnelAgent.cpp
          CWaiter.cpp
          CCommanderBase.cpp
          CSaveFileCmd.cpp
          CShellCmd.cpp
          CPythonExecutor.cpp
          CFileReciever.cpp
          CExecLoader.cpp
          CDirReciever.cpp
          CStrategyManager.cpp
          public_key.cpp # Generated Source
          CTunnelAgent.h
          CWaiter.h
          CCommanderBase.h
          CSaveFileCmd.h
          CShellCmd.h
          CPythonExecutor.h
          CFileReciever.h
          CExecLoader.h
          CDirReciever.h
          CStrategyManager.h
          public_key.h
  )

target_include_directories(TunnelAgent
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(TunnelAgent
  PRIVATE Logger
          Config
          Crypt
          Utility
          Boost::python
          libev::libev
 )

# Executable: TunnelClient #####################################################################
add_executable(TunnelClient)

target_sources(TunnelClient
  PRIVATE client.cpp
          CClient.cpp
          private_key.cpp # Generated Source
          CClient.h
          private_key.h
          )

target_include_directories(TunnelClient
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(TunnelClient
  PRIVATE Logger
          Config
          Crypt
          Utility
          CommonHeader
  )

# Executable: LibLoader #####################################################################
add_executable(LibLoader LibLoader.cpp)

target_include_directories(LibLoader
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(LibLoader
  PRIVATE Utility
          CommonHeader
          ${CMAKE_DL_LIBS}
  )

# Executable: PyLoader #####################################################################
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(PyLoader PyLoader.cpp)

target_include_directories(PyLoader
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(PyLoader
  PRIVATE Utility
          CommonHeader
          Python2::Python
          Threads::Threads
  )

# Executable: StgManager #####################################################################
add_executable(StgManager StgManager.cpp)

target_include_directories(StgManager
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(StgManager
  PRIVATE Utility
          CommonHeader
  )

# Install: config files ############################################################
configure_file(conf/logger.cnf.in conf/logger.cnf)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/conf/ ${CMAKE_CURRENT_BINARY_DIR}/conf/
  DESTINATION TunnelAgent/conf
  FILES_MATCHING PATTERN "*.cnf"
  )

# Install: scripts ############################################################
configure_file(scripts/check_alive.sh.in scripts/check_alive.sh)
configure_file(scripts/tunnel_svr.sh.in scripts/tunnel_svr.sh)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scripts
  DESTINATION TunnelAgent
  )

# Install: executables ########################################################
install(TARGETS TunnelAgent TunnelClient LibLoader PyLoader StgManager
  DESTINATION TunnelAgent/bin
  )

# Test ########################################################################
# TODO: test

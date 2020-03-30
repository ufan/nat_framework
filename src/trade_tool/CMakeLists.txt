#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Executable:
#     TradeTool
#
#]===============================================================================]

# Executables: TDEngineXXX ##############################################################
add_executable(TradeTool)

target_sources(TradeTool
  PRIVATE TradeTool.cpp
          CTraderBase.cpp
          CTraderFactory.cpp
          CCtpMiniTrader.cpp
          CCtpTrader.cpp
          CEESTrader.cpp
          CGlobalParameter.cpp
          CRisk.cpp
          CTraderBase.h
          CTraderFactory.h
          CCtpMiniTrader.h
          CCtpTrader.h
          CEESTrader.h
          CGlobalParameter.h
          CRisk.h
          EnumStatus.h
  )

target_compile_definitions(TradeTool
  PUBLIC FAST_LOGGER_
  )

target_include_directories(TradeTool
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(TradeTool
  PRIVATE Logger
          Utility
          Config
          EES::Trader
          CTP::CTP
          dl
 )

# Install: the headers ############################################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install: config files ############################################################
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/conf/
  DESTINATION TradeTool/conf
  FILES_MATCHING PATTERN "*.conf"
  )

# Install: scripts ############################################################
configure_file(scripts/TradeTool.sh.in scripts/TradeTool.sh)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/scripts
  DESTINATION TradeTool
  )

# Install: executables ########################################################
install(TARGETS TradeTool
  DESTINATION TradeTool/bin
  )

# Test ########################################################################
# TODO: test

# sources
set(SOURCES HeavyLogger.cpp FastLogger.cpp CFastLog.cpp CLogPool.cpp)
set(HEADERS HeavyLogger.h FastLogger.h CFastLog.h CLogPool.h SimpleLogger.h)

# target defintion
add_library(logger STATIC
  ${SOURCES}
  ${HEADERS} # TODO: explicit dependencies on header or not
  )

target_sources()

set_target_properties(logger PROPERTIES
  PUBLIC_HEADER Logger.h FastLogger.h HeavyLogger.h SimpleLogger.h
  )

set_target_properties(logger PROPERTIES
  PRIVATE_HEADER CFastLog.h CLogPool.h
  )

# generate position independent code
set_target_properties(logger PROPERTIES
  POSITION_INDEPENDENT_CODE 1
  )

target_include_directories(logger
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries(logger
  PUBLIC
  
  )

target_compile_definitions()
target_compile_features()
target_compile_options()

# installation
install(TARGETS logger
  ARCHIVE
  PUBLIC_HEADER
  PRIVATE_HEADER
  )

# TODO: test

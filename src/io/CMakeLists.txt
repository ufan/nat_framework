#[===============================================================================[.rst
#
# This will define the following variables::
#
#   VarName     - Variable Explanation
#
# and the following Imported/Interface/Alias targets::
#
#   Nat::IO
#
# Notes:
#
#]===============================================================================]

# Target: IO ##############################################################
add_library(IO STATIC)
add_library(Nat::IO ALIAS IO)

target_sources(IO
  PRIVATE   "CIOBase.cpp"
            "CRawIOReader.cpp"
            "CRawIOWriter.cpp"
            "CSignal.cpp"
            "CRawIOModify.cpp"
            "CGlobalSafeRawIOWriter.cpp"
            "CSystemIO.cpp"
            # PUBLIC
            "CIOBase.h"
            "CRawIOReader.h"
            "CRawIOWriter.h"
            "CSignal.h"
            "CRawIOModify.h"
            "CGlobalSafeRawIOWriter.h"
            "CSystemIO.h"
            "ioprotocol.h"
            # INTERFACE
            "CReaderPool.h"
  )

target_include_directories(IO
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>
  )

target_link_libraries(IO
  PUBLIC Logger
  )

# Install: the headers ############################################################

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  TYPE INCLUDE
  FILES_MATCHING PATTERN "*.h"
  )

# Install the targets ########################################################
install(TARGETS IO EXPORT IO_export)

install(EXPORT IO_export
        DESTINATION ${NAT_CMAKE_INSTALL_DIR}
        NAMESPACE Nat::
        FILE IOTargets.cmake
        )

# Test ##################################################################
# TODO: test

# TODO: Target: MemQueue ######################################################
# add_library(MemQueue OBJECT)

# target_sources(MemQueue
#   PRIVATE   "CQueueBase.cpp"
#             "CMemoryQueueBase.cpp"
#             "CMemQueue.cpp"
#             "CMemIPCPy.cpp"
#             "CLocalQueue.cpp"
#   PUBLIC    "CMemConfigBase.h"
#             "CReplyaMemQueue.h"
#             "CQueueBase.h"
#             "CMemoryQueueBase.h"
#             "CMemQueue.h"
#             "CMemIPCPy.h"
#             "CLocalQueue.h"
#   )

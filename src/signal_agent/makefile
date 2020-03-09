include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include
# io (part of)

# Internal (Library Linkage) #
# io (static)
# logger (static)

# External (Source Available)#
# log4cplus : 2.0.0 (static)
# libev : 4.24 (for TunnelAgent)

# External (Ubuntu-package available)
# C++11

# External (Source Unavailable)#
# None

#######################

CFLAGS      += -I${BASEDIR}/io -I${BASEDIR}/server_utils -I${BASEDIR}/logger ${log4cplus_CFLAGS} ${libev_CFLAGS}

LDFLAGS     += -L${BASEDIR}/io -lio -L${BASEDIR}/logger -llog ${log4cplus_LDFLAGS} ${libev_LDFLAGS}

VPATH 			+= $(BASEDIR)/io $(BASEDIR)/server_utils

RELEASE			:= ${BASEDIR}/../release/SignalAgent/bin

build			= ${RELEASE}/SignalAgent

build: release ${build}

${RELEASE}/SignalAgent: CSocket.o CService.o CListener.o CBuffer.o CIOBase.o utils.o CTimer.o CRawIOReader.o CRawIOWriter.o \
  MurmurHash2.o CGlobalLock.o CGlobalSafeRawIOWriter.o CSignal.o CSignalAgent.o CSignalRequester.o CSignalServer.o main.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include

# Internal (Library Linkage) #
# logger (static)

# External (Source Available)#
# log4cplus : 2.0.0 (static)

# External (Ubuntu-package available)
# C++11

# External (Source Unavailable)#
# NONE

#######################

CFLAGS  += -I${BASEDIR}/logger $(log4cplus_CFLAGS)

LDFLAGS += -L${BASEDIR}/logger -llog $(log4cplus_LDFLAGS)

VPATH   +=

build		= ${RELEASE}/libio.a ${RELEASE}/IOMonitor ${RELEASE}/OTMonitor ${RELEASE}/IOTools

build: release ${build}

${RELEASE}/libio.a: CIOBase.o utils.o CTimer.o CRawIOReader.o CRawIOWriter.o MurmurHash2.o CGlobalLock.o CGlobalSafeRawIOWriter.o CSignal.o CRawIOModify.o CSystemIO.o
	$(AR) $@ $^

${RELEASE}/IOTools: IOTools.o
	${CXX} $^ -lio ${LDFLAGS} -o $@

${RELEASE}/IOMonitor: IOMonitor.o
	${CXX} $^ -lio ${LDFLAGS} -o $@

${RELEASE}/OTMonitor: OTMonitor.o
	${CXX} $^ -lio ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include
# config (header only)

# Internal (Library Linkage) #
# logger (static)
# crypt (static)

# External (Source Available)#
# log4cplus : 2.0.0 (static)
# libev : 4.24 (for TunnelAgent)

# External (Ubuntu-package available)
# C++11
# openssl
# boost_python
# python

# External (Source Unavailable)#
# None

#######################

CFLAGS      += -I${BASEDIR}/logger ${log4cplus_CFLAGS} -I${BASEDIR}/crypt -I${BASEDIR}/config ${libev_CFLAGS} ${boost_CFLAGS} ${PYTHON_CFLAGS}

LDFLAGS      += -L${BASEDIR}/logger -llog ${log4cplus_LDFLAGS} -L${BASEDIR}/crypt -lccrypt ${OPENSSL_LDFLAGS} ${libev_LDFLAGS} ${boost_python_LDFLAGS} ${PYTHON_LDFLAGS}

VPATH 			+=

RELEASE			:= ${BASEDIR}/../release/TunnelAgent/bin

build			= GenRSAKey ${RELEASE}/TunnelAgent TunnelClient ${RELEASE}/LibLoader ${RELEASE}/PyLoader ${RELEASE}/StgManager

build: release ${build}

${RELEASE}/TunnelAgent: main.o CTunnelAgent.o CWaiter.o CCommanderBase.o CSaveFileCmd.o CShellCmd.o CPythonExecutor.o\
  CFileReciever.o utils.o CExecLoader.o CDirReciever.o public_key.o CStrategyManager.o MurmurHash2.o
	${CXX} $^ ${LDFLAGS} -o $@

TunnelClient: client.o CClient.o utils.o private_key.o
	${CXX} $^ ${LDFLAGS} -o $@

# Generate publib/private key pair and source file public_key.cpp, private_key.cpp
GenRSAKey:
	../tools/sshkeytransfer

##########################

${RELEASE}/LibLoader: LibLoader.o utils.o
	${CXX} $^ ${LDFLAGS} -o $@

${RELEASE}/PyLoader: PyLoader.o utils.o
	${CXX} $^ ${LDFLAGS} -o $@

${RELEASE}/StgManager: StgManager.o utils.o MurmurHash2.o
	${CXX} $^ ${LDFLAGS} -o $@



# must put at the end
include ../make/tail.mk


include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include

# Internal (Library Linkage) #
# io (static)
# logger (static)
# crypt (static, for sshkeytransfer)

# External (Source Available)#
# log4cplus : 2.0.0 (static)

# External (Ubuntu-package available)
# C++11

# External (Source Unavailable)#
# None

#######################

CFLAGS      += -I${BASEDIR}/io -I${BASEDIR}/logger ${log4cplus_CFLAGS} -I${BASEDIR}/crypt

LDFLAGS     += -L${BASEDIR}/io -lio -L${BASEDIR}/logger -llog ${log4cplus_LDFLAGS} -L${BASEDIR}/crypt -lccrypt

VPATH 			+= $(BASEDIR)/io

RELEASE  := ${BASEDIR}/../release/tools

build			= sshkeytransfer fixmday syscmd ctpdump2mmap dump2csv ${RELEASE}/orderextract


build: release ${build}

sshkeytransfer: sshkeytransfer.o
	${CXX} $^ ${LDFLAGS} -o $@

fixmday: fixmday.o
	${CXX} $^ ${LDFLAGS} -o $@

syscmd: syscmd.o
	${CXX} $^ ${LDFLAGS} -o $@

ctpdump2mmap : ctpdump2mmap.o CTradeBaseInfo.o
	${CXX} $^ ${LDFLAGS} -o $@
	@if [ ! -d ${BASEDIR}/../release/SDK ]; then  mkdir -p ${BASEDIR}/../release/SDK;  fi
	cp $@ ${BASEDIR}/../release/SDK/$@

dump2csv : dump2csv.o utils.o
	${CXX} $^ ${LDFLAGS} -o $@
	@if [ ! -d ${BASEDIR}/../release/SDK ]; then  mkdir -p ${BASEDIR}/../release/SDK;  fi
	cp $@ ${BASEDIR}/../release/SDK/$@


${RELEASE}/orderextract: orderextract.o 
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


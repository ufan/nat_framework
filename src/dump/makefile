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
# CTP

#######################


CFLAGS      += -I${BASEDIR}/logger ${log4cplus_CFLAGS} ${CTP_CFLAGS}

LDFLAGS     += -L${BASEDIR}/logger -llog ${log4cplus_LDFLAGS} ${CTP_LDFLAGS}

VPATH 			+=

RELEASE			:= ${BASEDIR}/../release/dump/bin


build			:= ${RELEASE}/CTPDumper


build: release ${build}

${RELEASE}/CTPDumper: CTPDumper.o CCTPTD.o CCTPMD.o FileMgr.o DailyInfoMgr.o utils.o CEncodeConv.o CTimer.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


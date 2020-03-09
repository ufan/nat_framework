include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include
# io (part of)
# account
# md
# td
# logger
# bar

# Internal (Library Linkage) #
# io (static)
# logger (static)

# External (Source Available)#
# log4cplus : 2.0.0 (static)

# External (Ubuntu-package available)
# C++11

# External (Source Unavailable)#
# None

#######################

CFLAGS      += -I${BASEDIR}/io -I${BASEDIR}/account -I${BASEDIR}/bar \
							 -I${BASEDIR}/md -I${BASEDIR}/td -I${BASEDIR}/logger \
							 ${log4cplus_CFLAGS}

LDFLAGS     +=  -L${BASEDIR}/io -lio -L${BASEDIR}/logger -llog ${log4cplus_LDFLAGS}

VPATH 			+= $(BASEDIR)/io $(BASEDIR)/account $(BASEDIR)/bar $(BASEDIR)/md $(BASEDIR)/td $(BASEDIR)/logger

RELEASE   = ${BASEDIR}/../release/test/bin

build			= ${RELEASE}/test testmdlatency

build: release ${build}

${RELEASE}/test: test.o CTradeBaseInfo.o CTimer.o MurmurHash2.o utils.o\
 UnitVol.o UnitAmt.o UnitPx.o UnitPnl.o ModInstr.o ModPrd.o ModAcc.o AccBase.o\
 ConfUtil.o RiskInstr.o RiskInstrStg.o RiskInstrTop.o RiskPrd.o RiskAcc.o RiskStg.o RiskTop.o \
 CLogPool.o CFastLog.o
	${CXX} $^ ${LDFLAGS} -o $@

testmdlatency: testmdlatency.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


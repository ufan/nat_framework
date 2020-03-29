include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include
# account
# risk
# md
# td
# pyext
# bar

# Internal (Library Linkage) #
# logger (static)
# io (static)

# External (Source Available)#
# None

# External (Ubuntu-package available)
# C++11
# boost_python
# python

# External (Source Unavailable)#
# None

#######################

CFLAGS      += -DFAST_LOGGER_ -I$(BASEDIR)/logger \
							 -I$(BASEDIR)/io -I$(BASEDIR)/md -I$(BASEDIR)/td -I$(BASEDIR)/account \
							 -I$(BASEDIR)/risk -I$(BASEDIR)/pyext -I$(BASEDIR)/bar \
							 ${boost_CFLAGS} ${PYTHON_CFLAGS}

LDFLAGS     += ${SHARED_LDFLAGS} ${boost_python_LDFLAGS} ${PYTHON_LDFLAGS}

VPATH 			+= $(BASEDIR)/io $(BASEDIR)/md $(BASEDIR)/td $(BASEDIR)/account \
							 $(BASEDIR)/risk $(BASEDIR)/pyext $(BASEDIR)/bar $(BASEDIR)/logger

RELEASE   = ${BASEDIR}/lib

build			= ${RELEASE}/libstrategy.so


build: release ${build}

${RELEASE}/libstrategy.so: CStrategyProcess.o CStrategy.o CMDHelperFactory.o CTDHelperFactory.o \
 IMDHelper.o ITDHelper.o CMDHelperComm.o CTDHelperComm.o CMDHelperReplayIO.o CTDHelperFake.o \
 StrategyShared.o CTradeBaseInfo.o \
 ATSysUtils.o PyExtension.o PyUtils.o PyWrapper.o BarMaker.o Bar2.o BarHelper.o \
 UnitVol.o UnitAmt.o UnitPx.o UnitPnl.o ModInstr.o ModPrd.o ModAcc.o AccBase.o\
 ConfUtil.o RiskInstr.o RiskInstrStg.o RiskInstrTop.o RiskPrd.o RiskAcc.o RiskStg.o RiskTop.o\
 CWareHouseReader.o CMDHelperReplayCtpDump.o CMDHelperPipe.o CTDHelperPipe.o CPipExecutor.o\
 CTDHelperPython.o CMDHelperPython.o PyExtExch.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk

 

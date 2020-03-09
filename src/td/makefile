include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include
# io (part of)
# risk
# account

# Internal (Library Linkage) #
# logger (static)
# io (static)

# External (Source Available)#
# log4cplus : 2.0.0 (static)
# json 2.1.1 (integration) https://github.com/nlohmann/json
# boost_intrusive (for risk)

# External (Ubuntu-package available)
# C++11

# External (Source Unavailable)#
# CTP (dynamic)
# EES (dynamic)
# Xt (dynamic)

#######################


CFLAGS      += -I${BASEDIR}/io -I${BASEDIR}/logger ${log4cplus_CFLAGS} -I${BASEDIR}/account -I${BASEDIR}/risk ${boost_CFLAGS} ${CTP_CFLAGS}

LDFLAGS     += -L${BASEDIR}/io -lio -L${BASEDIR}/logger -llog  ${log4cplus_LDFLAGS} ${CTP_LDFLAGS}

VPATH 			+= $(BASEDIR)/io $(BASEDIR)/account $(BASEDIR)/risk

RELEASE     := ${BASEDIR}/../release/TDEngine/bin

build			= ${RELEASE}/TDEngine ${RELEASE}/TDEngineEESV2 ${RELEASE}/TDEngineXt ${RELEASE}/TDEngineEESV31347

build: release ${build}

${RELEASE}/TDEngine: CFLAGS += ${EES_CFLAGS}
${RELEASE}/TDEngine: LDFLAGS += ${EES_LDFLAGS}
${RELEASE}/TDEngine: TDEngine.o ITDEngine.o CSystemIO.o CEncodeConv.o CTradeBaseInfo.o COrderTrackMmap.o \
 UnitVol.o UnitAmt.o UnitPx.o UnitPnl.o ModInstr.o ModPrd.o ModAcc.o AccBase.o\
 ConfUtil.o RiskInstr.o RiskInstrStg.o RiskInstrTop.o RiskPrd.o RiskAcc.o RiskStg.o RiskTop.o\
 CTDEngineCtp.o CTDEngineEES.o
	${CXX} $^ ${LDFLAGS} -o $@

${RELEASE}/TDEngineEESV2: CFLAGS += ${EESV2_CFLAGS}
${RELEASE}/TDEngineEESV2: LDFLAGS += ${EESV2_LDFLAGS}
${RELEASE}/TDEngineEESV2: TDEngineEESV2.o ITDEngine.o CSystemIO.o CEncodeConv.o CTradeBaseInfo.o COrderTrackMmap.o \
 UnitVol.o UnitAmt.o UnitPx.o UnitPnl.o ModInstr.o ModPrd.o ModAcc.o AccBase.o\
 ConfUtil.o RiskInstr.o RiskInstrStg.o RiskInstrTop.o RiskPrd.o RiskAcc.o RiskStg.o RiskTop.o\
 CTDEngineCtp.o CTDEngineEESV2.o
	${CXX} $^ ${LDFLAGS} -o $@

${RELEASE}/TDEngineXt: CFLAGS += ${XT_CFLAGS}
${RELEASE}/TDEngineXt: LDFLAGS += ${XT_LDFLAGS}
${RELEASE}/TDEngineXt: TDEngineXt.o ITDEngine.o CSystemIO.o CEncodeConv.o CTradeBaseInfo.o COrderTrackMmap.o \
 UnitVol.o UnitAmt.o UnitPx.o UnitPnl.o ModInstr.o ModPrd.o ModAcc.o AccBase.o\
 ConfUtil.o RiskInstr.o RiskInstrStg.o RiskInstrTop.o RiskPrd.o RiskAcc.o RiskStg.o RiskTop.o\
 CTDEngineXt.o
	${CXX} $^ ${LDFLAGS} -o $@

${RELEASE}/TDEngineEESV31347: CFLAGS += ${EESV3_CFLAGS}
${RELEASE}/TDEngineEESV31347: LDFLAGS += ${EESV3_LDFLAGS}
${RELEASE}/TDEngineEESV31347: TDEngine.o ITDEngine.o CSystemIO.o CEncodeConv.o CTradeBaseInfo.o COrderTrackMmap.o \
 UnitVol.o UnitAmt.o UnitPx.o UnitPnl.o ModInstr.o ModPrd.o ModAcc.o AccBase.o\
 ConfUtil.o RiskInstr.o RiskInstrStg.o RiskInstrTop.o RiskPrd.o RiskAcc.o RiskStg.o RiskTop.o\
 CTDEngineCtp.o CTDEngineEES.o
	${CXX} $^ ${LDFLAGS} -o $@

# must put at the end
include ../make/tail.mk


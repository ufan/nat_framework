include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include
# io (part of)

# Internal (Library Linkage) #
# logger (static)
# io (static)

# External (Source Available)#
# log4cplus : 2.0.0 (static)
# json 2.1.1 (integration) https://github.com/nlohmann/json

# External (Ubuntu-package available)
# C++11

# External (Source Unavailable)#
# CTP (dynamic)
# OpenOnload (static)
# EES (dynamic)

#######################

CFLAGS  += -I${BASEDIR}/io -I${BASEDIR}/logger ${log4cplus_CFLAGS} ${CTP_CFLAGS} $(OpenOnload_CFLAGS)

LDFLAGS += -L${BASEDIR}/io -lio -L${BASEDIR}/logger -llog  ${log4cplus_LDFLAGS} ${CTP_LDFLAGS} ${OpenOnload_LDFLAGS}

VPATH	  += $(BASEDIR)/io

RELEASE			:= ${BASEDIR}/../release/MDEngine/bin

build			= ${RELEASE}/MDEngine ${RELEASE}/MDEngineEESV2


build: release ${build}

${RELEASE}/MDEngine: CFLAGS += ${EES_CFLAGS}
${RELEASE}/MDEngine: LDFLAGS += ${EES_LDFLAGS}
${RELEASE}/MDEngine: MDEngine.o CMDEngineCtp.o CMDEngineEES.o IMDEngine.o CSystemIO.o CTradeBaseInfo.o ATSysUtils.o CEESOnload.o CMDEngineEESOnload.o
	${CXX} $^ ${LDFLAGS} -o $@

${RELEASE}/MDEngineEESV2: CFLAGS += ${EESV2_CFLAGS}
${RELEASE}/MDEngineEESV2: LDFLAGS += ${EESV2_LDFLAGS}
${RELEASE}/MDEngineEESV2: MDEngine.o CMDEngineCtp.o CMDEngineEES.o IMDEngine.o CSystemIO.o CTradeBaseInfo.o ATSysUtils.o CEESOnload.o CMDEngineEESOnload.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


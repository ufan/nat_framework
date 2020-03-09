include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include
# config (header only)

# Internal (Library Linkage) #
# None

# External (Source Available)#
# None

# External (Ubuntu-package available)
# C++11
# boost_algorithm

# External (Source Unavailable)#
# CTP (dynamic)
# EES (dynamic)

#######################

CFLAGS      += -I${BASEDIR}/config ${boost_CFLAGS} ${CTP_CFLAGS} ${EES_CFLAGS}

LDFLAGS     += ${CTP_LDFLAGS} ${EES_LDFLAGS}

VPATH 			+=

RELEASE			:= ${BASEDIR}/../release/TradeTool

build = ${RELEASE}/TradeTool

build: release ${build}

${RELEASE}/TradeTool: TradeTool.o CTraderFactory.o CTraderBase.o CGlobalParameter.o CCtpTrader.o CCtpMiniTrader.o CRisk.o CEncodeConv.o CEESTrader.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


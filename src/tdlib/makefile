include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include

# Internal (Library Linkage) #
# strategy (dynamic)

# External (Source Available)#
# log4cplus : 2.0.0 (static)

# External (Ubuntu-package available)
# C++11
# boost_python
# python

# External (Source Unavailable)#
# NONE

#######################

CFLAGS    += -DFAST_LOGGER_ -I${BASEDIR}/strategy \
						 -I$(BASEDIR)/logger ${log4cplus_CFLAGS} \
						 -I$(BASEDIR)/io -I$(BASEDIR)/md -I$(BASEDIR)/td -I$(BASEDIR)/account \
						 -I$(BASEDIR)/risk -I$(BASEDIR)/pyext -I$(BASEDIR)/bar \
						  ${boost_CFLAGS} ${PYTHON_CFLAGS}

LDFLAGS   += ${SHARED_LDFLAGS} -L$(BASEDIR)/lib -lstrategy ${boost_python_LDFLAGS} ${PYTHON_LDFLAGS} ${log4cplus_LDFLAGS}

VPATH 		+=

build			= libexecstg.so

build: ${build}

libexecstg.so: CMarketStruct.o CBaseSignal.o CABM.o  CAvgSpread.o CDriftModel.o CLiquidityModel.o CLogitPModel.o  CSigmaSignal.o CVolatilityModel.o CLossModel.o main.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk

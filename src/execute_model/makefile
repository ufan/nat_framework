include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include

# Internal (Library Linkage) #
# strategy (dynamic)

# External (Source Available)#
# None

# External (Ubuntu-package available)
# C++11
# boost_python
# python

# External (Source Unavailable)#
# NONE

#######################

CFLAGS    += -DFAST_LOGGER_ -I${BASEDIR}/strategy -I$(BASEDIR)/logger \
						 -I$(BASEDIR)/io -I$(BASEDIR)/md -I$(BASEDIR)/td -I$(BASEDIR)/account \
						 -I$(BASEDIR)/risk -I$(BASEDIR)/pyext -I$(BASEDIR)/bar \
						  ${boost_CFLAGS} ${PYTHON_CFLAGS}

LDFLAGS   += ${SHARED_LDFLAGS} -L$(BASEDIR)/lib -lstrategy ${boost_python_LDFLAGS} ${PYTHON_LDFLAGS}

VPATH 		+=

RELEASE		= $(BASEDIR)/lib

build			= ${RELEASE}/libexecstg.so


build: release ${build}

${RELEASE}/libexecstg.so: CMarketStruct.o CBaseSignal.o CABM.o  CAvgSpread.o CDriftModel.o CLiquidityModel.o CLogitPModel.o  CSigmaSignal.o CVolatilityModel.o CLossModel.o CExecuteStrategy.o \
 PyExecStgExtension.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


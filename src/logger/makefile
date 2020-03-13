include ../make/head.mk

#### DEPENDENCIES ###

# Internal (Source Integration) #
# common + include

# Internal (Library Linkage) #
# NONE

# External (Source Available)#
# log4cplus : 2.0.0 (static)

# External (Ubuntu-package available)
# C++11

# External (Source Unavailable)#
# NONE

#######################

CFLAGS += ${log4cplus_CFLAGS}

LDFLAGS += ${log4cplus_LDFLAGS}

VPATH   +=

build		= ${RELEASE}/liblog.a ${RELEASE}/test

build: release ${build}

${RELEASE}/liblog.a: HeavyLogger.o \
										 FastLogger.o CFastLog.o CLogPool.o
	${AR} $@ $^

# ${RELEASE}/test: LDFLAGS += ${STATIC_FLAGS}
${RELEASE}/test: test.o CLogPool.o CFastLog.o utils.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


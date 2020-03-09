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
# openSSL

# External (Source Unavailable)#
# CTP (dynamic)
# OpenOnload (static)
# EES (dynamic)

#######################


CFLAGS    += -I${BASEDIR}/logger ${log4cplus_CFLAGS}

LDFLAGS   += -L${BASEDIR}/logger -llog ${log4cplus_LDFLAGS} ${OPENSSL_LDFLAGS}

build			= ${RELEASE}/libccrypt.a test

build: release ${build}

${RELEASE}/libccrypt.a: CCrypt.o
	${AR} $@ $^

test: test.o CCrypt.o
	${CXX} $^ ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


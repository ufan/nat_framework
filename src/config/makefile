include ../make/head.mk

CFLAGS  += ${boost_CFLAGS}

build			= test

build: ${build}

test: test.o
	${CXX} $^ ${LDFLAGS} -o $@

# must put at the end
include ../make/tail.mk


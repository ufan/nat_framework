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

CFLAGS  += ${boost_CFLAGS} ${PYTHON_CFLAGS}

DFLAGS += ${boost_python_LDFLAGS} ${PYTHON_LDFLAGS}

VPATH 			+=

build = test testpython.so LogDB.so

build : ${build}

test: test.o CLogDB.o CLogTable.o
	${CXX} $^ ${LDFLAGS} -o $@

testpython.so: testpython.o
	${CXX} $^ --shared ${LDFLAGS} -o $@

LogDB.so: CLogDB.o CLogTable.o CPyLogTable.o CPyLogDB.o
	${CXX} $^ --shared ${LDFLAGS} -o $@


# must put at the end
include ../make/tail.mk


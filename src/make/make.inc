.PHONY:clean rebuild

CC				=	/usr/bin/g++
#LD     		=	$(CROSS_COMPILE)xild

RM				= 	rm -rf

BASEDIR			=	..

BUILD_FLAGS		= 	-g -O3 -std=c++11 -fPIC -Wno-unused-result

LIBPATHS		=  	-L/usr/local/lib -L/usr/lib/x86_64-linux-gnu -L$(BASEDIR)/lib -L./ -L$(BASEDIR)/logger \
	-L$(BASEDIR)/ThirdParty/log4cplus/lib

LIBS        	=  	-llog -llog4cplus -lpthread -lstdc++

INCLUDEPATHS	=  	-I./ -I$(BASEDIR)/include -I/usr/local/include -I/usr/include -I$(BASEDIR)/common -I$(BASEDIR)/logger \
	-I$(BASEDIR)/ThirdParty/log4cplus/include

VPATH	= ./ $(BASEDIR)/common


RELEASE		=	.
DEBUG		= 	.

SRC		= 	$(wildcard *.cpp)

OBJ		= 	$(patsubst %.cpp,%.o,$(SRC))


.cpp.o:
	${CC} ${BUILD_FLAGS} -c ${INCLUDEPATHS} -o $@ $<


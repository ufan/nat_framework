.PHONY : clean build release

CXX				=	/usr/bin/g++

RM				= 	rm -rf

AR        =  ar -rc

#####################################

BASEDIR			=	..

RELEASE		=	.

DEBUG		= 	.

#####################################

COMMON_INCLUDE	=  	-I./ -I$(BASEDIR)/include -I$(BASEDIR)/common

CFLAGS		= -g -O3 -std=c++11 -fPIC -Wno-unused-result -Wno-deprecated-declarations $(COMMON_INCLUDE)

LDFLAGS     =  -L$(BASEDIR)/lib -L./ -ldl -lpthread -lstdc++

# SHARED_CFLAGS = -fPIC

SHARED_LDFLAGS = -shared
STATIC_LDFLAGS = -static

VPATH	= ./ $(BASEDIR)/common

#####################################

# CTP
CTP_DIR = $(BASEDIR)/ThirdParty/CTP
CTP_CFLAGS = -I$(CTP_DIR)
CTP_LDFLAGS = -L$(CTP_DIR) -lthosttraderapi -lthostmduserapi

# CTP Mini
CTPMINI_DIR = $(BASEDIR)/ThirdParty/CTPMini
CTPMINI_CFLAGS = -I$(CTPMINI_DIR)
CTPMINI_LDFLAGS = -L$(CTPMINI_DIR) -lthosttraderapi -lthostmduserapi

# EES
EES_DIR = $(BASEDIR)/ThirdParty/EES
EES_CFLAGS = -I$(EES_DIR)
EES_LDFLAGS = -L$(EES_DIR) -lEESTraderApi -lEESQuoteApi

# EESV2
EESV2_DIR = $(BASEDIR)/ThirdParty/EESV2
EESV2_CFLAGS = -I$(EESV2_DIR)
EESV2_LDFLAGS = -L$(EESV2_DIR) -lEESTraderApi -lEESQuoteApi

# EESV3
EESV3_DIR = $(BASEDIR)/ThirdParty/EES3_1_3_47
EESV3_CFLAGS = -I$(EESV3_DIR)
EESV3_LDFLAGS = -L$(EESV3_DIR) -lEESTraderApi -lEESQuoteApi

# Xele
XELE_DIR = $(BASEDIR)/ThirdParty/Xele
XELE_CFLAGS = -I$(XELE_DIR)
XELE_LDFLAGS = -L$(XELE_DIR) -lXeleTdAPI64 -lXeleMdAPI64

# Xt
XT_DIR = $(BASEDIR)/ThirdParty/Xt
XT_CFLAGS = -I$(XT_DIR)
XT_LDFLAGS = -L$(XT_DIR) -lXtTraderApi

# OpenOnload
OpenOnload_DIR = $(BASEDIR)/ThirdParty/OpenOnload
OpenOnload_CFLAGS = -I$(OpenOnload_DIR)
OpenOnload_LDFLAGS = -L$(OpenOnload_DIR) -lonload_ext -lciul1

#####################################

# log4cplus
log4cplus_DIR = $(BASEDIR)/ThirdParty/log4cplus
log4cplus_CFLAGS = -I$(log4cplus_DIR)/include
log4cplus_LDFLAGS = -L$(log4cplus_DIR)/lib -llog4cplus

# libev
libev_DIR = $(BASEDIR)/ThirdParty/libev
libev_CFLAGS = -I$(libev_DIR)/include
libev_LDFLAGS = -L$(libev_DIR)/lib -lev

#####################################

# boost
boost_DIR = /home/yong/boost/1_72_0
boost_CFLAGS = -I$(boost_DIR)/include
boost_python_LDFLAGS = -L$(boost_DIR)/lib -lboost_python27

# python
PYTHON_BIN = python2.7
PYTHON_CONFIG = $(PYTHON_BIN)-config
PYTHON_CFLAGS := $(shell $(PYTHON_CONFIG) --cflags)
PYTHON_LDFLAGS := $(shell $(PYTHON_CONFIG) --ldflags)

# openssl
OPENSSL_LDFLAGS = -lssl -lcrypto
#####################################



SRC		= 	$(wildcard *.cpp)

OBJ		= 	$(patsubst %.cpp, %.o ,$(SRC))

%.o : %.cpp
	${CXX} -c $(CFLAGS) $< -o $@

#!/bin/bash

LIB=/home/jarvis/lib:/home/jarvis/lib/ThirdParty/CTP

ENGINE=/home/jarvis/dump/bin/CTPDumper
CONF=/home/jarvis/dump/conf/dump.json

function GetPid()
{
	PID=`ps -ef|grep ${ENGINE}|grep ${CONF}|grep -v grep|awk '{print $2}'`
	echo ${PID}
}

function Start()
{
	PID=`GetPid`
	if [[ -z ${PID} ]]
	then
		ulimit -c unlimited
		LD_LIBRARY_PATH=${LIB} ${ENGINE} ${CONF} 2>&1 &
	fi
}

function Stop()
{
	PID=`GetPid`
	if [[ -n ${PID} ]]
	then
		kill -s 2 ${PID}
	fi
}


if [[ $1 == "start" ]]
then
	Start
elif [[ $1 == "stop" ]]
then
	Stop
fi

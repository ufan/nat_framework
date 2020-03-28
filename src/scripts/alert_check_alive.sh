#!/bin/bash

BIN=/home/jarvis/scripts/alert_listener.py

function GetPid()
{
	PID=`ps -ef|grep ${BIN}|grep -v grep|awk '{print $2}'`
	echo ${PID}
}

PID=`GetPid`
if [[ -z ${PID} ]]
then
	wall "ALERT|$(date +'%Y%m%d %H:%M:%S')|${BIN} down!"
	echo "ALERT|$(date +'%Y%m%d %H:%M:%S')|${BIN} down!"
	nohup ${BIN} >/dev/null 2>&1 &
fi

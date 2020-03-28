#!/bin/bash

# library path
LIB=/home/yong/src/nat_framework/NAT/install/lib:/home/yong/src/nat_framework/NAT/install/lib/CTP:/home/yong/src/nat_framework/NAT/install/lib/Xele:/home/yong/src/nat_framework/NAT/install/lib/EES:/home/yong/src/nat_framework/NAT/install/lib/Xt

# configuration
#TDENGINE=/home/yong/src/nat_framework/NAT/install/TDEngine/bin/TDEngineXt
#TDCONF=/home/yong/src/nat_framework/NAT/install/conf/tdengine.json.xt
TDENGINE=/home/yong/src/nat_framework/NAT/install/TDEngine/bin/TDEngine
TDCONF=/home/yong/src/nat_framework/NAT/install/conf/tdengine.json

MDENGINE=/home/yong/src/nat_framework/NAT/install/MDEngine/bin/MDEngine
MDCONF=/home/yong/src/nat_framework/NAT/install/conf/mdengine_test.json

# unlimited core dump file size
ulimit -c unlimited

# get mdengine pid
function GetMDPid()
{
	PID=`ps -ef|grep ${MDENGINE}|grep ${MDCONF}|grep -v grep|awk '{print $2}'`
	echo ${PID}
}

# get tdengine pid
function GetTDPid()
{
	PID=`ps -ef|grep ${TDENGINE}|grep ${TDCONF}|grep -v grep|awk '{print $2}'`
	echo ${PID}
}

# start td engine
function StartTD()
{
	PID=`GetTDPid`
	if [[ -z ${PID} ]]
	then
		LD_LIBRARY_PATH=${LIB} ${TDENGINE} -f ${TDCONF}
	fi
}

# kill td engine
function StopTD()
{
	PID=`GetTDPid`
	if [[ -n ${PID} ]]
	then
		kill ${PID}
	fi
}

# start md engine
function StartMD()
{
	PID=`GetMDPid`
	if [[ -z ${PID} ]]
	then
		LD_LIBRARY_PATH=${LIB} ${MDENGINE} -f ${MDCONF}
	fi
}

# kill md engine
function StopMD()
{
	PID=`GetMDPid`
	if [[ -n ${PID} ]]
	then
		kill ${PID}
	fi
}

# start both td and md engine
function Start()
{
	 TIME=`date +"%Y-%m-%d %H:%M:%S"`
	 StartTD
	 
	 for ((i=0;i<30;i++))
	 do
	 	STARTLOG=`tail -n 5 /home/yong/src/nat_framework/NAT/install/logs/Engine/TDEngine.log|awk -F'|' '$1>"'"${TIME}"'"{print $0}'|grep "start listening"`
	 	if [[ x"${STARTLOG}" != "x" ]]; then break; fi
	 	sleep 2
	 done
	 
	 StartMD
}

# stop both td and md engine
function Stop()
{
	 StopTD
	 StopMD
}

# start td and md together, and immediately stop td engine
function StartMDOnly()
{
	Start
	StopTD
}

# check whether md engine alive
function checkMD()
{
	PID=`GetMDPid`
	if [[ -z ${PID} ]]
	then
		echo "ALERT|$(date +'%Y%m%d %H:%M:%S')|${MDENGINE} down!"
	fi
}

# check whether td engine alive
function checkTD()
{
	PID=`GetTDPid`
	if [[ -z ${PID} ]]
	then
		echo "ALERT|$(date +'%Y%m%d %H:%M:%S')|${TDENGINE} down!"
	fi
}

if [[ $1 == "start" ]]
then
	Start
elif [[ $1 == "stop" ]]
then
	Stop
elif [[ $1 == "restart" ]]
then
	Stop
	Start
elif [[ $1 == "startmd" ]]
then
	StartMD
elif [[ $1 == "starttd" ]]
then
	StartTD
elif [[ $1 == "startmdonly" ]]
then
	StartMDOnly
elif [[ $1 == "stopmd" ]]
then
	StopMD
elif [[ $1 == "stoptd" ]]
then
	StopTD
elif [[ $1 == "checktd" ]]
then
	checkTD
elif [[ $1 == "checkmd" ]]
then
	checkMD
elif [[ $1 == "check" ]]
then
	checkTD
	checkMD
else
	echo "usage: $0 [start|stop|restart|startmd|starttd|startmdonly|stopmd|stoptd|checktd|checkmd|check]"
	exit -1
fi



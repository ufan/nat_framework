#!/bin/bash

PWD=`pwd`

function ArchConf()
{
	if [[ $# -eq 3 ]]
	then
		CONFDIR=$1
		OLDTAR=$2
		ARCHDIR=$3
		
		HASH=`echo $CONFDIR|md5sum`
		TMPDIR=.tmp${HASH:0:10}
		
		mkdir TMPDIR
		tar xfz ${OLDTAR} -C TMPDIR/
		if [[ $? -eq 0 ]]
		then
			TARDIR=`ls TMPDIR|head -n 1`
			OLDCONF=TMPDIR/${TARDIR}
			
			if [[ -z `diff -r ${CONFDIR} ${OLDCONF}` ]]
			then
				rm -r TMPDIR
				return
			fi
		fi
		rm -r TMPDIR
	elif [[ $# -eq 2 ]]
	then
		CONFDIR=$1
		ARCHDIR=$2
	fi

	DIR=`dirname ${CONFDIR}`
	BASE=`basename ${CONFDIR}`
		
	TODAY=`date +"%Y%m%d"`
	(cd ${DIR}; tar cfz ${ARCHDIR}/${BASE}_${TODAY}.tgz ${BASE})
	echo "$(date +'%Y%m%d %H:%M:%S')|Archive $CONFDIR"
}

function ArchLog()
{
	LOGDIR=$1
	ARCHDIR=$2
	DIR=`dirname ${LOGDIR}`
	BASE=`basename ${LOGDIR}`
	if [ -d ${DIR} ]
	then
		(cd ${DIR}; tar cfz ${ARCHDIR}/${BASE}.tgz ${BASE})
	fi
}

function DailyMakeLogDir()
{
	TIMESTAMP=`date +%s`
	TIMESTAMP_INDAY=`expr $((TIMESTAMP)) % 86400 / 3600`
	if [ ${TIMESTAMP_INDAY} -lt 9 ]
	then
		NEXTDAY=`date +"%Y%m%d"`
	else
		WEEKDAY=`date +"%w"`
		NEXTDAY=`date -d'tomorrow' +"%Y%m%d"`
		if [ ${WEEKDAY} -eq 5 ]
		then
			NEXTDAY=`date -d'-3 days ago' +"%Y%m%d"`
		elif [ ${WEEKDAY} -eq 6 ]
		then
			NEXTDAY=`date -d'-2 days ago' +"%Y%m%d"`
		fi
	fi
	
	BASE=/home/jarvis/logdir/logs_${NEXTDAY}
	mkdir -p ${BASE}/Engine
	mkdir -p ${BASE}/Strategy
	mkdir -p ${BASE}/Dump
	rm /home/jarvis/logs 2>/dev/null
	ln -s ${BASE} /home/jarvis/logs

	RMDAY=`date +"%Y%m%d" -d "-5 days"`
	rm -rf /home/jarvis/logdir/logs_${RMDAY}
}

function DailyArchLog()
{
	LOG=`readlink /home/jarvis/logs`
	ArchLog ${LOG} /home/jarvis/archive
	echo "$(date +'%Y%m%d %H:%M:%S')|Archive ${LOG}"
}

function DailyArchConf()
{
	cd /home/jarvis
	TAR=`ls -t /home/jarvis/archive/conf*.tgz|head -n 1`
	ArchConf /home/jarvis/conf ${TAR} /home/jarvis/archive
	cd -
}

if [[ $# -eq 0 ]]
then
	echo "Usage $0 [arch|prepare]"
	exit -1
fi

case $1 in
	prepare)
		DailyMakeLogDir
	;;
	arch)
		DailyArchLog
		DailyArchConf
		DailyMakeLogDir
	;;
	*)
		echo "Unknown option $1"
	;;
esac

exit 0

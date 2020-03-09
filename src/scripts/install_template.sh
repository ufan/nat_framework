#!/bin/bash

PWD=`pwd`
ME=`whoami`
DEPLOYDIR="/home/jarvis"
THIS_DIR=$(DIRNAME=$(dirname "$0"); cd "$DIRNAME"; pwd)
THIS_FILE=$(basename "$0")
THIS_PATH="$THIS_DIR/$THIS_FILE"


function AddCron()
{    
	printf "Add crontab [yes|no]?"
	read -r ans
	if [[ ${ans} != "y" && ${ans} != "Y" && ${ans} != "yes" ]]; then
		return
	fi
	
	if [[ x`crontab -l|grep /home/jarvis` == "x" ]]
	then
		TODAY=`date +"%Y%m%d %H:%M:%S"`
		echo "----------------- ${TODAY} ---------------" >> /home/${ME}/cron_${ME}.bak || exit 1
		crontab -l >> /home/${ME}/cron_${ME}.bak || exit 1
		
		(
		crontab -l;
		echo "#---------------- add by at installer --------------"
		echo "40 08 * * 1-5 /home/jarvis/scripts/engine_svr.sh start" ;
		echo "00 17 * * 1-5 /home/jarvis/scripts/engine_svr.sh stop" ;
		echo "#45 08 * * 1-5 /home/jarvis/scripts/dump_svr.sh start" ;
		echo "#00 17 * * 1-5 /home/jarvis/scripts/dump_svr.sh stop" ;
		echo "50 08 * * 1-5 /home/jarvis/scripts/risk_svr.sh start" ;
		echo "10 15 * * 1-5 /home/jarvis/scripts/risk_svr.sh stop" ;
		echo "* 09-16 * * 1-5 /home/jarvis/scripts/engine_svr.sh pullup >> /home/jarvis/scripts/check_alive.log" ;
		
		echo "40 20 * * 1-5 /home/jarvis/scripts/engine_svr.sh start" ;
		echo "00 05 * * 2-6 /home/jarvis/scripts/engine_svr.sh stop" ;
		echo "#45 20 * * 1-5 /home/jarvis/scripts/dump_svr.sh start" ;
		echo "#00 05 * * 2-6 /home/jarvis/scripts/dump_svr.sh stop" ;
		echo "50 20 * * 1-5 /home/jarvis/scripts/risk_svr.sh start" ;
		echo "50 02 * * 1-5 /home/jarvis/scripts/risk_svr.sh stop" ;
		echo "* 21-23 * * 1-5 /home/jarvis/scripts/engine_svr.sh pullup >> /home/jarvis/scripts/check_alive.log" ;
		echo "* 00-04 * * 2-6 /home/jarvis/scripts/engine_svr.sh pullup >> /home/jarvis/scripts/check_alive.log" ;

		echo "* * * * * /home/jarvis/TunnelAgent/scripts/check_alive.sh >> /home/jarvis/TunnelAgent/scripts/down.log 2>&1" ;
		
		echo "* * * * * /home/jarvis/scripts/alert_check_alive.sh >> /home/jarvis/scripts/alert_check_alive.log 2>&1" ;
		
		echo "30 17 * * * /home/jarvis/scripts/tools.sh arch >> /home/jarvis/scripts/arch.log 2>&1" ;
		echo "#------------------ at crontab end ----------------"
		) | crontab - || exit 1
	fi
}

function UnTarDeploy()
{
	cd ${DEPLOYDIR}
	
	if ! tail -n +@LINE_CNT@ "$THIS_PATH" | tar xfz -; then
		printf "ERROR: could not extract tar starting at line @LINE_CNT@\\n" >&2
		exit 1
	fi
}

function AddPath()
{
	printf "Add /home/jarvis/lib to LD_LIBRARY_PATH and PYTHONPATH [yes|no]?"
	read -r ans
	if [[ ${ans} == "y" || ${ans} == "Y" || ${ans} == "yes" ]]
	then
		BASHRC="${HOME}/.bashrc"
		COMMENT="# added by at installer"
		if [[ -z `grep "${COMMENT}" ${BASHRC}` ]]; then
			echo -e "\n${COMMENT}" >> "${BASHRC}"
			echo "export PYTHONPATH=\"/home/jarvis/lib:\${PYTHONPATH}\"" >> "${BASHRC}"
			echo "export LD_LIBRARY_PATH=\"/home/jarvis/lib:\${LD_LIBRARY_PATH}\"" >> "${BASHRC}"
		fi
	fi 
}

function Deploy()
{
	mkdir -p /home/jarvis/ 2>/dev/null || sudo mkdir -p /home/jarvis/
	chown ${ME}:${ME} -R /home/jarvis/ 2>/dev/null || sudo chown ${ME}:${ME} -R /home/jarvis/

	UnTarDeploy

	AddCron

	sudo sh -c 'echo /var/crash/core-%e > /proc/sys/kernel/core_pattern'

	/home/jarvis/scripts/tools.sh prepare

	AddPath

	echo "Done!"
}

Deploy

exit 0
@@END_HEADER@@

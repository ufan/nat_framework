#!/bin/bash

SRCDIR=$(cd `dirname $0`/../;pwd)
RELDIR=`realpath "${SRCDIR}/../release"`
TEMPLATE_FILE="${SRCDIR}/scripts/install_template.sh"

cd ${RELDIR}
tar cfz deploy.tgz global lib conf archive MDEngine scripts TDEngine TunnelAgent TradeTool tools SignalAgent SDK dump risk libXtTraderApi.so 

cp ${TEMPLATE_FILE} ./install.sh
LINE_CNT=`wc -l ./install.sh |awk '{print $1}'`
((LINE_CNT++))

sed -i "s/@LINE_CNT@/${LINE_CNT}/g" ./install.sh

cat ./deploy.tgz >> ./install.sh
rm ./deploy.tgz

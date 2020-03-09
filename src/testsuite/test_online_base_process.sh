#!/bin/bash

echo "[Notice]: this need to make installer first!"
echo -e "\n"

BASEDIR=$(DIRNAME=$(dirname "$0"); cd "$DIRNAME"/../../; pwd)

${BASEDIR}/release/install.sh

/home/jarvis/scripts/engine_svr.sh restart
/home/jarvis/scripts/engine_svr.sh check

sleep 3
${BASEDIR}/release/SDK/demo/demo.py ${BASEDIR}/release/SDK/demo/demo.json

/home/jarvis/scripts/engine_svr.sh stop

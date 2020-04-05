#!/bin/bash

function Usage()
{
    echo "Usage: $0 -n name -c config -m python_script [-p python_parser] [-w workdir] [-s del_secs]"
}

parser=`which python`
delsec=-1

while getopts ":n:c:m:p:w:s:" opt
do
    case $opt in
        n ) name=$OPTARG;;
        c ) conf=$OPTARG;;
        m ) script=$OPTARG;;
        p ) parser=$OPTARG;;
		w ) workdir=$OPTARG;; 
		s ) delsec=$OPTARG;;
        *) ;;
    esac
done

if [[ -z $name || ! -f $conf || ! -f $script || ! -f $parser ]]
then 
    Usage
    exit -1
fi

conf=`cat $conf`
export STG_CFG="$conf"

script=`cat $script`
export STG_CONTENT="$script"

DIR=`dirname $0`

$DIR/StgManager reg $name

if [[ -z $workdir ]]
then
	$DIR/PyLoader -n $name -p $parser -s $delsec
else
	$DIR/PyLoader -n $name -p $parser -w $workdir -s $delsec
fi


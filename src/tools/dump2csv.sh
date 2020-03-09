#!/bin/bash

function Usage()
{
	echo "Usage: $0 translate_tool src_list_file dst_dir days_ago [keyword]"
}

if [[ $# -lt 4 ]]
then
	Usage
	exit -1
fi

TRANSTOOL=`realpath $1`
DIRLIST=`realpath $2`
DSTBASE=`realpath $3`

if [[ $4 -gt 0 ]]
then
	DATE=`date +"%Y%m%d" -d "-${4} days"`
else
	DATE=`date +"%Y%m%d"`
fi

DSTDIR=${DSTBASE}_${DATE}

if [[ $# -ge 5 ]]
then
	KEY=$5
fi

if [[ -z $TRANSTOOL || -z $DIRLIST || -z $DSTBASE ]]
then
	Usage
fi

function TransDir()
{
	local SRC_BASE=$1
	local DIR=$2
	local DST_BASE=$3
	
	mkdir -p $DST_BASE/$DIR
	echo "mkdir -p $DST_BASE/$DIR"
	pushd $SRC_BASE/$DIR > /dev/null
	for f in `ls`
	do
		if [[ -f $f && ${f##*.} == "data" && -n "`echo $f|grep ${DATE}`" ]]
		then
			if [[ -z "$KEY" || -n "`echo $f|grep "${KEY}"`" ]]
			then
				echo "$TRANSTOOL $f $DST_BASE/$DIR/${f%.*}.csv"
				$TRANSTOOL $f $DST_BASE/$DIR/${f%.*}.csv
			fi
		fi
		
		if [[ -d "$f" ]]
		then
			TransDir $SRC_BASE $DIR/$f $DST_BASE
		fi
	done
	popd > /dev/null
}


while read LINE
do
	if [[ -n $LINE ]]
	then
		BASE=$(dirname `realpath ${LINE}`)
		NAME=$(basename `realpath ${LINE}`)
		if [[ -n $BASE && -n $NAME ]]
		then
			TransDir $BASE $NAME $DSTDIR
		fi
	fi
done < $DIRLIST


CNT=0
for f in `ls -d ${DSTBASE}_*|sort -r`
do
	if [[ $CNT -ge 5 ]]
	then
		echo "rm -rf $f"
		rm -rf $f
	else
		((CNT++))
	fi
done




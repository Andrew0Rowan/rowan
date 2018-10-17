#!/bin/bash
#
#	@author Howard Chen
#	@brief if the input file violate rules, return 1, else 0 
#

if [ $# != 1 ] || [ ! -f $F ] || [ ! ${F##*.} = "c" ]
then
	echo "usage: wimac_rul.sh <c_source>"
	exit 1
fi


F=$1
CMD_P=$(cat -n $F | grep @nostrict-g)
if [ "$CMD_P" != "" ]
then
	echo "[PASS]: $F"
	exit 0
fi

LINES=$(cat -n $F | sed -e /@nostrict/d | sed -e /\#include/d | gcc -E - | grep extern | sed -e /@nostrict/d )

if [ "$LINES" != "" ]
then
	echo ""
	echo ""
	echo "[FAIL]: $F"
	echo "$LINES"
	echo "--------------------------------------------------------------------------------"
	exit 1
else
	echo "[PASS]: $F"
	exit 0
fi


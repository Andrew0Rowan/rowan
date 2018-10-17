#!/bin/bash
#
#	@author	Howard Chen
#	@brief  generate the top index for each module doc
SYSROOT=$1
for i in ${SYSROOT}/doc/* ;
do
	if [ -d ${i} ]; then 
	module=${i##*/}
	echo "<a href=\"${module}/index.html\">${module}</a>"
	echo "<p>"
	fi
done

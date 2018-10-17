#!/bin/bash
#
#	@author
#	@brief  generate the top index for each module doc
wait /dev/mtd10
ubiattach /dev/ubi_ctrl -m 10
mount -t ubifs ubi1_0 /data
mount --bind /data/var /var

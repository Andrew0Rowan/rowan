#!/bin/bash

PROJECT=`ls ${PWD}/build/tmp/deploy/images/ | grep aud8516* | head -1`
echo "project name is ${PROJECT}"

TOP_DIR=`pwd`

if [ ! -d ${TOP_DIR}/build/tmp/deploy/images/${PROJECT}/ ]
then
    echo "Please build ${PROJECT} firstly"
    exit
fi

echo "Start Download:"
cd ${TOP_DIR}/build/tmp/deploy/images/${PROJECT}

python fbtool.py -f dl_addr.ini
fastboot erase nand0
if [ $? -ne 0 ];then
    echo "erase nand0 fail, stop upgrade!"
    exit
fi

fastboot flash nand0 MBR_NAND
if [ $? -ne 0 ];then
    echo "flash nand MBR fail, stop upgrade!"
    exit
fi
fastboot flash UBOOT lk.img
if [ $? -ne 0 ];then
    echo "flash lk.img fail, stop upgrade!"
    exit
fi
fastboot flash TEE1 tz.img
if [ $? -ne 0 ];then
    echo "flash tz.img fail, stop upgrade!"
    exit
fi

fastboot flash BOOTIMG1 boot.img
if [ $? -ne 0 ];then
    echo "flash boot.img fail, stop upgrade!"
    exit
fi

fastboot flash ROOTFS1 rootfs.ubi
if [ $? -ne 0 ];then
    echo "flash rootfs.ubi fail, stop upgrade!"
    exit
fi

fastboot flash USRDATA userdata.ubi
if [ $? -ne 0 ];then
    echo "flash userdata.ubi fail, stop upgrade!"
    exit
fi
echo "upgrade successfully!"
fastboot reboot

cd ${TOP_DIR}

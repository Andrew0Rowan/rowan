#!/bin/sh

# connect ap
##vendor/bin/wmt_loader
##sleep 1
##vendor/bin/wmt_launcher -m 4 -p /vendor/firmware &
##sleep 1
##echo 1 > /dev/wmtWifi
##sleep 1
##move this 3 step into appmainprog.sh

ifconfig wlan0 up
sleep 1
wpa_supplicant -iwlan0 -c/etc/wpa_supplicant.conf &
sleep 5
dhcpcd
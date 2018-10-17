#!/bin/sh
# insmod mt7668 driver before appmainprog starts
echo insmod mt7668 driver
insmod /lib/modules/mt7668/btmtksdio.ko
insmod /lib/modules/mt7668/wlan_mt7668_sdio.ko

wpa_supplicant -Dnl80211 -iwlan0 -c/etc/wpa_supplicant.conf

#!/bin/bash
# Program:
#	WLAN manipulation
# History:
# 2015/04/27	MTK WLAN        Peter
# 2015/05/28	Add AP          Peter
# 2015/07/28    Use wpa_supplicant v2.4 Peter
#main loop
wlan_interface_enable()
{
    echo "Enable WLAN"
    echo 1 > /dev/wmtWifi
}

wlan_service_up()
{
    wpa_supplicant -Dnl80211 -iwlan0 -c/etc/wpa_supplicant.conf &
    dhcpcd
}

wlan_connect_user()
{
    echo "Start to connect $1 with key $2"
    wpa_cli -iwlan0 -p/tmp/wpa_supplicant DISCONNECT
    wpa_cli -iwlan0 -p/tmp/wpa_supplicant add_network
    net_id=$?
    wpa_cli -iwlan0 -p/tmp/wpa_supplicant set_network $net_id ssid $1
    wpa_cli -iwlan0 -p/tmp/wpa_supplicant set_network $net_id psk $2
    wpa_cli -iwlan0 -p/tmp/wpa_supplicant set_network $net_id key_mgmt WPA-PSK
    wpa_cli -iwlan0 -p/tmp/wpa_supplicant select_network $net_id
}
ap_up()
{
    echo AP > /dev/wmtWifi
    sleep 1
    echo "Enable ap0 interface..."
    ifconfig ap0 up
    ifconfig ap0 192.168.1.1 netmask 255.255.255.0
    echo "Enable udhcpd..."
    mkdir -p /var/lib/misc
    touch /var/lib/misc/udhcpd.leases
    udhcpd /etc/udhcpd.conf&
    echo "Enable HOSTAPD..."
    hostapd -dd /etc/hostapd_mtk.conf&
}

echo "start main loop"
mexit="no"
#while [ $mexit == "no" ]
while true
do 
    echo "========================================="
    echo "      WLAN Test Menu                     "
    echo "========================================="
    echo "(1)WLAN Interface Up"
    echo "(2)WLAN Connect(default)"
    echo "(3)WLAN Connect(user)"
    echo "(4)AP Up"    
    echo "(7)Exit"
    echo -n "Input Selection : "
    read option
    case $option in
      "1")
    	echo "WLAN UP"
    	wlan_interface_enable
    	;;
      "2")
    	echo "Connect with default setting..."
    	wlan_service_up
    	;;
      "3")
    	echo "Connect with user input..."
    	echo -n "Input SSID: "
    	read ssid
    	echo -n "Input PASSWORD for: "
    	read key
    	wlan_service_up
    	sleep 2
    	wlan_connect_user $ssid $key
    	;;
      "4")
    	echo "AP UP..."
    	ap_up
    	;;
      "7")
    	echo "Exit"
    	mexit="yes"
        break
    	;;
      *)
    	echo "No support opton, exit..."
    	mexit="yes"
        break
    	;;
    esac
done    
#------------------------Phase out function----------------------------
#Phase out, wpa_supplicant v2.0
#wlan_connect_user()
#{
#    echo "Start to connect $1 with key $2"
#    wpa_cli -iwlan0 -p/data/misc/wifi/sockets DISCONNECT
#    wpa_cli -iwlan0 -p/data/misc/wifi/sockets add_network
#    net_id=$?
#    wpa_cli -iwlan0 -p/data/misc/wifi/sockets set_network $net_id ssid $1
#    wpa_cli -iwlan0 -p/data/misc/wifi/sockets set_network $net_id psk $2
#    wpa_cli -iwlan0 -p/data/misc/wifi/sockets set_network $net_id key_mgmt WPA-PSK
#    wpa_cli -iwlan0 -p/data/misc/wifi/sockets select_network $net_id
#
#    wpa_cli -iwlan0 -p/data/misc/wifi/sockets set_network $net_id proto RSN WPA
#    wpa_cli -iwlan0 -p/data/misc/wifi/sockets set_network $net_id pairwise CCMP TKIP
#    wpa_cli -iwlan0 -p/data/misc/wifi/sockets set_network $net_id auth_alg OPEN
#
#}

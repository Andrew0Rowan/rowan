#enable dhcp client
echo "Set up dhcp client"
dhcpcd

#connect to AP
echo "Start to connect $1 with key $2"
wpa_cli -iwlan0 -p/tmp/wpa_supplicant DISCONNECT
wpa_cli -iwlan0 -p/tmp/wpa_supplicant add_network
net_id=$?
wpa_cli -iwlan0 -p/tmp/wpa_supplicant set_network $net_id ssid $1
wpa_cli -iwlan0 -p/tmp/wpa_supplicant set_network $net_id psk $2
wpa_cli -iwlan0 -p/tmp/wpa_supplicant set_network $net_id key_mgmt WPA-PSK
wpa_cli -iwlan0 -p/tmp/wpa_supplicant select_network $net_id
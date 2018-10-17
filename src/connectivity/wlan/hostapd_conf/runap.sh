#2635 AP mode
echo AP > /dev/wmtWifi
ifconfig ap0 up
ifconfig ap0 192.168.1.1
echo '' > /tmp/dhcpd.leases
dhcpd -cf /etc/dhcpd.conf ap0 -lf /tmp/dhcpd.leases &
hostapd /etc/hostapd_mtk.conf &

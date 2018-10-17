#get mac address of wlan0
mac_address=$(ifconfig wlan0 | grep HWaddr | sed -n '/HWaddr/ s/^.*HWaddr *//pg' | sed 's/://g' | sed s/[[:space:]]//g)

echo $mac_address is the mac of wlan0

sed -i "s/xxxxxxxx/$mac_address/g" /data/misc/dueros/dueros_config.json

cat /data/misc/dueros/dueros_config.json


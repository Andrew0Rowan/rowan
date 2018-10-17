#!/bin/sh
mount -t configfs none /sys/kernel/config
cd /sys/kernel/config/usb_gadget
mkdir g1
cd g1
echo "0x0e8d" > idVendor
echo "0x2007" > idProduct
mkdir strings/0x409
echo "0123456789ABCDEF" > strings/0x409/serialnumber
echo "Mediatek Inc." > strings/0x409/manufacturer
echo "acm gadget" > strings/0x409/product
mkdir functions/acm.usb0
mkdir configs/c.1
ln -s functions/acm.usb0/  configs/c.1
ls /sys/class/udc/
echo 11271000.usb > UDC
sleep 20
/sbin/tst_main

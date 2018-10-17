MOUNTFOLDER="/run/media/sda1"
MOUNTDEV="/dev/sda1"
BOOTIMAGE="${MOUNTFOLDER}/boot.img"
BOOT_PARTITION="/dev/mmcblk0p2"
ROOTFS="${MOUNTFOLDER}/rootfs.ext4"
ROOTFS_PARTITION="/dev/mmcblk0p3"

# mount USB device
if ! test -d ${MOUNTFOLDER} ; then
  mkdir -p ${MOUNTFOLDER}
  /bin/mount ${MOUNTDEV} ${MOUNTFOLDER}
fi

#Flashing boot.img (kernel image)
while ! test -f ${BOOTIMAGE} ; do
  echo "Please insert usb disk and place boot.img in usb disk"
  echo "Press ENTER key to Continue"
  read
done
echo "Flashing boot.img ..."
/bin/dd if=${BOOTIMAGE} of=${BOOT_PARTITION} bs=1048576

#Flashing rootfs.ext4
while ! test -f ${ROOTFS} ; do
  echo "Please insert usb disk and place rootfs.ext4 in usb disk"
  echo "Press ENTER key to Continue"
  read
done
echo "Flashing rootfs.ext4 ..."
/bin/dd if=${ROOTFS} of=${ROOTFS_PARTITION} bs=1048576

echo "Finish ... Press ENTER key to Reboot"
read
echo "Rebooting ..."
/sbin/reboot -f

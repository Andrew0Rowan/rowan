echo "unpacking rpm now"
rpm2cpio *.rpm | cpio -idmv

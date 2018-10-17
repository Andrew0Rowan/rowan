echo $1 is here
echo $2 is here

find ./$2 -name "*.rpm" | while read i; do $1 ${i} | cpio -idmv ; done

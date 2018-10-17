FILESEXTRAPATHS_append := ":${THISDIR}/files"
SRC_URI_append += "file://0001-wpa_supplicant-WPA2-Security-KRACK-issue-patch.patch"

do_install_append(){
}

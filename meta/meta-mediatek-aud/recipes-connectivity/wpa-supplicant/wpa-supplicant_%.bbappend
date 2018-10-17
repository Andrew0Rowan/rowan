FILESEXTRAPATHS_append := ":${THISDIR}/files"
SRC_URI_append += "file://wpa_supplicant.conf-sane \
                   file://0001-WOW-wpa_supplicant-add-event-WOW_MAGIC_PACKET_RECEIV.patch \
                   file://0001-WOW-wpa_supplicant-add-event-WOW_WAKEUP_REASON-Recei.patch"

do_install_append(){
}

do_configure_append () {
  # Activate config options needed by connman and tethering
  echo "wowlan_triggers=disconnect" >> ../wpa_supplicant.conf-sane
}

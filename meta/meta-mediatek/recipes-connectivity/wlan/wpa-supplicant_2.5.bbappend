do_configure_append () {
  # Activate config options needed by connman and tethering
  echo "CONFIG_P2P=y" >> wpa_supplicant/.config
  echo "p2p_no_group_iface=1" >> ../wpa_supplicant.conf-sane
  echo "config_methods=display push_button keypad" >> ../wpa_supplicant.conf-sane
  echo "driver_param=use_p2p_group_interface=1" >> ../wpa_supplicant.conf-sane
}

SUMMARY = "Mediatek 6630 firmware"
LICENSE = "MediaTekProprietary"

inherit packagegroup


RDEPENDS_packagegroup-mtk-6630 = "\
			mtkcombotool\
			mtkwlan \
			mtkcombo \
			mtkwifitesttool \
			connman-client \
                        gnsshal \
			${@base_contains("TARGET_PLATFORM", "mt2712", "lib32-mnld", "mnld", d)} \
                        gnsstest \
			hostapd \
			dhcp-server \
			dhcp-client \
			dhcp-relay \
			${@base_contains("ADR_SUPPORT", "YES", "adrd", "", d)} \
"

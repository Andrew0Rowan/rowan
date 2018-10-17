SUMMARY = "Mediatek 6630 firmware"
LICENSE = "MediaTekProprietary"

inherit packagegroup


RDEPENDS_packagegroup-mtk-6630 = "\
			mtkcombotool\
			mtkwlan \
			mtkcombo \
			mtkwifitesttool \
			connman-client \
			hostapd \
			dhcp-server \
			dhcp-client \
			dhcp-relay \
"

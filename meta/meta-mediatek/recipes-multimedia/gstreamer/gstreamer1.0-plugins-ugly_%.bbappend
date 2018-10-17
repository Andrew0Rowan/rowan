FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI += "file://0001-rmdemux-porting-from-1.4.5-to-1.6.3.patch \
			file://0002-asfdemux-asfdemux-push-header.patch \
			file://0003-rmdemux-sync-rmdemux-patch-from-1.4.5-to-1.6.3.patch \
			file://0004-asfdemux-fix-sending-multi-times-caps-event-in-WMV-v.patch \
			file://0005-rmdemux-fix-special-rm-file-play-hang-issue.patch \
			file://0006-Mpeg-audio-decoder-disable-mad.patch \
			file://0007-rmdemux-add-brokenupbyus-bit-handling-for-rv3040.patch"

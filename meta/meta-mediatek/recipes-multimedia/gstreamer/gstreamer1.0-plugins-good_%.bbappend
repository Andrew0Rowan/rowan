FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI += " \
	file://0001-gstreamer-upgrade-v4l2-for-good-plugins.patch \
	file://0002-pulsesink-resolve-deadlock-in-pause-function.patch \
	file://0003-gstreamer-upgrade-demux-for-good-plugins.patch \
	file://0004-v4l2-add-get-camera-link-number-feature.patch \
	file://0005-v4l2bufferpool-add-buffer-index-to-gstbuffer.patch \
	file://0006-gstreamer-rm-rpr-feature.patch \
	file://0007-v4l2-handle-eos-event-from-driver.patch \
	file://0008-change-MT21-to-MM21.patch \
	file://0009-v4l2-remove-get-camera-link-number-feature.patch \
	file://0010-v4l2-enc-fix-crash.patch \
	file://0011-v4l2src-support-mplane-capture-device.patch \
	file://0012-avi-demux-change-log-level.patch \
	file://0013-gst-remove-using-lib-mtkconv.patch \
	file://0014-gst-modify-makefile.patch \
"

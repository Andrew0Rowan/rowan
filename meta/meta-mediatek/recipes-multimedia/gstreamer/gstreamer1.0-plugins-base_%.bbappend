FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI += " \
	file://0001-gstreamer-upgrade-ogg-bugs.patch \
	file://0002-gstreamer-upgrade-remove-invalid-framerate-field.patch \
	file://0003-gstreamer-upgrade-play-EM2V-format-video-normal.patch \
	file://0004-gstreamer-upgrade-audio-bugs.patch \
	file://0005-gstreamer-upgrade-add-mdp-plugin-into-playbin.patch \
	file://0006-gstreamer-upgrade-MT21-block-mode-in-videoinfo.patch \
	file://0007-videoinfo-modify-I420-format-plane-offset-and-size.patch \
	file://0008-video-fomat-fix-MT21-info.patch \
"

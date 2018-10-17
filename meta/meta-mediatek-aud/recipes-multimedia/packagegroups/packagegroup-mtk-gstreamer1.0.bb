DESCRIPTION = "Package group used by Mediatek to provide all GStreamer plugins from the \
base, good, and bad packages, as well as the ugly and libav ones if commercial packages \
are whitelisted, and plugins for the required hardware acceleration (if supported by the SoC)."
SUMMARY = "Mediatek package group - full set of all GStreamer 1.0 plugins"
LICENSE = "MIT"

inherit packagegroup

RDEPENDS_packagegroup-mtk-gstreamer1.0 = "\
	gstreamer1.0 \
	gstreamer1.0-plugins-base-playback \
	gstreamer1.0-plugins-base-audioconvert \
	gstreamer1.0-plugins-base-volume \
	gstreamer1.0-plugins-base-alsa \
	gstreamer1.0-plugins-base-typefindfunctions \
	gstreamer1.0-plugins-good-audioparsers \
	gstreamer1.0-plugins-good-wavparse \
	gstreamer1.0-plugins-good-id3demux \
	gstreamer1.0-plugins-good-souphttpsrc \
	gstreamer1.0-plugins-bad-faad \
	gstreamer1.0-plugins-base-volume \
	${@base_contains('LICENSE_FLAGS_WHITELIST', 'commercial', 'gstreamer1.0-plugins-ugly-mad', '', d)} \
"

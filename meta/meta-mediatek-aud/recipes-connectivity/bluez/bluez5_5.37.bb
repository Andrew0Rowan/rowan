require bluez5.inc

REQUIRED_DISTRO_FEATURES = "bluez5"

SRC_URI[md5sum] = "33177e5743e24b2b3738f72be64e3ffb"
SRC_URI[sha256sum] = "c14ba9ddcb0055522073477b8fd8bf1ddf5d219e75fdfd4699b7e0ce5350d6b0"

# noinst programs in Makefile.tools that are conditional on READLINE
# support
NOINST_TOOLS_READLINE ?= " \
"

# noinst programs in Makefile.tools that are conditional on EXPERIMENTAL
# support
NOINST_TOOLS_EXPERIMENTAL ?= " \
    tools/mgmt-tester \
    tools/gap-tester \
    tools/l2cap-tester \
    tools/hci-tester \
    tools/bdaddr \
    tools/hcieventmask \
    tools/hcisecfilter \
    tools/btinfo \
    tools/btattach \
    tools/btsnoop \
    tools/btgatt-client \
    tools/btgatt-server \
    tools/gatt-service \
"

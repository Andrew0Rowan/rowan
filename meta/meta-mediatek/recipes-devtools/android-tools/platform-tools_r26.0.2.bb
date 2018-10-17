DESCRIPTION = "Android Platform Tools"
LICENSE = "Apache-2.0 & GPL-2.0 & BSD-2-Clause & BSD-3-Clause"

LIC_FILES_CHKSUM = "file://${S}/platform-tools-linux/platform-tools/NOTICE.txt;md5=d4b39e3d79095f051d1f61901cec410e"

BBCLASSEXTEND = "native"

SRC_URI = " \
    https://dl.google.com/android/repository/platform-tools_${PV}-windows.zip;name=platform-tools-windows \
    https://dl.google.com/android/repository/platform-tools_${PV}-darwin.zip;name=platform-tools-darwin \
    https://dl.google.com/android/repository/platform-tools_${PV}-linux.zip;name=platform-tools-linux \
"

SRC_URI[platform-tools-windows.md5sum] = "4a2d2e09d3ffdce8252c9136754a1ce9"
SRC_URI[platform-tools-windows.sha256sum] = "4850c695724ad0328d9100cc3b4475dcd1af1bec904675362259595a3e03ae2e"
SRC_URI[platform-tools-darwin.md5sum] = "00fa1426da56ed9477f65219213ae1a5"
SRC_URI[platform-tools-darwin.sha256sum] = "a6d0504e560713af2a3ae71449bcadf011b50ba78f7bf303a9d6d69bf855c73f"
SRC_URI[platform-tools-linux.md5sum] = "ef952bb31497f7535e061ad0e712bed8"
SRC_URI[platform-tools-linux.sha256sum] = "63b15a38c2b64e6ec8b54febe9f69fce5fe6c898c554c73b826b49daf7b52519"

do_unpack () {
    unzip -q ${DL_DIR}/platform-tools_${PV}-linux.zip -d ${S}/platform-tools-linux
    unzip -q ${DL_DIR}/platform-tools_${PV}-darwin.zip -d ${S}/platform-tools-darwin
    unzip -q ${DL_DIR}/platform-tools_${PV}-windows.zip -d ${S}/platform-tools-windows
}

inherit deploy

do_deploy () {
    install -d ${DEPLOYDIR}
    install -m 0755 ${S}/platform-tools-linux/platform-tools/fastboot ${DEPLOYDIR}/fastboot-linux-x86_64
    install -m 0755 ${S}/platform-tools-darwin/platform-tools/fastboot ${DEPLOYDIR}/fastboot-darwin
    install -m 0755 ${S}/platform-tools-windows/platform-tools/fastboot.exe ${DEPLOYDIR}/fastboot.exe
}

addtask deploy before do_build after do_compile


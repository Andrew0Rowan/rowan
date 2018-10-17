#!/usr/bin/python
import os

procedures = {
    # product   : fastboot args
    'DEFAULT'   : [['daWait'],
                    ['fbWait'],
                    ['fastboot', 'erase', 'nand0'],
                    ['fastboot', 'flash', 'nand0', 'MBR_NAND'],
                    ['fastboot', 'flash', 'UBOOT', 'lk.img'],
                    ['fastboot', 'flash', 'TEE1', 'tz.img'],
                    ['fastboot', 'flash', 'BOOTIMG1', 'boot.img'],
                    ['fastboot', 'flash', 'ROOTFS1', 'rootfs.ubi'],
                    ['fastboot', 'flash', 'USRDATA', 'userdata.ubi'] ]
}

userprocedures = {
    # product   : fastboot args
    'DEFAULT'   : [['daWait'],
                    ['fbWait'],
                    ['fastboot', 'erase', 'ROOTFS1'],
                    ['fastboot', 'flash', 'ROOTFS1', 'rootfs.ubi'] ]
}

bootprocedures = {
    # product   : fastboot args
    'DEFAULT'   : [['daWait'],
                    ['fbWait'],
                    ['fastboot', 'erase', 'nand0'],
                    ['fastboot', 'flash', 'nand0', 'MBR_NAND'],
                    ['fastboot', 'flash', 'UBOOT', 'lk.img'],
                    ['fastboot', 'flash', 'TEE1', 'tz.img'],
                    ['fastboot', 'flash', 'BOOTIMG1', 'boot.img'],
                    ['fastboot', 'flash', 'ROOTFS1', 'rootfs.ubi'],
                    ['fastboot', 'flash', 'USRDATA', 'userdata.ubi'] ]
}

testprocedures = {
    # product   : fastboot args
    'DEFAULT'   : [['daWait'],
                    ['fbWait'],
                    ['fastboot', 'erase', 'UBOOT'],
                    ['fastboot', 'erase', 'MISC'],
                    ['fastboot', 'erase', 'TEE1'],
                    ['fastboot', 'erase', 'BOOTIMG1'],
                    ['fastboot', 'erase', 'ROOTFS1'],
                    ['fastboot', 'erase', 'USRDATA'],
                    ['fastboot', 'flash', 'UBOOT', 'lk.img'],
                    ['fastboot', 'flash', 'TEE1', 'tz.img'],
                    ['fastboot', 'flash', 'BOOTIMG1', 'boot.img'],
                    ['fastboot', 'flash', 'ROOTFS1', 'rootfs.ubi'],
                    ['fastboot', 'flash', 'USRDATA', 'userdata.ubi'] ]
}

# return procedure list
def getFlashProc(product):
    try:
        ret = procedures[product.upper()]
        return ret
    except Exception, e:
        return None


def getFlashUserProc(product):
    try:
        ret = userprocedures[product.upper()]
        return ret
    except Exception, e:
        return None

def getFlashBootProc(product):
    try:
        ret = bootprocedures[product.upper()]
        return ret
    except Exception, e:
        return None

def getFlashTestProc(product):
    try:
        ret = testprocedures[product.upper()]
        return ret
    except Exception, e:
        return None

/*
 * Copyright (c) 2016 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <dev/udc.h>
#include <lib/bio.h>
#include <target/cust_usb.h>
#include <platform/mtk_wdt.h>
#include <platform/mtk_serial_key.h>
#include <reg.h>
#include "fastboot.h"
#include "dl_commands.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#ifndef SN_BUF_LEN
#define SN_BUF_LEN  19  /* fastboot use 13 bytes as default, max is 19 */
#endif
#define DEFAULT_SERIAL_NUM "0123456789ABCDEF"
char sn_buf[SN_BUF_LEN+1] = DEFAULT_SERIAL_NUM;

static struct udc_device surf_udc_device = {
    .vendor_id  = USB_VENDORID,
    .product_id = USB_PRODUCTID,
    .version_id = USB_VERSIONID,
    .manufacturer   = USB_MANUFACTURER,
    .product    = USB_PRODUCT_NAME,
};

static void register_commands(void)
{
    fastboot_register("getvar:", cmd_getvar);
    fastboot_register("download:", cmd_download);
    fastboot_register("flash:", cmd_flash);
    fastboot_register("erase:", cmd_erase);
    fastboot_register("reboot", cmd_reboot);
    fastboot_register("reboot-bootloader", cmd_reboot_bootloader);
    fastboot_register("oem reboot-recovery", cmd_reboot_recovery);
    fastboot_register("oem continue", cmd_continue_boot);
}

static void publish_attributes(void)
{
    char buffer_size[11];
    sprintf(buffer_size, "0x%x", SCRATCH_SIZE);
    fastboot_publish("version", "0.5");
    fastboot_publish("max-download-size", buffer_size);
}

static void storage_init(void)
{
    /* init storage that only use in fastboot, currently none */
    bio_dump_devices();
}

static char udc_chr[32] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ456789"};
static int get_serial(u64 hwkey, u32 chipid, char *ser)
{
    u16 hashkey[4];
    u32 idx, ser_idx;

    /* split to 4 key with 16-bit width each */
    for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
        hashkey[idx] = (u16)(hwkey & 0xffff);
        hwkey >>= 16;
    }

    /* hash the key with chip id */
    for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
        u32 digit = (chipid % 10);
        hashkey[idx] = (hashkey[idx] >> digit) | (hashkey[idx] << (16-digit));
        chipid = (chipid / 10);
    }

    /* generate serail using hashkey */
    ser_idx = 0;
    for (idx = 0; idx < ARRAY_SIZE(hashkey); idx++) {
        ser[ser_idx++] = (hashkey[idx] & 0x001f);
        ser[ser_idx++] = (hashkey[idx] & 0x00f8) >> 3;
        ser[ser_idx++] = (hashkey[idx] & 0x1f00) >> 8;
        ser[ser_idx++] = (hashkey[idx] & 0xf800) >> 11;
    }

    for (idx = 0; idx < ser_idx; idx++)
        ser[idx] = udc_chr[(int)ser[idx]];

    ser[ser_idx] = 0x00;

    return 0;
}

void ext_boot(void)
{
    u64 key = ((u64)readl(SERIAL_KEY_HI) << 32) | readl(SERIAL_KEY_LO);
    if (key != 0)
        get_serial(key, MACH_TYPE, sn_buf);

    surf_udc_device.serialno = sn_buf;
    udc_init(&surf_udc_device);

    storage_init();
    mtk_wdt_disable();
    register_commands();
    publish_attributes();

    fastboot_init((void *)(SCRATCH_ADDR), (unsigned long long)SCRATCH_SIZE);
}

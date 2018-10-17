/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#include <stdlib.h>
#include <app.h>
#include <debug.h>
#include <dev/udc.h>
#include <string.h>
#include <kernel/thread.h>
#include <kernel/event.h>
#include <arch/ops.h>
#include <target.h>
#include <platform.h>
#include <platform/mt_reg_base.h>
#include <lib/bio.h>
#include <lib/partition.h>
#include <platform/mtk_wdt.h>
#include <dev/timer/arm_generic.h>
#include "sparse_format.h"
#include "dl_commands.h"
#include "fastboot.h"
#include "fit.h"
#include "buffer.h"

#define MODULE_NAME "FASTBOOT_DOWNLOAD"
#define MAX_RSP_SIZE 64

extern void *download_base;
extern unsigned download_max;
extern unsigned download_size;
extern unsigned fastboot_state;

/*LXO: !Download related command*/

#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
#define INVALID_PTN -1

lk_bigtime_t start_time_ms;
#define TIME_STAMP current_time_hires()
#define TIME_START {start_time_ms = current_time_hires();}
#define TIME_ELAPSE (current_time_hires() - start_time_ms)

extern int usb_write(void *buf, unsigned len);
extern int usb_read(void *buf, unsigned len);


extern struct fastboot_var *varlist;
void cmd_getvar(const char *arg, void *data, unsigned sz)
{
    struct fastboot_var *var;
    char response[MAX_RSP_SIZE];

    if (!strcmp(arg, "all")) {
        for (var = varlist; var; var = var->next) {
            snprintf(response, MAX_RSP_SIZE,"\t%s: %s", var->name, var->value);
            fastboot_info(response);
        }
        fastboot_okay("Done!!");
        return;
    }
    for (var = varlist; var; var = var->next) {
        if (!strcmp(var->name, arg)) {
            fastboot_okay(var->value);
            return;
        }
    }
    fastboot_okay("");
}

void cmd_reboot(const char *arg, void *data, unsigned sz)
{
    dprintf(ALWAYS, "rebooting the device\n");
    fastboot_okay("");
    mtk_arch_reset(1); /* bypass pwr key when reboot */
}

void cmd_reboot_bootloader(const char *arg, void *data, unsigned sz)
{
    dprintf(ALWAYS, "rebooting the device to bootloader\n");
    fastboot_okay("");
    set_clr_fastboot_mode(true);
    mtk_arch_reset(1); /* bypass pwr key when reboot */
}

void cmd_reboot_recovery(const char *arg, void *data, unsigned sz)
{
    dprintf(ALWAYS, "rebooting the device to recovery\n");
    fastboot_okay("");
    set_clr_recovery_mode(true);
    mtk_arch_reset(1); /* bypass pwr key when reboot */
}

static void fastboot_fail_wrapper(const char *msg)
{
    fastboot_fail(msg);
}

static void fastboot_ok_wrapper(const char *msg, unsigned data_size)
{
    fastboot_okay("");
}

void cmd_download(const char *arg, void *data, unsigned sz)
{
    char response[MAX_RSP_SIZE];
    char *endptr;
    unsigned len = strtoul(arg, &endptr, 16);
    int r;

    download_size = 0;
    if (len > download_max) {
        fastboot_fail_wrapper("data is too large");
        return;
    }

    snprintf(response, MAX_RSP_SIZE, "DATA%08x", len);
    if (usb_write(response, strlen(response)) < 0) {
        return;
    }
    TIME_START;
    r = usb_read(download_base, len);
    if ((r < 0) || ((unsigned) r != len)) {
        fastboot_fail_wrapper("Read USB error");
        fastboot_state = STATE_ERROR;
        return;
    }
    download_size = len;

    fastboot_ok_wrapper("USB Transmission OK", len);
}

void cmd_flash_img(const char *arg, void *data, unsigned sz)
{
    bdev_t *bdev;
    int ret = 0;

    if (!strcmp(arg, "download:tz")) {
        cmd_download_tz(arg, data, sz);
    } else if (!strcmp(arg, "download:boot")) {
        cmd_download_boot(arg, data, sz);
    } else {
        bdev = bio_open_by_label(arg);
        if (!bdev) {
            bdev = bio_open(arg);
            if (!bdev) {
                fastboot_fail_wrapper("Partition is not exist.");
                return;
            }
        }

        dprintf(INFO, "src size is %d, dst size is %lld\n", ROUND_TO_PAGE(sz,bdev->block_size-1), bdev->total_size);
        if (bdev->total_size && ROUND_TO_PAGE(sz,bdev->block_size-1) > bdev->total_size) {
            fastboot_fail_wrapper("Image size is too large.");
            return;
        }

        ret = partition_update(bdev->name, 0x0, data, sz);
        if (ret < 0) {
            fastboot_fail_wrapper("Flash write failure.");
            return;
        }

        ret = partition_publish(bdev->name, 0x0);
        if (ret > 0)
            bio_dump_devices();

        fastboot_okay("");
    }
    return;
}

void cmd_flash_sparse_img(const char *arg, void *data, unsigned sz)
{
    bdev_t *bdev;
    size_t ret = 0;
    unsigned int chunk;
    unsigned int chunk_data_sz;
    uint32_t *fill_buf = NULL;
    uint32_t fill_val, pre_fill_val, erase_val;
    uint32_t chunk_blk_cnt = 0;
    uint32_t i;
    sparse_header_t *sparse_header;
    chunk_header_t *chunk_header;
    uint32_t total_blocks = 0;
    unsigned long long size = 0;

    bdev = bio_open_by_label(arg);
    if (!bdev) {
        fastboot_fail_wrapper("Partition is not exist.");
        return;
    }

    size = bdev->total_size;
    if (!size) {
        fastboot_fail_wrapper("Size is uncorrect.");
        return;
    }

    /* Read and skip over sparse image header */
    sparse_header = (sparse_header_t *) data;
    dprintf(ALWAYS, "Image size span 0x%llx, partition size 0x%llx\n", (unsigned long long)sparse_header->total_blks*sparse_header->blk_sz, size);
    if ((unsigned long long)sparse_header->total_blks*sparse_header->blk_sz > size) {
        fastboot_fail("sparse image size span overflow.");
        return;
    }

    data += sparse_header->file_hdr_sz;
    if (sparse_header->file_hdr_sz > sizeof(sparse_header_t)) {
        /* Skip the remaining bytes in a header that is longer than
         * we expected.
         */
        data += (sparse_header->file_hdr_sz - sizeof(sparse_header_t));
    }

    dprintf(INFO, "=== Sparse Image Header ===\n");
    dprintf(INFO, "magic: 0x%x\n", sparse_header->magic);
    dprintf(INFO, "major_version: 0x%x\n", sparse_header->major_version);
    dprintf(INFO, "minor_version: 0x%x\n", sparse_header->minor_version);
    dprintf(INFO, "file_hdr_sz: %d\n", sparse_header->file_hdr_sz);
    dprintf(INFO, "chunk_hdr_sz: %d\n", sparse_header->chunk_hdr_sz);
    dprintf(INFO, "blk_sz: %d\n", sparse_header->blk_sz);
    dprintf(INFO, "total_blks: %d\n", sparse_header->total_blks);
    dprintf(INFO, "total_chunks: %d\n", sparse_header->total_chunks);

    fill_buf = (uint32_t *)memalign(CACHE_LINE, ROUNDUP(sparse_header->blk_sz, CACHE_LINE));
    if (!fill_buf) {
        fastboot_fail_wrapper("Malloc failed for: CHUNK_TYPE_FILL");
        return;
    }
    erase_val = (int)bdev->erase_byte|bdev->erase_byte<<8|bdev->erase_byte<<16|bdev->erase_byte<<24;
    pre_fill_val = erase_val;
    dprintf(INFO, "Init previous fill value to 0x%08x\n", pre_fill_val);

    /* Start processing chunks */
    for (chunk=0; chunk<sparse_header->total_chunks; chunk++) {
        /* Read and skip over chunk header */
        chunk_header = (chunk_header_t *)data;
        data += sizeof(chunk_header_t);

        dprintf(INFO, "=== Chunk Header ===\n");
        dprintf(INFO, "chunk_type: 0x%x\n", chunk_header->chunk_type);
        dprintf(INFO, "chunk_data_sz: 0x%x\n", chunk_header->chunk_sz);
        dprintf(INFO, "total_size: 0x%x\n", chunk_header->total_sz);

        if (sparse_header->chunk_hdr_sz > sizeof(chunk_header_t)) {
            /* Skip the remaining bytes in a header that is longer than
             * we expected.
             */
            data += (sparse_header->chunk_hdr_sz - sizeof(chunk_header_t));
        }

        chunk_data_sz = sparse_header->blk_sz * chunk_header->chunk_sz;
        switch (chunk_header->chunk_type) {
            case CHUNK_TYPE_RAW:
                if (chunk_header->total_sz != (sparse_header->chunk_hdr_sz +
                                               chunk_data_sz)) {
                    fastboot_fail("Bogus chunk size for chunk type Raw");
                    goto error;
                }

                dprintf(INFO, "Raw: start block addr: 0x%x\n", total_blocks);

                dprintf(INFO, "addr 0x%llx, partsz 0x%x\n", ((unsigned long long)total_blocks*sparse_header->blk_sz) , chunk_data_sz);
                ret = (size_t)bio_write(bdev, data, ((unsigned long long)total_blocks*sparse_header->blk_sz), chunk_data_sz);
                if (ret != chunk_data_sz) {
                    fastboot_fail_wrapper("flash write failure");
                    goto error;
                }

                total_blocks += chunk_header->chunk_sz;
                data += chunk_data_sz;
                break;

            case CHUNK_TYPE_DONT_CARE:
                dprintf(INFO, "!!Blank: start: 0x%x  offset: 0x%x\n", total_blocks, chunk_header->chunk_sz);
                total_blocks += chunk_header->chunk_sz;
                break;

            case CHUNK_TYPE_FILL:
                dprintf(INFO, "%s %d: CHUNK_TYPE_FILL=0x%x size=%d chunk_data_sz=%d\n", __FUNCTION__, __LINE__, *(uint32_t *)data, ROUNDUP(sparse_header->blk_sz, CACHE_LINE), chunk_data_sz);
                if (chunk_header->total_sz != (sparse_header->chunk_hdr_sz + sizeof(uint32_t))) {
                    fastboot_fail_wrapper("Bogus chunk size for chunk type FILL");
                    goto error;
                }

                fill_val = *(uint32_t *)data;
                data = (char *) data + sizeof(uint32_t);
                chunk_blk_cnt = chunk_data_sz / sparse_header->blk_sz;

                if (fill_val != pre_fill_val) {
                    pre_fill_val = fill_val;
                    for (i = 0; i < (sparse_header->blk_sz / sizeof(fill_val)); i++)
                        fill_buf[i] = fill_val;
                }

                for (i = 0; i < chunk_blk_cnt; i++) {
                    if (fill_val == erase_val) {
                        /* To assume partition already erased */
                        /* skip fill value as same as erase_byte */
                        dprintf(INFO, "skip CHUNK_TYPE_FILL with value '%x'\n", fill_val);
                    } else {
                        ret = (size_t)bio_write(bdev, fill_buf, ((uint64_t)total_blocks*sparse_header->blk_sz), sparse_header->blk_sz);
                        if (ret != sparse_header->blk_sz) {
                            fastboot_fail_wrapper("CHUNK_TYPE_FILL flash write failure");
                            goto error;
                        }
                    }
                    total_blocks++;
                }
                break;

            case CHUNK_TYPE_CRC:
                if (chunk_header->total_sz != sparse_header->chunk_hdr_sz) {
                    fastboot_fail_wrapper("Bogus chunk size for chunk type Dont Care");
                    goto error;
                }
                total_blocks += chunk_header->chunk_sz;
                data += chunk_data_sz;
                break;

            default:
                fastboot_fail_wrapper("Unknown chunk type");
                goto error;
        }
    }

    dprintf(ALWAYS, "Wrote %d blocks, expected to write %d blocks\n",
            total_blocks, sparse_header->total_blks);

    if (total_blocks != sparse_header->total_blks) {
        fastboot_fail_wrapper("sparse image write failure");
    } else {
        fastboot_okay("");
    }

error:
    free(fill_buf);
    return;
}

void cmd_flash(const char *arg, void *data, unsigned sz)
{
    sparse_header_t *sparse_header;

    dprintf(ALWAYS, "cmd_flash: %s, %d\n", arg, (u32)sz);

    if (sz  == 0) {
        fastboot_okay("");
        return;
    }

    TIME_START;

    sparse_header = (sparse_header_t *) data;
    if (sparse_header->magic != SPARSE_HEADER_MAGIC)
        cmd_flash_img(arg, data, sz);
    else {
        cmd_flash_sparse_img(arg, data, sz);
    }
    return;
}

void cmd_erase(const char *arg, void *data, unsigned sz)
{
#define MAX_ERASE_SIZE  (UINT_MAX & ~(bdev->block_size - 1))
    bdev_t *bdev;
    ssize_t ret = 0, erase_len = 0;
	char response[MAX_RSP_SIZE];

    dprintf(ALWAYS, "cmd_erase\n");
    bdev = bio_open_by_label(arg);
    if (!bdev) {
        bdev = bio_open(arg);
        if (!bdev) {
            fastboot_fail_wrapper("Partition is not exist.");
            return;
        }
    }

    if (!bdev->total_size) {
        fastboot_fail_wrapper("Size is uncorrect.");
        return;
    }

    dprintf(INFO, "%s %s %lld %d\n", bdev->name, bdev->label, bdev->total_size, bdev->block_count);
    if (bdev->total_size > (off_t)MAX_ERASE_SIZE) {
        off_t offset = 0LL, size = bdev->total_size;
        size_t len;
        do {
            len = (size_t)MIN(size, (off_t)MAX_ERASE_SIZE);
            ret = bio_erase(bdev, offset, len);
            if (ret < 0) {
                fastboot_fail_wrapper("Erase failure.");
                return;
            }
            size -= len;
            offset += len;
        } while (size);
    } else {
        ret = bio_erase(bdev, 0, bdev->total_size);
        if (ret < 0) {
            fastboot_fail_wrapper("Erase failure.");
            return;
        }
		erase_len = ret;
    }

	snprintf(response, MAX_RSP_SIZE, "request sz: 0x%lx, real erase len: 0x%lx",
			 bdev->total_size, erase_len);
	fastboot_info(response);

    fastboot_okay("");

    return;
}

extern unsigned fastboot_state;
void cmd_continue_boot(const char *arg, void *data, unsigned sz)
{
    fastboot_okay("");
    /* set state to leave fastboot command loop and handler */
    fastboot_state = STATE_RETURN;
    mtk_wdt_init();
}

void cmd_download_tz(const char *arg, void *data, unsigned sz)
{
    int ret = 0;
    void *fit;

    if (sz < MAX_TEE_DRAM_SIZE) {
        ret = fit_get_image_from_buffer((char *)SCRATCH_ADDR, &fit, (char *)tz_buf);
        if (ret) {
            dprintf(CRITICAL, "%s can't get fit header\n", (char *)TZ_PART_NAME);
            fastboot_fail_wrapper("can't get fit header");
        }
        fastboot_okay("");
    } else {
        dprintf(CRITICAL, "%s size is too large\n", (char *)TZ_PART_NAME);
        fastboot_fail_wrapper("size is too large");
    }
}

void cmd_download_boot(const char *arg, void *data, unsigned sz)
{
    int ret = 0;
    void *fit;

    if (sz < MAX_KERNEL_SIZE) {
        ret = fit_get_image_from_buffer((char *)SCRATCH_ADDR, &fit, (char *)kernel_buf);
        if (ret) {
            dprintf(CRITICAL, "%s can't get fit header\n", (char *)BOOT_PART_NAME);
            fastboot_fail_wrapper("can't get fit header");
        }
        fastboot_okay("");
    } else {
        dprintf(CRITICAL, "%s size is too large\n", (char *)BOOT_PART_NAME);
        fastboot_fail_wrapper("size is too large");
    }
}
/**********END********/
/*LXO: END!Download related command*/

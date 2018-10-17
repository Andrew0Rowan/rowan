
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

#include <errno.h>
#include <lib/bio.h>
#include <libfdt.h>
#include <lib/decompress.h>
#include <kernel/thread.h>

#include "fit.h"
#include "image.h"

#define uswap_32(x) \
    ((((x) & 0xff000000) >> 24) | \
     (((x) & 0x00ff0000) >>  8) | \
     (((x) & 0x0000ff00) <<  8) | \
     (((x) & 0x000000ff) << 24))

int fit_image_get_node(const void *fit, const char *image_uname)
{
    int noffset, images_noffset;

    images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
    if (images_noffset < 0) {
        dprintf(CRITICAL,"Can't find images parent node '%s' (%s)\n",
                FIT_IMAGES_PATH, fdt_strerror(images_noffset));
        return images_noffset;
    }

    noffset = fdt_subnode_offset(fit, images_noffset, image_uname);
    if (noffset < 0) {
        dprintf(CRITICAL,"Can't get node offset for image unit name: '%s' (%s)\n",
                image_uname, fdt_strerror(noffset));
    }

    return noffset;
}

int fit_conf_get_prop_node(const void *fit, int noffset,
                           const char *prop_name)
{
    char *uname;
    int len;

    /* get kernel image unit name from configuration kernel property */
    uname = (char *)fdt_getprop(fit, noffset, prop_name, &len);
    if (uname == NULL)
        return len;

    return fit_image_get_node(fit, uname);
}

/**
 * fit_get_img_subnode_offset() - get a subnode offset for a given image name
 *
 * This finds subnode offset using given image name within node "/images"
 *
 * @fit:    fit image start address
 * @image_name: image name. "kernel", "fdt" or "ramdisk"...
 *
 * returns:
 *    great than or equal 0, on success
 *    otherwise, on failure
 *
 */
static int fit_get_img_subnode_offset(void *fit, const char *image_name)
{
    int noffset;

    /* get image node offset */
    noffset = fdt_path_offset(fit, "/images");
    if (noffset < 0) {
        dprintf(CRITICAL, "Can't find image node (%s)\n", fdt_strerror(noffset));
        return noffset;
    }

    /* get subnode offset */
    noffset = fdt_subnode_offset(fit, noffset, image_name);
    if (noffset < 0)
        dprintf(CRITICAL, "Can't get node offset for image unit name: '%s' (%s)\n",
                image_name, fdt_strerror(noffset));

    return noffset;
}

/**
 * fit_get_def_cfg_offset() - get a subnode offset from node "/configurations"
 *
 * This finds configuration subnode offset in node "configruations".
 * If "conf" is not given, it will find property "default" for the case.
 *
 * @fit:    fit image start address
 * @conf:   configuration name
 *
 * returns:
 *    great than or equal 0, on success
 *    otherwise, on failure
 *
 */
static int fit_get_def_cfg_offset(void *fit, const char *conf)
{
    int noffset, cfg_noffset, len;

    noffset = fdt_path_offset(fit, "/configurations");
    if (noffset < 0) {
        dprintf(CRITICAL, "can't find configuration node\n");
        return noffset;
    }

    if (conf == NULL) {
        conf = (char *)fdt_getprop(fit, noffset,
                                   "default", &len);
        if (conf == NULL) {
            dprintf(CRITICAL, "Can't get default conf name\n");
            return len;
        }
        dprintf(SPEW, "got default conf: %s\n", conf);
    }

    cfg_noffset = fdt_subnode_offset(fit, noffset, conf);
    if (cfg_noffset < 0)
        dprintf(CRITICAL, "Can't get conf subnode\n");

    return cfg_noffset;
}

int fit_get_image(const char *label, void **fit, void *load_buf)
{
    bdev_t *bdev;
    struct fdt_header fdt;
    size_t totalsize;
    int fdt_len, ret = 0;

    fdt_len = sizeof(struct fdt_header);
    bdev = bio_open_by_label(label);
    if (!bdev) {
        dprintf(CRITICAL, "Partition [%s] is not exist.\n", label);
        return -ENODEV;
    }
    bio_read(bdev, &fdt, 0, fdt_len);
    ret = fdt_check_header(&fdt);
    if (ret) {
        dprintf(CRITICAL, "[%s] check header failed\n", label);
        return ret;
    }
    totalsize = fdt_totalsize(&fdt);
    bio_read(bdev, load_buf, 0, totalsize);
    *fit = load_buf;

    return 0;
}

int fit_get_image_from_buffer(const char *buffer, void **fit, void *load_buf)
{
    struct fdt_header fdt;
    size_t totalsize;
    int fdt_len, ret = 0;

    fdt_len = sizeof(struct fdt_header);
    memcpy(&fdt, buffer, fdt_len);
    ret = fdt_check_header(&fdt);
    if (ret) {
        dprintf(CRITICAL, "[%s] check header failed\n", buffer);
        return ret;
    }
    totalsize = fdt_totalsize(&fdt);
    memcpy(load_buf, buffer, totalsize);
    *fit = load_buf;

    return 0;
}

int fit_load_image(const char *conf, const char *img_pro, void *fit,
                   ulong *load_addr, ulong *entry_addr)
{
    int noffset, len, cfg_noffset, ret;
    size_t size;
    const void *data, *compression;
    const uint32_t *prop_data;
    const char *image_name;
    paddr_t op_load_addr;


    /* get defualt configuration offset (conf@1, conf@2,...or confg@n) */
    cfg_noffset = fit_get_def_cfg_offset(fit, conf);
    if (cfg_noffset < 0) {
        dprintf(CRITICAL, "Can't get default conf offset\n");
        return cfg_noffset;
    }
    dprintf(SPEW, "defulat config name: %s\n",
            fdt_get_name(fit, cfg_noffset, NULL));

    /* TODO: possibly verify this config */
    thread_t *current_thread = get_current_thread();
    if (rsa_check_enabled()&&!strcmp(img_pro,"kernel")) {
        dprintf(ALWAYS,"Verifying %s Sign  \n",current_thread->name);
        if (fit_verify_sign(fit, cfg_noffset)) {
            puts("Sign verify fail !\n");
            return -EACCES;
        }
    }
    /* verify end */

    /* unit name: fdt@1, kernel@2, ramdisk@3 and so on */
    image_name = (char *)fdt_getprop(fit, cfg_noffset, img_pro, &len);
    if (image_name == NULL) {
        dprintf(INFO, "%s get image name failed\n", img_pro);
        return -ENOENT;
    }

    /* get thi sub image node offset */
    noffset = fit_get_img_subnode_offset(fit, image_name);
    if (noffset < 0) {
        dprintf(CRITICAL, "get sub image node (%s) failed\n", image_name);
        return noffset;
    }

    /* TODO: possibly verify this image */
    if (hash_check_enabled()&&(!strcmp(img_pro,"kernel")||!strcmp(img_pro,"fdt")|| !strcmp(img_pro,"ramdisk")||!strcmp(img_pro,"tee"))) {
        dprintf(ALWAYS, "checking %s - %s integrity \n",current_thread->name,img_pro);
        noffset = fit_conf_get_prop_node(fit, cfg_noffset,img_pro);
        ret = subimage_check_integrity(fit, noffset);
        if (ret < 0) {
            dprintf(CRITICAL, "check_integrity fail !\n");
            return -EACCES;
        }
    }
    /* verify end */

    data = fdt_getprop(fit, noffset, "data", &len);
    if (!data) {
        dprintf(CRITICAL, "%s can't get prop data\n", image_name);
        return len;
    }
    size = len;

    prop_data = fdt_getprop(fit, noffset, "load", &len);
    compression = fdt_getprop(fit, noffset, "compression", &len);

    if (!prop_data && !strcmp(img_pro, "kernel")) {
        dprintf(CRITICAL, "%s need load addr\n", img_pro);
        return -EINVAL;
    }

    if (!compression) {
        dprintf(CRITICAL, "%s compression is not specified\n", image_name);
        return -EINVAL;
    }

    op_load_addr = prop_data? uswap_32(*prop_data): 0;
    op_load_addr = op_load_addr? op_load_addr: (ulong)data;

    extern __WEAK void *dram_map(paddr_t pa);
    op_load_addr = dram_map?(ulong)dram_map(op_load_addr):op_load_addr;
    if (!strcmp((char *)compression, "lz4")) {
        ret = unlz4(data, size - 4, (void *)(op_load_addr));
        if (ret != LZ4_OK) {
            dprintf(ALWAYS, "lz4 decompress failure\n");
            return -LZ4_FAIL;
        }
    } else if (!strcmp((char *)compression, "none")) {
        memmove((void *)(op_load_addr), data, size);
    } else {
        dprintf(CRITICAL, "%s compression does not support\n", image_name);
        return -EINVAL;
    }

    dprintf(SPEW, "[%s] load_addr 0x%lx\n", image_name, op_load_addr);
    dprintf(SPEW, "[%s] fit = %p\n", image_name, fit);
    dprintf(SPEW, "[%s] data = %p\n", image_name, data);
    dprintf(SPEW, "[%s] size = %zu\n", image_name, size);

    /* get entry address */
    if (entry_addr) {
        const uint32_t *load_prop = fdt_getprop(fit, noffset, "load", &len);
        ulong tmp_load_addr = 0;

        if (load_prop)
            tmp_load_addr = uswap_32(*load_prop);
        prop_data = fdt_getprop(fit, noffset, "entry", &len);
        if (!prop_data) {
            *entry_addr = op_load_addr;
        } else if (tmp_load_addr) {
            /* the absolute address */
            *entry_addr = uswap_32(*prop_data);
        } else {
            /* the relative address */
            *entry_addr = (op_load_addr + uswap_32(*prop_data));
        }
        dprintf(SPEW, "[%s] entry_addr 0x%lx\n", image_name, *entry_addr);
    }

    /* return load address if caller spcified */
    if (load_addr)
        *load_addr = op_load_addr;

    return 0;
}

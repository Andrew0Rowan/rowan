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
#pragma once

#include <sys/types.h>

/**
 * fit_get_image() - load fit image from a partition
 *
 * the function will use bio to access a partition from a storage and
 * check fdt header. If pass, copy the whole image to load_buf
 *
 * @label:  partition name
 * @fit:    fit image start address in load_buf
 * @load_buf:   load fit image to the buffer
 *
 * returns:
 *     0, on success
 *     otherwise, on failure
 *
 */
int fit_get_image(const char *label, void **fit, void *load_buf);

/**
 * fit_get_image_from_buffer() - load fit image from a memory buffer
 *
 * the function will use memcpy to access a image from buffer and
 * check fdt header. If pass, copy the whole image to load_buf
 *
 * @buffer:  memory buffer
 * @fit:    fit image start address in load_buf
 * @load_buf:   load fit image to the buffer
 *
 * returns:
 *     0, on success
 *     otherwise, on failure
 *
 */
int fit_get_image_from_buffer(const char *buffer, void **fit, void *load_buf);

/**
 * fit_load_image() - load fit image to proper address
 *
 * This checks FIT configuration to find sub-image nodes image
 * and load the image to right address
 *
 * @conf:   configuration name
 * @img_pro:    image property name
 * @fit:    fit image start address
 * @load_addr:  returned load address
 * @entry_addr: returned entry address
 *
 * returns:
 *    0, on success
 *    otherwise, on failure
 *
 */
int fit_load_image(const char *conf, const char *img_pro, void *fit,
                   ulong *load_addr, ulong *entry_addr);

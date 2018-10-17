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

#define LZ4_OK 0 /* lz4 decode ok */
#define LZ4_FAIL 1 /* lz4 decode failure */
#define ELZ4WO 2 /* write overflow */
#define ELZ4IR 3 /* invalid return value */
#define ELZ4NP 4 /* null pointer */
#define ELZ4IH 5 /* invalid header */
#define ELZ4DF 6 /* decode failure */
#define ELZ4DC 7 /* data corrupt */

/**
 * unlz4() - do decompression for lz4 compressed format
 *
 * This decodes from input keeping compressed data
 * to output with uncompressed data. If input is equal to
 * output, the decompression will be failure.
 *
 * @input: lz4 compressed data address
 * @output: lz4 uncompressed data address
 * @in_len: size of lz4 compressed data
 *
 * returns:
 *    0 (i.e. LZ4_OK), on success
 *    otherwise, on failure
 *
 */
int unlz4(const void *input, int in_len, void *output);

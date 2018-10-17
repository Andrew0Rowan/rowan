/*
 * Copyright (c) 2017 MediaTek Inc.
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

#ifndef _MTK_NAND_COMMON_H_
#define _MTK_NAND_COMMON_H_

#include <platform.h>

#define NAND_BIT(nr)		(1UL << (nr))
#define NAND_GENMASK(h, l)	(((~0UL) << (l)) & (~0UL >> ((sizeof(unsigned long) * 8) - 1 - (h))))
#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))
#define max(a, b)		(a > b ? a : b)
#define min(a, b)		(a > b ? b : a)
#define clamp(val, lo, hi)	min((typeof(val))max(val, lo), hi)

#define MTK_TIMEOUT		(500000)

/* reserve 1M dram buffer*/
#define NAND_DRAM_BUF_DATABUF_ADDR	(NAND_BUF_ADDR)
#define NAND_DRAM_BUF_DATABUF_SIZE	(16384+2048)
#define NAND_DRAM_BUF_NFCBUF_ADDR	(NAND_DRAM_BUF_DATABUF_ADDR + NAND_DRAM_BUF_DATABUF_SIZE)
#define NAND_DRAM_BUF_NFCBUF_SIZE	(16384+2048)
#define NAND_DRAM_BUF_ECCDE_ADDR	(NAND_DRAM_BUF_NFCBUF_ADDR + NAND_DRAM_BUF_NFCBUF_SIZE)
#define NAND_DRAM_BUF_ECCDE_SIZE	(16384+2048)
#define NAND_DRAM_BUF_ECCEN_ADDR	(NAND_DRAM_BUF_ECCDE_ADDR + NAND_DRAM_BUF_ECCDE_SIZE)
#define NAND_DRAM_BUF_ECCEN_SIZE	(16384+2048)
#define NAND_DRAM_BUF_BAD_MAP_ADDR	(NAND_DRAM_BUF_ECCEN_ADDR + NAND_DRAM_BUF_ECCEN_SIZE)
#define NAND_DRAM_BUF_BAD_MAP_SIZE	(16384)

/*
 * wait until cond gets true or timeout.
 *
 * cond : C expression to wait
 * timeout : usecs
 *
 * Returns:
 * 0 : if cond = false after timeout elapsed.
 * 1 : if cond = true after timeout elapsed,
 * or the remain usecs if cond = true before timeout elapsed.
 */
#define check_with_timeout(cond, timeout) 					\
({										\
	lk_bigtime_t __ret;							\
	if (cond) {								\
		 __ret = timeout;						\
	} else {								\
		lk_bigtime_t __end = current_time_hires() + timeout;		\
										\
		for (;;) { 							\
			lk_bigtime_t __now = current_time_hires(); 		\
										\
			if (cond) {						\
				__ret = (__end > __now) ? (__end - __now) : 1;	\
				break; 						\
			}							\
										\
			if (__end <= __now) {					\
				__ret = 0; 					\
				break; 						\
			}							\
		}								\
	}									\
	__ret; 									\
})

#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while(0)

#endif

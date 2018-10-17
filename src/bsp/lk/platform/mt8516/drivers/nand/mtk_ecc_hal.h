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
#ifndef _MTK_ECC_HAL_H_
#define _MTK_ECC_HAL_H_

#include <sys/types.h>

#define ECC_PARITY_BITS		(14)

struct mtk_ecc;

enum mtk_ecc_mode {ECC_DMA_MODE = 0, ECC_NFI_MODE = 1, ECC_PIO_MODE = 2};
enum mtk_ecc_operation {ECC_ENCODE, ECC_DECODE};
enum mtk_ecc_deccon {ECC_DEC_FER = 1, ECC_DEC_LOCATE = 2, ECC_DEC_CORRECT = 3};

struct mtk_ecc_stats {
	u32 corrected;
	u32 bitflips;
	u32 failed;
};

struct mtk_ecc_config {
	enum mtk_ecc_operation op;
	enum mtk_ecc_mode mode;
	enum mtk_ecc_deccon deccon;
	u32 addr;
	u32 strength;
	u32 sectors;
	u32 len;
};

extern int mtk_ecc_encode(struct mtk_ecc *ecc, struct mtk_ecc_config *config,
		   u8 *data, u32 bytes, int polling);
extern int mtk_ecc_enable(struct mtk_ecc *ecc, struct mtk_ecc_config *config, int polling);
extern void mtk_ecc_disable(struct mtk_ecc *ecc);
extern void mtk_ecc_get_stats(struct mtk_ecc *ecc, struct mtk_ecc_stats *stats, int sectors);
extern int mtk_ecc_cpu_correct(struct mtk_ecc *ecc, struct mtk_ecc_stats *stats, u8 *data, u32 sector, int polling);
extern int mtk_ecc_wait_done(struct mtk_ecc *ecc, enum mtk_ecc_operation op, int polling);
extern int mtk_ecc_hw_init(struct mtk_ecc **ext_ecc);
extern int mtk_ecc_wait_decode_fsm_idle(struct mtk_ecc *ecc);
extern int mtk_ecc_decode(struct mtk_ecc *ecc, struct mtk_ecc_config *config, u8 *data, u32 len, int polling);

#endif

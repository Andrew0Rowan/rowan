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
#ifndef _MTK_NFI_HAL_H_
#define _MTK_NFI_HAL_H_

#include <kernel/event.h>
#include "mtk_nand_nal.h"

/*
 * used to do bad mark byte swap
 */
struct mtk_nfc_bad_mark_ctl {
	void (*bm_swap)(struct mtk_nand_chip *chip, u8 *buf, int raw);
	u32 sec;
	u32 pos;
};

/*
 * FDM: region used to store free OOB data
 */
struct mtk_nfc_fdm {
	u32 reg_size;
	u32 ecc_size;
};

struct mtk_nfc {
	mutex_t lock;
	event_t irq_event;
	struct mtk_ecc_config ecc_cfg;
	struct mtk_ecc *ecc;

	u64 regs;
	u8 *buffer;
};

struct mtk_nfc_nand_chip {
	struct mtk_nand_chip chip;
	struct mtk_nfc_bad_mark_ctl bad_mark;
	struct mtk_nfc_fdm fdm;

	u32 spare_per_sector;
	u32 acctiming;
};

extern int mtk_nfc_nand_chip_init(struct mtk_nand_chip **ext_nand);

enum mtk_randomizer_operation {RAND_ENCODE, RAND_DECODE};
extern void mtk_nfc_randomizer_disable(struct mtk_nand_chip *chip);
extern void mtk_nfc_randomizer_enable(struct mtk_nand_chip *chip, int page,
		enum mtk_randomizer_operation rand, int repage);
#endif

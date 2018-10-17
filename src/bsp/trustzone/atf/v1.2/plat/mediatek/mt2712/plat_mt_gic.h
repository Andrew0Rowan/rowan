/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __PLAT_MT_GIC_H__
#define __PLAT_MT_GIC_H__

#define IRQ_MASK_HEADER		0xF1F1F1F1
#define IRQ_MASK_FOOTER		0xF2F2F2F2

typedef struct mtk_irq_mask {
	uint32_t header;	/* for error checking */
	uint32_t mask0;
	uint32_t mask1;
	uint32_t mask2;
	uint32_t mask3;
	uint32_t mask4;
	uint32_t mask5;
	uint32_t mask6;
	uint32_t mask7;
	uint32_t mask8;
	uint32_t mask9;
	uint32_t mask10;
	uint32_t mask11;
	uint32_t mask12;
	uint32_t mask13;
	uint32_t mask14;
	uint32_t footer;	/* for error checking */
} mtk_irq_mask_t;

void mt_gic_dist_save(void);
void mt_gic_dist_restore(void);
void mt_gic_cpuif_setup(void);
uint32_t mt_irq_get_pending(uint32_t irq);
void mt_irq_set_pending(uint32_t irq);
int32_t mt_irq_mask_all(mtk_irq_mask_t *mask);
int32_t mt_irq_mask_restore(struct mtk_irq_mask *mask);
void mt_irq_mask_for_sleep(uint32_t irq);
void mt_irq_unmask_for_sleep(int32_t irq);
#endif  /*!__CIRQ_H__ */

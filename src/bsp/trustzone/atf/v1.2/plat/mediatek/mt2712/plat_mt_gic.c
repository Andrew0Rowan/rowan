/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <arch_helpers.h>
#include <gic_common.h>
#include <gicv2.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <bl_common.h>
#include <mt2712_def.h>
#include <platform_def.h>
#include <mmio.h>
#include <plat_mt_gic.h>
#include <debug.h>

const unsigned int mt_irq_sec_array[] = {
	MT_IRQ_SEC_SGI_0,
	MT_IRQ_SEC_SGI_1,
	MT_IRQ_SEC_SGI_2,
	MT_IRQ_SEC_SGI_3,
	MT_IRQ_SEC_SGI_4,
	MT_IRQ_SEC_SGI_5,
	MT_IRQ_SEC_SGI_6,
	MT_IRQ_SEC_SGI_7,
	//WDT_IRQ_BIT_ID,
};

gicv2_driver_data_t mt_gic_data = {
	.gicd_base = BASE_GICD_BASE,
	.gicc_base = BASE_GICC_BASE,
	.g0_interrupt_num = ARRAY_SIZE(mt_irq_sec_array),
	.g0_interrupt_array = mt_irq_sec_array,
};

void plat_mt_gic_driver_init(void)
{
	gicv2_driver_init(&mt_gic_data);
}

void plat_mt_gic_init(void)
{
	int idx;

	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	/* set pol control as non-secure */
	for (idx=0;idx<INT_POL_SECCTL_NUM;idx++)
		mmio_write_32(INT_POL_SECCTL0+idx*4, 0);

	/* enable axi bus for prepare mode */
	mmio_write_32(DEBUGSYS_BASE+0x40, 1);
}

static inline void gicd_write_sgir(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICD_SGIR, val);
}

void irq_raise_softirq(unsigned int map, unsigned int irq)
{
	int satt;

	satt = 1 << 15;

	if(plat_ic_get_interrupt_type(irq) == INTR_TYPE_S_EL1)
	{
	    satt = 0;
	}

	gicd_write_sgir(BASE_GICD_BASE, (map << 16) | satt | irq);

	dsb();
}

uint32_t mt_irq_get_pending(uint32_t irq)
{
	uint32_t bit = 1 << (irq % 32);

	return (mmio_read_32(BASE_GICD_BASE + GICD_ISPENDR + irq / 32 * 4) & bit) ? 1 : 0;
}


void mt_irq_set_pending(uint32_t irq)
{
	uint32_t bit = 1 << (irq % 32);

	mmio_write_32(BASE_GICD_BASE + GICD_ISPENDR + irq / 32 * 4, bit);
}
/*
 * mt_irq_mask_all: disable all interrupts
 * @mask: pointer to struct mtk_irq_mask for storing the original mask value.
 * Return 0 for success; return negative values for failure.
 * (This is ONLY used for the idle current measurement by the factory mode.)
 */
int32_t mt_irq_mask_all(mtk_irq_mask_t *mask)
{
	if (!mask)
		return -1;

	mask->mask0 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER));
	mask->mask1 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x4));
	mask->mask2 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x8));
	mask->mask3 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0xC));
	mask->mask4 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x10));
	mask->mask5 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x14));
	mask->mask6 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x18));
	mask->mask7 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x1C));
	mask->mask8 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x20));
	mask->mask9 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x24));
	mask->mask10 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x28));
	mask->mask11 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x2c));
	mask->mask12 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x30));
	mask->mask13 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x34));
	mask->mask14 = mmio_read_32((BASE_GICD_BASE + GICD_ISENABLER + 0x38));

	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x4), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x8), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0xc), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x10), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x14), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x18), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x1c), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x20), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x24), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x28), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x2c), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x30), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x34), 0xFFFFFFFF);
	mmio_write_32((BASE_GICD_BASE + GICD_ICENABLER + 0x38), 0xFFFFFFFF);

	mask->header = IRQ_MASK_HEADER;
	mask->footer = IRQ_MASK_FOOTER;

	return 0;
}



/*
 * mt_irq_mask_restore: restore all interrupts
 * @mask: pointer to struct mtk_irq_mask for storing the original mask value.
 * Return 0 for success; return negative values for failure.
 * (This is ONLY used for the idle current measurement by the factory mode.)
 */
int32_t mt_irq_mask_restore(struct mtk_irq_mask *mask)
{
	if (!mask)
		return -1;
	if (mask->header != IRQ_MASK_HEADER)
		return -1;
	if (mask->footer != IRQ_MASK_FOOTER)
		return -1;

	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER), mask->mask0);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x4), mask->mask1);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x8), mask->mask2);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0xC), mask->mask3);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x10), mask->mask4);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x14), mask->mask5);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x18), mask->mask6);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x1C), mask->mask7);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x20), mask->mask8);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x24), mask->mask9);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x28), mask->mask10);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x2C), mask->mask11);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x30), mask->mask12);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x34), mask->mask13);
	mmio_write_32((BASE_GICD_BASE + GICD_ISENABLER + 0x38), mask->mask14);


	return 0;
}


/*
 * mt_irq_mask_for_sleep: disable an interrupt for the sleep manager's use
 * @irq: interrupt id
 * (THIS IS ONLY FOR SLEEP FUNCTION USE. DO NOT USE IT YOURSELF!)
 */
void mt_irq_mask_for_sleep(uint32_t irq)
{
	uint32_t mask;

	mask = 1 << (irq % 32);
	if (irq < 16) {
		tf_printf("Fail to enable interrupt %d\n", irq);
		return;
	}

	mmio_write_32(BASE_GICD_BASE + GICD_ICENABLER + irq / 32 * 4, mask);
}

/*
 * mt_irq_unmask_for_sleep: enable an interrupt for the sleep manager's use
 * @irq: interrupt id
 * (THIS IS ONLY FOR SLEEP FUNCTION USE. DO NOT USE IT YOURSELF!)
 */
void mt_irq_unmask_for_sleep(int32_t irq)
{

	uint32_t mask;

	mask = 1 << (irq % 32);
	if (irq < 16) {
		tf_printf("Fail to enable interrupt %d\n", irq);
		return;
	}

	mmio_write_32(BASE_GICD_BASE + GICD_ISENABLER + irq / 32 * 4, mask);
}

//===========================================================================
// Set WDT interrupt pending. It is for simulation of WDT when processing SMC
// "AEEDump".
//===========================================================================
void mt_atf_trigger_WDT_FIQ(void)
{
	gicd_set_ispendr(MT_GIC_BASE, WDT_IRQ_BIT_ID);
}

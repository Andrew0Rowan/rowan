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
#include <arm_gic.h>
#include <bl_common.h>
#include <mcsi.h>
#include <debug.h>
#include <platform_def.h>
#include <xlat_tables.h>

uint64_t wdt_kernel_cb_addr = 0;

#ifdef DRAM_EXTENSION_SUPPORT
extern unsigned long __DRAM_RO_START__;
extern unsigned long __DRAM_RO_END__;
extern unsigned long __DRAM_RW_START__;
extern unsigned long __DRAM_RW_END__;

#define DRAM_RO_BASE (unsigned long)(&__DRAM_RO_START__)
#define DRAM_RO_END (unsigned long)(&__DRAM_RO_END__)
#define DRAM_RW_BASE (unsigned long)(&__DRAM_RW_START__)
#define DRAM_RW_END (unsigned long)(&__DRAM_RW_END__)
#endif



static const int cci_map[] = {
	PLAT_MT_CCI_CLUSTER0_SL_IFACE_IX,
	PLAT_MT_CCI_CLUSTER1_SL_IFACE_IX,
	PLAT_MT_CCI_CLUSTER2_SL_IFACE_IX
};

/* Table of regions to map using the MMU.  */
const mmap_region_t plat_mmap[] = {
	/* for TF text, RO, RW */
	MAP_REGION_FLAT(TZRAM_BASE, TZRAM_SIZE + TZRAM2_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MTK_DEV_RNG0_BASE, MTK_DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MTK_DEV_RNG1_BASE, MTK_DEV_RNG1_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(TZRAM2_BASE & ~(PAGE_SIZE_MASK), TZRAM2_SIZE,
					MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(RAM_CONSOLE_BASE & ~(PAGE_SIZE_MASK), RAM_CONSOLE_SIZE,
						MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(MT_GIC_BASE & ~(PAGE_SIZE_MASK), MCU_SYS_SIZE,
								MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PLAT_MT_CCI_BASE & ~(PAGE_SIZE_MASK), CCI_SIZE,
								MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(SPM_FW_BASE & ~(PAGE_SIZE_MASK), SPM_FW_SIZE,
					MT_MEMORY | MT_RW | MT_SECURE),
	{ 0 }

};

/*******************************************************************************
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 ******************************************************************************/
#ifdef DRAM_EXTENSION_SUPPORT
#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void plat_configure_mmu_el ## _el(unsigned long total_base,	\
					  unsigned long total_size,	\
					  unsigned long ro_start,	\
					  unsigned long ro_limit,	\
					  unsigned long coh_start,	\
					  unsigned long coh_limit)	\
	{								\
		mmap_add_region(total_base, total_base,			\
				total_size,				\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(DRAM_RO_BASE, DRAM_RO_BASE,			\
				DRAM_RO_END - DRAM_RO_BASE,			\
				MT_MEMORY | MT_RO | MT_SECURE);		\
		mmap_add_region(DRAM_RW_BASE, DRAM_RW_BASE, 		\
					DRAM_RW_END - DRAM_RW_BASE,			\
					MT_MEMORY | MT_RW | MT_SECURE); 	\
		mmap_add_region(ro_start, ro_start, 		\
					ro_limit - ro_start,			\
					MT_MEMORY | MT_RO | MT_SECURE); 	\
		mmap_add_region(coh_start, coh_start,			\
				coh_limit - coh_start,			\
				MT_DEVICE | MT_RW | MT_SECURE);		\
		mmap_add(plat_mmap);					\
		init_xlat_tables();					\
									\
		enable_mmu_el ## _el(0);				\
	}
#else
#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void plat_configure_mmu_el ## _el(unsigned long total_base,	\
					  unsigned long total_size,	\
					  unsigned long ro_start,	\
					  unsigned long ro_limit,	\
					  unsigned long coh_start,	\
					  unsigned long coh_limit)	\
	{								\
		mmap_add_region(total_base, total_base,			\
				total_size,				\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(ro_start, ro_start, 		\
					ro_limit - ro_start,			\
					MT_MEMORY | MT_RO | MT_SECURE); 	\
		mmap_add_region(coh_start, coh_start,			\
				coh_limit - coh_start,			\
				MT_DEVICE | MT_RW | MT_SECURE);		\
		mmap_add(plat_mmap);					\
		init_xlat_tables();					\
									\
		enable_mmu_el ## _el(0);				\
	}
#endif
/* Define EL3 variants of the function initialising the MMU */
DEFINE_CONFIGURE_MMU_EL(3)

uint64_t plat_get_syscnt_freq(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

#ifdef ERRATA_MCSIB_SW
static unsigned int CntRead1_1, CntRead2_1,CntRead3_1,CntRead1_2, CntRead2_2,CntRead3_2;
static unsigned int Check_1,Check_2,Check_3;

int mcsib_sw_workaround_R2(void)
{
	*(volatile unsigned int *)(0x0C507100) = 0x800001;
	CntRead1_2 = *(volatile unsigned int *)(0x0C507104);

	*(volatile unsigned int *)(0x0C507100) = 0x9c0001;
	CntRead2_2 = *(volatile unsigned int *)(0x0C507104);

	*(volatile unsigned int *)(0x0C507100) = 0xac0001;
	CntRead3_2 = *(volatile unsigned int *)(0x0C507104);

	if ((CntRead1_1 < 2)||(CntRead1_2 < 2)||(CntRead1_1<CntRead1_2) )
		Check_1 =1;
	else
		Check_1 =0;
	if ((CntRead2_1 < 2)||(CntRead2_2 < 2)||(CntRead2_1<CntRead2_2) )
		Check_2 =1;
	else
		Check_2 =0;
	if ((CntRead3_1 < 2)||(CntRead3_2 < 2)||(CntRead3_1<CntRead3_2) )
		Check_3 =1;
	else
		Check_3 =0;
	if (Check_1 && Check_2 && Check_3)
		return 1;	// pass
	else
		return 0;	// fail
}

int mcsib_sw_workaround_R1(void)
{
	*(volatile unsigned int *)(0x0C507100) = 0x8c0001;
	CntRead1_1 = *(volatile unsigned int *)(0x0C507104);

	*(volatile unsigned int *)(0x0C507100) = 0x9c0001;
	CntRead2_1 = *(volatile unsigned int *)(0x0C507104);

	*(volatile unsigned int *)(0x0C507100) = 0xac0001;
	CntRead3_1 = *(volatile unsigned int *)(0x0C507104);

	if ((CntRead1_1 < 2) && (CntRead2_1 < 2) && (CntRead3_1 < 2))
		return 1;	// pass
	else
		return 0;	// fail
}

int mcsib_sw_workaround_main(void)
{
	if (!mcsib_sw_workaround_R1()) {
		while(1) {
			if (mcsib_sw_workaround_R2())
				break;
			else {
				CntRead1_1 = CntRead1_2;
				CntRead2_1 = CntRead2_2;
				CntRead3_1 = CntRead3_2;
			}
		}
	}
	return 1;
}
#endif

void plat_cci_init(void)
{
	/*
	 * Initialize CCI driver
	 */
	mcsi_init(PLAT_MT_CCI_BASE, ARRAY_SIZE(cci_map));
}

void plat_cci_enable(void)
{
	/*
	 * Enable CCI coherency for this cluster.
	 */
	cci_enable_cluster_coherency(read_mpidr());
}

void plat_cci_disable(void)
{
	cci_disable_cluster_coherency(read_mpidr());
}

void plat_cci_init_sf(void)
{
	/*
	 * Init mcsi snoop filter.
	 */
	cci_init_sf();
}

unsigned long plat_get_ns_image_entrypoint(void)
{
	return PLAT_MTK_NS_IMAGE_OFFSET;
}

uint32_t plat_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL3-2 image.
	 */
	return 0;
}

uint32_t plat_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;
	unsigned int ee;
	unsigned long daif;

    INFO("LK is AArch32\n");
    mode = MODE32_svc;
    ee = 0;
    /*
     * TODO: Choose async. exception bits if HYP mode is not
     * implemented according to the values of SCR.{AW, FW} bits
     */
    daif = DAIF_ABT_BIT | DAIF_IRQ_BIT | DAIF_FIQ_BIT;

    spsr = SPSR_MODE32(mode, 0, ee, daif);

    /*
     * Pass boot argument to LK
     * ldr     w4, =pl_boot_argument
     * ldr     w5, =BOOT_ARGUMENT_SIZE
     */
//    bl33_ep_info->args.arg4=(unsigned long)(uintptr_t)BOOT_ARGUMENT_LOCATION;
//    bl33_ep_info->args.arg5=(unsigned long)(uintptr_t)BOOT_ARGUMENT_SIZE;

	return spsr;
}

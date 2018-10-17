/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <arm_gic.h>
#include <bl_common.h>
#include <cci400.h>
#include <debug.h>
#include <mmio.h>
#include <platform.h>
#include <plat_config.h>
#include <xlat_tables.h>
#include "../plat_def.h"

/*******************************************************************************
 * plat_config holds the characteristics of the differences between the three
 * FVP platforms (Base, A53_A57 & Foundation). It will be populated during cold
 * boot at each boot stage by the primary before enabling the MMU (to allow cci
 * configuration) & used thereafter. Each BL will have its own copy to allow
 * independent operation.
 ******************************************************************************/
plat_config_t plat_config;

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t plat_mmap[] = {
    /* for ATF text, RO, RW */
    {(TZRAM_BASE & PAGE_ADDR_MASK), (TZRAM_BASE & PAGE_ADDR_MASK),
    TZRAM_SIZE+TZRAM2_SIZE, MT_MEMORY | MT_RW | MT_SECURE},
//     ((TZRAM_SIZE & ~(PAGE_SIZE_MASK)) + PAGE_SIZE), MT_MEMORY | MT_RW | MT_SECURE},

    /* for T-OS, but we do not load T-OS in ATF. do not need it */
/*    { TZDRAM_BASE,	TZDRAM_SIZE,	MT_MEMORY | MT_RW | MT_SECURE },    */

    /* UART address mapping */
    { MTK_DEVICE_BASE, MTK_DEVICE_BASE, MTK_DEVICE_SIZE,	MT_DEVICE | MT_RW | MT_SECURE },

    /* GIC + CCI 400 address mapping */
    {(MT_DEV_BASE & PAGE_ADDR_MASK), (MT_DEV_BASE & PAGE_ADDR_MASK),MT_DEV_SIZE, MT_DEVICE | MT_RW | MT_SECURE},

    /* For TRNG and Clock Control address mapping */
    // including in MT_DEV_BASE and MT_DEV_SIZE, limit 4 XLAT_TABLES
//    {TRNG_BASE_ADDR, TRNG_BASE_SIZE, MT_DEVICE | MT_RW | MT_SECURE},
//    {TRNG_PDN_BASE_ADDR, TRNG_PDN_BASE_SIZE, MT_DEVICE | MT_RW | MT_SECURE},

	/* Top-level Reset Generator - WDT */
	/* including in MT_DEV_BASE and MT_DEV_SIZE */
//    { MTK_WDT_BASE, MTK_WDT_BASE, ((MTK_WDT_SIZE & ~(PAGE_SIZE_MASK)) + PAGE_SIZE), MT_DEVICE | MT_RW | MT_SECURE },

    /* 2nd GB as device for now...*/
    /* TZC-400 setting, we use Device-APC instead, do not use it yet */
/*    { DRAM1_BASE,	DRAM1_SIZE,	MT_MEMORY | MT_RW | MT_NS },    */
	{0}

};

/* Array of secure interrupts to be configured by the gic driver */
const unsigned int irq_sec_array[] = {
	IRQ_TZ_WDOG,
	IRQ_SEC_PHY_TIMER,
	IRQ_SEC_SGI_0,
	IRQ_SEC_SGI_1,
	IRQ_SEC_SGI_2,
	IRQ_SEC_SGI_3,
	IRQ_SEC_SGI_4,
	IRQ_SEC_SGI_5,
	IRQ_SEC_SGI_6,
	IRQ_SEC_SGI_7
};

const unsigned int num_sec_irqs = sizeof(irq_sec_array) /
	sizeof(irq_sec_array[0]);

/*******************************************************************************
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 ******************************************************************************/
#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void plat_configure_mmu_el##_el(unsigned long total_base,	\
				   unsigned long total_size,		\
				   unsigned long ro_start,		\
				   unsigned long ro_limit,		\
				   unsigned long coh_start,		\
				   unsigned long coh_limit)		\
	{								\
		mmap_add_region(total_base, total_base,			\
				total_size,				\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(ro_start, ro_start,			\
				ro_limit - ro_start,			\
				MT_MEMORY | MT_RO | MT_SECURE);		\
		mmap_add_region(coh_start, coh_start,			\
				coh_limit - coh_start,			\
				MT_DEVICE | MT_RW | MT_SECURE);		\
		mmap_add(plat_mmap);					\
		init_xlat_tables();					\
									\
		enable_mmu_el##_el(0);					\
	}

/* Define EL1 and EL3 variants of the function initialising the MMU */
DEFINE_CONFIGURE_MMU_EL(1)
DEFINE_CONFIGURE_MMU_EL(3)

/*******************************************************************************
 * A single boot loader stack is expected to work on both the Foundation FVP
 * models and the two flavours of the Base FVP models (AEMv8 & Cortex). The
 * SYS_ID register provides a mechanism for detecting the differences between
 * these platforms. This information is stored in a per-BL array to allow the
 * code to take the correct path.Per BL platform configuration.
 ******************************************************************************/
int plat_config_setup(void)
{
	unsigned int rev, hbi, bld, arch, sys_id;

	sys_id = mmio_read_32(VE_SYSREGS_BASE + V2M_SYS_ID);
	rev = (sys_id >> SYS_ID_REV_SHIFT) & SYS_ID_REV_MASK;
	hbi = (sys_id >> SYS_ID_HBI_SHIFT) & SYS_ID_HBI_MASK;
	bld = (sys_id >> SYS_ID_BLD_SHIFT) & SYS_ID_BLD_MASK;
	arch = (sys_id >> SYS_ID_ARCH_SHIFT) & SYS_ID_ARCH_MASK;

    arch = ARCH_MODEL;  //FIXME, bypass this stage
    bld = BLD_GIC_A53A57_MMAP;  //FIXME, bypass this stage
    hbi = HBI_MT_BASE;  //FIXME, bypass this stage

	if (arch != ARCH_MODEL) {
		ERROR("This firmware is for FVP models\n");
		panic();
	}

	/*
	 * The build field in the SYS_ID tells which variant of the GIC
	 * memory is implemented by the model.
	 */
	switch (bld) {
#if 0
	case BLD_GIC_VE_MMAP:
		plat_config.gicd_base = VE_GICD_BASE;
		plat_config.gicc_base = VE_GICC_BASE;
		plat_config.gich_base = VE_GICH_BASE;
		plat_config.gicv_base = VE_GICV_BASE;
		break;
#endif
	case BLD_GIC_A53A57_MMAP:
		plat_config.gicd_base = BASE_GICD_BASE;
		plat_config.gicc_base = BASE_GICC_BASE;
		plat_config.gich_base = BASE_GICH_BASE;
		plat_config.gicv_base = BASE_GICV_BASE;
		break;
	default:
		ERROR("Unsupported board build %x\n", bld);
		panic();
	}

	/*
	 * The hbi field in the SYS_ID is 0x020 for the Base FVP & 0x010
	 * for the Foundation FVP.
	 */
	switch (hbi) {
	case HBI_FOUNDATION:
		plat_config.max_aff0 = 4;
		plat_config.max_aff1 = 1;
		plat_config.flags = 0;

		/*
		 * Check for supported revisions of Foundation FVP
		 * Allow future revisions to run but emit warning diagnostic
		 */
		switch (rev) {
		case REV_FOUNDATION_V2_0:
		case REV_FOUNDATION_V2_1:
			break;
		default:
			WARN("Unrecognized Foundation FVP revision %x\n", rev);
			break;
		}
		break;
	case HBI_MT_BASE:
		plat_config.max_aff0 = 4;
		plat_config.max_aff1 = 2;
		// 4 cores do not need CCI
		// 8 cores need CCI only, CONFIG_HAS_CCI
		plat_config.flags |= CONFIG_BASE_MMAP;
		plat_config.flags |= CONFIG_HAS_CCI;

		/*
		 * Check for supported revisions
		 * Allow future revisions to run but emit warning diagnostic
		 */
		switch (rev) {
		case REV_MT:
			break;
		default:
			WARN("Unrecognized Base FVP revision %x\n", rev);
			break;
		}
		break;
	default:
		ERROR("Unsupported board HBI number 0x%x\n", hbi);
		panic();
	}

	return 0;
}

unsigned long plat_get_ns_image_entrypoint(void)
{
//	return NS_IMAGE_OFFSET;
    return BL33_START_ADDRESS;
}

uint64_t plat_get_syscnt_freq(void)
{
	uint64_t counter_base_frequency;

	/* Read the frequency from Frequency modes table */
	//counter_base_frequency = mmio_read_32(SYS_CNTCTL_BASE + CNTFID_OFF);
        counter_base_frequency = 13000000; //FIXME, 13 MHz

	/* The first entry of the frequency modes table must not be 0 */
	if (counter_base_frequency == 0)
		panic();

	return counter_base_frequency;
}

void plat_cci_init(void)
{
	/*
	 * Initialize CCI-400 driver
	 */
	if (plat_config.flags & CONFIG_HAS_CCI)
		cci_init(CCI400_BASE,
			CCI400_SL_IFACE3_CLUSTER_IX,
			CCI400_SL_IFACE4_CLUSTER_IX);
}

void plat_cci_enable(void)
{
	/*
	 * Enable CCI-400 coherency for this cluster. No need
	 * for locks as no other cpu is active at the
	 * moment
	 */
	if (plat_config.flags & CONFIG_HAS_CCI)
		cci_enable_cluster_coherency(read_mpidr());
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t plat_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
uint32_t plat_get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	if (el_status)
		mode = MODE_EL2;
	else
		mode = MODE_EL1;

    mode = MODE32_svc;
	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
//	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	spsr = SPSR_MODE32 (mode, SPSR_T_ARM, SPSR_E_LITTLE,
	            (DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT));

	return spsr;
}

/*******************************************************************************
 * Set SPSR and secure state for BL32 image
 ******************************************************************************/
void mt_set_bl32_ep_info(entry_point_info_t *bl32_ep_info)
{
	SET_SECURITY_STATE(bl32_ep_info->h.attr, SECURE);
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	bl32_ep_info->spsr = 0;
}

/*******************************************************************************
 * Set SPSR and secure state for BL33 image
 ******************************************************************************/
void mt_set_bl33_ep_info(entry_point_info_t *bl33_ep_info)
{
	unsigned long el_status;
	unsigned int mode;
    unsigned int rw, ee;
	unsigned long daif;


	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	if (el_status)
		mode = MODE_EL2;
	else
		mode = MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */


	/*
	 * Figure out whether the cpu enters the non-secure address space
	 * in aarch32 or aarch64
	 */

/*
    typedef enum {
        BOOT_OPT_64S3 = 0,
        BOOT_OPT_64S1,
        BOOT_OPT_32S3,
        BOOT_OPT_32S1,
        BOOT_OPT_64N2,
        BOOT_OPT_64N1,
        BOOT_OPT_32N2,
        BOOT_OPT_32N1,
        BOOT_OPT_UNKNOWN
    } boot_option_t;
  */

    //setting SPSR, RW 32 or 64
/*
    if ((BOOT_OPT_32N2 == pl_boot_argument.lk_boot_opt) ||
        (BOOT_OPT_32N1 == pl_boot_argument.lk_boot_opt) ||
        (BOOT_OPT_32S1 == pl_boot_argument.lk_boot_opt) ) {
*/
    if (1){
    	rw = 0;
    }else{
        rw = 1;
    }
	if (0 == rw) {
	    printf("LK is AArch32\n");
	    printf("LK start_addr=x0x%x\n", bl33_ep_info->pc);
    	mode = MODE32_svc;
		ee = 0;
		/*
		 * TODO: Choose async. exception bits if HYP mode is not
		 * implemented according to the values of SCR.{AW, FW} bits
		 */
		daif = DAIF_ABT_BIT | DAIF_IRQ_BIT | DAIF_FIQ_BIT;

		bl33_ep_info->spsr = SPSR_MODE32(mode, 0, ee, daif);

		/*
		 * Pass boot argument to LK
		 * ldr     w4, =pl_boot_argument
		 * ldr     w5, =BOOT_ARGUMENT_SIZE
		 */
		bl33_ep_info->args.arg4=(unsigned long)(uintptr_t)BOOT_ARGUMENT_LOCATION;
		bl33_ep_info->args.arg5=(unsigned long)(uintptr_t)BOOT_ARGUMENT_SIZE;
	} else {
        printf("LK is AArch64\n");
		bl33_ep_info->spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
    }
    SET_SECURITY_STATE(bl33_ep_info->h.attr, NON_SECURE);
}

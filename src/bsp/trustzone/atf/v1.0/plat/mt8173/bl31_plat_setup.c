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
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <console.h>
#include <mmio.h>
#include <platform.h>
#include <stddef.h>
#include "drivers/pwrc/plat_pwrc.h"
#include "plat_def.h"
#include "plat_private.h"
#include "aarch64/plat_helpers.h"
#include <stdio.h>  //for printf
#include <string.h> //for memcpy
#include <xlat_tables.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
extern unsigned long __RO_START__;
extern unsigned long __RO_END__;

extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)


#if RESET_TO_BL31
static entry_point_info_t  bl32_image_ep_info;
static entry_point_info_t  bl33_image_ep_info;
#else
/*******************************************************************************
 * Reference to structure which holds the arguments that have been passed to
 * BL31 from BL2.
 ******************************************************************************/
static bl31_params_t *bl2_to_bl31_params;
#endif

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{

#if RESET_TO_BL31
        if (type == NON_SECURE)
                return &bl33_image_ep_info;
        else
                return &bl32_image_ep_info;
#else
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ?
		bl2_to_bl31_params->bl33_ep_info :
		bl2_to_bl31_params->bl32_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
#endif

}

entry_point_info_t *bl31_plat_get_next_kernel64_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;
    unsigned long el_status;
    unsigned int mode;

    el_status = 0;
    mode = 0;

        assert(sec_state_is_valid(type));

    next_image_info = (type == NON_SECURE) ?
                &bl33_image_ep_info :
                &bl32_image_ep_info;

    /* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	if (el_status) {
		printf("Kernel_EL2\n");
		mode = MODE_EL2;
	}
	else {
		printf("Kernel_EL1\n");
		mode = MODE_EL1;
	}

        printf("Kernel is 64Bit\n");
	next_image_info->spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
        next_image_info->pc = get_kernel_info_pc();
        next_image_info->args.arg0=get_kernel_info_r0();
        next_image_info->args.arg1=get_kernel_info_r1();

        printf("pc=0x%llx, r0=0x%llx, r1=0x%llx\n",
               next_image_info->pc,
               next_image_info->args.arg0,
               next_image_info->args.arg1);

	SET_SECURITY_STATE(next_image_info->h.attr, NON_SECURE);


	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

entry_point_info_t *bl31_plat_get_next_kernel32_ep_info(uint32_t type)
{
    /* Compatible to already MP platform/project                  *
     * only 64 bits will re-enter ATF in previsous ARMv8 platform *
     * ignore k32_64 flag                                         *
     * call get_next_kernel32_ep_info, actually for 64 kernel     */
     return bl31_plat_get_next_kernel64_ep_info(type);
}



/*******************************************************************************
 * Perform any BL31 specific platform actions. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables. On the MTK_platform
 * we know that BL2 has populated the parameters in secure DRAM. So we just use
 * the reference passed in 'from_bl2' instead of copying. The 'data' parameter
 * is not used since all the information is contained in 'from_bl2'. Also, BL2
 * has flushed this information to memory, so we are guaranteed to pick up good
 * data
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t *from_bl2,
				void *plat_params_from_bl2)
{
    atf_arg_t_ptr teearg = (atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR;

	/* Initialize the console to provide early debug support */
	console_init(teearg->atf_log_port);
    printf("LK boot argument location=0x%x\n\r", BOOT_ARGUMENT_LOCATION);
    printf("KL boot argument size=0x%x\n\r", BOOT_ARGUMENT_SIZE);
    printf("teearg->atf_magic=0x%x\n\r", teearg->atf_magic);
    printf("teearg->tee_support=0x%x\n\r", teearg->tee_support);
    printf("teearg->tee_entry=0x%x\n\r", teearg->tee_entry);
    printf("teearg->tee_boot_arg_addr=0x%x\n\r", teearg->tee_boot_arg_addr);
    printf("teearg->atf_log_port=0x%x\n\r", teearg->atf_log_port);
    printf("teearg->atf_log_baudrate=0x%x\n\r", teearg->atf_log_baudrate);
    printf("teearg->atf_log_buf_start=0x%x\n\r", teearg->atf_log_buf_start);
    printf("teearg->atf_log_buf_size=0x%x\n\r", teearg->atf_log_buf_size);
    printf("teearg->atf_irq_num=%d\n\r", teearg->atf_irq_num);
    printf("BL33_START_ADDRESS=0x%x\n\r", BL33_START_ADDRESS);
    
    printf("atf chip_code[%x]\n", mt_get_chip_hw_code());
    printf("atf chip_ver[%x]\n", mt_get_chip_sw_ver());
    
	/* Initialize the platform config for future decision making */
	plat_config_setup();

    printf("bl31_setup\n\r");
#if RESET_TO_BL31
	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);
    printf("RESET_TO_BL31\n\r");


	/*
	 * Do initial security configuration to allow DRAM/device access. On
	 * Base MTK_platform only DRAM security is programmable (via TrustZone), but
	 * other platforms might have more programmable security devices
	 * present.
	 */
	//FIXME, do not set TZC400 now
	//plat_security_setup();

        /* Populate entry point information for BL3-2 and BL3-3 */
        SET_PARAM_HEAD(&bl32_image_ep_info,
                                PARAM_EP,
                                VERSION_1,
                                0);
        SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
        bl32_image_ep_info.pc = teearg->tee_entry;
        bl32_image_ep_info.spsr = plat_get_spsr_for_bl32_entry();

        SET_PARAM_HEAD(&bl33_image_ep_info,
                                PARAM_EP,
                                VERSION_1,
                                0);
        /*                                                                           * Tell BL31 where the non-trusted software image                            * is located and the entry state information
         */
        bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
        bl33_image_ep_info.spsr = plat_get_spsr_for_bl33_entry();

    /*
     * Pass boot argument to LK
     * ldr     w4, =pl_boot_argument
     * ldr     w5, =BOOT_ARGUMENT_SIZE
     */
    bl33_image_ep_info.args.arg4=(unsigned long)(uintptr_t)BOOT_ARGUMENT_LOCATION;
    bl33_image_ep_info.args.arg5=(unsigned long)(uintptr_t)BOOT_ARGUMENT_SIZE;
        SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#else
	/* Check params passed from BL2 should not be NULL,
	 * We are not checking plat_params_from_bl2 as NULL as we are not
	 * using it on MTK_platform
	 */
    printf("not RESET_TO_BL31\n");

	assert(from_bl2 != NULL);
	assert(from_bl2->h.type == PARAM_BL31);
	assert(from_bl2->h.version >= VERSION_1);

	bl2_to_bl31_params = from_bl2;
	assert(((unsigned long)plat_params_from_bl2) == MT_BL31_PLAT_PARAM_VAL);
#endif
}

/*******************************************************************************
 * Initialize the gic, configure the CLCD and zero out variables needed by the
 * secondaries to boot up correctly.
 ******************************************************************************/
void bl31_platform_setup()
{
//	unsigned int reg_val;

	/* Initialize the gic cpu and distributor interfaces */
	gic_setup();

#if 0   //do not init CLCD in ATF
	/*
	 * TODO: Configure the CLCD before handing control to
	 * linux. Need to see if a separate driver is needed
	 * instead.
	 */
	mmio_write_32(VE_SYSREGS_BASE + V2M_SYS_CFGDATA, 0);
	mmio_write_32(VE_SYSREGS_BASE + V2M_SYS_CFGCTRL,
		      (1ull << 31) | (1 << 30) | (7 << 20) | (0 << 16));
#endif

#if 0   //FIXME TIMER CTRL skip now
	/* Enable and initialize the System level generic timer */
	mmio_write_32(SYS_CNTCTL_BASE + CNTCR_OFF, CNTCR_FCREQ(0) | CNTCR_EN);

	/* Allow access to the System counter timer module */
	reg_val = (1 << CNTACR_RPCT_SHIFT) | (1 << CNTACR_RVCT_SHIFT);
	reg_val |= (1 << CNTACR_RFRQ_SHIFT) | (1 << CNTACR_RVOFF_SHIFT);
	reg_val |= (1 << CNTACR_RWVT_SHIFT) | (1 << CNTACR_RWPT_SHIFT);
	mmio_write_32(SYS_TIMCTL_BASE + CNTACR_BASE(0), reg_val);
	mmio_write_32(SYS_TIMCTL_BASE + CNTACR_BASE(1), reg_val);

	reg_val = (1 << CNTNSAR_NS_SHIFT(0)) | (1 << CNTNSAR_NS_SHIFT(1));
	mmio_write_32(SYS_TIMCTL_BASE + CNTNSAR, reg_val);
#endif


	/* Intialize the power controller */
	mt_pwrc_setup();

	/* Topologies are best known to the platform. */
	plat_setup_topology();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup()
{
    unsigned long mpidr = read_mpidr();

    /*
     * clear CNTVOFF for core 0
     */
    clear_cntvoff(mpidr);


        plat_cci_init();
#if RESET_TO_BL31
	plat_cci_enable();
#endif
    /* Enable non-secure access to CCI-400 registers */
    mmio_write_32(CCI400_BASE + CCI_SEC_ACCESS_OFFSET , 0x1);
    /* return the sram to ca53 l2 cache */
    mmio_write_32(MT_DEV_BASE, 0x300);
    mmio_write_32(INFRACFG_AO_BASE + 0x40, 1 << 7);

    {
        atf_arg_t_ptr teearg = (atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR;
        if(teearg->atf_log_buf_size !=0 ) {
            printf("mmap atf buffer : 0x%x, 0x%x\n\r", teearg->atf_log_buf_start,
                teearg->atf_log_buf_size);
            mmap_add_region((teearg->atf_log_buf_start & ~(PAGE_SIZE_2MB_MASK)),
                            (teearg->atf_log_buf_start & ~(PAGE_SIZE_2MB_MASK)),
                            PAGE_SIZE_2MB,
                            MT_MEMORY | MT_RW | MT_NS);
                            
            // add RAM_CONSOLE_BASE to memory map
            mmap_add_region(RAM_CONSOLE_BASE & ~(PAGE_SIZE_MASK),
                            RAM_CONSOLE_BASE & ~(PAGE_SIZE_MASK),
                            PAGE_SIZE,
                            MT_DEVICE | MT_RW | MT_NS);  
        }
    }

	plat_configure_mmu_el3(BL31_RO_BASE,
			      (BL31_COHERENT_RAM_LIMIT - BL31_RO_BASE),
			      BL31_RO_BASE,
			      BL31_RO_LIMIT,
			      BL31_COHERENT_RAM_BASE,
			      BL31_COHERENT_RAM_LIMIT);

	/*
	 * Without this, access to CPUECTRL from NS EL1
	 * will cause trap into EL3
	 */
	enable_ns_access_to_cpuectlr();
}

void enable_ns_access_to_cpuectlr(void) {
	unsigned int next_actlr;

	/* ACTLR_EL1 do not implement CUPECTLR  */
	next_actlr = read_actlr_el2();
	next_actlr |= ACTLR_CPUECTLR_BIT;
	write_actlr_el2(next_actlr);

	next_actlr = read_actlr_el3();
	next_actlr |= ACTLR_CPUECTLR_BIT;
	write_actlr_el3(next_actlr);
/*
	next_actlr = read_actlr_aarch32();
	next_actlr |= ACTLR_CPUECTLR_BIT;
	write_actlr_aarch32(next_actlr);
*/
}

unsigned int mt_get_chip_hw_code(void)
{
       return mmio_read_32(APHW_CODE);
}

CHIP_SW_VER mt_get_chip_sw_ver(void)
{
       CHIP_SW_VER sw_ver;
       unsigned int ver;
       //    unsigned int hw_subcode = DRV_Reg32(APHW_SUBCODE);

       ver = mmio_read_32(APSW_VER);
       if ( 0x0 == ver )
               sw_ver = CHIP_SW_VER_01;
       else
               sw_ver = CHIP_SW_VER_02;

       return sw_ver;
}


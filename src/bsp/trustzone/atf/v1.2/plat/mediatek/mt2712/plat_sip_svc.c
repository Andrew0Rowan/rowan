/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <console.h>
#include <debug.h>
#include <log.h>
#include <mcucfg.h>
#include <mmio.h>
#include <mt_cpuxgpt.h>
#include <mtk_sip_svc.h>
#include <mtk_udi_api.h>
#include <plat_pm.h>
#include <plat_private.h>   //for atf_arg_t_ptr
#include <platform.h>
#include <platform_def.h>
#include <runtime_svc.h>
#include <xlat_tables.h>

extern atf_arg_t gteearg;
/*******************************************************************************
 * SIP top level handler for servicing SMCs.
 ******************************************************************************/
extern void set_kernel_k32_64(uint64_t k32_64);
extern void bl31_prepare_kernel_entry(uint64_t k32_64);
extern void el3_exit(void);
extern uint64_t wdt_kernel_cb_addr;

/*******************************************************************************

 * SMC Call for Kernel MCUSYS register write
 ******************************************************************************/

static uint64_t mcusys_write_count = 0;
static uint64_t sip_mcusys_write(unsigned int reg_addr, unsigned int reg_value)
{
	if((reg_addr & 0xFFFF0000) != (MCUCFG_BASE & 0xFFFF0000))
	{
		INFO("[ATF] sip_mcusys_write: %x Invalid Range.\n", reg_addr);
		return SIP_SVC_E_INVALID_Range;
	}
	/* Perform range check */
	/* if(( MP0_MISC_CONFIG0 <= reg_addr && reg_addr <= MP0_MISC_CONFIG9 ) ||
		 ( MP1_MISC_CONFIG0 <= reg_addr && reg_addr <= MP1_MISC_CONFIG9 )) {
			return SIP_SVC_E_PERMISSION_DENY;
	}

	if (check_cpuxgpt_write_permission(reg_addr, reg_value) == 0) { */
	/* Not allow to clean enable bit[0], Force to set bit[0] as 1 */
	/* reg_value |= 0x1;
	} */

	mmio_write_32(reg_addr, reg_value);
	dsb();

	mcusys_write_count++;

	return SIP_SVC_E_SUCCESS;
}

/*******************************************************************************
 * SIP top level handler for servicing SMCs.
 ******************************************************************************/
uint64_t mediatek_sip_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	uint64_t rc = 0;
	uint32_t ns;
	atf_arg_t_ptr teearg = &gteearg;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns) {
		/* SiP SMC service secure world's call */
		;
	} else {
		switch (smc_fid) {
		case MTK_SIP_KERNEL_MCUSYS_WRITE_AARCH32:
		case MTK_SIP_KERNEL_MCUSYS_WRITE_AARCH64:
			rc = sip_mcusys_write(x1, x2);
			break;
		case MTK_SIP_KERNEL_MCUSYS_ACCESS_COUNT_AARCH32:
		case MTK_SIP_KERNEL_MCUSYS_ACCESS_COUNT_AARCH64:
			rc = mcusys_write_count;
			break;
		case MTK_SIP_KERNEL_BOOT_AARCH32:
			wdt_kernel_cb_addr = 0;
			console_init(gteearg.atf_log_port, MT2712_UART_CLOCK, MT2712_BAUDRATE);
			NOTICE("save kernel info\n");
			save_kernel_info(x1, x2, x3, x4);
			bl31_prepare_kernel_entry(x4);
			NOTICE("el3_exit\n");
			console_uninit();
			SMC_RET0(handle);
			break;
		case MTK_SIP_LK_WDT_AARCH32:
		case MTK_SIP_LK_WDT_AARCH64:
			set_kernel_k32_64(LINUX_KERNEL_32);
			wdt_kernel_cb_addr = x1;
			NOTICE("MTK_SIP_LK_WDT : 0x%lx \n", wdt_kernel_cb_addr);
			rc = teearg->atf_aee_debug_buf_start;
			break;
#ifdef MTK_ATF_RAM_DUMP
		case MTK_SIP_RAM_DUMP_ADDR_AARCH32:
			atf_ram_dump_base = x1<<32| (x2&0xffffffff);
			atf_ram_dump_size = x3<<32 | (x4&0xffffffff);
			break;
		case MTK_SIP_RAM_DUMP_ADDR_AARCH64:
			atf_ram_dump_base = x1;
			atf_ram_dump_size = x2;
			break;
#endif
		case MTK_SIP_KERNEL_WDT_AARCH32:
		case MTK_SIP_KERNEL_WDT_AARCH64:
			wdt_kernel_cb_addr = x1;
			NOTICE("MTK_SIP_KERNEL_WDT : 0x%lx \n", wdt_kernel_cb_addr);
			NOTICE("teearg->atf_aee_debug_buf_start : 0x%x \n",
				   teearg->atf_aee_debug_buf_start);
			rc = teearg->atf_aee_debug_buf_start;
			break;
		case MTK_SIP_KERNEL_MSG_AARCH32:
		case MTK_SIP_KERNEL_MSG_AARCH64:
			if (x1 == 0 && x2 == 2) {
				mtk_suspend_footprint_addr = x3;
				mmap_add_region(mtk_suspend_footprint_addr & ~(PAGE_SIZE_MASK),
						mtk_suspend_footprint_addr & ~(PAGE_SIZE_MASK),
						PAGE_SIZE, MT_DEVICE | MT_RW | MT_NS);
				init_xlat_tables();
			}
			rc = SIP_SVC_E_SUCCESS;
			break;
		/************* UDI begin *************/
		case MTK_SIP_KERNEL_UDI_JTAG_CLOCK_AARCH32:
		case MTK_SIP_KERNEL_UDI_JTAG_CLOCK_AARCH64:
			rc = UDIRead(x1, x2);
			break;
		case MTK_SIP_KERNEL_UDI_BIT_CTRL_AARCH32:
		case MTK_SIP_KERNEL_UDI_BIT_CTRL_AARCH64:
			rc = UDIBitCtrl(x1);
			break;
		case MTK_SIP_KERNEL_UDI_JTAG_CLOCK_BIG_AARCH32:
		case MTK_SIP_KERNEL_UDI_JTAG_CLOCK_BIG_AARCH64:
			rc = UDIRead_Big(x1, x2);
			break;
		case MTK_SIP_KERNEL_UDI_BIT_CTRL_BIG_AARCH32:
		case MTK_SIP_KERNEL_UDI_BIT_CTRL_BIG_AARCH64:
			rc = UDIBitCtrl_Big(x1);
			break;
#if UDI_DVT
		case MTK_SIP_KERNEL_OCP_WRITE_AARCH32:
		case MTK_SIP_KERNEL_OCP_WRITE_AARCH64:
			UDIRegWrite(x1, x2);
			break;
		case MTK_SIP_KERNEL_OCP_READ_AARCH32:
		case MTK_SIP_KERNEL_OCP_READ_AARCH64:
			rc = UDIRegRead(x1);
			break;
#endif
		/************* UDI end *************/
		case MTK_SIP_IDLE_DEBUG_READ:
			mt_sip_idle_debug_read((uint32_t)x1);
			rc = SIP_SVC_E_SUCCESS;
			break;
		case MTK_SIP_IDLE_DEBUG_WRITE:
			mt_sip_idle_debug_write((uint32_t)x1, (uint32_t)x2, (uint32_t)x3);
			rc = SIP_SVC_E_SUCCESS;
			break;
		default:
			rc = SMC_UNK;
			console_init(gteearg.atf_log_port, MT2712_UART_CLOCK, MT2712_BAUDRATE);
			ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
			console_uninit();
		}
	}
	SMC_RET1(handle, rc);
}


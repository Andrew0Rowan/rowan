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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "typedefs.h"
#include "tz_init.h"

typedef void (*jump_atf)(u64 addr ,u64 arg1) __attribute__ ((__noreturn__));

static u64 trustzone_get_atf_boot_param_addr(void)
{
    return ATF_BOOT_ARG_ADDR;
}

static u64 trustzone_get_tee_boot_param_addr(void)
{
    return TEE_BOOT_ARG_ADDR;
}

static void set_atf_parameters(mtk_bl_param_t *atf_arg)
{
    atf_arg->bootarg_loc = 0;
    atf_arg->bootarg_size = 0;
    atf_arg->bl33_start_addr = BL33;
    atf_arg->tee_info_addr = TEE_BOOT_ARG_ADDR;
}

static void set_tee_parameters(atf_arg_t *tee_arg)
{
    /* tee arguments */
    tee_arg->atf_magic = 0x4D415446;
    tee_arg->tee_support = 0x1;
    tee_arg->tee_entry = TRUSTEDOS_ENTRYPOINT;
    tee_arg->tee_boot_arg_addr = 0x43000100;
    tee_arg->hwuid[0] = 0x55C09893;
    tee_arg->hwuid[1] = 0x2B404DDF;
    tee_arg->hwuid[2] = 0x3ACE08B;
    tee_arg->hwuid[3] = 0x1092600D;
    tee_arg->HRID[0] = 0;
    tee_arg->HRID[1] = 0;
    tee_arg->atf_log_port = 0x11005000;
    tee_arg->atf_log_baudrate = 0xE1000;
    tee_arg->atf_log_buf_start = 0x0;
    tee_arg->atf_log_buf_size = 0x0;
    tee_arg->atf_irq_num = 0x119; /* reserve SPI ID 249 for ATF log, which is ID 281 */
    tee_arg->devinfo[0] = 0;
    tee_arg->devinfo[1] = 0;
    tee_arg->devinfo[2] = 0xFFFFFFFF;
    tee_arg->devinfo[3] = 0xFFFFFFFF;
    tee_arg->atf_aee_debug_buf_start = 0x0;
    tee_arg->atf_aee_debug_buf_size = 0x0;
}

int teeloader_main(void)
{
    u32 bl31_reserve = 0;
    jump_atf atf_entry;

    mtk_bl_param_t *atf_arg = (mtk_bl_param_t *)trustzone_get_atf_boot_param_addr();
    atf_arg_t *tee_arg = (atf_arg_t *)trustzone_get_tee_boot_param_addr();

    set_atf_parameters(atf_arg);
    set_tee_parameters(tee_arg);

    atf_entry = (jump_atf)BL31;
    /* jump to tz */
    tl_printf("[teeloader] tl jump to atf!\n");

    (*atf_entry)(ATF_BOOT_ARG_ADDR, bl31_reserve);

	return 0;
}

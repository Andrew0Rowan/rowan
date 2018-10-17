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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include "print.h"
#include "typedefs.h"
#include "tz_init.h"
#include "tz_emi_reg.h"
#include "tz_emi_mpu.h"

#define MOD "[TZ_EMI_MPU]"

#define READ_REGISTER_UINT32(reg) \
    (*(volatile UINT32 * const)(reg))

#define WRITE_REGISTER_UINT32(reg, val) \
    (*(volatile UINT32 * const)(reg)) = (val)


#define readl(addr) (READ_REGISTER_UINT32(addr))
#define writel(b,addr) (WRITE_REGISTER_UINT32(addr, b))
#define IOMEM(reg) (reg)

/*
 * emi_mpu_set_region_protection: protect a region.
 * @start: start address of the region
 * @end: end address of the region
 * @region: EMI MPU region id
 * @access_permission: EMI MPU access permission
 * Return 0 for success, otherwise negative status code.
 */
int emi_mpu_set_region_protection(unsigned int start, unsigned int end, int region, unsigned int access_permission)
{
    int ret = 0;
    unsigned int tmp;

    if((end != 0) || (start !=0))
    {
        /*Address 64KB alignment*/
        start -= EMI_PHY_OFFSET;
        end -= EMI_PHY_OFFSET;
        start = start >> 16;
        end = end >> 16;

        if (end < start)
        {
            return -1;
        }
    }

    switch (region) {
    case 0:
        tmp = readl(IOMEM(EMI_MPUI)) & 0xFFFF0000;
        writel((start << 16) | end, EMI_MPUA);
        writel(tmp | access_permission, EMI_MPUI);
        break;

    case 1:
        tmp = readl(IOMEM(EMI_MPUI)) & 0x0000FFFF;
        writel((start << 16) | end, EMI_MPUB);
        writel(tmp | (access_permission << 16), EMI_MPUI);
        break;

    case 2:
        tmp = readl(IOMEM(EMI_MPUJ)) & 0xFFFF0000;
        writel((start << 16) | end, EMI_MPUC);
        writel(tmp | access_permission, EMI_MPUJ);
        break;

    case 3:
        tmp = readl(IOMEM(EMI_MPUJ)) & 0x0000FFFF;
        writel((start << 16) | end, EMI_MPUD);
        writel(tmp | (access_permission << 16), EMI_MPUJ);
        break;

    case 4:
        tmp = readl(IOMEM(EMI_MPUK)) & 0xFFFF0000;
        writel((start << 16) | end, EMI_MPUE);
        writel(tmp | access_permission, EMI_MPUK);
        break;

    case 5:
        tmp = readl(IOMEM(EMI_MPUK)) & 0x0000FFFF;
        writel((start << 16) | end, EMI_MPUF);
        writel(tmp | (access_permission << 16), EMI_MPUK);
        break;

    case 6:
        tmp = readl(IOMEM(EMI_MPUL)) & 0xFFFF0000;
        writel((start << 16) | end, EMI_MPUG);
        writel(tmp | access_permission, EMI_MPUL);
        break;

    case 7:
        tmp = readl(IOMEM(EMI_MPUL)) & 0x0000FFFF;
        writel((start << 16) | end, EMI_MPUH);
        writel(tmp | (access_permission << 16), EMI_MPUL);
        break;

    default:
        ret = -1;
        break;
    }

    return ret;
}

void tz_emi_mpu_init(u32 start_add, u32 end_addr, u32 mpu_region)
{
    int ret = 0;
    unsigned int sec_mem_mpu_attr;
    unsigned int sec_mem_phy_start, sec_mem_phy_end;
    unsigned int temp;

    /* Caculate start/end address */
    sec_mem_phy_start = start_add;
    sec_mem_phy_end = end_addr;

    switch (mpu_region) {
    case SECURE_OS_MPU_REGION_ID:
    #ifdef DDR_RESERVE_MODE
        tl_printf(" MPU [UNLOCK\n");
        sec_mem_mpu_attr = SET_ACCESS_PERMISSON(UNLOCK, FORBIDDEN, FORBIDDEN, FORBIDDEN, SEC_RW);
    #else
        tl_printf(" MPU [LOCK\n");
        sec_mem_mpu_attr = SET_ACCESS_PERMISSON(LOCK, FORBIDDEN, FORBIDDEN, FORBIDDEN, SEC_RW);
    #endif
        break;
    case ATF_MPU_REGION_ID:
    #ifdef DDR_RESERVE_MODE
        tl_printf(" MPU [UNLOCK\n");
        sec_mem_mpu_attr = SET_ACCESS_PERMISSON(UNLOCK, FORBIDDEN, FORBIDDEN, FORBIDDEN, SEC_RW);
    #else
        tl_printf(" MPU [LOCK\n");
        sec_mem_mpu_attr = SET_ACCESS_PERMISSON(LOCK, FORBIDDEN, FORBIDDEN, FORBIDDEN, SEC_RW);
    #endif
        break;
    default:
        tl_printf("%s Warning - MPU region '%d' is not supported in pre-loader!\n", MOD, mpu_region);
        return;
    }

    tl_printf("%s MPU [0x%x-0x%x]\n", MOD, sec_mem_phy_start, sec_mem_phy_end);

    ret = emi_mpu_set_region_protection(sec_mem_phy_start,      /*START_ADDR*/
                                        sec_mem_phy_end,      /*END_ADDR*/
                                        mpu_region,       /*region*/
                                        sec_mem_mpu_attr);


    if(ret)
    {
        tl_printf("%s MPU error!!\n", MOD);
    }
}

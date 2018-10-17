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
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
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

#ifndef __BTMW_TEST_DEBUG_H__
#define __BTMW_TEST_DEBUG_H__

#define BTMW_TEST_LOG_LV_VBS  (1 << 0)
#define BTMW_TEST_LOG_LV_INF  (1 << 1)
#define BTMW_TEST_LOG_LV_DBG  (1 << 2)
#define BTMW_TEST_LOG_LV_WRN  (1 << 3)
#define BTMW_TEST_LOG_LV_ERR  (1 << 4)
#define BTMW_TEST_LOG_MASK    (BTMW_TEST_LOG_LV_VBS|BTMW_TEST_LOG_LV_INF|BTMW_TEST_LOG_LV_DBG|BTMW_TEST_LOG_LV_WRN|BTMW_TEST_LOG_LV_ERR)

#define BTMW_TEST_LOG_FLAG_COLOR             (1 << 8)
#define BTMW_TEST_LOG_FLAG_TIMESTAMP         (1 << 9)

#define BTMW_TEST_Logv(fmt, ...) BTMW_TEST_Log(BTMW_TEST_LOG_LV_VBS, "<V> " fmt , ## __VA_ARGS__)
#define BTMW_TEST_Logi(fmt, ...) BTMW_TEST_Log(BTMW_TEST_LOG_LV_INF, "<I> " fmt , ## __VA_ARGS__)
#define BTMW_TEST_Logd(fmt, ...) BTMW_TEST_Log(BTMW_TEST_LOG_LV_DBG, "<D> " fmt , ## __VA_ARGS__)
#define BTMW_TEST_Logw(fmt, ...) BTMW_TEST_Log(BTMW_TEST_LOG_LV_WRN, "<W> %s(). " fmt , __func__, ## __VA_ARGS__)
#define BTMW_TEST_Loge(fmt, ...) BTMW_TEST_Log(BTMW_TEST_LOG_LV_ERR, "<E> %s()[%d] : " fmt , __func__, __LINE__, ## __VA_ARGS__)

void BTMW_TEST_Log(unsigned char lv, const char *fmt, ...);
void BTMW_TEST_Log_SetFlag(unsigned short flag);

#endif  //__BTMW_TEST_DEBUG_H__

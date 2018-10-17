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


#ifndef __EMI_H__
#define __EMI_H__

#define IO_PHYS            	    0x10000000
#define EMI_BASE                (IO_PHYS + 0x00203000)

/* EMI Memory Protect Unit */
#define EMI_MPUA            ((P_U32)(EMI_BASE+0x0160))
#define EMI_MPUB            ((P_U32)(EMI_BASE+0x0168))
#define EMI_MPUC            ((P_U32)(EMI_BASE+0x0170))
#define EMI_MPUD            ((P_U32)(EMI_BASE+0x0178))
#define EMI_MPUE            ((P_U32)(EMI_BASE+0x0180))
#define EMI_MPUF	        ((P_U32)(EMI_BASE+0x0188))
#define EMI_MPUG	        ((P_U32)(EMI_BASE+0x0190))
#define EMI_MPUH	        ((P_U32)(EMI_BASE+0x0198))

#define EMI_MPUI	        ((P_U32)(EMI_BASE+0x01A0))
#define EMI_MPUJ            ((P_U32)(EMI_BASE+0x01A8))
#define EMI_MPUK            ((P_U32)(EMI_BASE+0x01B0))
#define EMI_MPUL            ((P_U32)(EMI_BASE+0x01B8))
#define EMI_MPUM            ((P_U32)(EMI_BASE+0x01C0))
#define EMI_MPUN            ((P_U32)(EMI_BASE+0x01C8))
#define EMI_MPUO            ((P_U32)(EMI_BASE+0x01D0))
#define EMI_MPUP            ((P_U32)(EMI_BASE+0x01D8))
#define EMI_MPUQ            ((P_U32)(EMI_BASE+0x01E0))
#define EMI_MPUR            ((P_U32)(EMI_BASE+0x01E8))
#define EMI_MPUS            ((P_U32)(EMI_BASE+0x01F0))
#define EMI_MPUT            ((P_U32)(EMI_BASE+0x01F8))

#endif // __EMI_H__

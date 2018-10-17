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


/*----------------------------------------------------------------------------*
 * $RCSfile$
 * $Revision: #39 $
 * $Date: 2016/08/24 $
 * $Author: zongren.yin $
 *
 * Description:
 *         This header file contains internal definitions common to the whole
 *         Middleware.
 *---------------------------------------------------------------------------*/

#ifndef _COMMON_H_
#define _COMMON_H_


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/

//#include "x_common.h"
#include "u_priority.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* Define a PCR stream type. The values 24 to 31 (inclusive) */
/* are reserved for internal usage.                          */
#define ST_PCR  ((STREAM_TYPE_T) 31)

#define ST_MASK_PCR  MAKE_BIT_MASK_32 (ST_PCR)


/* Define thread and message default settings after this comment. */

/* MIDXBULD notification and async execute thread and message queue definitions. */
#define MIDXBULD_NFY_ASYNC_EXEC_THREAD_NAME                "MIDX nfy asyc"
//#define MIDXBULD_NFY_ASYNC_EXEC_THREAD_DEFAULT_PRIORITY    ((UINT8)   254)
#define MIDXBULD_NFY_ASYNC_EXEC_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_IDLE, PRIORITY_LAYER_MIDDLEWARE, 0))
#define MIDXBULD_NFY_ASYNC_EXEC_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define MIDXBULD_NFY_ASYNC_EXEC_MSG_Q_NAME            "MIDX nfy asyc"
#define MIDXBULD_NFY_ASYNC_EXEC_NUM_OF_MSGS           ((UINT16)  20)
#define MIDXBULD_NFY_ASYNC_EXEC_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* MINFO notification and async execute thread and message queue definitions. */
#define MINFO_NFY_ASYNC_EXEC_THREAD_NAME                "MINFO nfy asyc"
//#define MINFO_NFY_ASYNC_EXEC_THREAD_DEFAULT_PRIORITY    ((UINT8)   200)
#define MINFO_NFY_ASYNC_EXEC_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_MIDDLEWARE, 0))
#define MINFO_NFY_ASYNC_EXEC_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define MINFO_NFY_ASYNC_EXEC_MSG_Q_NAME            "MINFO nfy asyc"
#define MINFO_NFY_ASYNC_EXEC_NUM_OF_MSGS           ((UINT16)  20)
#define MINFO_NFY_ASYNC_EXEC_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* MSVCTX async execute thread and message queue definitions. */
#define MSVCTX_ASYNC_THREAD_NAME                "MSVCTX_async"
//#define MSVCTX_ASYNC_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define MSVCTX_ASYNC_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define MSVCTX_ASYNC_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define MSVCTX_ASYNC_MSG_Q_NAME            "MSVCTX_async"
#define MSVCTX_ASYNC_SUB_MSG_Q_NAME        "MSVCTX_async_sub"
#define MSVCTX_ASYNC_NUM_OF_MSGS           ((UINT16)  20)
#define MSVCTX_ASYNC_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* MSVCTX notification thread and message queue definitions. */
#define MSVCTX_NOTIFY_THREAD_NAME                "MSVCTX_notify"
//#define MSVCTX_NOTIFY_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define MSVCTX_NOTIFY_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define MSVCTX_NOTIFY_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 1024)

#define MSVCTX_NOTIFY_MSG_Q_NAME            "MSVCTX_notify"
#define MSVCTX_NOTIFY_NUM_OF_MSGS           ((UINT16)  20)
#define MSVCTX_NOTIFY_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* IMAGE decode thread and decode message queue definitions. */
#define IMG_DECODE_THREAD_NAME                "IMG_decode"
//#define IMG_DECODE_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define IMG_DECODE_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define IMG_DECODE_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define IMG_DECODE_MSG_Q_NAME            "IMG_decode"
#define IMG_DECODE_NUM_OF_MSGS           ((UINT16)  20)
#define IMG_DECODE_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* IMAGE prefetch file thread and prefetch file message queue definitions. */
#define IMG_PREFETCH_THREAD_NAME                "IMG_prefetch"
//#define IMG_PREFETCH_THREAD_DEFAULT_PRIORITY    ((UINT8)   100)
#define IMG_PREFETCH_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_REALTIME, PRIORITY_LAYER_MIDDLEWARE, 0))
#if 1 // for http picture playback http sync read need larger stack size 2008/08/28 -- Wei Yu
#define IMG_PREFETCH_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) (12 * 1024))
#else
#define IMG_PREFETCH_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)
#endif

#define IMG_PREFETCH_MSG_Q_NAME            "IMG_prefetch"
#define IMG_PREFETCH_NUM_OF_MSGS           ((UINT16)  20)
#define IMG_PREFETCH_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* Resource Manager thread and message queue definitions. */
#define RM_THREAD_NAME                "rm_thread"
//#define RM_THREAD_DEFAULT_PRIORITY    ((UINT8)    128)
#define RM_THREAD_DEFAULT_PRIORITY    ((UINT8)    PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define RM_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T)  1024)

#define RM_MSG_Q_NAME            "rm_msg_q"
#define RM_NUM_OF_MSGS           ((UINT16)  20)
#define RM_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

//#if CONFIG_STREAMING_BUTTON_SOUND_MEM_PHASE3
#define STREAMING_SCC_THREAD_NAME   "streaming_scc_thread"
#define STREAMING_SCC_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define STREAMING_SCC_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

//#endif

/* Stream Manager thread and message queue definitions. */
#define SM_SLCTR_THREAD_NAME          "sm_slctr_thread"
//#define SM_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define SM_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define SM_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define SM_SLCTR_MSGQ_NAME      "sm_slctr_msgq"
#define SM_DEFAULT_NUM_OF_MSGS  ((UINT16) 4095)

/* Process Manager thread and message queue definitions. */
#define PM_THREAD_NAME         "pm_thread"
//#define PM_DEFAULT_PRIORITY    ((UINT8)   128)
#define PM_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define PM_DEFAULT_STACK_SIZE  ((SIZE_T) 1024)

#define PM_MSG_Q_NAME            "pm_msg_q"
#define PM_DEFAULT_NUM_MSGS      ((UINT16)  32)
#define PM_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* POD Handler thread and message queue definitions. */
#define POD_THREAD_NAME         "pod_thread"
//#define POD_DEFAULT_PRIORITY    ((UINT8)   128)
#define POD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define POD_DEFAULT_STACK_SIZE  ((SIZE_T) 1024)

#define POD_MSG_Q_NAME            "pod_msg_q"
#define POD_DEFAULT_NUM_MSGS      ((UINT16)  32)
#define POD_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* POD CA Handler thread and message queue definitions. */
#define PODCA_THREAD_NAME         "podca_thread"
//#define PODCA_DEFAULT_PRIORITY    ((UINT8)   128)
#define PODCA_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define PODCA_DEFAULT_STACK_SIZE  ((SIZE_T) 1024)

#define PODCA_MSG_Q_NAME            "podca_msg_q"
#define PODCA_DEFAULT_NUM_MSGS      ((UINT16)  32)
#define PODCA_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* CA Handler thread and message queue definitions. */
#define CA_THREAD_NAME         "ca_thread"
//#define CA_DEFAULT_PRIORITY    ((UINT8)   128)
#define CA_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define CA_DEFAULT_STACK_SIZE  ((SIZE_T) 1024)

#define CA_MSG_Q_NAME            "ca_msg_q"
#define CA_DEFAULT_NUM_MSGS      ((UINT16)  32)
#define CA_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

#if CONFIG_SUPPORT_LIRC
#define IOM_ADAPTER_LIRC_THREAD_NAME                 "iom_adapter_lirc"
#define IOM_ADAPTER_LIRC_THREAD_DEFAULT_PRIORITY     ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define IOM_ADAPTER_LIRC_THREAD_DEFAULT_STACK_SIZE   ((SIZE_T) 1024)
#endif

#define IOM_ADAPTER_LIRC_SIMULATOR_THREAD_NAME                 "iom_adapter_lirc_simulator"
#define IOM_ADAPTER_LIRC_SIMULATOR_THREAD_DEFAULT_PRIORITY     ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define IOM_ADAPTER_LIRC_SIMULATOR_THREAD_DEFAULT_STACK_SIZE   ((SIZE_T) 1024)

#define IOM_ADAPTER_KB_THREAD_NAME                 "iom_adapter_kb"
#define IOM_ADAPTER_KB_THREAD_DEFAULT_PRIORITY     ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define IOM_ADAPTER_KB_THREAD_DEFAULT_STACK_SIZE   ((SIZE_T) 1024)

#define IOM_DFB_KEY_THREAD_NAME                 "iom_dfb_key_thread"
#define IOM_DFB_KEY_THREAD_DEFAULT_PRIORITY     ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define IOM_DFB_KEY_THREAD_DEFAULT_STACK_SIZE   ((SIZE_T) 1024)

/* IO Manager thread and message queue definitions. */
#define IOM_THREAD_NAME                 "iom_thread"
//#define IOM_THREAD_DEFAULT_PRIORITY     ((UINT8)   128)
#define IOM_THREAD_DEFAULT_PRIORITY     ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define IOM_THREAD_DEFAULT_STACK_SIZE   ((SIZE_T) 3584)

#define IOM_MSGQ_NAME                   "iom_msgq"
#define IOM_NUM_OF_MSGS                 ((UINT16)  32)
#define IOM_MSGQ_DEFAULT_PRIORITY       ((UINT8)  128)

/* CLI thread and message queue definitions. */
#define CLI_THREAD_NAME                 "cli_thread"
#define CLI_THREAD_NAME_MPTOOL          "cli_thread_mptool"
//#define CLI_THREAD_DEFAULT_PRIORITY     ((UINT8)   235)
#define CLI_THREAD_DEFAULT_PRIORITY     ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 5))
#define CLI_THREAD_DEFAULT_STACK_SIZE   ((SIZE_T) 4096*4*2)

#define CLI_MSGQ_NAME                   "cli_msgq"
#define CLI_MSGQ_DEFAULT_PRIORITY       ((UINT8)   128)
#define CLI_NUM_OF_MSGS                 ((UINT16) 2048)

/* ATV Manager thread and message queue definitions. */
#define ATVM_RTX_THREAD_NAME                "atvm_rtx_thread"
//#define ATVM_RTX_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define ATVM_RTX_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define ATVM_RTX_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 2048)

#define ATVM_RTX_MSGQ_NAME              "atvm_rtx_msgq"
#define ATVM_RTX_MSGQ_DEFAULT_PRIORITY  ((UINT8)  128)
#define ATVM_RTX_NUM_OF_MSGS            ((UINT16)  40)

#define ATVM_FLOW_THREAD_NAME                "atvm_flow_thread"
//#define ATVM_FLOW_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define ATVM_FLOW_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define ATVM_FLOW_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 1024)

/* WM thread and message queue definitions. */
#define WM_THREAD_NAME                  "wm_thread"
//#define WM_THREAD_DEFAULT_PRIORITY      ((UINT8)   128)
#define WM_THREAD_DEFAULT_PRIORITY      ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define WM_THREAD_DEFAULT_STACK_SIZE    ((SIZE_T) 8192)

#define WM_MSG_Q_NAME                   "wm_msg_q"
#define WM_MSG_DEFAULT_PRIORITY         ((UINT8)  128)
#define WM_NUM_OF_MSGS                  ((UINT16)  32)

/* File Manager thread and message queue definitions. */
#define FM_BUF_THREAD_NAME                "fm_buf_thread"
//#define FM_BUF_THREAD_DEFAULT_PRIORITY    ((UINT8)  128)
#define FM_BUF_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define FM_BUF_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 1024)

//#define FM_DEV_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define FM_DEV_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define FM_DEV_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 1024)

/* Service Context thread and message queue definitions. */
#define SVCTX_THREAD_NAME                "svctx_thread"
#define SVCTX_MSG_Q_NAME                 "svctx_msgq"
//#define SVCTX_THREAD_DEFAULT_PRIORITY    ((UINT8)  128)
#define SVCTX_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define SVCTX_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)
#define SVCTX_MSGQ_DEFAULT_PRIORITY      ((UINT8)  128)
#define SVCTX_NUM_OF_MSGS                ((UINT16) 32)

/* Closed Caption Widget Core thread and message queue definitions */
#define CCCORE_THREAD_NAME                      "cccore_thread"
//#define CCCORE_THREAD_DEFAULT_PRIORITY          ((UINT8)   128)
#define CCCORE_THREAD_DEFAULT_PRIORITY          ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define CCCORE_THREAD_DEFAULT_STACK_SIZE        ((SIZE_T) 4096)

#define CCCORE_MSGQ_NAME                        "cccore_msgq"
#define CCCORE_NUM_OF_MSGS                      ((UINT16)  256)
#define CCCORE_MSGQ_DEFAULT_PRIORITY            ((UINT8)   128)

#define CCCORE_TPWTR_MSGQ_NAME_PREFIX           "cctwq_"
#define CCCORE_TPWTR_NUM_OF_MSGS                ((UINT16)   32)
#define CCCORE_TPWTR_MSGQ_DEFAULT_PRIORITY      ((UINT8)   128)


/* SVL Builder thread and message queue definitions. */
#define SB_SLCTR_MSGQ_NAME                  "sb_slctr_msgq"
#define SB_SLCTR_THREAD_NAME                "sb_slctr_thread"

#define SB_DEFAULT_NUM_OF_MSGS              ((UINT16)  256)
//#define SB_THREAD_DEFAULT_PRIORITY          ((UINT8)   128)
#define SB_THREAD_DEFAULT_PRIORITY          ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define SB_THREAD_DEFAULT_STACK_SIZE        ((SIZE_T) 4096)


/* Event Context thread and message queue definitions. */
#define EVCTX_SLCTR_MSGQ_NAME               "evctx_slctr_msgq"
#define EVCTX_SLCTR_THREAD_NAME             "evctx_slctr_thread"

#ifdef  MW_EVCTX_CACHE_SUPPORT
#define EVCTX_DEFAULT_NUM_OF_MSGS           ((UINT16)  192*8)
#else
#define EVCTX_DEFAULT_NUM_OF_MSGS           ((UINT16)  192)
#endif
//#define EVCTX_THREAD_DEFAULT_PRIORITY       ((UINT8)   128)
#define EVCTX_THREAD_DEFAULT_PRIORITY       ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define EVCTX_THREAD_DEFAULT_STACK_SIZE     ((SIZE_T) 4096)

/* Rating Region Context thread and message queue definitions. */
#define RRCTX_SLCTR_MSGQ_NAME               "rrctx_slctr_msgq"
#define RRCTX_SLCTR_THREAD_NAME             "rrctx_slctr_thread"

#define RRCTX_DEFAULT_NUM_OF_MSGS           ((UINT16)   24)
//#define RRCTX_THREAD_DEFAULT_PRIORITY       ((UINT8)   128)
#define RRCTX_THREAD_DEFAULT_PRIORITY       ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define RRCTX_THREAD_DEFAULT_STACK_SIZE     ((SIZE_T) 8192)

/* Section Manager thread and message queue definitions. */
#define SECM_THREAD_NAME                   "sec_mngr_thread"

//#define SECM_THREAD_DEFAULT_PRIORITY       ((UINT8)   128)
#define SECM_THREAD_DEFAULT_PRIORITY       ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define SECM_THREAD_DEFAULT_STACK_SIZE     ((SIZE_T) 4096)

/* Section Engine thread and message queue definitions. */
#define SECT_THREAD_NAME                   "sect_eng_thread"

//#define SECT_THREAD_DEFAULT_PRIORITY       ((UINT8)   128)
#define SECT_THREAD_DEFAULT_PRIORITY       ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define SECT_THREAD_DEFAULT_STACK_SIZE     ((SIZE_T) 4096)

/* Table Manager DVB-SI engine thread and message queue definitions. */
#define DVB_SI_ENG_THREAD_NAME                 "dvb_si_eng_thread"
//#define DVB_SI_ENG_THREAD_DEFAULT_PRIORITY     ((UINT8)   128)
#define DVB_SI_ENG_THREAD_DEFAULT_PRIORITY     ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DVB_SI_ENG_THREAD_DEFAULT_STACK_SIZE   ((SIZE_T) 4096)

#define DVB_SI_ENG_MSGQ_NAME                   "dvb_si_eng_msgq"
#define DVB_SI_ENG_MSGQ_DEFAULT_PRIORITY       ((UINT8)   128)

/* VBI Filter thread and message queue definitions */
#define VBIF_THREAD_NAME                "vbif_thread"
//#define VBIF_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define VBIF_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define VBIF_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define VBIF_MSG_Q_NAME                 "vbif_msg_q"
#define VBIF_NUM_OF_MSGS                ((UINT16) 256)

/* CEC Manager thread and message queue definitions. */
#define CECM_THREAD_NAME                    "cecm_thread"
//#define CECM_THREAD_DEFAULT_PRIORITY        ((UINT8)   128)
#define CECM_THREAD_DEFAULT_PRIORITY        ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
//#define CECM_THREAD_DEFAULT_STACK_SIZE      ((SIZE_T) 1024)
#define CECM_THREAD_DEFAULT_STACK_SIZE      ((SIZE_T) 4096)

#define CECM_MSGQ_NAME                      "cecm_msgq"
#define CECM_NUM_OF_MSGS                    ((UINT16) 128)
#define CECM_MSGQ_DEFAULT_PRIORITY          ((UINT8)  128)

/* Device Manager thread and message queue definitions */
#define DM_CB_MSG_Q_NAME                "dm_cb_msg_q"
#define DM_CB_MSG_Q_NUM                 ((UINT16) 256)

#define DM_NFY_REQ_Q_NAME               "dm_nfy_req_q"
#define DM_NFY_REQ_Q_NUM                ((UINT16) 256)

#define DM_IOC_NFY_Q_NAME               "dm_ioc_nfy_q"
#define DM_IOC_NFY_Q_NUM                ((UINT16) 256)

#define DM_PRG_NFY_Q_NAME               "dm_prg_nfy_q"
#define DM_PRG_NFY_Q_NUM                ((UINT16) 256)

#define DM_CB_MSG_THREAD_NAME                "dm_cb_msg_thrd"
//#define DM_CB_MSG_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define DM_CB_MSG_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DM_CB_MSG_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define DM_NFY_REQ_THREAD_NAME                "dm_nfy_req_thrd"
//#define DM_NFY_REQ_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define DM_NFY_REQ_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DM_NFY_REQ_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define DM_IOC_NFY_THREAD_NAME                "dm_ioc_nfy_thrd"
//#define DM_IOC_NFY_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define DM_IOC_NFY_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DM_IOC_NFY_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define DM_PRG_NFY_THREAD_NAME                "dm_prg_nfy_thrd"
//#define DM_PRG_NFY_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define DM_PRG_NFY_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DM_PRG_NFY_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

//#define DM_AUTOMNT_THREAD_DEFAULT_PRIORITY    ((UINT8)   128)
#define DM_AUTOMNT_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DM_AUTOMNT_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

/* bdplus task stack size */
#define CPSM_BDP_BDPLUS_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 40*1024)

/* bdplus task priority, it should be the lower than the priority of dmx */
//#define CPSM_BDP_BDPLUS_TASK_DEFAULT_PRIORITY   ((UINT8) 127)
#define CPSM_BDP_BDPLUS_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -9))

/* bd category task stack size */
#define CPSM_BDP_CATEGORY_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 40*1024)
#define CPSM_BDP_CATEGORY_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -9))

/* bdp cps service task stack size */
#define CPSM_BDP_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 32*1024)

/* bdp cps service task priority */
//#define CPSM_BDP_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) 127)
#define CPSM_BDP_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

/* lp css cps service task stack size */
#define CPSM_LP_CSS_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 16*1024)

/* lp css cps service task priority */
//#define CPSM_LP_CSS_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) 127)
#define CPSM_LP_CSS_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

/* lp cprm cps service task stack size */
#define CPSM_LP_CPRM_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 16*1024)

/* lp cprm cps service task priority */
//#define CPSM_LP_CPRM_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) 127)
#define CPSM_LP_CPRM_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

/* lp sacd cps service task stack size */
#define CPSM_LP_SACD_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 16*1024)

/* lp sacd cps service task priority */
//#define CPSM_LP_SACD_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) 127)
#define CPSM_LP_SACD_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

/* lp divxdrm cps service task stack size */
#define CPSM_LP_DIVXDRM_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 16*1024)

/* lp divxdrm cps service task priority */
//#define CPSM_LP_DIVXDRM_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) 127)
#define CPSM_LP_DIVXDRM_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

/* hdcp srm cps service task stack size */
#define CPSM_HDCP_SRM_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 16*1024)

/* hdcp srm cps service task priority */
//#define CPSM_HDCP_SRM_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) 127)
#define CPSM_HDCP_SRM_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

// wmdrm
/* lp wmdrm cps service task stack size */
#define CPSM_LP_WMDRM_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 128*1024)//add the stack size from 32 to 128 --mtk07075 @20140829

/* lp wmdrm cps service task priority */
#define CPSM_LP_WMDRM_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

/* lp cppm cps service task stack size */
#define CPSM_LP_CPPM_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 16*1024)

/* lp cppm cps service task priority */
#define CPSM_LP_CPPM_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

/* lp marlin cps service task stack size */
#define CPSM_LP_MARLIN_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 32*1024)

/* lp marlin cps service task priority */
#define CPSM_LP_MARLIN_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

/* managed copy translation task priority */
#define CPSM_MC_TRANSL_TASK_DEFAULT_PRIORITY        ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))

/* managed copy translation stack size */
#define CPSM_MC_TRANSL_TASK_DEFAULT_STACK_SIZE      ((SIZE_T) 8*1024)

/* bcp cps service task priority */
#define CPSM_BCP_CPS_SRV_TASK_DEFAULT_PRIORITY      ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

/* bcp cps service task stack size */
#define CPSM_BCP_CPS_SRV_TASK_DEFAULT_STACK_SIZE    ((SIZE_T) 16*1024)

/* drmsrv cps service task stack size */
#define CPSM_DRMSRV_CPS_SRV_TASK_DEFAULT_STACK_SIZE ((SIZE_T) 16*1024)

/*drmsrv cps service task priority */
#define CPSM_DRMSRV_CPS_SRV_TASK_DEFAULT_PRIORITY   ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))

#define PLAYREADY_ANTIROLLBACK_THREAD_NAME  "PR_ANTI_TH"
#define PLAYREADY_ANTIROLLBACK_THREAD_DEFAULT_STACK_SIZE ((SIZE_T) 16 * 1024)
#define PLAYREADY_ANTIROLLBACK_DEFAULT_PRIORITY ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))
#define GL_THREAD_NAME              "gl_thread"
#define GL_DEFAULT_NUM_MSGS         ((UINT16)  50)
//#define GL_DEFAULT_PRIORITY         ((UINT8)   128)
#define GL_DEFAULT_PRIORITY         ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define GL_DEFAULT_STACK_SIZE       ((SIZE_T) 2048)

#define LPCH_HANDLER_DEFAULT_STACK_SIZE    ((SIZE_T) 4096 * 2)
//#define LPCH_HANDLER_DEFAULT_PRIORITY      ((UINT8) 127)
#define LPCH_HANDLER_DEFAULT_PRIORITY      ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))

#define AIH_HANDLER_DEFAULT_STACK_SIZE    ((SIZE_T) 4096)
#define AIH_HANDLER_DEFAULT_PRIORITY      ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))


#define SCC_THREAD_NAME          "scc"
#define SCC_THREAD_STACK_SIZE    (2 * 1024)
//#define SCC_THREAD_PRIORITY      (128)
#define SCC_THREAD_PRIORITY      (PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))

#define SCOM_THREAD_NAME         "SCOM0_TH"
#define SCOM_THREAD_STACK_SIZE   ( 20 * 1024)
#define SCOM_THREAD_PRIORITY     (PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))

#define SM_SESS_BDP_THREAD_STACK_SIZE    (20480)
//#define SM_SESS_BDP_THREAD_PRIORITY      (99)
#define SM_SESS_BDP_THREAD_PRIORITY      (PRIORITY(PRIORITY_CLASS_REALTIME, PRIORITY_LAYER_MIDDLEWARE, -1))

/* Extraction Stream Handler thread and message queue definitions. */
#define SM_EXTRACT_HDLR_MSGQ_DFLT_MAX_SIZE           1024
#if 0
#define SM_EXTRACT_HDLR_THRD_DFLT_PRIORITY           127
#else
#define SM_EXTRACT_HDLR_THRD_DFLT_PRIORITY           ((UINT8) PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, -1))
#endif
#define SM_EXTRACT_HDLR_THRD_DFLT_STACK_SIZE         4096*2

/* Initialization thread arguments. */
#define INIT_THREAD_NAME        "init_thread"
#define INIT_THREAD_STACK_SIZE  ((SIZE_T) 1024*80)
//#define INIT_THREAD_PRIORITY    ((UINT8)  128)
//#define INIT_THREAD_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define INIT_THREAD_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_TIME_CRITICAL, 0))

#define SBTL_NAME_THREAD            "sbtl_thread"
#define SBTL_THREAD_STACK_SIZE      4096
//#define SBTL_THREAD_PRI             128
#define SBTL_THREAD_PRI             PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0)

/* Ticker Tape widget engine thread definitions */
#define TKTP_ENG_THREAD_NAME_PREFIX                    "tktp_"
//#define TKTP_ENG_THREAD_DEFAULT_PRIORITY        ((UINT8)   128)
#define TKTP_ENG_THREAD_DEFAULT_PRIORITY        ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define TKTP_ENG_THREAD_DEFAULT_STACK_SIZE      ((SIZE_T) 4096)

#define TTX_NAME_THREAD             "ttx_thread"
#define TTX_THREAD_STACK_SIZE       4096
//#define TTX_THREAD_PRI              129
#define TTX_THREAD_PRI              PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 1)

//#define TFX_DEFAULT_THREAD_PRIORITY     ( 199)
#define TFX_DEFAULT_THREAD_PRIORITY   (PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_MIDDLEWARE, -1))
#define TFX_THREAD_STACK_SIZE           (4096)
#define TFX_THREAD_NAME                 "TFX_THREAD"


/* NET_LIB_IP_RX_THREAD */
#define IP_STACK_RX_THREAD_PRIORITY          PRIORITY(PRIORITY_CLASS_REALTIME, PRIORITY_LAYER_DRIVER, 0)
#define IP_STACK_RX_THREAD_STACK_SIZE        (4096)
#define IP_STACK_RX_THREAD_NAME              ("IpDrvRx")

/* NET_LIB_IP_TIMER_THREAD */
#define IP_STACK_TIMER_PRIORITY              IP_STACK_RX_THREAD_PRIORITY
#define IP_STACK_TIMER_THREAD_STACK_SIZE     (4096)
#define IP_STACK_TIMER_THREAD_NAME           ("IpTimer")

/* NET_MT_GLUE_TX_THREAD */
#define IP_DRV_TX_THREAD_PRIORITY            IP_STACK_RX_THREAD_PRIORITY
#define IP_DRV_TX_THREAD_STACK_SIZE          (1024)
#define IP_DRV_TX_THREAD_NAME                ("IpDrvTx")

/* NET_NI_MONITOR_THREAD */
#define NI_MONITOR_THREAD_PRIORITY           PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0) //IP_STACK_RX_THREAD_PRIORITY
#define NI_MONITOR_THREAD_STACK_SIZE         (1024)
#define NI_MONITOR_THREAD_NAME               ("NiEvMon")

/* MPTOOL_SERVER_THREAD */
#define MPTOOL_SERVER_THREAD_PRIORITY        PRIORITY(PRIORITY_CLASS_IDLE, PRIORITY_LAYER_MIDDLEWARE, 0)
#define MPTOOL_SERVER_THREAD_STACK_SIZE      (8192)
#define MPTOOL_SERVER_THREAD_NAME            ("MPServer")

/* NET_LIB_DLNA_DMP_THREAD */
#define DLNA_DMP_THREAD_PRIORITY             PRIORITY(PRIORITY_CLASS_IDLE, PRIORITY_LAYER_DRIVER, 0) //IP_STACK_RX_THREAD_PRIORITY
#define DLNA_DMP_THREAD_STACK_SIZE           (8192)
#define DLNA_DMP_THREAD_NAME                 ("DmpPrgs")

/* NET_LIB_DTCP_THREAD */
#define DTCP_THREAD_PRIORITY                PRIORITY(PRIORITY_CLASS_IDLE, PRIORITY_LAYER_DRIVER, 0) //IP_STACK_RX_THREAD_PRIORITY
#define DTCP_THREAD_STACK_SIZE              (8192)
#define DTCP_THREAD_NAME                    ("DtcpEm")


/* NET_LIB_DLNA_DMP_HTTP_FM_THREAD */
#define DLNA_HTTP_FM_THREAD_PRIORITY         DLNA_DMP_THREAD_PRIORITY //PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 1)
#define DLNA_HTTP_FM_THREAD_STACK_SIZE       (8192)
#define DLNA_HTTP_FM_THREAD_NAME             ("HttpFm")

/* Network upgrade module */
#define HTTP_NETWORK_UPGRADE_PRIORITY         PRIORITY(PRIORITY_CLASS_IDLE, PRIORITY_LAYER_MIDDLEWARE, 0)
#define HTTP_NETWORK_UPGRADE_STACK_SIZE       (4096)
#define HTTP_NETWORK_UPGRADE_THREAD_NAME      ("NHDload")

/* HTTP client engine */
#define HTTP_CLIENT_ENGINE_PRIORITY           PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_MIDDLEWARE, 0)
#define HTTP_CLIENT_ENGINE_STACK_SIZE         (24 * 1024)
#define HTTP_CLIENT_ENGINE_THREAD_NAME        ("httpbrow")

/* HTTP streaming client */
#define HTTP_STREAMING_ENGINE_PRIORITY        PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_MIDDLEWARE, 0)
#define HTTP_STREAMING_ENGINE_STACK_SIZE      (24 * 1024)
#define HTTP_STREAMING_ENGINE_THREAD_NAME     ("httpstrm")

/* HTTP fm cache write */
#define HTTP_FM_CACHE_ENGINE_W_PRIORITY       PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_MIDDLEWARE, 0)
#define HTTP_FM_CACHE_ENGINE_W_STACK_SIZE     (24* 1024)
#define HTTP_FM_CACHE_ENGINE_W_THREAD_NAME    ("hfmcw__td")

/* HTTP fm cache read */
#define HTTP_FM_CACHE_ENGINE_R_PRIORITY       PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_MIDDLEWARE, 0)
#define HTTP_FM_CACHE_ENGINE_R_STACK_SIZE     (12 * 1024)
#define HTTP_FM_CACHE_ENGINE_R_THREAD_NAME    ("hfmcr__td")

/* Steaming Adaptor Thread */
#define SA_THREAD_NAME        "SAThread"
#define SA_STACK_SIZE           (2048)
#define SA_THREAD_PRIORITY     PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_DRIVER, 0)

/* WLAN WPA supplicant thread */
#define WLAN_WPA_THREAD_NAME        "wlan_wpa_thrd"
#define WLAN_WPA_STACK_SIZE         (6 * 1024)
#define WLAN_WPA_THREAD_PRIORITY    PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0)

/* WMSP fm control thread */
#define WMSP_CONTROL_PRIORITY       PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_MIDDLEWARE, 0)
#define WMSP_CONTROL_STACK_SIZE     (12 * 1024)
#define WMSP_CONTROL_THREAD_NAME    ("wmsp_control")

/* WMSP fm read thread */
#define WMSP_READ_PRIORITY       PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_MIDDLEWARE, 0)
#define WMSP_READ_STACK_SIZE     (12 * 1024)
#define WMSP_READ_THREAD_NAME    ("wmsp_read")

/* UFC prefetch file thread and prefetch file message queue definitions. */
#define UFC_PREFETCH_THREAD_NAME                "UFC_prefetch"
#define UFC_PREFETCH_THREAD_DEFAULT_PRIORITY    ((UINT8)   PRIORITY(PRIORITY_CLASS_REALTIME, PRIORITY_LAYER_MIDDLEWARE, 0))
#define UFC_PREFETCH_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define UFC_PREFETCH_MSG_Q_NAME            "UFC_prefetch"
#define UFC_PREFETCH_NUM_OF_MSGS           ((UINT16)  20)
#define UFC_PREFETCH_MSG_DEFAULT_PRIORITY  ((UINT8)  128)

/* DROP CACHE module priority */
#define DROP_CACHE_THREAD_NAME                "drop_cache_thrd"
#define DROP_CACHE_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DROP_CACHE_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#endif /* _COMMON_H_ */

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


#ifndef _U_ACFG_H_
#define _U_ACFG_H_

#include "u_common.h"
#include "mas_lib.h"
/***********************************************************************
                             common
***********************************************************************/
/* Return Value */
#define APP_CFGR_OK                         ((INT32)   0)
#define APP_CFGR_CANT_INIT                  ((INT32)  -1)
#define APP_CFGR_NOT_INIT                   ((INT32)  -2)
#define APP_CFGR_INV_ARG                    ((INT32)  -3)
#define APP_CFGR_NOT_SUPPORT                ((INT32)  -4)
#define APP_CFGR_INTERNAL_ERR               ((INT32)  -5)
#define APP_CFGR_REC_NOT_FOUND              ((INT32)  -6)
#define APP_CFGR_CAI_INTL_ERR               ((INT32)  -7)
#define APP_CFGR_CUST_CMD_NOT_FOUND         ((INT32)  -8)
#define APP_CFGR_TOO_MANY_NFY_FUNC          ((INT32)  -9)
#define APP_CFGR_OUT_OF_MEMORY              ((INT32)  -10)


/* to be used to create an ID to be used for config routines */
#define CFG_MAKE_ID(ui1_group, ui1_setting) (((ui1_group) << 8) | (ui1_setting))
#define CFG_GET_GROUP(ui2_id) ((ui2_id) >> 8)
#define CFG_GET_SETTING(ui2_id) ((ui2_id) & 0xff)

#define ACFG_NFY_IVD_ID     ((UINT16) 0xffff)
#define ACFG_NFY_MAX_NUM    64
#define WLAN_MAC_MAX_LEN     (20) /*!< Maximum length of SSID in ASCII string form */
#define WLAN_KEY_MAX_LEN      (64) /*!< Maximum length of key in ASCII/HEX string form */

typedef VOID (*pf_acfg_nfy_func)(UINT16 ui2_nfy_id, VOID* pv_tag, UINT16 ui2_id);
typedef struct _ACFG_NFY_REC_T
{
    BOOL                b_used;
    UINT16              ui2_group;
    VOID*               pv_tag;
    pf_acfg_nfy_func    pf_acfg_notify;
}ACFG_NFY_REC_T;


//size of type equal to their enum value
typedef  enum
{
    CFG_NONE    = 0,
    CFG_8BIT_T  = 1,
    CFG_16BIT_T = 2,
    CFG_32BIT_T = 4,
    CFG_64BIT_T = 8
} CFG_FIELD_T;

extern INT32 u_acfg_notify_reg(UINT16 ui2_group,
                                    VOID* pv_tag,
                                    pf_acfg_nfy_func pf_acfg_notify,
                                    UINT16* pui2_nfy_id);/* OUT */
extern INT32 u_acfg_notify_unreg(UINT16  ui2_nfy_id);

/***********************************************************************
                             user data
***********************************************************************/
/*A quick index to find the field related data*/
typedef enum _CFG_IDX_T
{
    /*system*/
    IDX_VOLUME,
    IDX_MUTE,
    IDX_SOURCE,
    IDX_HW_SPEAKER_CHMAP,
    IDX_USER_SPEAKER_CHMAP,
    IDX_BLUETOOTH_ENABLE,
    IDX_WLAN_MAC,
    IDX_WLAN_KEY,
    IDX_WLAN_AP,
    IDX_MAX
}CFG_IDX_T;

/*----------------------group system----------------------------------*/
#define VOLUME_MAX                            (100)

/* Group ID */
typedef enum _CFG_GRPID_T
{
    CFG_GRPID_SYSTEM,
    CFG_GRPID_BLUETOOTH,
    CFG_GRPID_NETWORK,
}CFG_GRPID_T;

/*Record ID*/
typedef enum _CFG_RECID_SYSTEM_T
{
    CFG_RECID_SYSTEM_VOLUME,
    CFG_RECID_SYSTEM_MUTE,
    CFG_RECID_SYSTEM_SOURCE,
    CFG_RECID_SYSTEM_HW_SPEAK,
    CFG_RECID_SYSTEM_USER_SPEAK,
}CFG_RECID_SYSTEM_T;

typedef enum _CFG_RECID_NETWORK_T
{
    CFG_RECID_NETWORK_SSID,
    CFG_RECID_NETWORK_KEY,
    CFG_RECID_NETWORK_AP,
}CFG_RECID_NETWORK_T;

//source
typedef enum
{
    CFG_SOURCE_DEFAULT,
    CFG_SOURCE_LINE_IN,
    CFG_SOURCE_IN_USB,
    CFG_SOURCE_IN_C4A,
    CFG_SOURCE_BLUETOOTH_IN,// 4
    CFG_SOURCE_MAX,
}SS_ACTIVE_SOURCE;

typedef enum
{
    MAS_STREAM_BT = 0,
    MAS_STREAM_URI,
    MAS_STREAM_PROMPT_TTS
} MAS_AUDIO_STREAM_E;

extern INT32  u_acfg_set_volume(UINT8 ui1_value, BOOL b_upload);
extern INT32  u_acfg_get_volume(UINT8* pui1_value);
extern INT32  u_acfg_set_stream_volume(MAS_AUDIO_STREAM_E e_stream, UINT8 ui1_value);
extern INT32  u_acfg_set_volume_default(VOID);
extern INT32  u_acfg_increase_volume(VOID);
extern INT32  u_acfg_reduce_volume(VOID);
extern INT32  u_acfg_set_mute(UINT8 b_mute);
extern INT32  u_acfg_get_mute(UINT8* b_mute);
extern INT32  u_acfg_set_source(UINT8 ui1_value);
extern INT32  u_acfg_get_source(UINT8* pui1_value);
extern INT32  u_acfg_set_source_default(VOID);
extern INT32  u_acfg_set_user_speaker(mas_chmap_desc_t *pt_user_speaker);
extern INT32  u_acfg_get_user_speaker(mas_chmap_desc_t *pt_user_speaker);
extern INT32  u_acfg_clean_ap(VOID);
extern INT32  u_acfg_reset_factory_default(VOID);
extern INT32  u_acfg_factory_reset(VOID);
extern BOOL   u_acfg_gen_uid(VOID);
extern INT32  u_acfg_start_set_uid_timer(VOID);
extern INT32  u_cfg_get_bluetooth_enable(UINT8 * pui1_value);
extern INT32  u_cfg_set_bluetooth_enable(UINT8 ui1_value);
extern INT32  u_cfg_set_bluetooth_enable_default(VOID);
/*----------------------other module----------------------------------*/



#endif/* _U_ACFG_H_ */


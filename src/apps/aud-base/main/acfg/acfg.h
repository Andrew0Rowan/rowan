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


#ifndef __ACFG_H_
#define __ACFG_H_

#include <string.h>

#include "u_dbg.h"
#include "u_acfg.h"

/***********************************************************************
                    Macros, Typedefs, Enumerations
***********************************************************************/
/* Debug */
#undef  DBG_INIT_LEVEL
#define DBG_INIT_LEVEL          (DBG_LEVEL_ERROR|DBG_LEVEL_API|DBG_LEVEL_INFO | DBG_LAYER_APP)

#undef  DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE        acfg_get_dbg_level()

#define ACFG_ASSERT(_expr)      do{if(!(_expr))DBG_ACFG_ERR("assert fail\n");}while(0)

#define ACFG_TAG "<ACFG>"
#define DBG_ACFG_INFO(fmt, args...) do{DBG_INFO(("%s[%s:%d]:"fmt, ACFG_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define DBG_ACFG_API(fmt, args...)  do{DBG_API(("%s[%s:%d]:"fmt, ACFG_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define DBG_ACFG_CRIT(fmt, args...) do{DBG_ERROR(("%s[%s:%d]:"fmt, ACFG_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define DBG_ACFG_ERR(fmt, args...)  do{DBG_ERROR(("%s[%s:%d]:"fmt, ACFG_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define DBG_ACFG_DATA(...) do{DBG_ERROR((__VA_ARGS__));}while(0)


/* Return on error */
#define ACFG_CHK_FAIL(_ret, _ret_on_err)                                      \
do{                                                                           \
    if (_ret < 0)                                                             \
    {                                                                         \
        DBG_ACFG_ERR("Err:return fail:%d\r\n", _ret);                       \
        return _ret_on_err;                                                   \
    }                                                                         \
}while(FALSE)

#define ACFG_BREAK_ON_FAIL(_ret, _break_on_err)                               \
if (_ret < 0)                                                                 \
{                                                                             \
    DBG_ACFG_ERR("Err: return fail:%d\r\n", _ret);                          \
    _ret = _break_on_err;                                                     \
    break;                                                                    \
}

#define ACFG_LOG_ON_FAIL(_ret)                                                \
if (_ret < 0)                                                                 \
{                                                                             \
    DBG_ACFG_ERR("Err: return fail:%d\r\n", _ret);                          \
}

/**********************************************************************/
#define ACFG_CONFIG_SAVE_PATH       "/data/misc/app.cfg" /*need use ubi file system path*/
#define ACFG_CONFIG_SAVE_PATH_BACKUP       "/data/misc/app.cfg.backup"
#define SMARTBOX_AP_SAVE_PATH       "/data/smartbox/param.json"

#define ACFG_MAX_SAVE_PATH_LEN      32
#define ACFG_STORE_TIMEOUT          (1000) /* ms */

#define ACFG_MAX_REC_SIZE   (1 * 1024)  //max size for single item

#define ACFG_HEADER_SIZE    16
#define ACFG_REC_NAME_SIZE  32
#define ACFG_REC_TYPE_SIZE  2
#define ACFG_REC_NUM_SIZE   2
#define ACFG_REC_INFO_SIZE  (ACFG_REC_NAME_SIZE + ACFG_REC_TYPE_SIZE + ACFG_REC_NUM_SIZE)
#define ACFG_MAGIC_NUMBER   (UINT32)0xACF6FADA



/* This function is the prototype of validation callback function
   The z_size represent the number of bytes in pt_value */
typedef BOOL (*pf_app_cfg_validate_fct)(UINT16  ui2_id,
                                            VOID*   pt_value,
                                            SIZE_T  z_size);
/* This function is the prototype of update hook function*/
typedef VOID (*pf_app_cfg_update_fct)(UINT16  ui2_id);

typedef struct _ACFG_HEADER_T
{
    UINT32   ui4_magic;
    UINT32   ui4_rec_num;
    UINT32   ui4_rec_size;
    UINT32   ui4_resv;
}ACFG_HEADER_T;


/* ACFG data for individual record */
typedef struct _ACFG_DESCR_T
{

    CHAR                        ac_name[ACFG_REC_NAME_SIZE];

    UINT16                      ui2_id;         /* setting ID */
    CFG_FIELD_T                 e_type;         /* type of element */
    UINT16                      ui2_num_elem;   /* number of elements */
    VOID*                       pt_def_value;   /* default value */
    UINT16                      ui2_def_num;    /* number of default elements */
    pf_app_cfg_validate_fct     pf_validate;    /* validate function, if any */
    pf_app_cfg_update_fct       pf_update;      /* update hook fucntion, if any */
    BOOL                        b_enable;       /* If it is enabled */

    UINT32                      ui4_cache_off;  /* offset in cache */

} ACFG_DESCR_T;


#define SETUP_FIELD(_enable, _idx, _gid, _rid, _type, _nelem,   \
                          _def, _def_sz, _pf_valid, _pf_update) \
do{                                                             \
    ACFG_DESCR_T* pt_acfg;                                      \
    pt_acfg = acfg_get_item(_idx);                              \
    if (pt_acfg==NULL){DBG_ACFG_ERR("Err: get item fail idx=%d\n",_idx);break;}\
    strncpy(pt_acfg->ac_name,                                   \
            (const CHAR *)(#_idx),                              \
            ACFG_REC_NAME_SIZE);                                \
    pt_acfg->ui2_id          = CFG_MAKE_ID(_gid, _rid);         \
    pt_acfg->e_type          = _type;                           \
    pt_acfg->ui2_num_elem    = _nelem;                          \
    pt_acfg->pt_def_value    = _def;                            \
    pt_acfg->ui2_def_num     = _def_sz;                         \
    pt_acfg->pf_validate     = _pf_valid;                       \
    pt_acfg->pf_update       = _pf_update;                      \
    pt_acfg->b_enable        = _enable;                         \
}while(0)


/*---------------*/
/* ACFG Messages */
/*---------------*/
typedef enum{
    ACFG_PRI_MSG_STORE,             /* start a timer for flash store */
    ACFG_PRI_MSG_FLASH_STORE,       /* to do flash store */

    ACFG_PRI_MAX_MSG
} ACFG_PRI_MSG_T;


/*ACFG TIMER TYPE*/
typedef enum
{
	ACFG_TIMER_FLASH_STORE_REPEATER,
	ACFG_TIMER_SET_UID_REPEATER,
	ACFG_TIMER_MAX_NUM
}ACFG_TIMER_MSG_E;

/*-----------------------------------------------------------------------------
                    function declarations
-----------------------------------------------------------------------------*/

/* core functions*/
extern INT32 acfg_set_alsa_volume(UINT32 ui4_volume);

extern INT32 acfg_set(UINT16 ui2_idx, VOID* pt_values, UINT16 ui2_elem_num);
extern INT32 acfg_get(UINT16 ui2_idx, VOID* pt_values,SIZE_T* pz_size);
extern INT32 acfg_get_size(UINT16 ui2_idx, SIZE_T* pz_size);
extern INT32 acfg_store(VOID);
extern ACFG_DESCR_T* acfg_get_item(UINT16 ui2_idx);
extern INT32 acfg_set_default(UINT16 ui2_idx);
extern INT32 acfg_get_default(UINT16 ui2_idx, VOID* pt_values, SIZE_T* pz_size);
extern INT32 acfg_send_msg_to_self(UINT32 ui4_msg);

extern INT32 _acfg_store_request(VOID);
extern VOID acfg_set_b_g_app_cfg_init(BOOL flag);

/* flash Functions */
extern INT32 acfg_flash_init(VOID);
extern INT32 acfg_flash_uninit(VOID);
extern INT32 acfg_flash_set(UINT16 ui2_idx, VOID* pt_values, UINT16 ui2_elem_num);
extern INT32 acfg_flash_get(UINT16 ui2_idx, VOID* pt_values, SIZE_T* pz_size);
extern INT32 acfg_flash_store(VOID);


/* CLI Functions */
extern UINT16 acfg_get_dbg_level(VOID);
extern INT32 acfg_attach_cmd_tbl(VOID);
extern INT32 acfg_detach_cmd_tbl(VOID);

#endif /* __ACFG_H_ */


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


/* FILE NAME:  bt_mw_log.h
 * AUTHOR:
 * PURPOSE:
 *      It provides bluetooth mw log structure to MW.
 * NOTES:
 */

#ifndef _BT_MW_LOG_H_
#define _BT_MW_LOG_H_

#include"u_bt_mw_types.h"
#include"u_bt_mw_common.h"

#define BLUETOOTH_STACK_CONFIG_FOLDER  CONFIG_PATH

/* Define trace levels */
#define BT_MW_TRACE_LEVEL_NONE    0          /* No trace messages to be generated    */
#define BT_MW_TRACE_LEVEL_ERROR   1          /* Error condition trace messages       */
#define BT_MW_TRACE_LEVEL_WARNING 2          /* Warning condition trace messages     */
#define BT_MW_TRACE_LEVEL_API     3          /* API traces                           */
#define BT_MW_TRACE_LEVEL_EVENT   4          /* Debug messages for events            */
#define BT_MW_TRACE_LEVEL_DEBUG   5          /* Full debug messages                  */
#define BT_MW_TRACE_LEVEL_VERBOSE 6          /* Verbose debug messages               */

#define MAX_MW_TRACE_LEVEL        6

#define BT_MW_TRC_ID_START                   1
#define BT_MW_TRC_ID_END                     10

#define BT_DBG_COMM   "<BT><COM>"
#define BT_DBG_GAP    "<BT><GAP>"
#define BT_DBG_A2DP   "<BT><A2DP>"
#define BT_DBG_AVRCP  "<BT><AVRCP>"
#define BT_DBG_HID    "<BT><HID>"
#define BT_DBG_SPP    "<BT><SPP>"
#define BT_DBG_GATT   "<BT><GATT>"
#define BT_DBG_HFP    "<BT><HFP>"
#define BT_DBG_PB     "<BT><PB>"
#define BT_DBG_UPL    "<BT><UPL>"



typedef struct {
    const char                       *trc_name;
    UINT8                            trace_level;
}BT_MW_TRC_MAP;

extern BT_MW_TRC_MAP btmw_trc_map[];
void log_write(const char *format, ...);
void mw_log_init(void);

void bt_mw_log_setStackLevel(const char *section_name, int level);

/* define traces for application */
#define BT_DBG_ERROR(tag, fmt, args...)          {if (btmw_trc_map[tag].trace_level >= BT_MW_TRACE_LEVEL_ERROR)      log_write("%s<E>%s@%d " fmt "\n", btmw_trc_map[tag].trc_name, __FUNCTION__, __LINE__, ## args);}
#define BT_DBG_WARNING(tag, fmt, args...)        {if (btmw_trc_map[tag].trace_level >= BT_MW_TRACE_LEVEL_WARNING)    log_write("%s<W>%s@%d " fmt "\n", btmw_trc_map[tag].trc_name, __FUNCTION__, __LINE__, ## args);}
#define BT_DBG_NORMAL(tag, fmt, args...)         {if (btmw_trc_map[tag].trace_level >= BT_MW_TRACE_LEVEL_API)        log_write("%s<N>%s@%d " fmt "\n", btmw_trc_map[tag].trc_name, __FUNCTION__, __LINE__, ## args);}
#define BT_DBG_NOTICE(tag, fmt, args...)         {if (btmw_trc_map[tag].trace_level >= BT_MW_TRACE_LEVEL_EVENT)      log_write("%s<T>%s@%d " fmt "\n", btmw_trc_map[tag].trc_name, __FUNCTION__, __LINE__, ## args);}
#define BT_DBG_INFO(tag, fmt, args...)           {if (btmw_trc_map[tag].trace_level >= BT_MW_TRACE_LEVEL_DEBUG)      log_write("%s<I>%s@%d " fmt "\n", btmw_trc_map[tag].trc_name, __FUNCTION__, __LINE__, ## args);}
#define BT_DBG_MINOR(tag, fmt, args...)          {if (btmw_trc_map[tag].trace_level >= BT_MW_TRACE_LEVEL_VERBOSE)    log_write("%s<M>%s@%d " fmt "\n", btmw_trc_map[tag].trc_name, __FUNCTION__, __LINE__, ## args);}

INT32 bt_get_dbg_level(BT_DEBUG_LAYER_NAME_T layer);
VOID bt_set_dbg_level(BT_DEBUG_LAYER_NAME_T layer, INT32 level);

//CHAR* print_error_str(BT_ERR_TYPE errorId);

#define FUNC_ENTRY BT_DBG_NORMAL(BT_DEBUG_COMM, "+++ Enter +++")
#define FUNC_EXIT BT_DBG_NORMAL(BT_DEBUG_COMM, "--- Exit ---")

#define BT_MW_FUNC_ENTER(tag, fmt, args...) BT_DBG_NORMAL(tag, "+++ Enter "fmt, ## args)
#define BT_MW_FUNC_EXIT BT_DBG_NORMAL(BT_DEBUG_COMM, "--- Exit ---")


///< check the NULL pointer
#define BT_CHECK_POINTER(module, p)                                     \
        do                                                              \
        {                                                               \
            if (NULL == p)                                              \
            {                                                           \
                BT_DBG_ERROR(module, "Invalid Pointer!"#p);             \
                return BT_ERR_STATUS_NULL_POINTER;                      \
            }                                                           \
        }while(0)

#define BT_CHECK_POINTER_RETURN(module, p)                              \
        do                                                              \
        {                                                               \
            if (NULL == p)                                              \
            {                                                           \
                BT_DBG_ERROR(module, "Invalid Pointer!"#p);             \
                return;                      \
            }                                                           \
        }while(0)


#define BT_CHECK_RESULT(module, result)                                 \
        do                                                              \
        {                                                               \
            if (BT_SUCCESS != result)                                   \
            {                                                           \
                BT_DBG_ERROR(module,"result:%ld", (long)result);        \
                return result;                                          \
            }                                                           \
        }while(0)

#define BT_CHECK_PARAM(module, op, ret)\
        do {\
            if (op){\
                BT_DBG_ERROR(module,"return:%d", ret);\
                return (ret);\
            }\
        }while(0)

#endif /* _BT_MW_LOG_H_ */


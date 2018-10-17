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

/* FILE NAME:  bt_mw_gap.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include "bluetooth.h"
#include "bt_mw_common.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "bt_mw_gap.h"
#include "bt_mw_message_queue.h"
#include "dev_list.h"

#define BT_MW_GAP_DISC_REASON_ILLEGAL_COMMAND                         0x01
#define BT_MW_GAP_DISC_REASON_NO_CONNECTION                           0x02
#define BT_MW_GAP_DISC_REASON_HW_FAILURE                              0x03
#define BT_MW_GAP_DISC_REASON_PAGE_TIMEOUT                            0x04
#define BT_MW_GAP_DISC_REASON_AUTH_FAILURE                            0x05
#define BT_MW_GAP_DISC_REASON_KEY_MISSING                             0x06
#define BT_MW_GAP_DISC_REASON_MEMORY_FULL                             0x07
#define BT_MW_GAP_DISC_REASON_CONNECTION_TOUT                         0x08
#define BT_MW_GAP_DISC_REASON_MAX_NUM_OF_CONNECTIONS                  0x09
#define BT_MW_GAP_DISC_REASON_MAX_NUM_OF_SCOS                         0x0A
#define BT_MW_GAP_DISC_REASON_CONNECTION_EXISTS                       0x0B
#define BT_MW_GAP_DISC_REASON_COMMAND_DISALLOWED                      0x0C
#define BT_MW_GAP_DISC_REASON_HOST_REJECT_RESOURCES                   0x0D
#define BT_MW_GAP_DISC_REASON_HOST_REJECT_SECURITY                    0x0E
#define BT_MW_GAP_DISC_REASON_HOST_REJECT_DEVICE                      0x0F
#define BT_MW_GAP_DISC_REASON_HOST_TIMEOUT                            0x10
#define BT_MW_GAP_DISC_REASON_UNSUPPORTED_VALUE                       0x11
#define BT_MW_GAP_DISC_REASON_ILLEGAL_PARAMETER_FMT                   0x12
#define BT_MW_GAP_DISC_REASON_PEER_USER                               0x13
#define BT_MW_GAP_DISC_REASON_PEER_LOW_RESOURCES                      0x14
#define BT_MW_GAP_DISC_REASON_PEER_POWER_OFF                          0x15
#define BT_MW_GAP_DISC_REASON_CONN_CAUSE_LOCAL_HOST                   0x16
#define BT_MW_GAP_DISC_REASON_REPEATED_ATTEMPTS                       0x17
#define BT_MW_GAP_DISC_REASON_PAIRING_NOT_ALLOWED                     0x18
#define BT_MW_GAP_DISC_REASON_UNKNOWN_LMP_PDU                         0x19
#define BT_MW_GAP_DISC_REASON_UNSUPPORTED_REM_FEATURE                 0x1A
#define BT_MW_GAP_DISC_REASON_SCO_OFFSET_REJECTED                     0x1B
#define BT_MW_GAP_DISC_REASON_SCO_INTERVAL_REJECTED                   0x1C
#define BT_MW_GAP_DISC_REASON_SCO_AIR_MODE                            0x1D
#define BT_MW_GAP_DISC_REASON_INVALID_LMP_PARAM                       0x1E
#define BT_MW_GAP_DISC_REASON_UNSPECIFIED                             0x1F
#define BT_MW_GAP_DISC_REASON_UNSUPPORTED_LMP_FEATURE                 0x20
#define BT_MW_GAP_DISC_REASON_ROLE_CHANGE_NOT_ALLOWED                 0x21
#define BT_MW_GAP_DISC_REASON_LMP_RESPONSE_TIMEOUT                    0x22
#define BT_MW_GAP_DISC_REASON_LMP_ERR_TRANS_COLLISION                 0x23
#define BT_MW_GAP_DISC_REASON_LMP_PDU_NOT_ALLOWED                     0x24
#define BT_MW_GAP_DISC_REASON_ENCRY_MODE_NOT_ACCEPTABLE               0x25
#define BT_MW_GAP_DISC_REASON_UNIT_KEY_USED                           0x26
#define BT_MW_GAP_DISC_REASON_QOS_NOT_SUPPORTED                       0x27
#define BT_MW_GAP_DISC_REASON_INSTANT_PASSED                          0x28
#define BT_MW_GAP_DISC_REASON_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED     0x29
#define BT_MW_GAP_DISC_REASON_DIFF_TRANSACTION_COLLISION              0x2A
#define BT_MW_GAP_DISC_REASON_UNDEFINED_0x2B                          0x2B
#define BT_MW_GAP_DISC_REASON_QOS_UNACCEPTABLE_PARAM                  0x2C
#define BT_MW_GAP_DISC_REASON_QOS_REJECTED                            0x2D
#define BT_MW_GAP_DISC_REASON_CHAN_CLASSIF_NOT_SUPPORTED              0x2E
#define BT_MW_GAP_DISC_REASON_INSUFFCIENT_SECURITY                    0x2F
#define BT_MW_GAP_DISC_REASON_PARAM_OUT_OF_RANGE                      0x30
#define BT_MW_GAP_DISC_REASON_UNDEFINED_0x31                          0x31
#define BT_MW_GAP_DISC_REASON_ROLE_SWITCH_PENDING                     0x32
#define BT_MW_GAP_DISC_REASON_UNDEFINED_0x33                          0x33
#define BT_MW_GAP_DISC_REASON_RESERVED_SLOT_VIOLATION                 0x34
#define BT_MW_GAP_DISC_REASON_ROLE_SWITCH_FAILED                      0x35
#define BT_MW_GAP_DISC_REASON_INQ_RSP_DATA_TOO_LARGE                  0x36
#define BT_MW_GAP_DISC_REASON_SIMPLE_PAIRING_NOT_SUPPORTED            0x37
#define BT_MW_GAP_DISC_REASON_HOST_BUSY_PAIRING                       0x38
#define BT_MW_GAP_DISC_REASON_REJ_NO_SUITABLE_CHANNEL                 0x39
#define BT_MW_GAP_DISC_REASON_CONTROLLER_BUSY                         0x3A
#define BT_MW_GAP_DISC_REASON_UNACCEPT_CONN_INTERVAL                  0x3B
#define BT_MW_GAP_DISC_REASON_DIRECTED_ADVERTISING_TIMEOUT            0x3C
#define BT_MW_GAP_DISC_REASON_CONN_TOUT_DUE_TO_MIC_FAILURE            0x3D
#define BT_MW_GAP_DISC_REASON_CONN_FAILED_ESTABLISHMENT               0x3E
#define BT_MW_GAP_DISC_REASON_MAC_CONNECTION_FAILED                   0x3F

/* ConnectionLess Broadcast errors */
#define BT_MW_GAP_DISC_REASON_LT_ADDR_ALREADY_IN_USE                  0x40
#define BT_MW_GAP_DISC_REASON_LT_ADDR_NOT_ALLOCATED                   0x41
#define BT_MW_GAP_DISC_REASON_CLB_NOT_ENABLED                         0x42
#define BT_MW_GAP_DISC_REASON_CLB_DATA_TOO_BIG                        0x43

#define BT_MW_GAP_DISC_REASON_MAX_ERR                                 0x43


#define BT_MW_GAP_CASE_RETURN_STR(const) case const: return #const;

#define HID_COD_MASK 0x0500
#define RENDER_SERVICE 0x040000
#define CAPTURE_SERVICE 0x080000
//INT32 A2DP_SNK_COD[] = {0x0428, 0x0418, 0x0414, 0x0420};
//INT32 A2DP_SRC_COD[] = {0x0428, 0x0424, 0x042C};

BtAppGapEventCbk      BtAppGapCbk = NULL;;
BtAppGapGetPairingKeyCbk AppGetPairingKeyCbk = NULL;
BtAppLogOut AppLogOut = NULL;
BtAppGapInquiryResponseCbk AppInquiryCbk = NULL;


static BOOL fg_bt_base_init = FALSE;
static UINT32 fg_bt_inquiry_type;
static BOOL fg_bt_scan_ongoing;
static INT32 fg_rssi_value = 0;
static BOOL fg_rssi_got = FALSE;


list_t  *scan_device_list;
list_t *bonded_device_list;
pthread_rwlock_t scan_list_sync_lock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t bonded_list_sync_lock = PTHREAD_RWLOCK_INITIALIZER;

BT_LOCAL_DEV bt_local_dev;



static profile_operator_t profiles[BTWM_ID_MAX] = { NULL };

extern void log_reg(void (*log_cb)(char *log_str));

static CHAR* bt_mw_gap_get_disc_reason_str(INT32 reason)
{
    switch((INT32)reason)
    {
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_ILLEGAL_COMMAND);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_NO_CONNECTION);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_HW_FAILURE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_PAGE_TIMEOUT);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_AUTH_FAILURE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_KEY_MISSING);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_MEMORY_FULL);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_CONNECTION_TOUT);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_MAX_NUM_OF_CONNECTIONS);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_MAX_NUM_OF_SCOS);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_CONNECTION_EXISTS);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_COMMAND_DISALLOWED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_HOST_REJECT_RESOURCES);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_HOST_REJECT_SECURITY);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_HOST_REJECT_DEVICE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_HOST_TIMEOUT);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNSUPPORTED_VALUE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_ILLEGAL_PARAMETER_FMT);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_PEER_USER);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_PEER_LOW_RESOURCES);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_PEER_POWER_OFF);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_CONN_CAUSE_LOCAL_HOST);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_REPEATED_ATTEMPTS);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_PAIRING_NOT_ALLOWED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNKNOWN_LMP_PDU);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNSUPPORTED_REM_FEATURE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_SCO_OFFSET_REJECTED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_SCO_INTERVAL_REJECTED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_SCO_AIR_MODE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_INVALID_LMP_PARAM);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNSPECIFIED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNSUPPORTED_LMP_FEATURE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_ROLE_CHANGE_NOT_ALLOWED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_LMP_RESPONSE_TIMEOUT);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_LMP_ERR_TRANS_COLLISION);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_LMP_PDU_NOT_ALLOWED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_ENCRY_MODE_NOT_ACCEPTABLE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNIT_KEY_USED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_QOS_NOT_SUPPORTED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_INSTANT_PASSED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_DIFF_TRANSACTION_COLLISION);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNDEFINED_0x2B);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_QOS_UNACCEPTABLE_PARAM);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_QOS_REJECTED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_CHAN_CLASSIF_NOT_SUPPORTED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_INSUFFCIENT_SECURITY);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_PARAM_OUT_OF_RANGE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNDEFINED_0x31);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_ROLE_SWITCH_PENDING);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNDEFINED_0x33);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_RESERVED_SLOT_VIOLATION);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_ROLE_SWITCH_FAILED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_INQ_RSP_DATA_TOO_LARGE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_SIMPLE_PAIRING_NOT_SUPPORTED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_HOST_BUSY_PAIRING);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_REJ_NO_SUITABLE_CHANNEL);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_CONTROLLER_BUSY);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_UNACCEPT_CONN_INTERVAL);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_DIRECTED_ADVERTISING_TIMEOUT);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_CONN_TOUT_DUE_TO_MIC_FAILURE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_CONN_FAILED_ESTABLISHMENT);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_LT_ADDR_ALREADY_IN_USE);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_LT_ADDR_NOT_ALLOCATED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_CLB_NOT_ENABLED);
        BT_MW_GAP_CASE_RETURN_STR(BT_MW_GAP_DISC_REASON_CLB_DATA_TOO_BIG);
        default: return "UNKNOWN_REASON";
   }
}


BOOLEAN check_cod_valid(INT32 cod)
{
    if(fg_bt_inquiry_type == BT_INQUIRY_FILTER_TYPE_ALL)
    {
        return TRUE;
    }
    if(fg_bt_inquiry_type & BT_INQUIRY_FILTER_TYPE_HID)
    {
        if((cod & HID_COD_MASK) == HID_COD_MASK)
        {
            return TRUE;
        }
    }
    if(fg_bt_inquiry_type & BT_INQUIRY_FILTER_TYPE_A2DP_SRC)
    {
        if((cod & CAPTURE_SERVICE) == CAPTURE_SERVICE)
        {
            return TRUE;
        }
    }
    if(fg_bt_inquiry_type & BT_INQUIRY_FILTER_TYPE_A2DP_SNK)
    {
        if((cod & RENDER_SERVICE) == RENDER_SERVICE)
        {
            return TRUE;
        }
    }
    if(fg_bt_inquiry_type & BT_INQUIRY_FILTER_TYPE_HFP)
    {
        if((cod & RENDER_SERVICE) == RENDER_SERVICE)
        {
            return TRUE;
        }
    }
    return FALSE;
}
void bt_mw_register_profile(UINT8 id, profile_operator_t* operator)
{
    if(id >= BTWM_ID_MAX)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP,"Enter error register id = %d ", id);
        return ;
    }
    BT_DBG_NORMAL(BT_DEBUG_GAP,"register id = %d operator=%p", id, operator);
    if (NULL == operator)
    {
        profiles[id].init = NULL;
        profiles[id].deinit = NULL;
        profiles[id].notify_acl_state = NULL;
        profiles[id].facotry_reset = NULL;
        profiles[id].rm_dev = NULL;
    }
    else
    {
        profiles[id].init = operator->init;
        profiles[id].deinit = operator->deinit;
        profiles[id].notify_acl_state = operator->notify_acl_state;
        profiles[id].facotry_reset = operator->facotry_reset;
        profiles[id].rm_dev = operator->rm_dev;
    }
    if (GAP_STATE_ON == bt_local_dev.state
       && profiles[id].init)
    {
        profiles[id].init();
    }
}

/*call when bt power on*/
VOID bt_mw_init_profiles(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP,"Enter ");
    for(int i=0; i<BTWM_ID_MAX; i++)
    {
       if(profiles[i].init)
       {
           profiles[i].init();
       }
    }
}

/*call when bt power off*/
VOID bt_mw_deinit_profiles(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP,"Enter ");
    for(int i=0; i<BTWM_ID_MAX; i++)
    {
       if(profiles[i].deinit)
       {
           profiles[i].deinit();
       }
    }
}

VOID bt_mw_notify_acl_state(tBTMW_GAP_STATE *gap_state)
{
    //BT_DBG_NORMAL(BT_DEBUG_GAP,"Enter ");
    for(int i=0; i<BTWM_ID_MAX; i++)
    {
        //BT_DBG_NORMAL(BT_DEBUG_GAP,"i=%d, notify_acl_state=%p ",
        //    i, profiles[i].notify_acl_state);
       if(profiles[i].notify_acl_state)
       {
           profiles[i].notify_acl_state(gap_state);
       }
    }
}

INT32 bt_mw_gap_get_device_info(BLUETOOTH_DEVICE* dev_info, CHAR* bd_addr)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP, "bt_local_dev.state:%d", bt_local_dev.state);
    if(bt_local_dev.state == GAP_STATE_ON)
    {
       list_node_t *node = NULL;
       if(bonded_device_list != NULL)
        {
            pthread_rwlock_rdlock(&bonded_list_sync_lock);
            node = list_contains(bonded_device_list, bd_addr);
            if(node)
            {
                memcpy(dev_info, list_node(node), sizeof(BLUETOOTH_DEVICE));
                pthread_rwlock_unlock(&bonded_list_sync_lock);
                return BT_SUCCESS;
            }
            pthread_rwlock_unlock(&bonded_list_sync_lock);
        }
       else
        {
          BT_DBG_NORMAL(BT_DEBUG_GAP, "bonded_device_list is NULL");
        }
       if(scan_device_list != NULL)
        {
            pthread_rwlock_rdlock(&scan_list_sync_lock);
            node = list_contains(scan_device_list, bd_addr);
            if(node)
            {
                memcpy(dev_info, list_node(node), sizeof(BLUETOOTH_DEVICE));
                pthread_rwlock_unlock(&scan_list_sync_lock);
                return BT_SUCCESS;
            }
            pthread_rwlock_unlock(&scan_list_sync_lock);
        }
       else
        {
          BT_DBG_NORMAL(BT_DEBUG_GAP, "scan_device_list is NULL");
        }
        return BT_ERR_STATUS_FAIL;
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "bt_local_dev.state is off");
        return BT_ERR_STATUS_FAIL;
    }

}

VOID bt_mw_gap_device_info_handle(tBTMW_MSG *p_msg)
{
     FUNC_ENTRY;
     BLUETOOTH_DEVICE device;
     memcpy(&device, &p_msg->data.device_info.device, sizeof(BLUETOOTH_DEVICE));

     if (p_msg->data.device_info.device_kind == BT_DEVICE_LOCAL)
     {
         if(strlen(device.bdAddr))
         {
             strncpy(bt_local_dev.bdAddr, device.bdAddr, sizeof(device.bdAddr));
         }
         if(strlen(device.name))
         {
             strncpy(bt_local_dev.name, device.name, sizeof(device.name));
         }
         BT_DBG_NORMAL(BT_DEBUG_GAP, "get adapter addr:%s,name:%s", bt_local_dev.bdAddr, bt_local_dev.name);
         return;
     }
     if(p_msg->data.device_info.device_kind == BT_DEVICE_SCAN)
     {
         if(!check_cod_valid(device.cod))
         {
            BT_DBG_NORMAL(BT_DEBUG_GAP,
                "cod filter device addr:%s, name:%s, cod:0x%x",
                device.bdAddr,device.name, device.cod);
            return;
         }

         BT_DBG_NORMAL(BT_DEBUG_GAP, "get scan device addr:%s,name:%s,cod:0x%x",
                       device.bdAddr, device.name, device.cod);
         if(strlen(device.bdAddr) != 0)
         {
            pthread_rwlock_wrlock(&scan_list_sync_lock);

            list_node_t *node = list_contains(scan_device_list, device.bdAddr);
            if(node)
            {
                BT_DBG_NORMAL(BT_DEBUG_GAP, "dev list contains the same device");
                BLUETOOTH_DEVICE *dev_info = list_node(node);
                if((strlen(dev_info->name) == 0) && strlen(device.name))
                {
                    memcpy(dev_info->name, device.name,  strlen(device.name));
                    BT_DBG_NORMAL(BT_DEBUG_GAP, "update the device name");
                }
            }
            else
            {
                BT_DBG_NORMAL(BT_DEBUG_GAP, "add to dev list");
                BLUETOOTH_DEVICE *p_dev_res = (BLUETOOTH_DEVICE *)calloc(1, sizeof(BLUETOOTH_DEVICE));
                memcpy(p_dev_res, &device,  sizeof(device));
                list_append(scan_device_list, device.bdAddr, p_dev_res);
            }
            pthread_rwlock_unlock(&scan_list_sync_lock);
            // notify to app
            if(fg_bt_scan_ongoing)
            {
                bt_mw_nty_send_msg(p_msg);
            }
         }
         else
         {
            BT_DBG_ERROR(BT_DEBUG_GAP, "device addr is null");
         }
     }
     else
     {
        BT_DBG_NOTICE(BT_DEBUG_GAP, "get bonded device addr:%s,name:%s", device.bdAddr, device.name);
        if(strlen(device.bdAddr) != 0)
        {
            pthread_rwlock_wrlock(&bonded_list_sync_lock);
            if(list_contains(bonded_device_list,device.bdAddr))
            {
                BT_DBG_NORMAL(BT_DEBUG_GAP, "dev list contains the same device");
            }
            else
            {
                BT_DBG_NORMAL(BT_DEBUG_GAP, "add to dev list");
                BLUETOOTH_DEVICE *p_dev_res = (BLUETOOTH_DEVICE *)calloc(1, sizeof(BLUETOOTH_DEVICE));
                memcpy(p_dev_res, &device,  sizeof(device));
                list_append(bonded_device_list, device.bdAddr, p_dev_res);
            }
            pthread_rwlock_unlock(&bonded_list_sync_lock);
            // notify to app
            bt_mw_nty_send_msg(p_msg);

        }
     }
}

VOID bt_mw_clear_dev_list(VOID)
{
   INT32 i4_ret;
   pthread_rwlock_wrlock(&scan_list_sync_lock);
   list_free(scan_device_list);
   scan_device_list = NULL;
   pthread_rwlock_unlock(&scan_list_sync_lock);
   i4_ret = pthread_rwlock_destroy(&scan_list_sync_lock);
   if(i4_ret != 0)
   {
       BT_DBG_ERROR(BT_DEBUG_COMM, "fail to destroy rwlock:%ld", (long)i4_ret);
   }
   pthread_rwlock_wrlock(&bonded_list_sync_lock);
   list_free(bonded_device_list);
   bonded_device_list = NULL;
   pthread_rwlock_unlock(&bonded_list_sync_lock);

   i4_ret = pthread_rwlock_destroy(&bonded_list_sync_lock);
   if(i4_ret != 0)
   {
       BT_DBG_ERROR(BT_DEBUG_COMM, "fail to destroy rwlock:%ld", (long)i4_ret);
   }
}

VOID bt_mw_gap_state_handle(tBTMW_MSG *p_msg)
{
     FUNC_ENTRY;
     CHAR *bd_addr = p_msg->data.gap_state.bd_addr;
     BTMW_GAP_STATE state = p_msg->data.gap_state.state;

     switch (state)
     {
     case GAP_STATE_ON:
        BT_DBG_WARNING(BT_DEBUG_GAP, "BT STATE ON");
        bt_local_dev.state = GAP_STATE_ON;
        /* init A2DP/AVRCP/HID/HFP profiles*/
        bt_mw_init_profiles();
        bt_mw_nty_send_msg(p_msg);
        break;

     case GAP_STATE_OFF:
        BT_DBG_WARNING(BT_DEBUG_GAP, "BT STATE OFF");
        bt_local_dev.state = GAP_STATE_OFF;
        bt_mw_nty_send_msg(p_msg);
        bt_mw_deinit_profiles();
        bt_mw_clear_dev_list();
        break;

     case GAP_STATE_ACL_CONNECTED:
        BT_DBG_NORMAL(BT_DEBUG_GAP, "acl is connected:%s", bd_addr);
        bt_mw_nty_send_msg(p_msg);
        break;

     case GAP_STATE_ACL_DISCONNECTED:
        if(p_msg->data.gap_state.reason)
        {
            BT_DBG_NORMAL(BT_DEBUG_GAP, "disconnect reason %s(%d)",
                bt_mw_gap_get_disc_reason_str(p_msg->data.gap_state.reason),
                p_msg->data.gap_state.reason);
        }
        else
        {
            bt_mw_notify_acl_state(&p_msg->data.gap_state);
            BT_DBG_NORMAL(BT_DEBUG_GAP, "acl is disconnected:%s", bd_addr);
        }
        bt_mw_nty_send_msg(p_msg);
        break;

     case GAP_STATE_BONDED:
     case GAP_STATE_DISCOVERY_STARTED:
     case GAP_STATE_DISCOVERY_STOPED:
        bt_mw_nty_send_msg(p_msg);
        break;
     case GAP_STATE_BONDING:
        break;
     case GAP_STATE_NO_BOND:
        //bd_addr = p_msg->data.gap_state.bd_addr;
        pthread_rwlock_wrlock(&bonded_list_sync_lock);
        if(list_contains(bonded_device_list, bd_addr))
        {
            BT_DBG_NORMAL(BT_DEBUG_GAP, "dev list contains the same device");
            list_remove(bonded_device_list, bd_addr);
        }

        pthread_rwlock_unlock(&bonded_list_sync_lock);
        bt_mw_nty_send_msg(p_msg);
        break;
     default:
        BT_DBG_ERROR(BT_DEBUG_GAP, "bluetooth gap msg handle , no define event:%d",p_msg->hdr.event);
        break;
     }

}

VOID bt_mw_gap_msg_handle(tBTMW_MSG *p_msg)
{
    BT_DBG_NOTICE(BT_DEBUG_GAP, "bluetooth gap msg handle , event:%d", p_msg->hdr.event);

    if (p_msg->hdr.event == BTMW_GAP_STATE_EVT)
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "bluetooth gap msg handle , state event");
        bt_mw_gap_state_handle(p_msg);
    }
    else if (p_msg->hdr.event == BTMW_GAP_DEVICE_INFO_EVT)
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "bluetooth gap msg handle , device properties event");
        bt_mw_gap_device_info_handle(p_msg);
    }
    else
    {
       BT_DBG_ERROR(BT_DEBUG_GAP, "bluetooth gap msg handle , no define event");
    }
}

VOID bt_mw_gap_nty_state_handle(tBTMW_MSG *p_msg)
{
     FUNC_ENTRY;

     tBTMW_GAP_STATE state ;
     memcpy(&state, &p_msg->data.gap_state, sizeof(tBTMW_GAP_STATE));
     /*  call the app callback function*/
     if(BtAppGapCbk)
     {
        BtAppGapCbk(&state);
     }
}

VOID bt_mw_gap_nty_device_info_handle(tBTMW_MSG *p_msg)
{
     FUNC_ENTRY;
     tBTMW_GAP_DEVICE_INFO device_info ;
     memcpy(&device_info, &p_msg->data.device_info, sizeof(tBTMW_GAP_DEVICE_INFO));
     if(AppInquiryCbk)
     {
         AppInquiryCbk(&device_info);
     }
     else
     {
         BT_DBG_ERROR(BT_DEBUG_GAP, "don't register AppInquiryCbk");
     }
}

VOID bt_mw_gap_nty_handle(tBTMW_MSG *p_msg)
{
    BT_DBG_NOTICE(BT_DEBUG_GAP, "bluetooth gap notify msg handle , event:%d",p_msg->hdr.event);

    if (p_msg->hdr.event == BTMW_GAP_STATE_EVT)
    {
        BT_DBG_NOTICE(BT_DEBUG_GAP, "bluetooth gap msg handle , state event");
        bt_mw_gap_nty_state_handle(p_msg);
    }
    else if (p_msg->hdr.event == BTMW_GAP_DEVICE_INFO_EVT)
    {
        BT_DBG_NOTICE(BT_DEBUG_GAP, "bluetooth gap msg handle , device properties event");
        bt_mw_gap_nty_device_info_handle(p_msg);
    }
    else
    {
       BT_DBG_ERROR(BT_DEBUG_GAP, "bluetooth gap msg handle , no define event");
    }
}
/* FUNCTION NAME: bt_mw_set_local_dev_name
 * PURPOSE:
 *      { set_local_dev_name }
 * INPUT:
 *      parameter-1  --  name : device name
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 (1) this API will sendMsg to stack to set local device name
 (2) local device name will also stored in the variable g_localProperty
 *
 */
INT32 bt_mw_set_local_dev_name(CHAR *name)
{
    FUNC_ENTRY;
    BT_CHECK_POINTER(BT_DEBUG_GAP, name);

    BT_DBG_NORMAL(BT_DEBUG_GAP, "<<< call bt_setlocal_name: %s>>>",name);
    linuxbt_gap_set_device_name_handler(name);
    return BT_SUCCESS;
} /*bt_set_name*/

INT32 bt_mw_get_local_dev_info(BT_LOCAL_DEV* local_dev)
{

    BT_DBG_NORMAL(BT_DEBUG_GAP, "addr:%s,name:%s,power state:%s",
        bt_local_dev.bdAddr,bt_local_dev.name,
        (bt_local_dev.state == GAP_STATE_ON)?"ON":"OFF");

    memcpy(local_dev, &bt_local_dev, sizeof(BT_LOCAL_DEV));
    return BT_SUCCESS;
}

/* FUNCTION NAME: bt_mw_get_properties
 * PURPOSE:
 *      { get the information of local device }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_get_properties(VOID)
{
    FUNC_ENTRY;

    //set_bt_get_dev_info_done(FALSE);
    linuxbt_gap_get_adapter_properties_handler();

    return BT_SUCCESS;
} /*bt_mw_get_properties*/

/* FUNCTION NAME: bt_mw_set_power
 * PURPOSE:
 *      { set the power on or off}
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_set_power(BOOL fg_on)
{
    FUNC_ENTRY;
    INT32 i4_ret;
    if (fg_on)
    {
        scan_device_list = list_new(free);
        if(!scan_device_list)
        {
            BT_DBG_ERROR(BT_DEBUG_GAP, "can't new list for scan device");
        }

        i4_ret = pthread_rwlock_init(&scan_list_sync_lock, NULL);
        if(i4_ret != 0)
        {
            BT_DBG_ERROR(BT_DEBUG_COMM, "fail to init rwlock:%ld", (long)i4_ret);
        }

        bonded_device_list = list_new(free);
        if(!bonded_device_list)
        {
            BT_DBG_ERROR(BT_DEBUG_GAP, "can't new list for scan device");
        }
        i4_ret = pthread_rwlock_init(&bonded_list_sync_lock, NULL);
        if(i4_ret != 0)
        {
            BT_DBG_ERROR(BT_DEBUG_COMM, "fail to init rwlock:%ld", (long)i4_ret);
        }

        BT_DBG_NORMAL(BT_DEBUG_GAP, "<<< call btenable >>>");
        return linuxbt_gap_enable_handler();
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "<<< call btdisable >>>");
        return linuxbt_gap_disable_handler();
    }
} /*bt_set_power_on*/

/* FUNCTION NAME: bt_mw_set_power_sync
 * PURPOSE:
 *      { set the power on or off ,sync mode }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_set_power_sync(BOOL fg_on)
{
    FUNC_ENTRY;
    UINT32 ui4_loop;
    INT32 i4_ret = 0;
    i4_ret = bt_mw_set_power(fg_on);
    if (BT_ERR_STATUS_FAIL == i4_ret)
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "no need wait power %s", fg_on?"on":"off");
        return BT_ERR_STATUS_FAIL;
    }
    /*waiting for power off event*/
    /* vendor lib init timeout is DEFAULT_STARTUP_TIMEOUT_MS(8s) */
    ui4_loop = 20;
    while (0 < ui4_loop)
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "ui4_loop:%ld,power is %s:",
                      (long)ui4_loop,
                      (bt_local_dev.state == GAP_STATE_ON)?"on":"off"
                     );
        if (((bt_local_dev.state == GAP_STATE_ON)&& fg_on)
            || ((bt_local_dev.state == GAP_STATE_OFF)&& !fg_on))
        {
            break;
        }
        usleep(500*1000);
        ui4_loop--;
    }
    if (((bt_local_dev.state == GAP_STATE_ON)&& !fg_on)
        || ((bt_local_dev.state == GAP_STATE_OFF)&& fg_on))
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "can't power %s", fg_on?"on":"off");
        return BT_ERR_STATUS_FAIL;
    }
    FUNC_EXIT;
    return BT_SUCCESS;
} /*bt_mw_set_power_sync*/

/* FUNCTION NAME: bt_mw_set_connectable_and_discoverable
 * PURPOSE:
 *      {set the state of local device connectable_and_discoverable }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_set_connectable_and_discoverable(BOOL fg_conn, BOOL fg_disc)
{
    FUNC_ENTRY;

    return bt_mw_set_scanmode_sync(fg_conn, fg_disc);
} /*bt_mw_set_connectable_and_discoverable*/

/* FUNCTION NAME: bt_mw_set_scanmode_sync
 * PURPOSE:
 *      { set the state of local device connectable_and_discoverable  }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES: this function works the same as bt_mw_set_connectable_and_discoverable
 *
 */
INT32 bt_mw_set_scanmode_sync(BOOL fg_conn, BOOL fg_disc)
{
    FUNC_ENTRY;
    bt_scan_mode_t scan_mode;
    INT32 i4_ret = BT_ERR_STATUS_FAIL;

    if (bt_local_dev.state == GAP_STATE_OFF)
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "power is off");
        return BT_SUCCESS;
    }

    if (fg_conn)
    {
        /*iscan_on --> discoverable on*/
        if (fg_disc)
        {
            BT_DBG_NORMAL(BT_DEBUG_GAP, "<<< call bt_setscan_mode(%ld) >>>", (long)BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
            scan_mode = BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE;
        }
        else
        {
            BT_DBG_NORMAL(BT_DEBUG_GAP, "<<< call bt_setscan_mode(%ld) >>>", (long)BT_SCAN_MODE_CONNECTABLE);
            scan_mode = BT_SCAN_MODE_CONNECTABLE;
        }
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "<<< call bt_setscan_mode(%ld) >>>", (long)BT_SCAN_MODE_NONE);
        scan_mode = BT_SCAN_MODE_NONE;
    }
    i4_ret = linuxbt_gap_set_scan_mode(scan_mode);
    FUNC_EXIT;
    return i4_ret;
}

/* FUNCTION NAME: bt_mw_scan
 * PURPOSE:
 *      { to scan the remote device}
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_scan(UINT32 ui4_filter_type)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP, "<<< call btstartDiscovery , filter_type= %d>>>",ui4_filter_type);
    fg_bt_inquiry_type = ui4_filter_type;
    fg_bt_scan_ongoing = TRUE;
    pthread_rwlock_wrlock(&scan_list_sync_lock);
    if (scan_device_list)
	{
        list_clear(scan_device_list);
	}
    pthread_rwlock_unlock(&scan_list_sync_lock);

    linuxbt_gap_start_discovery_handler();
    FUNC_EXIT;
    return BT_SUCCESS;
}

/* FUNCTION NAME: bt_mw_get_rssi
 * PURPOSE:
 *      { to get the rssi  }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_get_rssi(CHAR *address, INT16 *rssi_value)
{
    FUNC_ENTRY;

    UINT32 ui4_loop = 5;
    fg_rssi_got = FALSE;
    BT_DBG_NORMAL(BT_DEBUG_GAP, "<<< call bt_get_rssi >>>");
    linuxbt_gap_get_rssi_handler(address);

    /*wait response from stack*/
    while (!fg_rssi_got && (0 < ui4_loop))
    {
        /*x_thread_delay(200);*/
        usleep(200*1000);
        ui4_loop--;
    }
    if (fg_rssi_got)
    {
        *rssi_value = fg_rssi_value;
        BT_DBG_INFO(BT_DEBUG_GAP, "rssi value:%ld", (long)*rssi_value);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_GAP, "not receive get rssi cbk!");
        fg_rssi_got = FALSE;
        return BT_ERR_STATUS_FAIL;
    }

    fg_rssi_got = FALSE;
    return BT_SUCCESS;
}


/* FUNCTION NAME: bt_mw_stop_scan
 * PURPOSE:
 *      {to stop scan }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 bt_mw_stop_scan(VOID)
{
    FUNC_ENTRY;
    BT_DBG_NORMAL(BT_DEBUG_GAP, "<<< call btstopDiscovery >>>");
    fg_bt_scan_ongoing = FALSE;
    linuxbt_gap_cancel_discovery_handler();

    return BT_SUCCESS;
} /*bt_cancel_inquire*/

/* FUNCTION NAME: bt_mw_pair
 * PURPOSE:
 *      { to pair }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_pair(CHAR *addr, int transport)
{
    FUNC_ENTRY;
    BT_CHECK_POINTER(BT_DEBUG_GAP, addr);

    BT_DBG_NORMAL(BT_DEBUG_GAP, "the MAC is: %s; transport:%d", addr,transport);
    return linuxbt_gap_create_bond_handler(addr, transport);
} /*bt_pairing_request*/

/* FUNCTION NAME: bluetooth_remove_paired_dev
 * PURPOSE:
 *      {to remove paired device}
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_unpair(CHAR *addr)
{
    FUNC_ENTRY;
    BT_CHECK_POINTER(BT_DEBUG_GAP, addr);
    BT_DBG_INFO(BT_DEBUG_GAP, "unpair the address: %s", addr);

    for(int i=0; i<BTWM_ID_MAX; i++)
    {
       if(profiles[i].rm_dev)
       {
           profiles[i].rm_dev(addr);
       }
    }

    return linuxbt_gap_remove_bond_handler(addr);
} /*bt_delete_pairing_request*/

/* FUNCTION NAME: bt_mw_gap_init
 * PURPOSE:
 *      {gap profile initial}
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_gap_init(VOID)
{
    FUNC_ENTRY;
    //memset(&all_target_dev_info, 0x0, sizeof(BT_DEV_MAPPING_LIST));

    linuxbt_hdl_register(BTWM_ID_GAP, bt_mw_gap_msg_handle);
    bt_mw_nty_hdl_register(BTWM_ID_GAP, bt_mw_gap_nty_handle);
    bt_mw_nty_queue_init_new();
    linuxbt_msg_queue_init_new();
    //remote_device_list = list_new(NULL);

    return linuxbt_gap_init();

} /*bt_mw_gap_init*/

/* FUNCTION NAME: bt_mw_gap_deinit
 * PURPOSE:
 *      {gap profile deinitial}
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_gap_deinit(VOID)
{
    FUNC_ENTRY;
    linuxbt_gap_deinit();

    return BT_SUCCESS;
} /*bt_mw_gap_init*/

/* FUNCTION NAME: bt_mw_set_virtual_sniffer
 * PURPOSE:
 *      {to enable/disable sniffer log}
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 bt_mw_set_virtual_sniffer(INT32 enable)
{
    FUNC_ENTRY;
    linuxbt_gap_config_hci_snoop_log_handler(enable);

    return BT_SUCCESS;
} /*bt_mw_set_virtual_sniffer*/

/* FUNCTION NAME: bt_gap_get_rssi_result_cb
 * PURPOSE:
 *      { get rssi result callback }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
VOID bt_gap_get_rssi_result_cb(INT16 rssi_value)
{

    BT_DBG_NORMAL(BT_DEBUG_GAP, "rssi value: %ld", (long)rssi_value);

    fg_rssi_got = TRUE;
    fg_rssi_value = rssi_value;
}

/* FUNCTION NAME: bt_gap_get_pin_code_cb
 * PURPOSE:
 *      { get pin code callback }
 * INPUT:
 *      parameter-1  -- pin code
 * OUTPUT:
 *      parameter-n  -- NULL
 * RETURN:
 *      return-value-1 -- NULL
 * NOTES:
 *
 */
VOID bt_gap_get_pin_code_cb(bt_pin_code_t *pin, UINT8 *fg_accept)
{
    FUNC_ENTRY;
    pairing_key_value_t t_pairing_key_struct;
    memset(&t_pairing_key_struct, 0, sizeof(pairing_key_value_t));
    t_pairing_key_struct.key_type = PIN_CODE;
    memcpy(t_pairing_key_struct.pin_code, pin, sizeof(t_pairing_key_struct.pin_code));

    if (AppGetPairingKeyCbk != NULL)
    {
        AppGetPairingKeyCbk(&t_pairing_key_struct, fg_accept);
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "AppGetPairingKeyCbk not register.");
    }
}

/* FUNCTION NAME: bt_gap_get_passkey_cb
 * PURPOSE:
 *      { get passkey callback }
 * INPUT:
 *      parameter-1  -- passkey
 * OUTPUT:
 *      parameter-n  -- NULL
 * RETURN:
 *      return-value-1 -- NULL
 * NOTES:
 *
 */
VOID bt_gap_get_passkey_cb(UINT32 passkey, UINT8 *fg_accept)
{
    FUNC_ENTRY;
    pairing_key_value_t t_pairing_key_struct;
    memset(&t_pairing_key_struct, 0, sizeof(pairing_key_value_t));
    t_pairing_key_struct.key_type = PASSKEY;
    t_pairing_key_struct.key_value = passkey;
    if (AppGetPairingKeyCbk != NULL)
    {
        AppGetPairingKeyCbk(&t_pairing_key_struct, fg_accept);
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "AppGetPairingKeyCbk not register.");
    }
}

/* FUNCTION NAME: bt_mw_send_hci
 * PURPOSE:
 *      { send buffer to hci }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_send_hci(CHAR* buffer)
{

    FUNC_ENTRY;
    return linuxbtgap_send_hci_handler(buffer);

}

INT32 bt_mw_get_bonded_device(VOID)
{
    FUNC_ENTRY;
    //memset(&bond_target_dev_info, 0, sizeof(bond_target_dev_info));
    bt_mw_get_properties();
    return BT_SUCCESS;
}

/*register APP callback function*/
INT32 bt_mw_register_cbk_fct(BT_APP_CB_FUNC *func)
{
    INT32 i4_ret = BT_ERR_STATUS_FAIL;

    BT_DBG_NORMAL(BT_DEBUG_COMM, "start register cbk");
    if (NULL == func)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "callback func is null!");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if (func->bt_gap_event_cb)
    {
        BtAppGapCbk = func->bt_gap_event_cb;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "event callback func is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (func->bt_get_pairing_key_cb)
    {
        AppGetPairingKeyCbk = func->bt_get_pairing_key_cb;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "get pairing key func is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }


    if (func->bt_dev_info_cb)
    {
        AppInquiryCbk = func->bt_dev_info_cb;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "bt_dev_info_cb(Inquiry,Bonded) is null!");
        return BT_ERR_STATUS_NULL_POINTER;
    }

    if (func->bt_app_log_cb)
    {
        AppLogOut = func->bt_app_log_cb;
        log_reg(AppLogOut);
        BT_DBG_NORMAL(BT_DEBUG_COMM, "bt_app_log_cb reg!");
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "bt_app_log_cb func is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    return i4_ret;
}

VOID bt_mw_check_default_folder(VOID)
{
    FUNC_ENTRY;
    struct stat fileStat;

    /* Create MW folder */
    if (stat(BLUETOOTH_MW_LOCAL_FOLDER,&fileStat) < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "[GAP] %s %ld", BLUETOOTH_MW_LOCAL_FOLDER, (long)errno);
        if (mkdir((CHAR *)BLUETOOTH_MW_LOCAL_FOLDER, 0777) != 0)
        {
            BT_DBG_ERROR(BT_DEBUG_COMM, "[GAP] Create Folder Failed! (%s)(%ld)", BLUETOOTH_MW_LOCAL_FOLDER, (long)errno);
            return;
        }
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_COMM, "[GAP] have Folder: %s", BLUETOOTH_MW_LOCAL_FOLDER);
    }
    /* Create MW logs folder */
    if (stat(BLUETOOTH_MW_LOCAL_FOLDER"/logs",&fileStat) < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "[GAP] %s %ld", BLUETOOTH_MW_LOCAL_FOLDER"/logs", (long)errno);
        if (mkdir((CHAR *)BLUETOOTH_MW_LOCAL_FOLDER"/logs", 0777) != 0)
        {
            BT_DBG_ERROR(BT_DEBUG_COMM, "[GAP] Create Folder Failed! (%s)(%ld)", BLUETOOTH_MW_LOCAL_FOLDER"/logs", (long)errno);
            return;
        }
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_COMM, "[GAP] have Folder: %s", BLUETOOTH_MW_LOCAL_FOLDER"/logs");
    }
    /* Create Stack folder */
    if (stat(BLUETOOTH_STACK_LOCAL_FOLDER,&fileStat) < 0)
    {
        BT_DBG_ERROR(BT_DEBUG_COMM, "[GAP] %s %ld", BLUETOOTH_STACK_LOCAL_FOLDER, (long)errno);
        BT_DBG_NORMAL(BT_DEBUG_COMM, "[GAP] Create Folder: %s", BLUETOOTH_STACK_LOCAL_FOLDER);
        if (mkdir((CHAR *)BLUETOOTH_STACK_LOCAL_FOLDER, 0777) != 0)
        {
            BT_DBG_ERROR(BT_DEBUG_COMM, "[GAP] Create Folder Failed! (%s)(%ld)", BLUETOOTH_STACK_LOCAL_FOLDER, (long)errno);
            return;
        }
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_COMM, "[GAP] have Folder: %s", BLUETOOTH_STACK_LOCAL_FOLDER);
    }
}

/*bluetooth init function, before run this API should run stack first*/
INT32 bt_mw_base_init(BT_APP_CB_FUNC *func)
{
    FUNC_ENTRY;

    BT_DBG_NORMAL(BT_DEBUG_COMM, "the fg_bt_base_init is:%d!", fg_bt_base_init);
    if (!fg_bt_base_init)
    {
        memset(&profiles, 0, sizeof(profiles));
        bt_mw_check_default_folder();
        mw_log_init();
        /* register app callback function to mw */
        bt_mw_register_cbk_fct( func );

        bt_mw_gap_init();

        fg_bt_base_init = TRUE;
        BT_DBG_NORMAL(BT_DEBUG_COMM, "after init the fg_bt_base_init is:%d!", fg_bt_base_init);
    }
    return BT_SUCCESS;
}


INT32 bt_mw_on_off(BOOL fg_on)
{

    INT32 i4_ret = BT_ERR_STATUS_FAIL;
    BT_DBG_NORMAL(BT_DEBUG_COMM, "%s",fg_on?"on":"off");
    if(((fg_on == TRUE) && (bt_local_dev.state == GAP_STATE_ON))
        || ((fg_on == FALSE) && (bt_local_dev.state == GAP_STATE_OFF)))
    {
        BT_DBG_NORMAL(BT_DEBUG_COMM, "bt already in %s state",fg_on?"on":"off");
        return BT_SUCCESS;
    }
    if (fg_on)
    {
        i4_ret = bt_mw_set_power_sync(TRUE);
    }
    else
    {
        //bt_mw_deinit_profiles();
        i4_ret = bt_mw_set_power_sync(FALSE);
        usleep(500*1000);
    }

    return i4_ret;
}

INT32 bt_mw_factory_reset(VOID)
{
    FUNC_ENTRY;

    system(BLUETOOTH_RM_BT_CONFIG_CMD);
    system(BLUETOOTH_RM_BT_SNOOP_HCI_CMD);
    system(BLUETOOTH_RM_BT_STACK_LOG_CMD);
    system(BLUETOOTH_RM_BAK_FILE_CMD);
    system(BLUETOOTH_RM_BT_PROPERTY_CMD);

    for(int i=0; i<BTWM_ID_MAX; i++)
    {
       if(profiles[i].facotry_reset)
       {
           profiles[i].facotry_reset();
       }
    }

    BT_DBG_NORMAL(BT_DEBUG_COMM, "rm file done!");
    return BT_SUCCESS;
}

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)

/* FUNCTION NAME: bt_mw_database_add
 * PURPOSE:
 *      { add interop_database }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_database_add(UINT16 feature, bt_bdaddr_t *remote_bd_addr, size_t len)
{
    FUNC_ENTRY;
    linuxbt_interop_database_add((uint16_t)feature, remote_bd_addr, len);
    return BT_SUCCESS;
}

/* FUNCTION NAME: bt_mw_database_add
 * PURPOSE:
 *      { clear interop_database }
 * INPUT:
 *      parameter-1  --
 * OUTPUT:
 *      parameter-n  --
 * RETURN:
 *      return-value-1 --
 * NOTES:
 *
 */
INT32 bt_mw_database_clear(VOID)
{
    FUNC_ENTRY;
    linuxbt_interop_database_clear();
    return BT_SUCCESS;
}

#endif

#if defined(BT_RPC_DBG_SERVER)
EXPORT_SYMBOL int dbg_gap_get_result_number(int array_index, int offset, char *name, char *data, int length)
{
    if (offset >= 1)
    {
        return 0;
    }
    sprintf(name, "g_pt_result_number");
    sprintf(data, "%d", g_pt_result_number);

    return offset+1;
}

EXPORT_SYMBOL int dbg_gap_get_inquiry_type(int array_index, int offset, char *name, char *data, int length)
{
    if (offset >= 1)
    {
        return 0;
    }
    sprintf(name, "fg_bt_inquiry_type");
    sprintf(data, "%d", fg_bt_inquiry_type);

    return offset+1;
}

#endif

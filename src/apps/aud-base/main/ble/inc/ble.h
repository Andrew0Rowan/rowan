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



#ifndef    _BLE_H_
#define    _BLE_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/

/* return value */
#define BLE_OK                         ((INT32)0)
#define BLE_FAIL                       ((INT32)-1) /* abnormal return must < 0 */
#define BLE_INV_ARG                    ((INT32)-2)

/* log level */
#define BLE_LOG_ERR 2
#define BLE_LOG_INFO 1
#define BLE_LOG_DBG 0

/*ble server description*/
#define BT_LEN_UUID_LEN 37
#define BT_DEVICE_NAME_LEN 16

/* message */

/* application structure */
typedef struct _BLE_MSG_T
{
    UINT32          ui4_msg_id;
    UINT32          ui4_data1;
    UINT32          ui4_data2;
    UINT32          ui4_data3;
} BLE_MSG_T;

typedef enum{
    BLE_TO_BLE_MSG_INIT_AND_OPEN_SVR,
    BLE_TO_BLE_MSG_INIT_SVR,
    BLE_TO_BLE_MSG_OPEN_SVR,
    BLE_TO_BLE_MSG_CLOSE_SVR,

    BLE_TO_BLE_MSG_MAX
}BLE_TO_BLE_MSG_TYPE;

typedef enum _BLE_TO_WIFI_CMD_INDEX_E
{
    BLE_TO_WIFI_CMD_WIFI_CONNECT = 0,
    BLE_TO_WIFI_CMD_WIFI_DISCONNECT,
    BLE_TO_WIFI_CMD_MAX
}BLE_TO_WIFI_CMD_INDEX_E;

#define BLE_FUNCTION_BEGIN         printf("[BLE] %s %d line begin \n",__FUNCTION__,__LINE__)
#define BLE_FUNCTION_END           printf("[BLE] %s %d line end \n",__FUNCTION__,__LINE__)
#define BLE_LOG(tag, fmt, args...)           {if (tag >= BLE_LOG_DBG)      printf("[BLE][%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ## args);}

INT32 ble_app_send_msg_to_ble(UINT32  sender_id,VOID *data,SIZE_T len, BLE_TO_BLE_MSG_TYPE event);

#endif /*_BLE_H_*/


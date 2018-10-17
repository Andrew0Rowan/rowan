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


/* FILE NAME:  bt_mw_hid.h
 * AUTHOR: zwei chen
 * PURPOSE:
 *      It provides hid  interface to c_bt_mw_hid.c.
 * NOTES:
 */

#ifndef __BT_MW_HID_H__
#define __BT_MW_HID_H__

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "u_bt_mw_hidh.h"

#define             MAX_HID_DEV_NUM                       ((UINT32)     4)
#define             MAX_HID_CONN_NUM                      ((UINT32)     3)

enum
{
    BTMW_HIDH_STATE_EVENT = BTMW_EVT_START(BTWM_ID_HIDH),

    BTMW_HIDH_STATE_MAX,
};  // event


typedef enum
{
    /* hid host local device API events */
    BTMW_HIDH_CONNECTED = 0,
    BTMW_HIDH_CONNECTING,
    BTMW_HIDH_DISCONNECTED,
    BTMW_HIDH_DISCONNECTING,
    BTMW_HIDH_CONNECT_FAIL,
    BTMW_HIDH_DISCONNECT_FAIL,
    BTMW_HIDH_CONNECTION_REJEC,
    BTMW_HIDH_MAX_EVT,
} BTMW_HID_EVENT;  // event


typedef struct BT_MW_HIDH_MSG
{
    BTMW_HID_EVENT event;
    CHAR addr[MAX_BDADDR_LEN];
}tBT_MW_HIDH_MSG;


#if 0
typedef struct _BT_HOGP_Status
{
    UINT8              status;/*0-->idle; 1 -> connecting ; 2--> connected*/
    CHAR               bdAddr[MAX_BDADDR_LEN];/* Bluetooth Address */
}BT_HOGP_STATUS;
#endif
/****************************************************
 * FUNCTION NAME: bluetooth_hid_activate
 * PURPOSE:
 *      The function is used to activate hid
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
//INT32 bluetooth_hid_activate(VOID);

/****************************************************
 * FUNCTION NAME: bluetooth_hid_deactivate
 * PURPOSE:
 *      The function is used to deactivate hid
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
//INT32 bluetooth_hid_deactivate(VOID);

/****************************************************
 * FUNCTION NAME: bluetooth_hidh_init
 * PURPOSE:
 *      The function is used to init hid profile
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */

INT32 bluetooth_hidh_init(BtAppHidhCbk *hidhcbk);

/****************************************************
 * FUNCTION NAME: bluetooth_hid_deinit
 * PURPOSE:
 *      The function is used to deinit hid profile
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_deinit(VOID);

/****************************************************
 * FUNCTION NAME: bluetooth_hid_connect
 * PURPOSE:
 *      The function is used to connect  hid device
 * INPUT:
 *      pbt_addr:device address
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_connect(CHAR *pbt_addr);

/****************************************************
 * FUNCTION NAME: bluetooth_hid_disconnect
 * PURPOSE:
 *      The function is used to disconnect  hid device
 * INPUT:
 *      pbt_addr:device address
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_disconnect(CHAR *pbt_addr);

/****************************************************
 * FUNCTION NAME: bluetooth_hid_disconnect_sync
 * PURPOSE:
 *      The function is used to disconnect hid device
 * INPUT:
 *      no input
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */

INT32 bluetooth_hid_disconnect_sync();

/****************************************************
 * FUNCTION NAME: bluetooth_hid_set_output_report
 * PURPOSE:
 *      The function is used to set output report
 * INPUT:
 *      pbt_addr:device address
 *      preport_data: output report data
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_set_output_report(char *pbt_addr, char *preport_data);

/****************************************************
 * FUNCTION NAME: bluetooth_hid_get_output_report
 * PURPOSE:
 *      The function is used to get output report
 * INPUT:
 *      pbt_addr:device address
 *      reportId: report id
 *     bufferSize : size of the buffer for output data
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_get_output_report(char *pbt_addr, int reportId, int bufferSize);


/****************************************************
 * FUNCTION NAME: bluetooth_hid_set_input_report
 * PURPOSE:
 *      The function is used to set input report
 * INPUT:
 *      pbt_addr:device address
 *      preport_data: input report data
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_set_input_report(char *pbt_addr, char *preport_data);

/****************************************************
 * FUNCTION NAME: bluetooth_hid_get_input_report
 * PURPOSE:
 *      The function is used to get input report
 * INPUT:
 *      pbt_addr:device address
 *      reportId: report id
 *     bufferSize : size of the buffer for input data
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_get_input_report(char *pbt_addr, int reportId, int bufferSize);

/****************************************************
 * FUNCTION NAME: linuxbt_hid_get_feature_report_handler
 * PURPOSE:
 *      The function is used to get feature report
 * INPUT:
 *      pbt_addr:device address
 *      reportId: report id
 *     bufferSize : size of the buffer for feature data
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_get_feature_report(char *pbt_addr, int reportId, int bufferSize);

/****************************************************
 * FUNCTION NAME: bluetooth_hid_set_feature_report
 * PURPOSE:
 *      The function is used to set feature report
 * INPUT:
 *      pbt_addr:device address
 *      preport_data: input report data
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_set_feature_report(char *pbt_addr, char *preport_data);


/****************************************************
 * FUNCTION NAME: bluetooth_hid_get_protocol
 * PURPOSE:
 *      The function is used to get  protocol
 * INPUT:
 *      pbt_addr:device address
 *
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_get_protocol(char *pbt_addr);


/****************************************************
 * FUNCTION NAME: bluetooth_hid_set_protocol
 * PURPOSE:
 *      The function is used to set  protocol
 * INPUT:
 *      pbt_addr:device address
 *      protocol_mode: protocol mode:boot or report mode
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hid_set_protocol(char *pbt_addr, int protocol_mode);

INT32 bluetooth_hid_virtual_unplug(char *pbt_addr);
INT32 bluetooth_hid_send_data(char *pbt_addr, char *data);
/****************************************************
 * FUNCTION NAME: bluetooth_hid_send_control
 * PURPOSE:
 *      The function is used to suspend or exit suspend mode
 * INPUT:
 *      pbt_addr:device address
 *      pcontrol_mode: suspend or exit suspend mode
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
//INT32 bluetooth_hid_send_control(char *pbt_addr, int pcontrol_mode);

/****************************************************
 * FUNCTION NAME: bluetooth_handle_hid_connect_cb
 * PURPOSE:
 *      The function is used for callback for connected
 * INPUT:
 *      bd_addr:device address
 *
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_handle_hid_connected_cb(CHAR *pbt_addr);


/****************************************************
 * FUNCTION NAME: bluetooth_handle_hid_connect_fail_cb
 * PURPOSE:
 *      The function is used for callback for connect failed
 * INPUT:
 *      bd_addr: device address
 *      event: HID connected or disconnected related event
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
    INT32 bluetooth_handle_hid_connect_fail_cb(CHAR *pbt_addr);

/****************************************************
 * FUNCTION NAME: bluetooth_handle_hid_disconnect_cb
 * PURPOSE:
 *      The function is used for callback for disconnected
 * INPUT:
 *      bd_addr:device address
 *
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_handle_hid_disconnected_cb(CHAR *pbt_addr);



#if 0
INT32 get_hogp_connect_state(CHAR *pbt_add);


void set_hogp_connect_state(UINT8 status,CHAR *pbt_add);
#endif

#endif /*__BT_MW_HID_H__*/

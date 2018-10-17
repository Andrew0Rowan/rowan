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


/* FILE NAME:  c_bt_mw_hid.c
 * AUTHOR: zwei chen
 * PURPOSE:
 *      It provides HID HOST  API to APP.
 * NOTES:
 */


//#include "bt_mw_hidh.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bt_mw_common.h"
#include "bluetooth.h"
#include "c_bt_mw_hidh.h"
//#include "bluetooth_sync.h"
#include "bt_mw_hidh.h"
#include "u_bt_mw_hidh.h"



EXPORT_SYMBOL INT32 c_btm_hid_register_callback(BtAppHidhCbk *hidhcbk)
{
    FUNC_ENTRY;
    return bluetooth_hidh_init(hidhcbk);
}


/****************************************************
 * FUNCTION NAME: c_btm_bluetooth_hid_set_output_report
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
EXPORT_SYMBOL INT32 c_btm_bluetooth_hid_set_output_report(char *pbt_addr, char *preport_data)
{
    FUNC_ENTRY;
    return bluetooth_hid_set_output_report(pbt_addr, preport_data);
}


/****************************************************
 * FUNCTION NAME: c_btm_hid_connect
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
EXPORT_SYMBOL INT32 c_btm_hid_connect(char *pbt_addr)
{
    FUNC_ENTRY;
    return bluetooth_hid_connect(pbt_addr);
}

/****************************************************
 * FUNCTION NAME: c_btm_hid_disconnect
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
EXPORT_SYMBOL INT32 c_btm_hid_disconnect(char *pbt_addr)
{
    FUNC_ENTRY;
    return bluetooth_hid_disconnect(pbt_addr);
}

/****************************************************
 * FUNCTION NAME: c_btm_hid_auto_disconnection
 * PURPOSE:
 *      The function is used to disconnect all hid connected device.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_btm_hid_disconnect_all(VOID)
{
    FUNC_ENTRY;
    INT32 u_i4 = BT_SUCCESS;
    INT32 i_ret = BT_SUCCESS;

    return i_ret;
}


/****************************************************
 * FUNCTION NAME: c_btm_hid_set_output_report
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
EXPORT_SYMBOL INT32 c_btm_hid_set_output_report(char *pbt_addr, char *preport_data)
{
    FUNC_ENTRY;
    return bluetooth_hid_set_output_report(pbt_addr, preport_data);
}

/****************************************************
 * FUNCTION NAME: c_btm_hid_get_input_report
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
EXPORT_SYMBOL INT32 c_btm_hid_get_input_report(char *pbt_addr, int reportId, int bufferSize)
{
    FUNC_ENTRY;
    return bluetooth_hid_get_input_report(pbt_addr, reportId, bufferSize);
}

/****************************************************
 * FUNCTION NAME: c_btm_hid_get_output_report
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
EXPORT_SYMBOL INT32 c_btm_hid_get_output_report(char *pbt_addr, int reportId, int bufferSize)
{
    FUNC_ENTRY;
    return bluetooth_hid_get_output_report(pbt_addr, reportId, bufferSize);
}

/****************************************************
 * FUNCTION NAME: c_btm_hid_set_input_report
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
EXPORT_SYMBOL INT32 c_btm_hid_set_input_report(char *pbt_addr, char *preport_data)
{
    FUNC_ENTRY;
    return bluetooth_hid_set_input_report(pbt_addr, preport_data);
}

/****************************************************
 * FUNCTION NAME: c_btm_hid_get_feature_report
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
EXPORT_SYMBOL INT32 c_btm_hid_get_feature_report(char *pbt_addr, int reportId, int bufferSize)
{
    FUNC_ENTRY;
    return bluetooth_hid_get_feature_report(pbt_addr, reportId, bufferSize);
}

/****************************************************
 * FUNCTION NAME: c_btm_hid_set_feature_report
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
EXPORT_SYMBOL INT32 c_btm_hid_set_feature_report(char *pbt_addr, char *preport_data)
{
    FUNC_ENTRY;
    return bluetooth_hid_set_feature_report(pbt_addr, preport_data);
}

#if defined(MTK_LINUX_HIDH_PTS_TEST) && (MTK_LINUX_HIDH_PTS_TEST == TRUE)
/****************************************************
 * FUNCTION NAME: c_btm_hid_virtual_unplug
 * PURPOSE:
 *      The function is used to send virtual unplug request for test
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
EXPORT_SYMBOL INT32 c_btm_hid_virtual_unplug(char *pbt_addr)
{
    FUNC_ENTRY;
    return bluetooth_hid_virtual_unplug(pbt_addr);
}

/****************************************************
 * FUNCTION NAME: c_btm_hid_send_data
 * PURPOSE:
 *      The function is used to send data for test
 * INPUT:
 *      pbt_addr:device address
 *      psend_data: input send data
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_btm_hid_send_data(char *pbt_addr, char *psend_data)
{
    FUNC_ENTRY;
    return bluetooth_hid_send_data(pbt_addr, psend_data);
}
#endif

/****************************************************
 * FUNCTION NAME: c_btm_hid_get_protocol
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
EXPORT_SYMBOL INT32 c_btm_hid_get_protocol(char *pbt_addr)
{
    FUNC_ENTRY;
    return bluetooth_hid_get_protocol(pbt_addr);
}

/****************************************************
 * FUNCTION NAME: c_btm_hid_set_protocol
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
EXPORT_SYMBOL INT32 c_btm_hid_set_protocol(char *pbt_addr, int protocol_mode)
{
    FUNC_ENTRY;
    return bluetooth_hid_set_protocol(pbt_addr, protocol_mode);
}
#if 0
/****************************************************
 * FUNCTION NAME: c_bt_hid_send_control
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
INT32 c_bt_hid_send_control(char *pbt_addr, int pcontrol_mode)
{
    FUNC_ENTRY;
    return bluetooth_hid_send_control(pbt_addr, pcontrol_mode);
}
#endif


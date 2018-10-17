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


/* FILE NAME:  linuxbt_gattc_if.h
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTC operation interface definition to MW higher layer.
 * NOTES:
 */


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/

#ifndef __LINUXBT_GATTC_IF_H__
#define __LINUXBT_GATTC_IF_H__

#include "u_bt_mw_gattc.h"
#include "bluetooth.h"
#include "bt_gatt_client.h"

#if defined(MTK_LINUX_GATT) && (MTK_LINUX_GATT == TRUE)
#include "mtk_bt_gatt_client.h"
#endif


#define LINUXBT_GATTC_APP_UUID  "49557E50-D815-11E4-8830-0800200C9A66"

/**
 * FUNCTION NAME: linuxbt_gattc_init
 * PURPOSE:
 *      The function is used to initialize gatt client inteface.
 * INPUT:
 *      interface            -- gatt client infrace structure
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS             -- Operate success.
 * NOTES:
 *      None
 */
int linuxbt_gattc_init(const btgatt_client_interface_t *interface);

#if defined(MTK_LINUX_GATT) && (MTK_LINUX_GATT == TRUE)
/**
 * FUNCTION NAME: linuxbt_gattc_ex_init
 * PURPOSE:
 *      The function is used to initialize gatt client extentional inteface.
 * INPUT:
 *      ex_interface            -- extentional gatt client infrace structure
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS             -- Operate success.
 * NOTES:
 *      None
 */
int linuxbt_gattc_ex_init(const btgatt_ex_client_interface_t *ex_interface);
#endif

/**
 * FUNCTION NAME: linuxbt_gattc_deinit
 * PURPOSE:
 *      The function is used to deinitialize gatt client.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS             -- Operate success.
 * NOTES:
 *      None
 */
int linuxbt_gattc_deinit(void);

/**
 * FUNCTION NAME: linuxbt_gattc_register_app
 * PURPOSE:
 *      The function is used to register gatt client APP.
 * INPUT:
 *      uuid               -- app uuid
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_register_app(char *pt_uuid);

/**
 * FUNCTION NAME: linuxbt_gattc_unregister_app
 * PURPOSE:
 *      The function is used to deregister gatt client APP.
 * INPUT:
 *      client_if              -- registered app identifer
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_unregister_app(int client_if);

/**
 * FUNCTION NAME: linuxbt_gattc_scan
 * PURPOSE:
 *      The function is used for gatt client scan remote device.
 * INPUT:
 *      start              -- start scan or stop scan. (1:start scan, 0:stop  scan)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_scan(bool start);

/**
 * FUNCTION NAME: linuxbt_gattc_connect
 * PURPOSE:
 *      The function is used for gatt client connect with remote device.
 * INPUT:
 *      client_if              -- registered app identifer
 *      bt_addr                -- remote device bt address
 *      is_direct              -- is direct connection or background connection
 *      transport              -- transport type:(0 : auto, 1 : BREDR, 2 : LE)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_connect(int client_if, char *bt_addr,
                                bool is_direct, int transport);

/**
 * FUNCTION NAME: linuxbt_gattc_disconnect
 * PURPOSE:
 *      The function is used for gatt client disconnect with remote device.
 * INPUT:
 *      client_if              -- registered app identifer
 *      bt_addr                -- remote device bt address
 *      conn_id                -- connection id
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_disconnect(int client_if, char *bt_addr, int conn_id);

/**
 * FUNCTION NAME: linuxbt_gattc_listen
 * PURPOSE:
 *      The function is used for gatt client listening operation.
 * INPUT:
 *      client_if              -- registered app identifer
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_listen(int client_if);

/**
 * FUNCTION NAME: linuxbt_gattc_refresh
 * PURPOSE:
 *      The function is used for gatt client refresh operation.
 * INPUT:
 *      client_if              -- registered app identifer
 *      bt_addr                -- remote device bt address
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_refresh(int client_if, char *bt_addr);

/**
 * FUNCTION NAME: linuxbt_gattc_search_service
 * PURPOSE:
 *      The function is used for gatt client search service on the remote device.
 * INPUT:
 *      conn_id                -- connection id
 *      pt_uuid                -- service uuid
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_search_service(int conn_id, char *pt_uuid);

/**
 * FUNCTION NAME: linuxbt_gattc_get_gatt_db
 * PURPOSE:
 *      The function is used for gatt client to get gatt database on on loacal cache database.
 * INPUT:
 *      conn_id                -- connection id
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_get_gatt_db(int conn_id);

/**
 * FUNCTION NAME: linuxbt_gattc_read_char
 * PURPOSE:
 *      The function is used for gatt client read characteristic from remote device
 * INPUT:
 *      conn_id                -- connection id
 *      char_handle         -- characteristic handle in database
 *      auth_req               -- authentication request flag
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_read_char(int conn_id, int char_handle, int auth_req);

/**
 * FUNCTION NAME: linuxbt_gattc_read_descr
 * PURPOSE:
 *      The function is used for gatt client read char description from remote device
 * INPUT:
 *      conn_id                -- connection id
 *      descr_handle       -- char descriptor handle in database
 *      auth_req              -- authentication request flag
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
 int linuxbt_gattc_read_descr(int conn_id, int descr_handle, int auth_req);

/**
 * FUNCTION NAME: linuxbt_gattc_write_char
 * PURPOSE:
 *      The function is used for gatt client write characteristic to remote device
 * INPUT:
 *      conn_id                -- connection id
 *      char_handle         -- characteristic handle in database
 *      write_type             -- write type(1:WRITE_TYPE_NO_RSP  2:WRITE_TYPE_REQUEST
 *                                                          3:WRITE_TYPE_PREPARE)
 *      len                    -- write characteristic value length
 *      auth_req               -- authentication request flag
 *      value                  -- write characteristic value
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_write_char(int conn_id, int char_handle, int write_type,
                                    int len, int auth_req, char *value);

/**
 * FUNCTION NAME: linuxbt_gattc_write_descr
 * PURPOSE:
 *      The function is used for gatt client write char description to remote device
 * INPUT:
 *      conn_id                -- connection id
 *      descr_handle       -- char descriptor handle in database
 *      write_type           -- write type(1:WRITE_TYPE_NO_RSP  2:WRITE_TYPE_REQUEST  3:WRITE_TYPE_PREPARE)
 *      len                       -- write characteristic value length
 *      auth_req             -- authentication request flag
 *      value                  -- write characteristic value
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_write_descr(int conn_id, int descr_handle, int write_type,
                                     int len, int auth_req, char *value);

/**
 * FUNCTION NAME: linuxbt_gattc_execute_write
 * PURPOSE:
 *      The function is used for gatt client execute write operation
 * INPUT:
 *      conn_id              -- connection id
 *      execute              -- execute write or not
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_execute_write(int conn_id, int execute);

/**
 * FUNCTION NAME: linuxbt_gattc_set_reg_noti
 * PURPOSE:
 *      The function is used for gatt client register notification
 * INPUT:
 *      client_if                -- registered client app identifier
 *      bt_addr                -- remote device bt address
 *      char_handle         -- characteristic handle in database
 *      enable                   -- register or deregister notification. (1:register, 0:deregister)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_set_reg_noti(int client_if, char *bt_addr,
                                      int char_handle, bool enable);

/**
 * FUNCTION NAME: linuxbt_gattc_read_rssi
 * PURPOSE:
 *      The function is used for gatt client read the received signal strength indicator
 * INPUT:
 *      client_if              -- registered client app identifier
 *      bt_addr                -- remote device bt address
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_read_rssi(int client_if, char *bt_addr);

/**
 * FUNCTION NAME: linuxbt_gattc_conn_parameter_update
 * PURPOSE:
 *      The function is used for gatt client to update connection paramters
 * INPUT:
 *      pbt_addr               -- remote device bt address
 *      min_interval           -- the minimum allowed connection interval
 *      max_interval           -- the maximum allowed connection interval
 *      latency                -- the maximum allowed connection latency
 *      timeout                -- define the link supervision timeout
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_conn_parameter_update(char *pbt_addr,
                                                     int min_interval,
                                                     int max_interval,
                                                     int latency,
                                                     int timeout);

/**
 * FUNCTION NAME: linuxbt_gattc_set_scan_parameters
 * PURPOSE:
 *      The function is used for gatt client to set scan paramters
 * INPUT:
 *      scan_interval          -- how frequently the controller should scan
 *      scan_window            -- how long the controller should scan
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_set_scan_parameters(int client_if, int scan_interval,
                                                 int scan_window);

/**
 * FUNCTION NAME: linuxbt_gattc_multi_adv_enable
 * PURPOSE:
 *      The function is used for gatt to enable multiple advertising
 * INPUT:
 *      client_if              -- registered client app identifier
 *      min_interval           -- minimum advertising interval
 *      max_interval           -- maximum advertising interval
 *      adv_type               -- advertising type(0:ADV_IND  1:ADV_DIRECT_IND   2:ADV_SCAN_IND
 *                                                 3:ADV_NONCONN_IND  4:SCAN_RSP)
 *      chnl_map               -- advertising channel map
 *      tx_power               -- transmit power(unit in dBm)
 *      timeout_s              -- not used.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_multi_adv_enable(int client_if,
                                             int min_interval,
                                             int max_interval,
                                             int adv_type,
                                             int chnl_map,
                                             int tx_power,
                                             int timeout_s);

/**
 * FUNCTION NAME: linuxbt_gattc_multi_adv_update
 * PURPOSE:
 *      The function is used for gatt to update multiple advertising parameters
 * INPUT:
 *      client_if              -- registered client app identifier
 *      min_interval           -- minimum advertising interval
 *      max_interval           -- maximum advertising interval
 *      adv_type               -- advertising type(0:ADV_IND  1:ADV_DIRECT_IND   2:ADV_SCAN_IND
 *                                                 3:ADV_NONCONN_IND  4:SCAN_RSP)
 *      chnl_map               -- advertising channel map
 *      tx_power               -- transmit power(unit in dBm)
 *      timeout_s              -- not used.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_multi_adv_update(int client_if,
                                             int min_interval,
                                             int max_interval,
                                             int adv_type,
                                             int chnl_map,
                                             int tx_power,
                                             int timeout_s);

/**
 * FUNCTION NAME: linuxbt_gattc_multi_adv_setdata
 * PURPOSE:
 *      The function is used for gatt to set multiple advertising data
 * INPUT:
 *      client_if              -- registered client app identifier
 *      set_scan_rsp           -- set scan response flag
 *      include_name           -- include device name flag
 *      include_txpower        -- include transmit power flag
 *      appearance             -- appearance data
 *      manufacturer_len       -- manufacturer data length
 *      manufacturer_data      -- manufacturer data
 *      service_data_len       -- service data length
 *      service_data           -- service data
 *      service_uuid_len       -- service uuid length
 *      service_uuid           -- service uuid
 *      le_name_len            -- device name length
 *      le_name                -- device name
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_multi_adv_setdata(int client_if,
                                             uint8_t set_scan_rsp,
                                             uint8_t include_name,
                                             uint8_t include_txpower,
                                             int appearance,
                                             int manufacturer_len,
                                             char* manufacturer_data,
                                             int service_data_len,
                                             char* service_data,
                                             int service_uuid_len,
                                             char* service_uuid);

/**
 * FUNCTION NAME: linuxbt_gattc_multi_adv_disable
 * PURPOSE:
 *      The function is used for gatt to disable multiple advertising
 * INPUT:
 *      client_if              -- registered client app identifier
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_multi_adv_disable(int client_if);

/**
 * FUNCTION NAME: linuxbt_gattc_batchscan_cfg_storage
 * PURPOSE:
 *      The function is used for gatt client to config batch scan storage paramter
 * INPUT:
 *      client_if             --  registered AP identifier
 *      batch_scan_full_max   -- full max storage space(in %) for batch scan
 *      batch_scan_trunc_max   --  truncate max storage space(in %)  for batch scan
 *      batch_scan_notiy_threshold   --  notify threshold value (in %)  for batch scan storage
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_batchscan_cfg_storage(int client_if,
                                                   int batch_scan_full_max,
                                                   int batch_scan_trunc_max,
                                                   int batch_scan_notify_threshold);


/**
 * FUNCTION NAME: linuxbt_gattc_batchscan_enb_batch_scan
 * PURPOSE:
 *      The function is used for gatt client to enable batch scan
 * INPUT:
 *      client_if             --  registered AP identifier
 *      scan_mode       --  scan mode , disable: 0x00, truncate mode enable: 0x01,
 *                                     full mode enable: 0x02, truncate and full mode enable: 0x03
 *      scan_interval    --  how frequently to scan
 *      scan_window    -- how long to scan
 *      addr_type         -- address type, public:0x00, random: 0x01
 *      discard_rule    -- discard rule, discard oldest advertisement:0,
 *                                   discard advertisement with weakest rssi: 1
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_batchscan_enb_batch_scan(int client_if,
                                                         int scan_mode,
                                                         int scan_interval,
                                                         int scan_window,
                                                         int addr_type,
                                                         int discard_rule);

/**
 * FUNCTION NAME: linuxbt_gattc_batchscan_dis_batch_scan
 * PURPOSE:
 *      The function is used for gatt client to disable batch scan
 * INPUT:
 *      client_if             --  registered AP identifier
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_batchscan_dis_batch_scan(int client_if);

/**
 * FUNCTION NAME: linuxbt_gattc_batchscan_read_reports
 * PURPOSE:
 *      The function is used for gatt client to batch scan read reports
 * INPUT:
 *      client_if             --  registered AP identifier
 *      scan_mode       --  scan mode, truncate mode: 1,   full mode: 2
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_batchscan_read_reports(int client_if, int scan_mode);


/**
 * FUNCTION NAME: linuxbt_gattc_scan_filter_param_setup
 * PURPOSE:
 *      The function is used for gatt client to setup scan filter parameters
 * INPUT:
 *      scan_filt_param.client_if      -- registered AP identifier
 *      scan_filt_param.action       -- 0: add   1: delete  2: clear
 *      scan_filt_param.filt_index     -- filter index
 *      scan_filt_param.feat_seln     -- filter selection number,
 *                                                        Bit 0: Set to enable Broadcast Address filter
 *                                                        Bit 1: Set to enable Service Data Change filter
 *                                                        Bit 2: Set to enable Service UUID check
 *                                                        Bit 3: Set to enable Service Solicitation UUID check
 *                                                        Bit 4: Set to enable Local Name check
 *                                                        Bit 5: Set to enable Manufacturer Data Check
 *                                                        Bit 6: Set to enable Service Data Check
 *      scan_filt_param.list_logic_type     -- the logic to list for each filter field
 *      scan_filt_param.filt_logic_type     -- the logic to filter for each filter field,
 *                                                                 it only applicable for (Bit 3~Bit 6) four fields
 *                                                                 of APCF_Feature_Selection
 *      scan_filt_param.rssi_high_thres     -- upper threshold of the rssi value
 *      scan_filt_param.rssi_low_thres     -- lower threshold of the rssi value
 *      scan_filt_param.dely_mode     --   0: immediate    1:on_found   2: batched
 *      scan_filt_param.found_timeout     -- time for firmware to linger and collect
 *                                                                 additional advertisements before reporting,
 *                                                                 valid only if dely_mode is on_found
 *      scan_filt_param.lost_timeout     -- If an advertisement, after being found,
 *                                                              is not seen contiguously for the lost_timeout period,
 *                                                              it will be reported lost. Valid only if dely_mode is on _found
 *      scan_filt_param.found_timeout_cnt   -- count of found timeout
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_scan_filter_param_setup(BT_GATTC_FILT_PARAM_SETUP_T scan_filt_param);

/**
 * FUNCTION NAME: linuxbt_gattc_scan_filter_enable
 * PURPOSE:
 *      The function is used for gatt client to enable scan filter
 * INPUT:
 *      client_if             --  registered AP identifier
 *      enable              --  enable or disable scan filter(1: enable, 0:disable)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_scan_filter_enable(int client_if, bool enable);

/**
 * FUNCTION NAME: linuxbt_gattc_scan_filter_add_remove
 * PURPOSE:
 *      The function is used for gatt client to add or remove device from scan filter
 * INPUT:
 *      client_if             --  registered AP identifier
 *      action               -- 0: add    1: delete  2: clear
 *      filt_type            -- 0: addr    1: service data   2: service uuid   3: solicited service uuid
 *                                   4: local name   5: manufacture data      6: service data pattern
 *      filt_index          -- filter index
 *      company_id       -- company id
 *      company_id_mask    -- the mask of company id
 *      p_uuid    -- bluetooth 128-bit UUID, e.g. "49557E51D81511E488300800200C9A66"
 *      p_uuid_mask    -- the mask of Bluetooth 128-bit UUID
 *      bd_addr    -- bluetooth device address, e.g. "AA:BB:CC:DD:EE:FF"
 *                            [NAP-AA:BB][UAP-CC][LAP-DD:EE:FF]
 *      addr_type    -- address type
 *      data_len    -- length of value
 *      p_data      -- value
 *      mask_len    -- the length of mask
 *      p_mask    -- mask
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_scan_filter_add_remove(int client_if,
                                                     int action,
                                                     int filt_type,
                                                     int filt_index,
                                                     int company_id,
                                                     int company_id_mask,
                                                     const char *p_uuid,
                                                     const char *p_uuid_mask,
                                                     const char *bd_addr,
                                                     char addr_type,
                                                     int data_len,
                                                     char* p_data,
                                                     int mask_len,
                                                     char* p_mask);

/**
 * FUNCTION NAME: linuxbt_gattc_scan_filter_clear
 * PURPOSE:
 *      The function is used for gatt client to clear device from scan filter
 * INPUT:
 *      client_if             --  registered AP identifier
 *      filt_index           --  filter index
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_scan_filter_clear(int client_if, int filt_index);

/**
 * FUNCTION NAME: linuxbt_gattc_get_device_type
 * PURPOSE:
 *      The function is used for gatt client to get device type
 * INPUT:
 *      bd_addr   -- address of discovered device(input), e.g. "AA:BB:CC:DD:EE:FF"
 *                            [NAP-AA:BB][UAP-CC][LAP-DD:EE:FF]
 * OUTPUT:
 *      None
 * RETURN:
 *       0: unknown
 *       1: BR/EDR
 *       2: LE
 *       3: dual mode
 * NOTES:
 *      None
 */
int linuxbt_gattc_get_device_type(char *bt_addr);

/**
 * FUNCTION NAME: linuxbt_gattc_configure_mtu
 * PURPOSE:
 *      The function is used for gatt client to config mtu
 * INPUT:
 *      conn_id             --  identifier of connection
 *      mtu                   --  maximum transmit unit
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_configure_mtu(int conn_id, int mtu);


#if MTK_LINUX_GATTC_LE_NAME
/**
 * FUNCTION NAME: linuxbt_gattc_set_local_le_name
 * PURPOSE:
 *      The function is used for gatt to set local le device name
 * INPUT:
 *      client_if             --  registered AP identifier
 *      le_name           --  local le device name
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_set_local_le_name(int client_if, char *name);
#endif

#if MTK_LINUX_GATTC_RPA
/**
 * FUNCTION NAME: linuxbt_gattc_get_local_adv_rpa
 * PURPOSE:
 *      The function is used for gatt to get local advertising rpa
 * INPUT:
 *      client_if             --  registered AP identifier
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_get_local_adv_rpa(int client_if);
#endif

#if MTK_LINUX_GATTC_DISC_MODE
/**
 * FUNCTION NAME: linuxbt_gattc_set_local_disc_mode
 * PURPOSE:
 *      The function is used for gatt to set advertising discoverable mode
 * INPUT:
 *      client_if             --  registered AP identifier
 *      disc_mode        --  discoverable mode (default:0,
 *                                     6:BTM_BLE_GEN_DISC_FLAG  | BTM_BLE_BREDR_NOT_SPT)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
int linuxbt_gattc_set_local_disc_mode(int client_if, int disc_mode);
#endif

#if MTK_LINUX_GATTC_PTS_TEST
int linuxbt_gattc_read_using_char_uuid(int conn_id,
                                                 int start_handle,
                                                 int end_handle,
                                                 char *uuid,
                                                 int auth_req);

int linuxbt_gattc_read_long_characteristic(int conn_id, uint16_t handle,
                                                     uint16_t offset, int auth_req);

int linuxbt_gattc_read_multi_characteristic(int conn_id, uint8_t num_attr,
                                                      uint16_t *handles, int auth_req);

int linuxbt_gattc_read_long_descr(int conn_id, uint16_t handle,
                                          uint16_t offset, int auth_req);

int linuxbt_gattc_write_long_characteristic(int conn_id, uint16_t handle, int write_type,
                                         int len, uint16_t offset, int auth_req, char* p_value);

int linuxbt_gattc_write_long_descr(int conn_id, uint16_t handle, int write_type,
                                         int len, uint16_t offset, int auth_req, char* p_value);

int linuxbt_gattc_set_pts_flag(uint8_t pts_flag);
#endif

/*for test mode interface*/
int linuxbt_gattc_test_command(int command, BT_GATTC_TEST_PARAMS_T test_params);
#endif /* __LINUXBT_GATTC_IF_H__ */

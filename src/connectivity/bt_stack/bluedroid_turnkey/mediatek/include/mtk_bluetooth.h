/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MTK_BLUETOOTH_H
#define MTK_BLUETOOTH_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <hardware/hardware.h>
#include <hardware/bluetooth.h>

__BEGIN_DECLS

#define BT_PROFILE_GAP_EX_ID "gap_ex"
#define BT_PROFILE_GATT_EX_ID "gatt_ex"
#define BT_PROFILE_AVRCP_EX_ID "avrcp_ctrl_ex"
#define BT_PROFILE_AVRCP_TG_EX_ID "avrcp_ex"
#define BT_PROFILE_HANDSFREE_CLIENT_EX_ID "hfclient_ex"
#define BT_PROFILE_ADVANCED_AUDIO_EXT_ID "a2dp_ext"
#define BT_PROFILE_ADVANCED_AUDIO_SRC_EXT_ID "a2dp_src_ext"

/** Bluetooth Adapter State */
#define BT_STATE_HCI_TIMEOUT (BT_STATE_OFF + 1)

/** TODO: Add callbacks for Link Up/Down and other generic
  *  notifications/callbacks */
typedef void (*get_rssi_callback)(bt_status_t status, bt_bdaddr_t *remote_bd_addr, int rssi_value);
typedef void (*send_hci_callback)(bt_status_t status, int cmd_length, uint8_t *event );
/** Bluetooth ACL disconnection reason callback */
typedef void (*acl_disconnect_reason_callback)(bt_bdaddr_t *remote_bd_addr, uint8_t reason);

/** Bluetooth DM extened callback structure. */
typedef struct {
    /** set to sizeof(btgap_ex_callbacks_t) */
    size_t size;
    get_rssi_callback get_rssi_cb;
    acl_disconnect_reason_callback acl_disconnect_reason;
} btgap_ex_callbacks_t;


/** Represents the standard Bluetooth DM extened interface. */
typedef struct {
    /** set to sizeof(btgap_ex_interface_t) */
    size_t size;

    /**
     * Opens the interface and provides the callback routines
     * to the implemenation of this interface.
     */
    int (*init)(btgap_ex_callbacks_t* callbacks );

    int (* get_rssi)(const bt_bdaddr_t *bd_addr);

    int (* send_hci)(uint8_t *buf, uint8_t len);
} btgap_ex_interface_t;

__END_DECLS

#endif /* MTK_BLUETOOTH_H */

/*
 * Copyright (C) 2013 The Android Open Source Project
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


#ifndef MTK_BT_GATT_CLIENT_H
#define MTK_BT_GATT_CLIENT_H

#include <stdint.h>
#include <hardware/bt_gatt_types.h>
#include <hardware/bt_common_types.h>
#include <hardware/bt_gatt_client.h>

__BEGIN_DECLS

typedef void (*get_adv_rpa_callback)(int client_if, int status, bt_bdaddr_t* rpa);

typedef struct {
    get_adv_rpa_callback get_adv_rpa_cb;
} btgatt_ex_client_callbacks_t;

/** Represents the standard BT-GATT client interface. */
typedef struct {
    /** set local lename*/
    bt_status_t (*set_local_le_name)(int client_if, char *p_name);

    /** get local rpa address*/
    bt_status_t (*get_adv_rpa)(int client_if);

    /** set discoverable mode*/
    bt_status_t (*set_discoverable_mode)(int client_if, int disc_mode);

    /** read using characteristic uuid*/
    bt_status_t (*read_using_characteristic_uuid)(int conn_id, uint16_t start_handle,
         uint16_t end_handle, bt_uuid_t *char_uuid, int auth_req);
    /** read long characteristic*/
    bt_status_t (*read_long_characteristic)(int conn_id, uint16_t handle, uint16_t offset,
        int auth_req);

    /** read multiple characteristic*/
    bt_status_t (*read_multi_characteristic)(int conn_id, uint8_t num_attr, uint16_t *handles,
        int auth_req);

    /** read long char descr*/
    bt_status_t (*read_long_char_descr)(int conn_id, uint16_t handle, uint16_t offset,
        int auth_req);

    /** write long char*/
    bt_status_t (*write_long_char)(int conn_id, uint16_t handle, int write_type, int len,
        uint16_t offset, int auth_req, char *p_value);

    /** write long char descr*/
    bt_status_t (*write_long_char_descr)(int conn_id, uint16_t handle, int write_type, int len,
        uint16_t offset, int auth_req, char *p_value);

    /** pts test mode flag*/
    bt_status_t (*set_pts_test_flag)(uint8_t pts_flag);
} btgatt_ex_client_interface_t;

__END_DECLS

#endif /* MTK_BT_GATT_CLIENT_H */

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


#ifndef MTK_BT_GATT_H
#define MTK_BT_GATT_H

#include <stdint.h>
#include <hardware/bt_gatt.h>
#include "mtk_bt_gatt_client.h"
#include <hardware/bt_gatt_server.h>

__BEGIN_DECLS

/** BT-GATT extended callbacks */
typedef struct {
    /** Set to sizeof(btgatt_ex_callbacks_t) */
    size_t size;

    /** GATT Client extended callbacks */
    const btgatt_ex_client_callbacks_t* ex_client;
} btgatt_ex_callbacks_t;

/** Represents the standard Bluetooth GATT extended interface. */
typedef struct {
    /** Set to sizeof(btgatt_ex_interface_t) */
    size_t          size;

    /**
     * Initializes the extended interface and provides callback routines
     */
    bt_status_t (*init)( const btgatt_ex_callbacks_t* callbacks );

    /** Pointer to the GATT client extended interface methods.*/
    const btgatt_ex_client_interface_t* ex_client;
} btgatt_ex_interface_t;

__END_DECLS

#endif /* MTK_BT_GATT_H */

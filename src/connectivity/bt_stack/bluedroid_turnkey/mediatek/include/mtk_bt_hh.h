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

#ifndef MTK_BT_HH_H
#define MTK_BT_HH_H


#include <stdint.h>

__BEGIN_DECLS;

#define BT_PROFILE_HIDHOST_EX_ID "hidhost_ex"

/** Callback for get remote device name.
 */
typedef void (*bthh_get_dev_name_callback)(bt_bdaddr_t *remote_bd_addr, char* remote_name);


/** BT-HH extened callback structure. */
typedef struct {
    /** set to sizeof(BtHfCallbacks) */
    size_t size;
    bthh_get_dev_name_callback get_remote_name_cb;

} bthh_ex_callbacks_t;

/** Represents the standard BT-HH extended interface. */
typedef struct {

    /** set to sizeof(bthh_ex_interface_t) */
    size_t          size;

    /**
     * Register the extended interface
     */
    bt_status_t (*btif_hh_init_ex)( const bthh_ex_callbacks_t *callbacks );

} bthh_ex_interface_t;

__END_DECLS

#endif /* MTK_BT_HH_H */



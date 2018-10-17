/******************************************************************************
 *
 *  Copyright (C) 2010 MediaTek Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/*******************************************************************************
 *
 *  Filename:      btif_b3ds.h
 *
 *  Description:   Bluetooth 3d Synchronization Interface
 *
 *******************************************************************************/

#ifndef BTIF_B3DS_H
#define BTIF_B3DS_H

#include <hardware/bt_b3ds.h>

btb3ds_interface_t *btif_b3ds_interface();
void btif_b3ds_init(UINT8 enable_legacy_reference_protocol);
void btif_b3ds_cleanup();

#endif


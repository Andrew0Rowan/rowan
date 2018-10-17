/******************************************************************************
 *
 *  Copyright (C) 2015 Google, Inc.
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

#pragma once

#include <stdbool.h>

#include "btcore/include/bdaddr.h"

#ifdef MTK_BLUEDROID_PATCH
#include "mdroid_buildcfg.h"
#endif

static const char INTEROP_MODULE[] = "interop_module";

// NOTE:
// Only add values at the end of this enum and do NOT delete values
// as they may be used in dynamic device configuration.
typedef enum {
  // Disable secure connections
  // This is for pre BT 4.1/2 devices that do not handle secure mode
  // very well.
  INTEROP_DISABLE_LE_SECURE_CONNECTIONS = 0,

  // Some devices have proven problematic during the pairing process, often
  // requiring multiple retries to complete pairing. To avoid degrading the user
  // experience for those devices, automatically re-try pairing if page
  // timeouts are received during pairing.
  INTEROP_AUTO_RETRY_PAIRING,

  // Devices requiring this workaround do not handle Bluetooth Absolute Volume
  // control correctly, leading to undesirable (potentially harmful) volume levels
  // or general lack of controlability.
  INTEROP_DISABLE_ABSOLUTE_VOLUME,

  // Disable automatic pairing with headsets/car-kits
  // Some car kits do not react kindly to a failed pairing attempt and
  // do not allow immediate re-pairing. Blacklist these so that the initial
  // pairing attempt makes it to the user instead.
  INTEROP_DISABLE_AUTO_PAIRING,

  // Use a fixed pin for specific keyboards
  // Keyboards should use a variable pin at all times. However, some keyboards
  // require a fixed pin of all 0000. This workaround enables auto pairing for
  // those keyboards.
  INTEROP_KEYBOARD_REQUIRES_FIXED_PIN,

  // Some headsets have audio jitter issues because of increased re-transmissions as the
  // 3 Mbps packets have a lower link margin, and are more prone to interference. We can
  // disable 3DH packets (use only 2DH packets) for the ACL link to improve sensitivity
  // when streaming A2DP audio to the headset. Air sniffer logs show reduced
  // re-transmissions after switching to 2DH packets.
  //
  // Disable 3Mbps packets and use only 2Mbps packets for ACL links when streaming audio.
  INTEROP_2MBPS_LINK_ONLY,

#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
  //Some device like Arc Touch BT Mouse will behave abnormally if their required interval
  //which is less than BTM_BLE_CONN_INT_MIN_LIMIT is rejected
  INTEROP_MTK_LE_CONN_INT_MIN_LIMIT_ACCEPT,
  INTEROP_MTK_HOGP_CONN_INT_UPDATE_ADJUST,

  //Some device like Casio watch request a minor link supervision timeout which can cause
  //the link timeout frequently. So adjust their link supervision timeout to default value
  INTEROP_MTK_LE_CONN_TIMEOUT_ADJUST,

  // Sony Ericsson HBH-DS205 has some special request for
  // opening the sco time, so work around for this device
  // (Nexus has the same problem with this device).
  INTEROP_MTK_HFP_DEALY_OPEN_SCO,

  // Some HID devices have issue if SDP is initiated while HID connection is in progress
  INTEROP_MTK_HID_DISABLE_SDP,

  // Some devices can't use MSBC codec
  INTEROP_MTK_HFP_15_ESCO_NOT_USE_MSBC,

  // MTK bluedroid is necessary to initiate Av connect even when sdp fails to a particular carkit
  INTEROP_MTK_AV_CONNECT_DO_SDP_EVEN_FAIL,

  // In particular case, it not relase key when push the FF key.
  INTEROP_MTK_AVRCP_DO_RELEASE_KEY,

  // Some special device want perform START cmd itself first
  // If it not send START cmd, will close current link.
  // So for this special device, we need delay send A2DP START cmd
  // which from DUT to receive the special device cmd.
  INTEROP_MTK_A2DP_DELAY_START_CMD,

  // In particular case, IOT device has wrong song position when FF/REW.
  // Disable song position for IOT device.
  INTEROP_MTK_AVRCP_DISABLE_SONG_POS,

  // Some devices support hfp 1.5 but not use eSCO connection
  INTEROP_MTK_HFP_15_NOT_USE_ESCO,

  // Some IOT devices can not work with SCSM-T normmaly
  // Do not select SCMS-T as set configurate
  INTEROP_MTK_A2DP_NOT_SET_SCMT,

  // Some CT devices support AVRCP version 1.4 instead of 1.5
  // Send the AVRCP version as 1.4 for these devices
  INTEROP_MTK_AVRCP_15_TO_14,

  // Some CT devices have IOT issue with AVRCP 1.5 device
  // Send the AVRCP version as 1.3 for these devices
  INTEROP_MTK_AVRCP_15_TO_13,

  // Not do sniif mode for HID profile.
  INTEROP_MTK_HID_NOT_DO_SNIFF_SUBRATING,

 // Some headset have IOT issue that Peer device only close A2dp data channel
 // Without closing signaling channel which will result to follow-up connection statemachine confused
  INTEROP_MTK_CLOSE_AVDTP_SIG_CH,

 //Some mouse will request for master role,but DUT will send role switch after ACL link setup
 //This process will block hid device send data to DUT and mose will not smooth
  INTEROP_MTK_HID_NOT_SET_SLAVE,

 //Some devices may reconnect DUT after whole chip reset, which maybe cause ACL collision and
 //hen HFP setup fail, so extend the timer to reduce the conllision.
  INTEROP_MTK_HFP_ACL_COLLISION,
#endif
} interop_feature_t;

// Check if a given |addr| matches a known interoperability workaround as identified
// by the |interop_feature_t| enum. This API is used for simple address based lookups
// where more information is not available. No look-ups or random address resolution
// are performed on |addr|.
bool interop_match_addr(const interop_feature_t feature, const bt_bdaddr_t *addr);

// Check if a given remote device |name| matches a known interoperability workaround.
// Name comparisons are case sensitive and do not allow for partial matches. As in, if
// |name| is "TEST" and a workaround exists for "TESTING", then this function will
// return false. But, if |name| is "TESTING" and a workaround exists for "TEST", this
// function will return true.
// |name| cannot be null and must be null terminated.
bool interop_match_name(const interop_feature_t feature, const char *name);

// Add a dynamic interop database entry for a device matching the first |length| bytes
// of |addr|, implementing the workaround identified by |feature|. |addr| may not be
// null and |length| must be greater than 0 and less than sizeof(bt_bdaddr_t).
// As |interop_feature_t| is not exposed in the public API, feature must be a valid
// integer representing an optoin in the enum.
void interop_database_add(const uint16_t feature, const bt_bdaddr_t *addr, size_t length);

// Clear the dynamic portion of the interoperability workaround database.
void interop_database_clear(void);

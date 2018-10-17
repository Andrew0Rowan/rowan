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

#include "device/include/interop.h"

typedef struct {
  bt_bdaddr_t addr;
  size_t length;
  interop_feature_t feature;
} interop_addr_entry_t;

static const interop_addr_entry_t interop_addr_database[] = {
  // Nexus Remote (Spike)
  // Note: May affect other Asus brand devices
  {{{0x08, 0x62, 0x66,      0,0,0}}, 3, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},
  {{{0x38, 0x2c, 0x4a, 0xc9,  0,0}}, 4, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},
  {{{0x38, 0x2c, 0x4a, 0xe6,  0,0}}, 4, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},
  {{{0x54, 0xa0, 0x50, 0xd9,  0,0}}, 4, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},
  {{{0xac, 0x9e, 0x17,      0,0,0}}, 3, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},
  {{{0xf0, 0x79, 0x59,      0,0,0}}, 3, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},

  // Ausdom M05 - unacceptably loud volume
  {{{0xa0, 0xe9, 0xdb,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // BMW car kits (Harman/Becker)
  {{{0x9c, 0xdf, 0x03,      0,0,0}}, 3, INTEROP_AUTO_RETRY_PAIRING},

  // Flic smart button
  {{{0x80, 0xe4, 0xda, 0x70,  0,0}}, 4, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},

  // iKross IKBT83B HS - unacceptably loud volume
  {{{0x00, 0x14, 0x02,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // Jabra EXTREME 2 - unacceptably loud volume
  {{{0x1c, 0x48, 0xf9,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // JayBird BlueBuds X - low granularity on volume control
  {{{0x44, 0x5e, 0xf3,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},
  {{{0xd4, 0x9c, 0x28,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // JayBird Family
  {{{0x00, 0x18, 0x91,      0,0,0}}, 3, INTEROP_2MBPS_LINK_ONLY},

  // LG Tone HBS-730 - unacceptably loud volume
  {{{0x00, 0x18, 0x6b,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},
  {{{0xb8, 0xad, 0x3e,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // LG Tone HV-800 - unacceptably loud volume
  {{{0xa0, 0xe9, 0xdb,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // Motorola Key Link
  {{{0x1c, 0x96, 0x5a,      0,0,0}}, 3, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},

  // Mpow Cheetah - unacceptably loud volume
  {{{0x00, 0x11, 0xb1,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // Nissan car kits (ALPS) - auto-pairing fails and rejects next pairing
  {{{0x34, 0xc7, 0x31,      0,0,0}}, 3, INTEROP_DISABLE_AUTO_PAIRING},

  // SOL REPUBLIC Tracks Air - unable to adjust volume back off from max
  {{{0xa4, 0x15, 0x66,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // Subaru car kits (ALPS) - auto-pairing fails and rejects next pairing
  {{{0x00, 0x07, 0x04,      0,0,0}}, 3, INTEROP_DISABLE_AUTO_PAIRING},
  {{{0xe0, 0x75, 0x0a,      0,0,0}}, 3, INTEROP_DISABLE_AUTO_PAIRING},

  // Swage Rokitboost HS - unacceptably loud volume
  {{{0x00, 0x14, 0xf1,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // VW Car Kit - not enough granularity with volume
  {{{0x00, 0x26, 0x7e,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},
  {{{0x90, 0x03, 0xb7,      0,0,0}}, 3, INTEROP_DISABLE_ABSOLUTE_VOLUME},

  // Unknown keyboard (carried over from auto_pair_devlist.conf)
  {{{0x00, 0x0F, 0xF6,      0,0,0}}, 3, INTEROP_KEYBOARD_REQUIRES_FIXED_PIN},

#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
  // Jaybird Family
  {{{0x00, 0x18, 0x91,       0,0,0}}, 3, INTEROP_2MBPS_LINK_ONLY},

  //BSM mouse
  {{{0x00, 0x1b, 0xdc,       0,0,0}}, 3, INTEROP_MTK_HOGP_CONN_INT_UPDATE_ADJUST},
  //CASIO watch
  {{{0xff, 0x40, 0x3a,       0,0,0}}, 3, INTEROP_MTK_LE_CONN_TIMEOUT_ADJUST},
  {{{0xda, 0x58, 0x98,       0,0,0}}, 3, INTEROP_MTK_LE_CONN_TIMEOUT_ADJUST},
  {{{0xc2, 0x80, 0x29,       0,0,0}}, 3, INTEROP_MTK_LE_CONN_TIMEOUT_ADJUST},
  {{{0xff, 0x74, 0xe1,       0,0,0}}, 3, INTEROP_MTK_LE_CONN_TIMEOUT_ADJUST},
  {{{0xd9, 0xe6, 0xea,       0,0,0}}, 3, INTEROP_MTK_LE_CONN_TIMEOUT_ADJUST},

  // INTEROP_MTK_HFP_DEALY_OPEN_SCO
  // {0x00, 0x1E, 0xDC}   /* DS205 */
  // {0x00, 0x58, 0x50}    /* BELKIN */
  {{{0x00, 0x1E, 0xDC,       0,0,0}}, 3, INTEROP_MTK_HFP_DEALY_OPEN_SCO},
  {{{0x00, 0x58, 0x50,       0,0,0}}, 3, INTEROP_MTK_HFP_DEALY_OPEN_SCO},

  // INTEROP_MTK_HID_DISABLE_SDP
  // {0x04, 0x0C, 0xCE},  /* Apple Magic Mouse */
  // {0x00, 0x07, 0x61},  /* Bluetooth Laser Travel Mouse */
  // {0x00, 0x1d, 0xd8},  /* Microsoft Bluetooth Notebook Mouse 5000 */
  // {0x00, 0x1f, 0x20},  /* Logitech MX Revolution Mouse */
  // {0x6c, 0x5d, 0x63},  /* Rapoo 6080 mouse */
  // {0x28, 0x18, 0x78}   /* Microsoft Sculpt Touch Mouse */
  {{{0x04, 0x0C, 0xCE,       0,0,0}}, 3, INTEROP_MTK_HID_DISABLE_SDP},
  {{{0x00, 0x07, 0x61,       0,0,0}}, 3, INTEROP_MTK_HID_DISABLE_SDP},
  {{{0x00, 0x1d, 0xd8,       0,0,0}}, 3, INTEROP_MTK_HID_DISABLE_SDP},
  {{{0x00, 0x1f, 0x20,       0,0,0}}, 3, INTEROP_MTK_HID_DISABLE_SDP},
  {{{0x6c, 0x5d, 0x63,       0,0,0}}, 3, INTEROP_MTK_HID_DISABLE_SDP},
  {{{0x28, 0x18, 0x78,       0,0,0}}, 3, INTEROP_MTK_HID_DISABLE_SDP},

  // INTEROP_MTK_HID_NOT_SET_SLAVE
  // {0x6c, 0x5d, 0x63},  /* Rapoo 6610 mouse */
  {{{0x6c, 0x5d, 0x63,       0,0,0}}, 3, INTEROP_MTK_HID_NOT_SET_SLAVE},

   // INTEROP_MTK_AV_CONNECT_DO_SDP_EVEN_FAIL
   // {00:1e:3d}, // ALPS ELECTRIC CO.,LTD.
   // {00:1b:fb}, // ALPS ELECTRIC CO.,LTD.
   // {00:26:7e}, // Parrot SA
  {{{0x00, 0x1e, 0x3d,       0,0,0}}, 3, INTEROP_MTK_AV_CONNECT_DO_SDP_EVEN_FAIL},
  {{{0x00, 0x1b, 0xfb,       0,0,0}}, 3, INTEROP_MTK_AV_CONNECT_DO_SDP_EVEN_FAIL},
  {{{0x00, 0x26, 0x7e,       0,0,0}}, 3, INTEROP_MTK_AV_CONNECT_DO_SDP_EVEN_FAIL},

   // INTEROP_MTK_AVRCP_DO_RELEASE_KEY
   // {0x00, 0x1e, 0xb2},  /* MTS255 */
   {{{0x00, 0x1e, 0xb2,       0,0,0}}, 3, INTEROP_MTK_AVRCP_DO_RELEASE_KEY},

   // INTEROP_MTK_A2DP_DELAY_START_CMD
   // {0x00, 0x17, 0x53}   /* Tiggo5 */
   // {0x00, 0x13, 0x04}   /* CASKA */
   {{{0x00, 0x17, 0x53,       0,0,0}}, 3, INTEROP_MTK_A2DP_DELAY_START_CMD},
   {{{0x00, 0x13, 0x04,       0,0,0}}, 3, INTEROP_MTK_A2DP_DELAY_START_CMD},

   // INTEROP_MTK_AVRCP_DISABLE_SONG_POS
   // {0x00, 0x0e, 0x9f},  /* Toyota Touch&Go */
   {{{0x00, 0x0e, 0x9f,       0,0,0}}, 3, INTEROP_MTK_AVRCP_DISABLE_SONG_POS},

   // INTEROP_MTK_HFP_15_NOT_USE_ESCO
   // {0x00, 0x58, 0x76},  /* BT800 */
   {{{0x00, 0x58, 0x76,       0,0,0}}, 3, INTEROP_MTK_HFP_15_NOT_USE_ESCO},

   // INTEROP_MTK_A2DP_NOT_SET_SCMT
   // {0x00, 0x12, 0x6F}   /* Bury CC9060 */
   // {0x00, 0x1C, 0xA4}   /* Sony Ericsson HBH-DS980*/
   // {0x9C, 0xDF, 0x03}   /* BMW 02581*/
   // {0x00, 0x0a, 0x08}   /* BMW 525*/
   // {0x00, 0x16, 0x94}   /* MM 550*/
   {{{0x00, 0x12, 0x6F,       0,0,0}}, 3, INTEROP_MTK_A2DP_NOT_SET_SCMT},
   {{{0x00, 0x1C, 0xA4,       0,0,0}}, 3, INTEROP_MTK_A2DP_NOT_SET_SCMT},
   {{{0x9C, 0xDF, 0x03,       0,0,0}}, 3, INTEROP_MTK_A2DP_NOT_SET_SCMT},
   {{{0x00, 0x0a, 0x08,       0,0,0}}, 3, INTEROP_MTK_A2DP_NOT_SET_SCMT},
   {{{0x00, 0x16, 0x94,	      0,0,0}}, 3, INTEROP_MTK_A2DP_NOT_SET_SCMT},

   // INTEROP_MTK_AVRCP_15_TO_14
   // {0x34, 0xC7, 0x31},  /* AUDI MIB Standard */
   // {0x00, 0x07, 0x04},  /* VW MIB Standard */
   // {0x00, 0x26, 0xB4},  /* MyFord Touch Gen2 */
   // {0x10, 0x08, 0xC1},  /* GEN 2.0 PREM */
   // {0x04, 0x98, 0xF3},  /* VW MIB Entry */
   // {0x64, 0xD4, 0xBD},  /* HONDA CAN2BENCH */
   // {0x0C, 0xD9, 0xC1},  /* HONDA CAN2BENCH-02 */
   // {0x18, 0x6D, 0x99},  /* GRANDUER MTS */
   // {0xFC, 0x62, 0xB9},  /* VW Golf VII */
   // {0x90, 0x03, 0xB7},  /* VW Jetta TSI 2013 */
   // {0x00, 0x26, 0x7E}   /* VW Jetta TSI 2012 */
   {{{0x34, 0xC7, 0x31,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0x00, 0x07, 0x04,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0x00, 0x26, 0xB4,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0x10, 0x08, 0xC1,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0x04, 0x98, 0xF3,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0x64, 0xD4, 0xBD,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0x0C, 0xD9, 0xC1,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0x18, 0x6D, 0x99,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0xFC, 0x62, 0xB9,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0x90, 0x03, 0xB7,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},
   {{{0x00, 0x26, 0x7E,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_14},

   // INTEROP_MTK_AVRCP_15_TO_13
   // {0x00, 0x1D, 0xBA},  /* JVC carkit */
   // {0x00, 0x1E, 0xB2},  /* AVN 3.0 Hyundai */
   // {0x00, 0x0E, 0x9F},  /* Porshe car kit */
   // {0x00, 0x13, 0x7B},  /* BYOM Opel */
   // {0x68, 0x84, 0x70},  /* KIA MOTOR */
   // {0x00, 0x21, 0xCC},  /* FORD FIESTA */
   // {0x30, 0x14, 0x4A},  /* Mini Cooper */
   // {0x38, 0xC0, 0x96},  /* Seat Leon */
   // {0x00, 0x54, 0xAF},  /* Chrysler */
   // {0x04, 0x88, 0xE2},  /* BeatsStudio Wireless */
   // {0x9C, 0xDF, 0x03},  /* BMW 2012 carkit */
   // {0xA8, 0x54, 0xB2},  /* BMW 2015 carkit */
   // {0x94, 0x44, 0x44},  /* AVN1.0 K9 */
   // {0x00, 0x05, 0xC9},  /* FS AVN */
   // {0xA0, 0x14, 0x3D},  /* VW Sharen */
   // {0xE0, 0x75, 0x0A},  /* VW GOLF */
   // {0x10, 0x08, 0xC1},  /* Hyundai SantaFe */
   // {0x00, 0x21, 0xCC},  /* FORD SYNC TDK */
   // {0x00, 0x0A, 0x30},  /* Honda TFT */
   // {0x00, 0x1E, 0x43},  /* AUDI MMI 3G+ */
   // {0x00, 0x18, 0x09},  /* AT-PHA05BT */
   // {0xC8, 0x02, 0x10},  /* KIA SportageR 2015 */
   // {0x34, 0xB1, 0xF7},  /* G-BOOK 2013 */
   // {0x7C, 0x66, 0x9D},  /* G-BOOK 2014 */
   // {0x00, 0x09, 0x93},  /* Nissan Altima 2014 */
   // {0x04, 0xF8, 0xC2}   /* HAVAL H2 */
   {{{0x00, 0x1D, 0xBA,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x1E, 0xB2,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x0E, 0x9F,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x13, 0x7B,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x68, 0x84, 0x70,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x21, 0xCC,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x30, 0x14, 0x4A,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x38, 0xC0, 0x96,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x54, 0xAF,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x04, 0x88, 0xE2,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x9C, 0xDF, 0x03,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0xA8, 0x54, 0xB2,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x94, 0x44, 0x44,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x05, 0xC9,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0xA0, 0x14, 0x3D,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0xE0, 0x75, 0x0A,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x10, 0x08, 0xC1,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x21, 0xCC,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x0A, 0x30,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x1E, 0x43,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x18, 0x09,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0xC8, 0x02, 0x10,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x34, 0xB1, 0xF7,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x7C, 0x66, 0x9D,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x00, 0x09, 0x93,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},
   {{{0x04, 0xF8, 0xC2,       0,0,0}}, 3, INTEROP_MTK_AVRCP_15_TO_13},

   // INTEROP_MTK_HID_NOT_DO_SNIFF_SUBRATING
   // {0x54, 0x46, 0x6b},  /* JW MT002 Bluetooth Mouse */
   // {0x6c, 0x5d, 0x63},  /* Rapoo 6610 Bluetooth Mouse */
   // /*LMP version&subversion  - 5, 8721 & LMP Manufacturer - 15*/
   {{{0x54, 0x46, 0x6b,       0,0,0}}, 3, INTEROP_MTK_HID_NOT_DO_SNIFF_SUBRATING},
   {{{0x6c, 0x5d, 0x63,       0,0,0}}, 3, INTEROP_MTK_HID_NOT_DO_SNIFF_SUBRATING},

   // INTEROP_MTK_CLOSE_AVDTP_SIG_CH
   // {0x00, 0x18, 0x6b},  /* LG HBM-280 */
   // {0x00, 0x0B, 0xD5},  /* JABRA DRIVE*/
   {{{0x00, 0x18, 0x6b,       0,0,0}}, 3, INTEROP_MTK_CLOSE_AVDTP_SIG_CH},
   {{{0x00, 0x0B, 0xD5,       0,0,0}}, 3, INTEROP_MTK_CLOSE_AVDTP_SIG_CH},

   // INTEROP_MTK_HFP_ACL_COLLISION
   // {0x48, 0xc1, 0xac}, /*PLT M155*/ will reconnect DUT when BT chip reset.
   // {0x00, 0x0d, 0x18}, /*S1/X3-M*/ Carkit will reconnect.
   // {0x00, 0x1E, 0x7C}, /*Philips SHB9000*/ Headset will reconnect.
   // {0x74, 0xDE, 0x2B}, /*Jabra Clipper*/ HFP connect collision.
   {{{0x48, 0xC1, 0xAC,       0,0,0}}, 3, INTEROP_MTK_HFP_ACL_COLLISION},
   {{{0x00, 0x0D, 0x18,       0,0,0}}, 3, INTEROP_MTK_HFP_ACL_COLLISION},
   {{{0x00, 0x1E, 0x7C,       0,0,0}}, 3, INTEROP_MTK_HFP_ACL_COLLISION},
   {{{0x74, 0xDE, 0x2B,       0,0,0}}, 3, INTEROP_MTK_HFP_ACL_COLLISION},

   // Huitong BLE Remote
   {{{0x7c, 0x66, 0x9d,       0,0,0}}, 3, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},
#endif
};

typedef struct {
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
  // Modify for long name device.
  char name[30];
#else
  char name[20];
#endif
  size_t length;
  interop_feature_t feature;
} interop_name_entry_t;

static const interop_name_entry_t interop_name_database[] = {
  // Carried over from auto_pair_devlist.conf migration
  {"Audi",    4, INTEROP_DISABLE_AUTO_PAIRING},
  {"BMW",     3, INTEROP_DISABLE_AUTO_PAIRING},
  {"Parrot",  6, INTEROP_DISABLE_AUTO_PAIRING},
  {"Car",     3, INTEROP_DISABLE_AUTO_PAIRING},

#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
  {"honor zero-", 11, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},
  {"小米蓝牙遥控", 18, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},
  {"Arc Touch BT Mouse", 18, INTEROP_MTK_LE_CONN_INT_MIN_LIMIT_ACCEPT},
  {"BSMBB09DS", 9, INTEROP_MTK_HOGP_CONN_INT_UPDATE_ADJUST},
  {"CASIO GB-6900A*", 15, INTEROP_MTK_LE_CONN_TIMEOUT_ADJUST},
  {"Parrot ASTEROID Smart", 21, INTEROP_MTK_HFP_15_ESCO_NOT_USE_MSBC},
  {"Toyota Touch&Go", 15, INTEROP_MTK_AVRCP_DISABLE_SONG_POS},
  {"Huitong BLE Remote", 18, INTEROP_DISABLE_LE_SECURE_CONNECTIONS},
#endif

  // Nissan Quest rejects pairing after "0000"
  {"NISSAN",  6, INTEROP_DISABLE_AUTO_PAIRING},

  // Subaru car kits ("CAR M_MEDIA")
  {"CAR",     3, INTEROP_DISABLE_AUTO_PAIRING},
};


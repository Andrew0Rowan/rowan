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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#ifdef MTK_BT_C4A
#include <cutils/properties.h>
#endif
#include "libnvram.h"

/* use nvram */
#include "CFG_BT_File.h"
#include "CFG_BT_Default.h"
#include "CFG_file_lid.h"

//#include "bt_hci_bdroid.h"
#include "bt_vendor_lib.h"
#include "bt_mtk.h"


/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

BT_INIT_VAR_T btinit[1];
BT_INIT_CB_T  btinit_ctrl;

static uint16_t wOpCode;
static HCI_CMD_STATUS_T cmd_status;
extern bt_vendor_callbacks_t *bt_vnd_cbacks;

static BOOL fgGetEFUSE = FALSE;
static UCHAR ucDefaultAddr[6];
static UCHAR ucZeroAddr[6];

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

static BOOL GORMcmd_HCC_Get_Local_BD_Addr(HC_BT_HDR *);
static VOID GORMevt_HCE_Get_Local_BD_Addr(VOID *);
static BOOL GORMcmd_HCC_Set_Local_BD_Addr(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_PCM(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_Radio(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_TX_Power_Offset(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_Sleep_Timeout(HC_BT_HDR *);
#ifdef MTK_MT6630
static BOOL GORMcmd_HCC_Coex_Performance_Adjust(HC_BT_HDR *);
#endif
#ifndef MTK_MT6630 /* for unused warning */
static BOOL GORMcmd_HCC_RESET(HC_BT_HDR *);
#endif
static BOOL GORMcmd_HCC_Set_FW_SysLog(HC_BT_HDR *);
static BOOL GORMcmd_HCC_Set_SSP_Debug_Mode(HC_BT_HDR *);

static VOID GORMevt_HCE_Common_Complete(VOID *);
static VOID GetRandomValue(UCHAR *);
static BOOL WriteBDAddrToNvram(UCHAR *);

//===================================================================
// Combo chip
#ifdef MTK_MT8167
HCI_SEQ_T bt_init_preload_script_8167[] =
{
    {  GORMcmd_HCC_Get_Local_BD_Addr       }, /*0x1009*/
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  GORMcmd_HCC_Set_FW_SysLog           }, /*0xFCBE*/
    {  GORMcmd_HCC_Set_SSP_Debug_Mode      }, /*0x1804*/
    {  0  },
};

HCI_SEQ_T bt_init_postload_script_8167[] =
{
    {  0  },
};

#endif

#ifdef MTK_MT6630
HCI_SEQ_T bt_init_preload_script_6630[] =
{
    {  GORMcmd_HCC_Get_Local_BD_Addr       }, /*0x1009*/
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_Coex_Performance_Adjust }, /*0xFC22*/
    {  GORMcmd_HCC_Set_FW_SysLog           }, /*0xFCBE*/
    {  GORMcmd_HCC_Set_SSP_Debug_Mode      }, /*0x1804*/
    {  0  },
};

HCI_SEQ_T bt_init_postload_script_6630[] =
{
    {  GORMcmd_HCC_Set_PCM                 }, /*0xFC72*/
    {  0  },
};
#endif


/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

static BOOL GORMcmd_HCC_Get_Local_BD_Addr(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x1009;

    p_cmd->len = 3;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 0;

    LOG_DBG("GORMcmd_HCC_Get_Local_BD_Addr\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Get_Local_BD_Addr);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static VOID GORMevt_HCE_Get_Local_BD_Addr(VOID *p_evt)
{
    HC_BT_HDR *p_buf = (HC_BT_HDR *)p_evt;
    uint8_t *p;
    uint8_t event, status;
    uint16_t opcode;
    BOOL success;

    UCHAR ucBDAddr[6];

    LOG_DBG("GORMevt_HCE_Get_Local_BD_Addr\n");

    p = (uint8_t *)(p_buf + 1);
    event = *p;
    p += 3;
    STREAM_TO_UINT16(opcode, p)
    status = *p++;

    if ((event == 0x0E) && /* Command Complete Event */
        (opcode == 0x1009) && /* OpCode correct */
        (status == 0)) /* Success */
    {
        success = TRUE;
    }
    else {
        success = FALSE;
    }

    ucBDAddr[5] = *p++;
    ucBDAddr[4] = *p++;
    ucBDAddr[3] = *p++;
    ucBDAddr[2] = *p++;
    ucBDAddr[1] = *p++;
    ucBDAddr[0] = *p++;

    LOG_WAN("Retrieve eFUSE address: %02x-%02x-%02x-%02x-%02x-%02x\n",
            ucBDAddr[0], ucBDAddr[1], ucBDAddr[2], ucBDAddr[3], ucBDAddr[4], ucBDAddr[5]);

    memcpy(btinit->bt_nvram.fields.addr, ucBDAddr, 6);

    if (bt_vnd_cbacks) {
        bt_vnd_cbacks->dealloc(p_buf);
    }

    pthread_mutex_lock(&btinit_ctrl.mutex);
    cmd_status = success ? CMD_SUCCESS : CMD_FAIL;
    /* Wake up command tx thread */
    pthread_cond_signal(&btinit_ctrl.cond);
    pthread_mutex_unlock(&btinit_ctrl.mutex);

    return;
}

static BOOL GORMcmd_HCC_Set_Local_BD_Addr(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC1A;

    if ((0 == memcmp(btinit->bt_nvram.fields.addr, ucDefaultAddr, 6)) ||
        (0 == memcmp(btinit->bt_nvram.fields.addr, ucZeroAddr, 6))) {
        if (fgGetEFUSE) {
            LOG_WAN("eFUSE address default value\n");
        }
        else {
            LOG_WAN("NVRAM BD address default value\n");
        }

        #ifdef BD_ADDR_AUTOGEN
        GetRandomValue(btinit->bt_nvram.fields.addr);
        #endif

        /* Save BD address to NVRAM */
        WriteBDAddrToNvram(btinit->bt_nvram.fields.addr);
    }
    else {
        if (fgGetEFUSE) {
            LOG_WAN("eFUSE address has valid value\n");
            /* Save BD address to NVRAM */
            WriteBDAddrToNvram(btinit->bt_nvram.fields.addr);
         }
         else {
            LOG_WAN("NVRAM BD address has valid value\n");
         }
    }

    fgGetEFUSE = FALSE; /* Clear flag */

    p_cmd->len = 9;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 6;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.addr[5];
    *p++ = btinit->bt_nvram.fields.addr[4];
    *p++ = btinit->bt_nvram.fields.addr[3];
    *p++ = btinit->bt_nvram.fields.addr[2];
    *p++ = btinit->bt_nvram.fields.addr[1];
    *p++ = btinit->bt_nvram.fields.addr[0];

    LOG_DBG("GORMcmd_HCC_Set_Local_BD_Addr\n");

    LOG_WAN("Write BD address: %02x-%02x-%02x-%02x-%02x-%02x\n",
            btinit->bt_nvram.fields.addr[0], btinit->bt_nvram.fields.addr[1],
            btinit->bt_nvram.fields.addr[2], btinit->bt_nvram.fields.addr[3],
            btinit->bt_nvram.fields.addr[4], btinit->bt_nvram.fields.addr[5]);


    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

#ifdef MTK_MT6628
static BOOL GORMcmd_HCC_Set_LinkKeyType(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC1B;

    p_cmd->len = 4;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 1;

    /* HCI cmd params */
    *p++ = 0x01; /* 00: Unit key; 01: Combination key */

    LOG_DBG("GORMcmd_HCC_Set_LinkKeyType\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_UnitKey(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC75;

    p_cmd->len = 19;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 16;

    /* HCI cmd params */
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;
    *p++ = 0x00;

    LOG_DBG("GORMcmd_HCC_Set_UnitKey\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Encryption(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC76;

    p_cmd->len = 6;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 3;

    /* HCI cmd params */
    *p++ = 0x00;
    *p++ = 0x02;
    *p++ = 0x10;

    LOG_DBG("GORMcmd_HCC_Set_Encryption\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_PinCodeType(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x0C0A;

    p_cmd->len = 4;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 1;

    /* HCI cmd params */
    *p++ = 0x00; /* 00: Variable PIN; 01: Fixed PIN */

    LOG_DBG("GORMcmd_HCC_Set_PinCodeType\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Voice(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x0C26;

    p_cmd->len = 5;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 2;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.Voice[0];
    *p++ = btinit->bt_nvram.fields.Voice[1];

    LOG_DBG("GORMcmd_HCC_Set_Voice\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}
#endif

static BOOL GORMcmd_HCC_Set_PCM(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC72;

    p_cmd->len = 7;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 4;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.Codec[0];
    *p++ = btinit->bt_nvram.fields.Codec[1];
    *p++ = btinit->bt_nvram.fields.Codec[2];
    *p++ = btinit->bt_nvram.fields.Codec[3];

    LOG_DBG("GORMcmd_HCC_Set_PCM\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Radio(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC79;

    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);

    if (btinit->chip_id != 0x6632) {
        p_cmd->len = 9;
        *p++ = 6;

        /* HCI cmd params */
        *p++ = btinit->bt_nvram.fields.Radio[0];
        *p++ = btinit->bt_nvram.fields.Radio[1];
        *p++ = btinit->bt_nvram.fields.Radio[2];
        *p++ = btinit->bt_nvram.fields.Radio[3];
        *p++ = btinit->bt_nvram.fields.Radio[4];
        *p++ = btinit->bt_nvram.fields.Radio[5];
    } else {
        p_cmd->len = 11;
        *p++ = 8;

        /* HCI cmd params */
        *p++ = btinit->bt_nvram.fields.Radio[0];
        *p++ = btinit->bt_nvram.fields.Radio[1];
        *p++ = btinit->bt_nvram.fields.Radio[2];
        *p++ = btinit->bt_nvram.fields.Radio[3];
        *p++ = btinit->bt_nvram.fields.Radio[4];
        *p++ = btinit->bt_nvram.fields.Radio[5];
#if 0 // cc_temp
        *p++ = btinit->bt_nvram.fields.Radio_ext[0];
        *p++ = btinit->bt_nvram.fields.Radio_ext[1];
#endif
    }

    LOG_DBG("GORMcmd_HCC_Set_Radio\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_TX_Power_Offset(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC93;

    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);

    if (btinit->chip_id != 0x6632) {
        p_cmd->len = 6;
        *p++ = 3;

        /* HCI cmd params */
        *p++ = btinit->bt_nvram.fields.TxPWOffset[0];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[1];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[2];
    } else {
        p_cmd->len = 9;
        *p++ = 6;

        /* HCI cmd params */
        *p++ = btinit->bt_nvram.fields.TxPWOffset[0];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[1];
        *p++ = btinit->bt_nvram.fields.TxPWOffset[2];
#if 0 //cc_temp
        *p++ = btinit->bt_nvram.fields.TxPWOffset_ext[0];
        *p++ = btinit->bt_nvram.fields.TxPWOffset_ext[1];
        *p++ = btinit->bt_nvram.fields.TxPWOffset_ext[2];
#endif
    }

    LOG_DBG("GORMcmd_HCC_Set_TX_Power_Offset\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static BOOL GORMcmd_HCC_Set_Sleep_Timeout(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC7A;

    p_cmd->len = 10;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 7;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.Sleep[0];
    *p++ = btinit->bt_nvram.fields.Sleep[1];
    *p++ = btinit->bt_nvram.fields.Sleep[2];
    *p++ = btinit->bt_nvram.fields.Sleep[3];
    *p++ = btinit->bt_nvram.fields.Sleep[4];
    *p++ = btinit->bt_nvram.fields.Sleep[5];
    *p++ = btinit->bt_nvram.fields.Sleep[6];

    LOG_DBG("GORMcmd_HCC_Set_Sleep_Timeout\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

#ifdef MTK_MT6630
static BOOL GORMcmd_HCC_Coex_Performance_Adjust(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFC22;

    p_cmd->len = 9;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 6;

    /* HCI cmd params */
    *p++ = btinit->bt_nvram.fields.CoexAdjust[0];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[1];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[2];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[3];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[4];
    *p++ = btinit->bt_nvram.fields.CoexAdjust[5];

    LOG_DBG("GORMcmd_HCC_Coex_Performance_Adjust\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}
#endif

#ifndef MTK_MT6630 /* for unused warning */
static BOOL GORMcmd_HCC_RESET(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x0C03;

    p_cmd->len = 3;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 0;

    LOG_DBG("GORMcmd_HCC_RESET\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    }
    else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}
#endif

static BOOL GORMcmd_HCC_Set_FW_SysLog(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0xFCBE;

    p_cmd->len = 4;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 1;

    /* HCI cmd params */
    *p++ = 0x05;
    LOG_DBG("GORMcmd_HCC_Set_FW_SysLog enable\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    } else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

/*
 * Simple Pairing Debug Mode:
 *
 * \brief
 *  Send HCI_Write_Simple_Pairing_Debug_Mode with 'enabled' parameter if Bluetooth SSP Debug Mode
 *  is set enabled in Engineer Mode UI for debugging purpose.
 *  This command configures the BR/EDR Controller to use a predefined Diffie Hellman private key
 *  for Simple Pairing, in order for debug equipment to be able to determine the link key
 *  and therefore, be able to monitor the encrypted connection.
 *
 *  Command parameters:
 *  - 0x00: Simple Pairing debug mode disabled (default)
 *  - 0x01: Simple Pairing debug mode enabled
 *
 *  Event generated:
 *  - When HCI_Write_Simple_Pairing_Debug_Mode command has completed, a Command Complete event
 *    shall be generated.
 */
static BOOL GORMcmd_HCC_Set_SSP_Debug_Mode(HC_BT_HDR *p_cmd)
{
    uint8_t *p, ret;
    wOpCode = 0x1804;

    p_cmd->len = 4;
    p = (uint8_t *)(p_cmd + 1);
    UINT16_TO_STREAM(p, wOpCode);
    *p++ = 1;

    /* HCI cmd params */
    *p++ = 0x01;
    LOG_DBG("GORMcmd_HCC_Set_SSP_Debug_Mode enable\n");

    if (bt_vnd_cbacks) {
        ret = bt_vnd_cbacks->xmit_cb(wOpCode, p_cmd, GORMevt_HCE_Common_Complete);
    } else {
        LOG_ERR("No HCI packet transmit callback\n");
        ret = FALSE;
    }

    return ret;
}

static VOID GORMevt_HCE_Common_Complete(VOID *p_evt)
{
    HC_BT_HDR *p_buf = (HC_BT_HDR *)p_evt;
    uint8_t *p;
    uint8_t event, status;
    uint16_t opcode;
    BOOL success;

    LOG_DBG("GORMevt_HCE_Common_Complete\n");

    p = (uint8_t *)(p_buf + 1);
    event = *p;
    p += 3;
    STREAM_TO_UINT16(opcode, p);
    status = *p;

    if ((event == 0x0E) && /* Command Complete Event */
        (opcode == wOpCode) && /* OpCode correct */
        (status == 0)) /* Success */
    {
        success = TRUE;
    }
    else {
        success = FALSE;
    }

    if (bt_vnd_cbacks) {
        bt_vnd_cbacks->dealloc(p_buf);
    }

    pthread_mutex_lock(&btinit_ctrl.mutex);
    cmd_status = success ? CMD_SUCCESS : CMD_FAIL;
    /* Wake up command tx thread */
    pthread_cond_signal(&btinit_ctrl.cond);
    pthread_mutex_unlock(&btinit_ctrl.mutex);

    return;
}


VOID thread_exit(INT32 signo)
{
#if BT_DRIVER_DEBUG /* for unused warning */
    pthread_t tid = pthread_self();
    LOG_DBG("Thread %lu is forced to exit...\n", tid);
#endif
    if (pthread_mutex_unlock(&btinit_ctrl.mutex) != 0) {
        LOG_ERR("pthread_mutex_unlock error\n");
    }
    pthread_mutexattr_destroy(&btinit_ctrl.attr);
    pthread_mutex_destroy(&btinit_ctrl.mutex);
    pthread_cond_destroy(&btinit_ctrl.cond);
    pthread_exit(0);
}

VOID *GORM_FW_Init_Thread(VOID *ptr)
{
    INT32 i = 0;
    HC_BT_HDR  *p_buf = NULL;
    bt_vendor_op_result_t ret = BT_VND_OP_RESULT_FAIL;
#ifdef MTK_BT_C4A
    char bt_syslog_val[PROPERTY_VALUE_MAX];
    char bt_ssp_debug_val[PROPERTY_VALUE_MAX];
#endif
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    pthread_condattr_t condattr;
    pthread_condattr_init(&condattr);
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
#endif

    LOG_DBG("FW init thread starts\n");

    pthread_mutexattr_init(&btinit_ctrl.attr);
    pthread_mutexattr_settype(&btinit_ctrl.attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&btinit_ctrl.mutex, &btinit_ctrl.attr);
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    pthread_cond_init(&btinit_ctrl.cond, &condattr);
#else
    pthread_cond_init(&btinit_ctrl.cond, NULL);
#endif

    /* preload init script */
    switch (btinit->chip_id) {
#ifdef MTK_MT8167
        case 0x8167:
            btinit->cur_script = bt_init_preload_script_8167;
            memcpy(ucDefaultAddr, stBtDefault.addr, 6);
            break;
#endif
#ifdef MTK_MT6630
        case 0x6630:
            btinit->cur_script = bt_init_preload_script_6630;
            memcpy(ucDefaultAddr, stBtDefault.addr, 6);
            break;
#endif
        default:
            LOG_ERR("Unknown combo chip id: %04x\n", btinit->chip_id);
            break;
    }

    /* Can not find matching script, simply skip */
    if ((btinit->cur_script) == NULL) {
        LOG_ERR("No matching init script\n");
        goto exit;
    }

    if ((0 == memcmp(btinit->bt_nvram.fields.addr, ucDefaultAddr, 6)) ||
        (0 == memcmp(btinit->bt_nvram.fields.addr, ucZeroAddr, 6))) {
        /* NVRAM BD address default value */
        /* Want to retrieve module eFUSE address on combo chip */
        fgGetEFUSE = TRUE;
    }

    if (fgGetEFUSE) /* perform GORMcmd_HCC_Get_Local_BD_Addr */
        i = 0;
    else /* skip GORMcmd_HCC_Get_Local_BD_Addr */
        i = 1;

    while (btinit->cur_script[i].command_func)
    {
        if(btinit->cur_script[i].command_func == GORMcmd_HCC_Set_FW_SysLog) {
#ifdef MTK_BT_C4A
            if (!property_get("persist.bt.syslog.enable", bt_syslog_val, NULL) ||
                0 != strcmp(bt_syslog_val, "1")) {
#endif
                i++;
                continue;
#ifdef MTK_BT_C4A
            }
#endif
        } else if(btinit->cur_script[i].command_func == GORMcmd_HCC_Set_SSP_Debug_Mode) {
#ifdef MTK_BT_C4A
            if (!property_get("persist.bt.sspdebug.enable", bt_ssp_debug_val, NULL) ||
                0 != strcmp(bt_ssp_debug_val, "1")) {
#endif
                i++;
                continue;
#ifdef MTK_BT_C4A
            }
#endif
        }

        p_buf = NULL;

        if (bt_vnd_cbacks) {
            p_buf = (HC_BT_HDR *)bt_vnd_cbacks->alloc(BT_HC_HDR_SIZE + \
                                                          HCI_CMD_MAX_SIZE);
        }
        else {
            LOG_ERR("No libbt-hci callbacks!\n");
        }

        if (p_buf) {
            p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
            p_buf->offset = 0;
            p_buf->layer_specific = 0;

            cmd_status = CMD_PENDING;

            if (btinit->cur_script[i].command_func(p_buf) == FALSE) {
                LOG_ERR("Send command %d fails\n", i);
                if (bt_vnd_cbacks) {
                    bt_vnd_cbacks->dealloc(p_buf);
                }
                goto exit;
            }
        }
        else {
            LOG_ERR("Alloc command %d buffer fails\n", i);
            goto exit;
        }

        /* Wait for event returned */
        pthread_mutex_lock(&btinit_ctrl.mutex);
        while (cmd_status == CMD_PENDING) {
            pthread_cond_wait(&btinit_ctrl.cond, &btinit_ctrl.mutex);
        }

        if (cmd_status == CMD_FAIL) {
            LOG_ERR("The event of command %d error\n", i);
            pthread_mutex_unlock(&btinit_ctrl.mutex);
            goto exit;
        }
        else {
            LOG_DBG("The event of command %d success\n", i);
            pthread_mutex_unlock(&btinit_ctrl.mutex);
        }

        i ++;
    }

    ret = BT_VND_OP_RESULT_SUCCESS;

exit:
    pthread_mutexattr_destroy(&btinit_ctrl.attr);
    pthread_mutex_destroy(&btinit_ctrl.mutex);
    pthread_cond_destroy(&btinit_ctrl.cond);

    if (bt_vnd_cbacks) {
        bt_vnd_cbacks->fwcfg_cb(ret);
    }

    btinit_ctrl.worker_thread_running = FALSE;
    return NULL;
}

VOID *GORM_SCO_Init_Thread(VOID *ptr)
{
    INT32 i = 0;
    HC_BT_HDR  *p_buf = NULL;
    bt_vendor_op_result_t ret = BT_VND_OP_RESULT_FAIL;
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    pthread_condattr_t condattr;
    pthread_condattr_init(&condattr);
    pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
#endif

    LOG_DBG("SCO init thread starts\n");

    pthread_mutexattr_init(&btinit_ctrl.attr);
    pthread_mutexattr_settype(&btinit_ctrl.attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&btinit_ctrl.mutex, &btinit_ctrl.attr);
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    pthread_cond_init(&btinit_ctrl.cond, &condattr);
#else
    pthread_cond_init(&btinit_ctrl.cond, NULL);
#endif

    /* postload init script */
    switch (btinit->chip_id) {

    #ifdef MTK_MT8167
      case 0x8167:
        btinit->cur_script = bt_init_postload_script_8167;
            break;
    #endif
    #ifdef MTK_MT6630
      case 0x6630:
        btinit->cur_script = bt_init_postload_script_6630;
            break;
    #endif
        default:
            LOG_ERR("Unknown combo chip id: %04x\n", btinit->chip_id);
            break;
    }

    /* Can not find matching script, simply skip */
    if ((btinit->cur_script) == NULL) {
        LOG_ERR("No matching init script\n");
        goto exit;
    }

    while (btinit->cur_script[i].command_func)
    {
        p_buf = NULL;

        if (bt_vnd_cbacks) {
            p_buf = (HC_BT_HDR *)bt_vnd_cbacks->alloc(BT_HC_HDR_SIZE + \
                                                          HCI_CMD_MAX_SIZE);
        }
        else {
            LOG_ERR("No libbt-hci callbacks!\n");
        }

        if (p_buf) {
            p_buf->event = MSG_STACK_TO_HC_HCI_CMD;
            p_buf->offset = 0;
            p_buf->layer_specific = 0;

            cmd_status = CMD_PENDING;

            if (btinit->cur_script[i].command_func(p_buf) == FALSE) {
                LOG_ERR("Send command %d fails\n", i);
                if (bt_vnd_cbacks) {
                    bt_vnd_cbacks->dealloc(p_buf);
                }
                goto exit;
            }
        }
        else {
            LOG_ERR("Alloc command %d buffer fails\n", i);
            goto exit;
        }

        /* Wait for event returned */
        pthread_mutex_lock(&btinit_ctrl.mutex);
        while (cmd_status == CMD_PENDING) {
            pthread_cond_wait(&btinit_ctrl.cond, &btinit_ctrl.mutex);
        }

        if (cmd_status == CMD_FAIL) {
            LOG_ERR("The event of command %d error\n", i);
            pthread_mutex_unlock(&btinit_ctrl.mutex);
            goto exit;
        }
        else {
            LOG_DBG("The event of command %d success\n", i);
            pthread_mutex_unlock(&btinit_ctrl.mutex);
        }

        i ++;
    }

    ret = BT_VND_OP_RESULT_SUCCESS;

exit:
    pthread_mutexattr_destroy(&btinit_ctrl.attr);
    pthread_mutex_destroy(&btinit_ctrl.mutex);
    pthread_cond_destroy(&btinit_ctrl.cond);

    if (bt_vnd_cbacks) {
        bt_vnd_cbacks->scocfg_cb(ret);
    }

    btinit_ctrl.worker_thread_running = FALSE;
    return NULL;
}


static VOID GetRandomValue(UCHAR string[6])
{
    INT32 iRandom = 0;
    INT32 fd = 0;
    UINT32 seed;

    LOG_WAN("Enable random generation\n");

    /* Initialize random seed */
    srand(time(NULL));
    iRandom = rand();
    LOG_WAN("iRandom = [%d]", iRandom);
    string[0] = (((iRandom>>24|iRandom>>16) & (0xFE)) | (0x02)); /* Must use private bit(1) and no BCMC bit(0) */

    /* second seed */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);
    iRandom = rand();
    LOG_WAN("iRandom = [%d]", iRandom);
    string[1] = ((iRandom>>8) & 0xFF);

    /* third seed */
    fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        if (read(fd, &seed, sizeof(UINT32)) > 0) {
            srand(seed);
            iRandom = rand();
        }
        close(fd);
    }

    LOG_WAN("iRandom = [%d]", iRandom);
    string[5] = (iRandom & 0xFF);

    return;
}

static BOOL WriteBDAddrToNvram(UCHAR *pucBDAddr)
{
#ifdef MTK_BT_NVRAM
    F_ID bt_nvram_fd = {0};
    int rec_size = 0;
    int rec_num = 0;

    bt_nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISWRITE);
    if (bt_nvram_fd.iFileDesc < 0) {
        LOG_WAN("Open BT NVRAM fails errno %d\n", errno);
        return FALSE;
    }

    if (rec_num != 1) {
        LOG_ERR("Unexpected record num %d\n", rec_num);
        NVM_CloseFileDesc(bt_nvram_fd);
        return FALSE;
    }

    if (rec_size != sizeof(ap_nvram_btradio_struct)) {
        LOG_ERR("Unexpected record size %d ap_nvram_btradio_struct %d\n",
                rec_size, (int)sizeof(ap_nvram_btradio_struct));
        NVM_CloseFileDesc(bt_nvram_fd);
        return FALSE;
    }

    lseek(bt_nvram_fd.iFileDesc, 0, 0);

    /* Update BD address */
    if (write(bt_nvram_fd.iFileDesc, pucBDAddr, 6) < 0) {
        LOG_ERR("Write BT NVRAM fails errno %d\n", errno);
        NVM_CloseFileDesc(bt_nvram_fd);
        return FALSE;
    }

    NVM_CloseFileDesc(bt_nvram_fd);
#endif
    return TRUE;
}


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
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>

/* use nvram */
#include "CFG_BT_File.h"
#include "CFG_BT_Default.h"
#include "CFG_file_lid.h"
//#ifdef MTK_BT_C4A
#include "libnvram.h"
//#endif
#include "bt_kal.h"

/**************************************************************************
 *                       D E F I N I T I O N S                            *
***************************************************************************/

typedef union {
  ap_nvram_btradio_struct fields;
  unsigned char raw[sizeof(ap_nvram_btradio_struct)];
} BT_NVRAM_DATA_T;

typedef ENUM_BT_STATUS_T (*HCI_CMD_FUNC_T)(VOID);
typedef struct {
  HCI_CMD_FUNC_T command_func;
} HCI_SEQ_T;

typedef struct {
  UINT32 chip_id;
  BT_NVRAM_DATA_T bt_nvram;
  HCI_SEQ_T *cur_script;
} BT_INIT_VAR_T;

/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

static HCI_CMD_T hciCmd;
static BT_INIT_VAR_T btinit[1];
static INT32  bt_com_port;

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

static ENUM_BT_STATUS_T GORMcmd_HCC_Set_Local_BD_Addr(VOID);
static ENUM_BT_STATUS_T GORMcmd_HCC_Set_PCM(VOID);
static ENUM_BT_STATUS_T GORMcmd_HCC_Set_Radio(VOID);
static ENUM_BT_STATUS_T GORMcmd_HCC_Set_TX_Power_Offset(VOID);
static ENUM_BT_STATUS_T GORMcmd_HCC_Set_Sleep_Timeout(VOID);
#ifdef MTK_MT6630
static ENUM_BT_STATUS_T GORMcmd_HCC_Coex_Performance_Adjust(VOID);
#endif
#ifndef MTK_MT6630 /* for unused warning */
static ENUM_BT_STATUS_T GORMcmd_HCC_RESET(VOID);
#endif

static BOOL BT_Get_Local_BD_Addr(UCHAR *);
static VOID GetRandomValue(UCHAR *);
static BOOL WriteBDAddrToNvram(UCHAR *);


//===================================================================
// Combo chip
#ifdef MTK_MT8167
HCI_SEQ_T bt_init_preload_script_8167[] =
{
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_RESET                   }, /*0x0C03*/
    {  0  },
};
#endif

#ifdef MTK_MT6630
HCI_SEQ_T bt_init_script_6630[] =
{
    {  GORMcmd_HCC_Set_Local_BD_Addr       }, /*0xFC1A*/
    {  GORMcmd_HCC_Set_PCM                 }, /*0xFC72*/
    {  GORMcmd_HCC_Set_Radio               }, /*0xFC79*/
    {  GORMcmd_HCC_Set_TX_Power_Offset     }, /*0xFC93*/
    {  GORMcmd_HCC_Set_Sleep_Timeout       }, /*0xFC7A*/
    {  GORMcmd_HCC_Coex_Performance_Adjust }, /*0xFC22*/
    {  0  },
};
#endif


/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

static ENUM_BT_STATUS_T GORMcmd_HCC_Set_Local_BD_Addr(VOID)
{
    UCHAR ucDefaultAddr[6] = {0};
    UCHAR ucZeroAddr[6] = {0};

    hciCmd.opCode = 0xFC1A;
    hciCmd.len = 6;

    LOG_DBG("GORMcmd_HCC_Set_Local_BD_Addr\n");

    switch (btinit->chip_id) {
#ifdef MTK_MT6630
        case 0x6630:
            memcpy(ucDefaultAddr, stBtDefault.addr, 6);
            break;
#endif
#ifdef MTK_MT8167
        case 0x8167:
            memcpy(ucDefaultAddr, stBtDefault.addr, 6);
            break;
#endif
        default:
            LOG_ERR("Unknown combo chip id: %04x\n", btinit->chip_id);
            break;
    }

    if ((0 == memcmp(btinit->bt_nvram.fields.addr, ucDefaultAddr, 6)) ||
        (0 == memcmp(btinit->bt_nvram.fields.addr, ucZeroAddr, 6))) {
        LOG_DBG("NVRAM BD address default value\n");
        /* Want to retrieve module eFUSE address on combo chip */
        BT_Get_Local_BD_Addr(btinit->bt_nvram.fields.addr);

        if ((0 == memcmp(btinit->bt_nvram.fields.addr, ucDefaultAddr, 6)) ||
            (0 == memcmp(btinit->bt_nvram.fields.addr, ucZeroAddr, 6))) {
            LOG_WAN("eFUSE address default value\n");
            #ifdef BD_ADDR_AUTOGEN
            GetRandomValue(btinit->bt_nvram.fields.addr);
            #endif
        }
        else {
            LOG_WAN("eFUSE address has valid value\n");
        }

        /* Save BD address to NVRAM */
        WriteBDAddrToNvram(btinit->bt_nvram.fields.addr);
    }
    else {
        LOG_DBG("NVRAM BD address has valid value\n");
    }

    hciCmd.parms[0] = btinit->bt_nvram.fields.addr[5];
    hciCmd.parms[1] = btinit->bt_nvram.fields.addr[4];
    hciCmd.parms[2] = btinit->bt_nvram.fields.addr[3];
    hciCmd.parms[3] = btinit->bt_nvram.fields.addr[2];
    hciCmd.parms[4] = btinit->bt_nvram.fields.addr[1];
    hciCmd.parms[5] = btinit->bt_nvram.fields.addr[0];

    LOG_DBG("Write BD address: %02x-%02x-%02x-%02x-%02x-%02x\n",
            hciCmd.parms[5], hciCmd.parms[4], hciCmd.parms[3],
            hciCmd.parms[2], hciCmd.parms[1], hciCmd.parms[0]);


    if (BT_SendHciCommand(bt_com_port, &hciCmd) == TRUE) {
        return BT_STATUS_SUCCESS;
    }
    else {
        return BT_STATUS_FAILED;
    }
}

static ENUM_BT_STATUS_T GORMcmd_HCC_Set_PCM(VOID)
{
    hciCmd.opCode = 0xFC72;
    hciCmd.len = 4;

    hciCmd.parms[0] = btinit->bt_nvram.fields.Codec[0];
    hciCmd.parms[1] = btinit->bt_nvram.fields.Codec[1];
    hciCmd.parms[2] = btinit->bt_nvram.fields.Codec[2];
    hciCmd.parms[3] = btinit->bt_nvram.fields.Codec[3];

    LOG_DBG("GORMcmd_HCC_Set_PCM\n");

    if (BT_SendHciCommand(bt_com_port, &hciCmd) == TRUE) {
        return BT_STATUS_SUCCESS;
    }
    else {
        return BT_STATUS_FAILED;
    }
}

static ENUM_BT_STATUS_T GORMcmd_HCC_Set_Radio(VOID)
{
    hciCmd.opCode = 0xFC79;

    if (btinit->chip_id != 0x6632) {
        hciCmd.len = 6;
        hciCmd.parms[0] = btinit->bt_nvram.fields.Radio[0];
        hciCmd.parms[1] = btinit->bt_nvram.fields.Radio[1];
        hciCmd.parms[2] = btinit->bt_nvram.fields.Radio[2];
        hciCmd.parms[3] = btinit->bt_nvram.fields.Radio[3];
        hciCmd.parms[4] = btinit->bt_nvram.fields.Radio[4];
        hciCmd.parms[5] = btinit->bt_nvram.fields.Radio[5];
    } else {
        hciCmd.len = 8;
        hciCmd.parms[0] = btinit->bt_nvram.fields.Radio[0];
        hciCmd.parms[1] = btinit->bt_nvram.fields.Radio[1];
        hciCmd.parms[2] = btinit->bt_nvram.fields.Radio[2];
        hciCmd.parms[3] = btinit->bt_nvram.fields.Radio[3];
        hciCmd.parms[4] = btinit->bt_nvram.fields.Radio[4];
        hciCmd.parms[5] = btinit->bt_nvram.fields.Radio[5];
#if 0
        hciCmd.parms[6] = btinit->bt_nvram.fields.Radio_ext[0];
        hciCmd.parms[7] = btinit->bt_nvram.fields.Radio_ext[1];
#endif
    }

    LOG_DBG("GORMcmd_HCC_Set_Radio\n");

    if (BT_SendHciCommand(bt_com_port, &hciCmd) == TRUE) {
        return BT_STATUS_SUCCESS;
    }
    else {
        return BT_STATUS_FAILED;
    }
}

static ENUM_BT_STATUS_T GORMcmd_HCC_Set_TX_Power_Offset(VOID)
{
    hciCmd.opCode = 0xFC93;

    if (btinit->chip_id != 0x6632) {
        hciCmd.len = 3;
        hciCmd.parms[0] = btinit->bt_nvram.fields.TxPWOffset[0];
        hciCmd.parms[1] = btinit->bt_nvram.fields.TxPWOffset[1];
        hciCmd.parms[2] = btinit->bt_nvram.fields.TxPWOffset[2];
    } else {
        hciCmd.len = 6;
        hciCmd.parms[0] = btinit->bt_nvram.fields.TxPWOffset[0];
        hciCmd.parms[1] = btinit->bt_nvram.fields.TxPWOffset[1];
        hciCmd.parms[2] = btinit->bt_nvram.fields.TxPWOffset[2];
#if 0
        hciCmd.parms[3] = btinit->bt_nvram.fields.TxPWOffset_ext[0];
        hciCmd.parms[4] = btinit->bt_nvram.fields.TxPWOffset_ext[1];
        hciCmd.parms[5] = btinit->bt_nvram.fields.TxPWOffset_ext[2];
#endif
    }

    LOG_DBG("GORMcmd_HCC_Set_TX_Power_Offset\n");

    if (BT_SendHciCommand(bt_com_port, &hciCmd) == TRUE) {
        return BT_STATUS_SUCCESS;
    }
    else {
        return BT_STATUS_FAILED;
    }
}

static ENUM_BT_STATUS_T GORMcmd_HCC_Set_Sleep_Timeout(VOID)
{
    hciCmd.opCode = 0xFC7A;
    hciCmd.len = 7;

    hciCmd.parms[0] = btinit->bt_nvram.fields.Sleep[0];
    hciCmd.parms[1] = btinit->bt_nvram.fields.Sleep[1];
    hciCmd.parms[2] = btinit->bt_nvram.fields.Sleep[2];
    hciCmd.parms[3] = btinit->bt_nvram.fields.Sleep[3];
    hciCmd.parms[4] = btinit->bt_nvram.fields.Sleep[4];
    hciCmd.parms[5] = btinit->bt_nvram.fields.Sleep[5];
    hciCmd.parms[6] = btinit->bt_nvram.fields.Sleep[6];

    LOG_DBG("GORMcmd_HCC_Set_Sleep_Timeout\n");

    if (BT_SendHciCommand(bt_com_port, &hciCmd) == TRUE) {
        return BT_STATUS_SUCCESS;
    }
    else {
        return BT_STATUS_FAILED;
    }
}

#ifdef MTK_MT6630
static ENUM_BT_STATUS_T GORMcmd_HCC_Coex_Performance_Adjust(VOID)
{
    hciCmd.opCode = 0xFC22;
    hciCmd.len = 6;

    hciCmd.parms[0] = btinit->bt_nvram.fields.CoexAdjust[0];
    hciCmd.parms[1] = btinit->bt_nvram.fields.CoexAdjust[1];
    hciCmd.parms[2] = btinit->bt_nvram.fields.CoexAdjust[2];
    hciCmd.parms[3] = btinit->bt_nvram.fields.CoexAdjust[3];
    hciCmd.parms[4] = btinit->bt_nvram.fields.CoexAdjust[4];
    hciCmd.parms[5] = btinit->bt_nvram.fields.CoexAdjust[5];

    LOG_DBG("GORMcmd_HCC_Coex_Performance_Adjust\n");

    if (BT_SendHciCommand(bt_com_port, &hciCmd) == TRUE) {
        return BT_STATUS_SUCCESS;
    }
    else{
        return BT_STATUS_FAILED;
    }
}
#endif


#ifndef MTK_MT6630 /* for unused warning */
static ENUM_BT_STATUS_T GORMcmd_HCC_RESET(VOID)
{
    hciCmd.opCode = 0x0C03;
    hciCmd.len = 0;

    LOG_DBG("GORMcmd_HCC_RESET\n");

    if (BT_SendHciCommand(bt_com_port, &hciCmd) == TRUE) {
        return BT_STATUS_SUCCESS;
    }
    else {
        return BT_STATUS_FAILED;
    }
}
#endif

ENUM_BT_STATUS_T GORM_Init(
    INT32   comPort,
    UINT32  chipId,
    PUCHAR  pucPatchExtData,
    UINT32  u4PatchExtLen,
    PUCHAR  pucPatchData,
    UINT32  u4PatchLen,
    PUCHAR  pucNvRamData,
    UINT32  u4Baud,
    UINT32  u4HostBaud,
    UINT32  u4FlowControl
    )
{
    INT32   i = 0;
    ENUM_BT_STATUS_T status;
    UINT8   ucEventBuf[MAX_EVENT_SIZE];
    UINT32  u4EventLen;

    LOG_DBG("GORM_Init\n");

    /* Save com port fd for GORMcmds */
    bt_com_port = comPort;
    /* Save chip id */
    btinit->chip_id = chipId;
    /* Copy NVRAM data */
    memcpy(btinit->bt_nvram.raw, pucNvRamData, sizeof(ap_nvram_btradio_struct));

    /* General init script */
    switch (btinit->chip_id) {
#ifdef MTK_MT8167
        case 0x8167:
            btinit->cur_script = bt_init_preload_script_8167;
            break;
#endif
#ifdef MTK_MT6630
        case 0x6630:
            btinit->cur_script = bt_init_script_6630;
            break;
#endif
        default:
            LOG_ERR("Unknown combo chip id: %04x\n", btinit->chip_id);
            break;
    }

    /* Can not find matching script, simply skip */
    if ((btinit->cur_script) == NULL) {
        LOG_ERR("No matching init script\n");
        return BT_STATUS_FAILED;
    }

    i = 0;

    while (btinit->cur_script[i].command_func)
    {
        status = btinit->cur_script[i].command_func();
        if (status == BT_STATUS_CANCELLED) {
            i ++;
            continue; /*skip*/
        }

        if (status == BT_STATUS_FAILED) {
            LOG_ERR("Command %d fails\n", i);
            return status;
        }

        if (BT_ReadExpectedEvent(
              comPort,
              ucEventBuf,
              MAX_EVENT_SIZE,
              &u4EventLen,
              0x0E,
              TRUE,
              hciCmd.opCode,
              TRUE,
              0x00) == FALSE) {

            LOG_ERR("Read event of command %d fails\n", i);
            return BT_STATUS_FAILED;
        }

        i ++;
    }

    return BT_STATUS_SUCCESS;
}


static BOOL BT_Get_Local_BD_Addr(UCHAR *pucBDAddr)
{
    HCI_CMD_T cmd;
    UINT32 u4ReadLen = 0;
    UINT8 ucAckEvent[20];

    cmd.opCode = 0x1009;
    cmd.len = 0;

    LOG_DBG("BT_Get_Local_BD_Addr\n");

    if (BT_SendHciCommand(bt_com_port, &cmd) == FALSE) {
        LOG_ERR("Write get BD address command fails\n");
        return FALSE;
    }

    /* Read local BD address from F/W */
    if (BT_ReadExpectedEvent(
          bt_com_port,
          ucAckEvent,
          sizeof(ucAckEvent),
          &u4ReadLen,
          0x0E,
          TRUE,
          0x1009,
          TRUE,
          0x00) == FALSE) {

        LOG_ERR("Read local BD address fails\n");
        return FALSE;
    }

    LOG_WAN("Local BD address: %02x-%02x-%02x-%02x-%02x-%02x\n",
            ucAckEvent[12], ucAckEvent[11], ucAckEvent[10],
            ucAckEvent[9], ucAckEvent[8], ucAckEvent[7]);

    pucBDAddr[0] = ucAckEvent[12];
    pucBDAddr[1] = ucAckEvent[11];
    pucBDAddr[2] = ucAckEvent[10];
    pucBDAddr[3] = ucAckEvent[9];
    pucBDAddr[4] = ucAckEvent[8];
    pucBDAddr[5] = ucAckEvent[7];

    return TRUE;
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

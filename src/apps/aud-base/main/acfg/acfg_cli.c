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
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>

#include "acfg.h"

#include "u_ipcd.h"
#include "u_dbg.h"
#include "u_cli.h"
#include "u_wifi_utils.h"
#include "u_acfg.h"

static UINT16 ui2_g_acfg_dbg_level = DBG_INIT_LEVEL;

UINT16 acfg_get_dbg_level(VOID)
{
    return ui2_g_acfg_dbg_level | DBG_LAYER_APP;
}

/*-----------------------------------------------------------------------------
                     functions declarations
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    data declarations
 ----------------------------------------------------------------------------*/

#ifdef CLI_SUPPORT

static INT32 _cmd_set_dbg_level(INT32 i4_argc, const CHAR** pps_argv)
{
    UINT16 ui2_dbg_level;
    if (u_cli_parse_dbg_level (i4_argc, pps_argv, &ui2_dbg_level) == CLIR_OK)
    {
        acfg_set_dbg_level(ui2_dbg_level);
    }
    return CLIR_OK;
}


static INT32 _cmd_get_dbg_level(INT32 i4_argc, const CHAR** pps_argv)
{
    return u_cli_show_dbg_level (acfg_get_dbg_level());
}


static INT32 _cmd_list(INT32 i4_argc, const CHAR** pps_argv)
{
    UINT16  ui2_i;
    CHAR    s_buff[ACFG_REC_NAME_SIZE+1] = {0};
    ACFG_DESCR_T* pt_acfg;

    DBG_ACFG_DATA(("%4s   %-32s   %4s   %5s   %4s   %5s   %6s\n",
        "IDX", "NAME", "ID", "TYPE", "ELEM", "OFF", "ENABLE"));
    DBG_ACFG_DATA(("--------------------------------------------------------------------------------\n"));


    for(ui2_i = 0; ui2_i < IDX_MAX; ui2_i++)
    {
        pt_acfg = acfg_get_item(ui2_i);
        /*if(pt_acfg->b_enable == FALSE)
        {
            continue;
        }*/

        strncpy(s_buff, pt_acfg->ac_name, ACFG_REC_NAME_SIZE);

        s_buff[16] = 0;

        DBG_ACFG_DATA(("%4d   %-32s   %04X   %2dBIT   %4d   0x%03X   %d\n",
                       ui2_i,
                       s_buff,
                       pt_acfg->ui2_id,
                       (pt_acfg->e_type << 3),
                       pt_acfg->ui2_num_elem,
                       pt_acfg->ui4_cache_off,
                       pt_acfg->b_enable));
    }

    return CLIR_OK;
}

static INT32 _cmd_set_int(INT32 i4_argc, const CHAR** pps_argv)
{

    if(i4_argc != 4)
    {
        DBG_ACFG_ERR("Err: invalid cmd usage,[cmd] [idx] [value] [type]\n");
        return CLIR_INV_CMD_USAGE;
    }

    CFG_FIELD_T e_type = atoi(pps_argv[3]);

    if (CFG_8BIT_T == e_type)
    {
        UINT8 _value = (UINT8)atoi(pps_argv[2]);
        return acfg_set(atoi(pps_argv[1]), &_value, 1);
    }
    else if (CFG_16BIT_T == e_type)
    {
        UINT16 _value = (UINT16)atoi(pps_argv[2]);
        return acfg_set(atoi(pps_argv[1]), &_value, 1);
    }
    else if (CFG_32BIT_T == e_type)
    {
        UINT32 _value = (UINT32)atoi(pps_argv[2]);
        return acfg_set(atoi(pps_argv[1]), &_value, 1);
    }

    DBG_ACFG_ERR("Err: value length error.type: 1(8BIT),2(16BIT),4(32BIT)\r\n");
    return CLIR_INV_ARG;

}

static INT32 _cmd_get(INT32 i4_argc, const CHAR** pps_argv)
{
    CHAR ac_buffer[ACFG_MAX_REC_SIZE] = {0};
    SIZE_T z_size;
    UINT32 ui4_i = 0;
    UINT16 ui2_idx;

    if(i4_argc != 2)
    {
        DBG_ACFG_ERR("Err: invalid cmd usage\n");
        return CLIR_INV_CMD_USAGE;
    }

    ui2_idx = atoi(pps_argv[1]);

    DBG_ACFG_CRIT("get item idx=%d value\n");

    if (acfg_get_size(ui2_idx, &z_size) != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("Err: get item size fail\n");
        return CLIR_CMD_EXEC_ERROR;
    }

    if (ACFG_MAX_REC_SIZE < z_size)
    {
        DBG_ACFG_ERR("Err: size for the item is wrong.\n");
        return CLIR_CMD_EXEC_ERROR;
    }

    acfg_get(ui2_idx, ac_buffer, &z_size);
    DBG_ACFG_DATA(("HEX:"));
    for(ui4_i = 0; ui4_i < z_size; ui4_i++)
    {
        DBG_ACFG_DATA(("%02X", ac_buffer[ui4_i]));
    }
    DBG_ACFG_DATA(("\n"));

    DBG_ACFG_DATA(("INT:"));
    for(ui4_i = 0; ui4_i < z_size; ui4_i++)
    {
        DBG_ACFG_DATA(("%d ", ac_buffer[ui4_i]));
    }
    DBG_ACFG_DATA(("\n"));

    DBG_ACFG_DATA(("STR:"));
    for(ui4_i = 0; ui4_i < z_size; ui4_i++)
    {
        DBG_ACFG_DATA(("%c", ac_buffer[ui4_i]));
    }
    DBG_ACFG_DATA(("\n"));

    return CLIR_OK;
}

static INT32 _cmd_get_flash_need_size(INT32 i4_argc, const CHAR** pps_argv)
{
    UINT16 ui2_i;
    UINT32 ui4_total_size = 0;
    ACFG_DESCR_T* pt_acfg;

    if (i4_argc != 1)
    {
        DBG_ACFG_ERR("Err: invalid cmd usage.\n");
        return CLIR_INV_CMD_USAGE;
    }

    for(ui2_i = 0; ui2_i < IDX_MAX; ui2_i++)
    {
        pt_acfg = acfg_get_item(ui2_i);
        if(pt_acfg->b_enable == TRUE)
        {
            ui4_total_size += pt_acfg->e_type * pt_acfg->ui2_num_elem;
        }
    }

    DBG_ACFG_CRIT("Total size needed for flash : %d\n", ui4_total_size);

    return CLIR_OK;
}

static INT32 _cmd_set_user_speaker(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret;
    unsigned int i;
    unsigned int ui4_channels;
    mas_chmap_desc_t t_user_speaker;

    ui4_channels = atoi(pps_argv[1]);

    if (ui4_channels > MAX_PROC_CH)
    {
        DBG_ACFG_ERR("channels(%d) must less than %d\n", ui4_channels, MAX_PROC_CH);
        return CLIR_INV_ARG;
    }

    for (i = 0; i < ui4_channels; i++)
    {
        t_user_speaker.chmap[i] = atoi(pps_argv[i + 2]);
    }

    t_user_speaker.channels = ui4_channels;

    i4_ret = u_acfg_set_user_speaker(&t_user_speaker);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("u_acfg_set_user_speaker error.\n");
        return CLIR_CMD_EXEC_ERROR;
    }

    DBG_ACFG_API("_cmd_set_user_speaker success.\n");

    return CLIR_OK;
}

static INT32 _cmd_show_user_speaker(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret;
    unsigned int i;
    mas_chmap_desc_t t_user_speaker;

    i4_ret = u_acfg_get_user_speaker(&t_user_speaker);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("u_acfg_get_user_speaker error.\n");
        return CLIR_CMD_EXEC_ERROR;
    }

    printf("user speaer config:\n");
    printf("channel:%d\n", t_user_speaker.channels);
    for (i = 0; i < t_user_speaker.channels; i++)
    {
        printf("channel[i]:%d\n", t_user_speaker.chmap[i]);
    }

    DBG_ACFG_API("_cmd_show_user_speaker success.\n");

    return CLIR_OK;
}

static INT32 _cmd_reset_default(INT32 i4_argc, const CHAR** pps_argv)
{
    if (i4_argc != 1)
    {
        DBG_ACFG_ERR("Err: invalid cmd usage.\n");
        return CLIR_INV_CMD_USAGE;
    }

    u_acfg_reset_factory_default();

    DBG_ACFG_API("reset to default success.\n");

    return CLIR_OK;
}


static INT32 _cmd_acfg_store(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret;

    if (i4_argc != 1)
    {
        DBG_ACFG_ERR("Err: invalid cmd usage.\n");
        return CLIR_INV_CMD_USAGE;
    }

    i4_ret = acfg_store();
    if (i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("Err: store to flash error\n");
        return CLIR_CMD_EXEC_ERROR;
    }

    DBG_ACFG_API("store to flash success.\n");

    return CLIR_OK;
}

static INT32 _cmd_set_volume(INT32 i4_argc, const CHAR** pps_argv)
{
    UINT8 ui1_volume;

    ui1_volume = atoi(pps_argv[1]);

    if (ui1_volume > VOLUME_MAX)
    {
        DBG_ACFG_ERR("volume value(%d) is error\n",ui1_volume);
        return CLIR_INV_ARG;
    }

    u_acfg_set_volume(ui1_volume, TRUE);

    DBG_ACFG_API("_cmd_set_volume success.\n");

    return CLIR_OK;
}

static INT32 _cmd_get_volume(INT32 i4_argc, const CHAR** pps_argv)
{
    UINT8 ui1_volume;
    INT32 i4_ret;

    i4_ret = u_acfg_get_volume(&ui1_volume);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("get volume error\n");
        return CLIR_CMD_EXEC_ERROR;
    }

    DBG_ACFG_API("get_volume(%d) success.\n", ui1_volume);

    return CLIR_OK;
}

static INT32 _cmd_clean_ap(INT32 i4_argc, const CHAR** pps_argv)
{
    return u_acfg_clean_ap();
}

static INT32 _cmd_factory_reset(INT32 i4_argc, const CHAR** pps_argv)
{
    return u_acfg_factory_reset();
}

static INT32 _cmd_wifi_status(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 index = 0;
    UCHAR wifi_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    u_wifi_get_connect_status();

    u_wifi_get_mac_address(wifi_mac);

    printf("<acfg>get wifi_mac is ");

    for(index=0;index<6;index++)
        printf("0x%02x ",wifi_mac[index]);

    printf("\n");

    return CLIR_OK;
}

static INT32 _cmd_set_wlan_ssid(INT32 i4_argc, const CHAR** pps_argv)
{
    CHAR ac_ssid[WLAN_MAC_MAX_LEN+1]= {0};

    if(i4_argc != 2)
    {
        DBG_ACFG_ERR("Input error. Please input 'set_ssid ssid' ");
    }

    a_cfg_set_wlan_mac(pps_argv[1]);
    DBG_ACFG_ERR(" %s success.\n",pps_argv[1]);

    a_cfg_get_wlan_mac(ac_ssid);
    DBG_ACFG_ERR(" get ssid:%s success.\n",ac_ssid);
    return CLIR_OK;
}
static INT32 _cmd_get_wlan_ssid(INT32 i4_argc, const CHAR** pps_argv)
{
    CHAR ac_ssid[WLAN_MAC_MAX_LEN+1]= {0};

    a_cfg_get_wlan_mac(ac_ssid);
    DBG_ACFG_ERR(" get ssid:%s success.\n",ac_ssid);
    return CLIR_OK;
}

static INT32 _cmd_set_wlan_key(INT32 i4_argc, const CHAR** pps_argv)
{
    CHAR ac_key[WLAN_KEY_MAX_LEN+1] = {0};

    if(i4_argc != 2)
    {
        DBG_ACFG_ERR("Input error. Please input 'set_ssid key' ");
    }

    a_cfg_set_wlan_key(pps_argv[1]);
    DBG_ACFG_ERR(" %s success.\n",pps_argv[1]);

    a_cfg_get_wlan_key(ac_key);
    DBG_ACFG_ERR(" get ssid:%s success.\n",ac_key);
    return CLIR_OK;
}
static INT32 _cmd_get_wlan_key(INT32 i4_argc, const CHAR** pps_argv)
{
    CHAR ac_key[WLAN_KEY_MAX_LEN+1]= {0};

    a_cfg_get_wlan_key(ac_key);
    DBG_ACFG_ERR(" get ssid:%s success.\n",ac_key);
    return CLIR_OK;
}

#if 0
static INT32 _cmd_set_wlan_ap(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret = 0;
    NET_802_11_ASSOCIATE_T _rCurWlanAssociate = {0};
    NET_802_11_ASSOCIATE_T _rCurWlanAssociate1 = {0};

    _rCurWlanAssociate.t_Bssid[0] = 0x20;
    _rCurWlanAssociate.t_Bssid[1] = 0x76;
    _rCurWlanAssociate.t_Bssid[2] = 0x93;
    _rCurWlanAssociate.t_Bssid[3] = 0x25;
    _rCurWlanAssociate.t_Bssid[4] = 0x60;
    _rCurWlanAssociate.t_Bssid[5] = 0xa6;

    if(i4_argc==2)
        _rCurWlanAssociate.t_Bssid[5] = (UINT8)atoi(pps_argv[1]);

    strncpy(_rCurWlanAssociate.t_Ssid.ui1_aSsid , "Audio_5G",sizeof("Audio_5G"));
    _rCurWlanAssociate.t_Ssid.ui4_SsidLen = 8;
    _rCurWlanAssociate.e_AssocCase = 2;
    _rCurWlanAssociate.e_AuthMode = 16;
    _rCurWlanAssociate.e_AuthCipher = 24;
    _rCurWlanAssociate.ui1_Priority = 0;

    _rCurWlanAssociate.t_Key.b_IsAscii = 1;
    strncpy(_rCurWlanAssociate.t_Key.key_body,"11111111",sizeof("11111111"));
    _rCurWlanAssociate.t_Key.pui1_PassPhrase=_rCurWlanAssociate.t_Key.key_body;

    i4_ret = a_cfg_set_wlan_key(_rCurWlanAssociate.t_Key.pui1_PassPhrase);
    i4_ret = a_cfg_set_wlan_ap(_rCurWlanAssociate);

    i4_ret = a_cfg_get_wlan_ap(&_rCurWlanAssociate1);
    DBG_ACFG_ERR("ssid:%s,key:%s.\n",_rCurWlanAssociate1.t_Ssid.ui1_aSsid,_rCurWlanAssociate1.t_Key.key_body);
    return 0;
}
#endif

static CLI_EXEC_T at_acfg_cmd_tbl[] =
{
    {CLI_SET_DBG_LVL_STR,   NULL,       _cmd_set_dbg_level,         NULL, CLI_SET_DBG_LVL_HELP_STR,         CLI_GUEST},
    {CLI_GET_DBG_LVL_STR,   NULL,       _cmd_get_dbg_level,         NULL, CLI_GET_DBG_LVL_HELP_STR,         CLI_GUEST},

    {"set_volume",          "volume",   _cmd_set_volume,            NULL, "set volume",                     CLI_GUEST},
    {"get_volume",          NULL,       _cmd_get_volume,            NULL, "get volume",                     CLI_GUEST},
    {"list",                NULL,       _cmd_list,                  NULL, "List all config",                CLI_GUEST},
    {"get",                 NULL,       _cmd_get,                   NULL, "get a config value",             CLI_GUEST},
    {"set_int",             NULL,       _cmd_set_int,               NULL, "set a int config",               CLI_GUEST},
    {"get_flash_need_size", "gfns",     _cmd_get_flash_need_size,   NULL, "Get total size need for flash",  CLI_GUEST},
    {"set_user_speaker",    NULL,       _cmd_set_user_speaker,      NULL, "set user speaker confg",         CLI_GUEST},
    {"show_user_speaker",   NULL,       _cmd_show_user_speaker,     NULL, "show user speaker confg",        CLI_GUEST},
    {"reset_default",       "reset",    _cmd_reset_default,         NULL, "Reset all config to default",    CLI_GUEST},
    {"clean_ap",            NULL,       _cmd_clean_ap,              NULL, "clean ap",                       CLI_GUEST},
    {"factory_reset",       NULL,       _cmd_factory_reset,         NULL, "factory reset",                  CLI_GUEST},
    {"store",               NULL,       _cmd_acfg_store,            NULL, "Store config to flash",          CLI_GUEST},
    {"wifi_status",         NULL,       _cmd_wifi_status,           NULL, "wifi_status",                    CLI_GUEST},
    {"set_ssid",            "ssid",     _cmd_set_wlan_ssid,         NULL, "set ssid",                     CLI_GUEST},
    {"set_key",             "key",      _cmd_set_wlan_key,          NULL, "set key",                     CLI_GUEST},
    {"get_ssid",            "ssid",     _cmd_get_wlan_ssid,         NULL, "get ssid",                     CLI_GUEST},
    {"get_key",             "key",      _cmd_get_wlan_key,          NULL, "get key",                     CLI_GUEST},
    //{"set_ap",              "ap",       _cmd_set_wlan_ap,           NULL, "set ap",                     CLI_GUEST},

    END_OF_CLI_CMD_TBL
};

static CLI_EXEC_T at_acfg_root_cmd_tbl[] =
{
    {"acfg", NULL,  NULL,  at_acfg_cmd_tbl, "App Config",   CLI_GUEST},
    END_OF_CLI_CMD_TBL
};

VOID acfg_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_acfg_dbg_level = ui2_db_level;
}


INT32 acfg_attach_cmd_tbl(VOID)
{
    /* attach the command table */
    return (u_cli_attach_cmd_tbl(at_acfg_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_NONE));
}

INT32 acfg_detach_cmd_tbl(VOID)
{
    /* detach the command table */
    return (u_cli_detach_cmd_tbl(at_acfg_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_NONE));
}
#endif



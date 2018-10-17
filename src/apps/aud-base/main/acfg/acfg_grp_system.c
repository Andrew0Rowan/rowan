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
#include <stdlib.h>
#include "acfg.h"
#include "u_common.h"
#include "u_dbg.h"
#include "u_c4a_stub.h"
#include "u_app_def.h"
#include "u_bluetooth_audio.h"
#include "u_assistant_stub.h"
#include "u_app_thread.h"
#include "../state_mngr/sm.h"

#define VOLUME_STEP     6

extern void* g_mas_handle;
/*-----------------------------------------------------------------------------
                    private variable definition
-----------------------------------------------------------------------------*/

static UINT8 ui1_volume_def = (UINT8)40; //0~100 for IR
static UINT8 ui1_mute_def = (UINT8)0; //0:unmute 1:mute
static UINT8 ui1_source_def = (UINT8)CFG_SOURCE_DEFAULT;
static mas_chmap_desc_t g_t_hw_speaker =
    {
        2,
        {MAS_CHMAP_FL, MAS_CHMAP_FR},
    };

static mas_chmap_desc_t g_t_user_speaker =
    {
        2,
        {MAS_CHMAP_FL, MAS_CHMAP_FR},
    };


static INT32 _acfg_send_playback_status_to_assistant_stub(VOID)
{
    UINT8 b_mute;
    UINT8 volume = 0;
    INT32 i4_ret = APP_CFGR_OK;
    HANDLE_T h_app = NULL_HANDLE;
    ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T player_status_change = {0};

    memcpy(player_status_change.command, "/playback/player_status_change", ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    u_acfg_get_mute(&b_mute);
    if (b_mute)
    {
        volume = 0;
    }
    else
    {
        u_acfg_get_volume(&volume);
    }

    switch(u_sm_get_current_source())
    {
        case SOURCE_URI:
        {
            u_playback_uri_get_player_status(&player_status_change);
            break;
        }
        case SOURCE_BT:
        {
#if CONFIG_SUPPORT_BT_APP
            memcpy(&player_status_change,u_bluetooth_get_player_status(),sizeof(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T));
#endif /*CONFIG_SUPPORT_BT_APP*/
			break;
        }
        default:
        {
            player_status_change.player.volume = volume;
            strncpy(player_status_change.player.status,"stop",ASSISTANT_STUB_STATUS_MAX_LENGTH);
            strncpy(player_status_change.player.source,"none",ASSISTANT_STUB_SOURCE_MAX_LENGTH);
            break;
        }
    }
    DBG_ERROR(("volume:%d,status:%s.\n",player_status_change.player.volume,player_status_change.player.status));

    u_am_get_app_handle_from_name(&h_app,ASSISTANT_STUB_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
            E_APP_MSG_TYPE_ASSISTANT_STUB,
            MSG_FROM_URI,
            ASSISTANT_STUB_CMD_PLAYER_STATUS_CHANGE,
            &player_status_change,
            sizeof(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T));
    if (AEER_OK != i4_ret)
    {
        DBG_ERROR(("ui_send_playback_status_to_assistant_stub failed, i4_ret[%ld]\n", i4_ret));
    }

    return APP_CFGR_OK;
}

/*-----------------------------------------------------------------------------
                    function definition
-----------------------------------------------------------------------------*/
/***********************************************************************
                                           volume
***********************************************************************/
/*! \fn static BOOL _acfg_volume_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
    \brief this function is to validate if value for the record is legal of not
    \param ui2_id index of the record
    \param pt_value pointer to the value data
    \param z_size size of the value data
    \retval BOOL if TRUE, the value is legal, otherwise FALSE
*/
static BOOL _acfg_volume_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
{
    UINT8 ui1_temp;

    if (NULL == pt_value)
    {
        DBG_ACFG_ERR("Err: pt_value can't be NULL");
        return FALSE;
    }

    ui1_temp = *(UINT8 *)pt_value;

    if (ui1_temp > VOLUME_MAX)
    {
        DBG_ACFG_ERR("Err: invalid value : %d\n", ui1_temp);
        return FALSE;
    }

    return TRUE;
}


/*! \fn static VOID _acfg_volume_update(UINT16 ui2_id)
    \brief this function is called when the record is set
    \param ui2_id index of the record
    \retval BOOL if TRUE, the value is legal, otherwise FALSE
*/
static VOID _acfg_volume_update(UINT16 ui2_id)
{
    UINT8 ui1_volume;
    INT32 i4_ret;
    SIZE_T z_size;

    i4_ret = acfg_get(ui2_id, &ui1_volume , &z_size);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_get_volume fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

#if CONFIG_SUPPORT_OPEN_ALSA_VOLUME
    i4_ret = acfg_set_alsa_volume(ui1_volume);
    DBG_ACFG_INFO("[%s:%d]acfg_set_alsa_volume ret:%d\n", __FUNCTION__, __LINE__, i4_ret);
#else
    i4_ret = mas_set_master_vol(g_mas_handle, ui1_volume);
    DBG_ACFG_INFO("[%s:%d]mas_set_master_vol ret:%d\n", __FUNCTION__, __LINE__, i4_ret);
#endif

}


/*! \fn INT32 u_acfg_set_volume(UINT8 ui1_value, BOOL b_upload)
    \brief this function is to set a value to the record
    \param ui1_value value to be set
    \retval INT32 APP_CFGR_OK if successful
*/
INT32 u_acfg_set_volume(UINT8 ui1_value, BOOL b_upload)
{
    INT32 i4_ret;

    DBG_ACFG_INFO("u_acfg_set_volume, value=%d\n", ui1_value);

    i4_ret = acfg_set(IDX_VOLUME, &ui1_value , 1);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_set_volume fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    if (b_upload)
    {
        //upload volume change to customer
        _acfg_send_playback_status_to_assistant_stub();
    }

    return APP_CFGR_OK;
}


/*! \fn INT32 u_acfg_get_volume(UINT8* pui1_value)
    \brief this function is to set a value to the record
    \param pui1_value pointer to receive the value of the record
    \retval INT32 APP_CFGR_OK if successful
*/
INT32 u_acfg_get_volume(UINT8* pui1_value)
{
    INT32    i4_ret;
    SIZE_T   z_size;
    UINT8    ui1_tz_data;

    if (NULL == pui1_value)
    {
        DBG_ACFG_ERR("pui1_value can't be NULL\n");
        return APP_CFGR_INV_ARG;
    }

    i4_ret = acfg_get(IDX_VOLUME, &ui1_tz_data , &z_size);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_get_volume fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    *pui1_value = ui1_tz_data;

    DBG_ACFG_INFO("u_acfg_get_volume, value=%d\n", *pui1_value);

    return APP_CFGR_OK;
}

/*! \fn INT32 u_acfg_set_stream_volume(MAS_AUDIO_STREAM_E e_stream, UINT8 ui1_value)
    \brief this function is to set stream volumn, and not record
    \param ui1_value value to be set
    \retval INT32 APP_CFGR_OK if successful
*/
INT32 u_acfg_set_stream_volume(MAS_AUDIO_STREAM_E e_stream, UINT8 ui1_value)
{
    INT32 i4_ret;

    DBG_ACFG_INFO("u_acfg_set_stream_volume stream:%d, value=%d\n", e_stream, ui1_value);

    if (ui1_value > VOLUME_MAX)
    {
        DBG_ACFG_ERR("volume[%d] is too bigger!\n", ui1_value);
        return APP_CFGR_INV_ARG;
    }

    i4_ret = mas_set_stream_vol(g_mas_handle, e_stream, ui1_value);
    DBG_ACFG_INFO("[%s:%d]mas_set_stream_vol ret:%d\n", __FUNCTION__, __LINE__, i4_ret);

    return APP_CFGR_OK;
}

INT32 u_acfg_increase_volume(VOID)
{
    UINT8 ui1_volume;
    INT32 i4_ret;

    DBG_ACFG_INFO("u_acfg_increase_volume\n");

    i4_ret = u_acfg_set_mute(FALSE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    i4_ret = u_acfg_get_volume(&ui1_volume);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    ui1_volume += VOLUME_STEP;
    if (ui1_volume > VOLUME_MAX)
    {
        ui1_volume = VOLUME_MAX;
    }

    i4_ret = u_acfg_set_volume(ui1_volume, TRUE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 u_acfg_reduce_volume(VOID)
{
    UINT8 ui1_volume;
    INT32 i4_ret;

    DBG_ACFG_INFO("u_acfg_reduce_volume\n");

    i4_ret = u_acfg_set_mute(FALSE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    i4_ret = u_acfg_get_volume(&ui1_volume);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    if (ui1_volume < VOLUME_STEP)
    {
        ui1_volume = 0;
    }
    else
    {
        ui1_volume -= VOLUME_STEP;
    }

    i4_ret = u_acfg_set_volume(ui1_volume, TRUE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    return APP_CFGR_OK;
}

/*! \fn INT32 u_acfg_set_volume_default(VOID)
    \brief this function is to restore the record as default
    \retval INT32 APP_CFGR_OK if successful
*/
INT32 u_acfg_set_volume_default(VOID)
{
    INT32 i4_ret;

    DBG_ACFG_INFO("u_acfg_set_volume_default\n");

    i4_ret = acfg_set_default(IDX_VOLUME);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_set_volume_default fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

/***********************************************************************
                                           mute
***********************************************************************/
static BOOL _acfg_mute_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
{
    UINT8 ui1_mute;

    if (NULL == pt_value)
    {
        DBG_ACFG_ERR("Err: pt_value can't be NULL");
        return FALSE;
    }

    ui1_mute = *(UINT8 *)pt_value;

    if ((0 != ui1_mute) && (1 != ui1_mute))
    {
        DBG_ACFG_ERR("Err: invalid value : %d\n", ui1_mute);
        return FALSE;
    }

    return TRUE;
}

static VOID _acfg_mute_update(UINT16 ui2_id)
{
    UINT8 ui1_mute;
    INT32 i4_ret;
    SIZE_T z_size;

    i4_ret = acfg_get(ui2_id, &ui1_mute , &z_size);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_get_mute fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    if (ui1_mute)
    {
#if CONFIG_SUPPORT_OPEN_ALSA_VOLUME
        i4_ret = acfg_set_alsa_volume(0);
        DBG_ACFG_INFO("[%s:%d]acfg_set_alsa_volume 0 ret:%d\n", __FUNCTION__, __LINE__, i4_ret);
#else
        i4_ret = mas_set_master_vol(g_mas_handle, 0);
        DBG_ACFG_INFO("[%s:%d]mas_set_master_vol ret:%d\n", __FUNCTION__, __LINE__, i4_ret);
#endif
    }
}

INT32 u_acfg_get_mute(UINT8* b_mute)
{
    INT32    i4_ret;
    SIZE_T   z_size;
    UINT8    ui1_data;

    if (NULL == b_mute)
    {
        DBG_ACFG_ERR("pui1_value can't be NULL\n");
        return APP_CFGR_INV_ARG;
    }

    i4_ret = acfg_get(IDX_MUTE, &ui1_data , &z_size);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_get_mute fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    *b_mute = ui1_data;

    DBG_ACFG_INFO("u_acfg_get_mute, value=%d\n", *b_mute);

    return i4_ret;
}

INT32 u_acfg_set_mute(UINT8 b_mute)
{
    INT32 i4_ret;

    DBG_ACFG_INFO("u_acfg_set_mute, value=%d\n", b_mute);

    i4_ret = acfg_set(IDX_MUTE, &b_mute, sizeof(UINT8));
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_set_mute fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 u_acfg_set_mute_default(VOID)
{
    INT32 i4_ret;

    DBG_ACFG_INFO("u_acfg_set_mute_default\n");

    i4_ret = acfg_set_default(IDX_MUTE);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_set_source_default fail, errno=%d\n", i4_ret);
        return i4_ret;
    }
    return APP_CFGR_OK;
}


/***********************************************************************
                                           source
***********************************************************************/
static BOOL _acfg_source_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
{
    UINT8 ui1_temp;

    if (NULL == pt_value)
    {
        DBG_ACFG_ERR("Err: pt_value can't be NULL");
        return FALSE;
    }

    ui1_temp = *(UINT8 *)pt_value;
    switch (ui1_temp)
    {
        case CFG_SOURCE_DEFAULT:
        case CFG_SOURCE_LINE_IN:
        case CFG_SOURCE_BLUETOOTH_IN:
        case CFG_SOURCE_IN_USB:
        case CFG_SOURCE_IN_C4A:
            break;

        default:
            DBG_ACFG_ERR("Err: _acfg_source_validate, invalid value : %d\n", ui1_temp);
            return FALSE;
    }
    return TRUE;
}

static VOID _acfg_source_update(UINT16 ui2_id)
{
}


INT32 u_acfg_set_source(UINT8 ui1_value)
{
    INT32 i4_ret;

    DBG_ACFG_INFO("u_acfg_set_source, value : %d\n", ui1_value);

    i4_ret = acfg_set(IDX_SOURCE, &ui1_value , 1);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_set_source fail, errno: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 u_acfg_get_source(UINT8* pui1_value)
{
    INT32    i4_ret;
    SIZE_T   z_size;
    UINT8    ui1_tz_data;

    if (NULL == pui1_value)
    {
        DBG_ACFG_ERR("pui1_value can't be NULL\n");
        return APP_CFGR_INV_ARG;
    }

    i4_ret = acfg_get(IDX_SOURCE, &ui1_tz_data , &z_size);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: u_acfg_get_source fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    *pui1_value = ui1_tz_data;

    DBG_ACFG_INFO("u_acfg_get_source, value=%d\n", *pui1_value);

    return APP_CFGR_OK;
}

INT32 u_acfg_set_source_default(VOID)
{
    INT32 i4_ret;

    DBG_ACFG_INFO("u_acfg_set_source_default\n");

    i4_ret = acfg_set_default(IDX_SOURCE);
    if(i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("Err: u_acfg_set_source_default fail, errno=%d\n", i4_ret);
        return i4_ret;
    }
    return APP_CFGR_OK;
}

/***********************************************************************
                                           hw_speak
***********************************************************************/
static BOOL _acfg_hw_speaker_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
{
    mas_chmap_desc_t *pt_speaker = pt_value;

    if (NULL == pt_value)
    {
        DBG_ACFG_ERR("Err: pt_value can't be NULL");
        return FALSE;
    }

    if (z_size != sizeof(*pt_speaker))
    {
        DBG_ACFG_ERR("z_size(%d) must be (%d)\n", z_size, sizeof(*pt_speaker));
        return FALSE;
    }

    if (pt_speaker->channels > MAX_PROC_CH)
    {
        DBG_ACFG_ERR("pt_speaker->channels(%d) must less than %d\n", pt_speaker->channels, MAX_PROC_CH);
        return FALSE;
    }

    return TRUE;
}

static VOID _acfg_hw_speaker_update(UINT16 ui2_id)
{
    //do nothing
    ;
}

/***********************************************************************
                                           user_speak
***********************************************************************/
static BOOL _acfg_user_speaker_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
{
    return _acfg_hw_speaker_validate(ui2_id, pt_value, z_size);
}

static VOID _acfg_user_speaker_update(UINT16 ui2_id)
{
    INT32 i4_ret;
    SIZE_T t_size;
    mas_chmap_desc_t t_hw_speaker;
    mas_chmap_desc_t t_user_speaker;
    mas_device_config_t t_dev_config;

    i4_ret = acfg_get(IDX_HW_SPEAKER_CHMAP, &t_hw_speaker, &t_size);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: get IDX_HW_SPEAKER_CHMAP fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    i4_ret = acfg_get(IDX_USER_SPEAKER_CHMAP, &t_user_speaker, &t_size);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: get IDX_HW_SPEAKER_CHMAP fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    t_dev_config.hw_spk_chmap = &t_hw_speaker;
    t_dev_config.user_spk_chmap = &t_user_speaker;
    //i4_ret = mas_set_device_desc(g_mas_handle, MT_AUDIO_OUT_SPEAKER, &t_dev_config);
    DBG_ACFG_INFO("[%s:%d]mas_set_device_desc ret:%d\n", __FUNCTION__, __LINE__, i4_ret);
}

INT32 u_acfg_set_user_speaker(mas_chmap_desc_t *pt_user_speaker)
{
    INT32 i4_ret;

    i4_ret = acfg_set(IDX_USER_SPEAKER_CHMAP, pt_user_speaker, sizeof(*pt_user_speaker));
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: IDX_USER_SPEAKER_CHMAP set fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 u_acfg_get_user_speaker(mas_chmap_desc_t *pt_user_speaker)
{
    INT32 i4_ret;
    SIZE_T t_size;

    i4_ret = acfg_get(IDX_USER_SPEAKER_CHMAP, pt_user_speaker, t_size);
    if(APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("Err: IDX_USER_SPEAKER_CHMAP get fail, errno=%d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 u_acfg_clean_ap(VOID)
{
    INT32 i;
    INT32 i4_ret;
    char *ps_clean_ap = "dd of=/data/wifi/wpa_supplicant.conf if=/etc/wpa_supplicant.conf.in";
    char *ps_delete_cast_shell[] = {"rm -r /data/chrome/*",
                                    "rm -r /data/chrome/.eureka.conf",
                                    "rm -r /data/chrome/.pki"};
    char *ps_delete_file = "sync";

    i4_ret = system(ps_clean_ap);
    if (-1 == i4_ret)
    {
        DBG_ACFG_ERR("exec %s error(%d)\n", ps_clean_ap, i4_ret);
        //return APP_CFGR_CANT_INIT;
    }

    for (i = 0; i < sizeof(ps_delete_cast_shell)/sizeof(ps_delete_cast_shell[0]); i++)
    {
        i4_ret = system(ps_delete_cast_shell[i]);
        if (-1 == i4_ret)
        {
            DBG_ACFG_ERR("exec %s error(%d)\n", ps_delete_cast_shell[i], i4_ret);
            //return APP_CFGR_CANT_INIT;
        }
    }

    i4_ret = system(ps_delete_file);
    if (-1 == i4_ret)
    {
        DBG_ACFG_ERR("exec %s error(%d)\n", ps_delete_file, i4_ret);
        //return APP_CFGR_CANT_INIT;
    }

    return APP_CFGR_OK;
}

INT32 u_acfg_reset_factory_default(VOID)
{
    INT32 i;
    INT32 i4_ret;

    for (i = 0; i < IDX_MAX; i++)
    {
        i4_ret = acfg_set_default(i);
        if(APP_CFGR_OK != i4_ret)
        {
            DBG_ACFG_ERR("Err: u_acfg_set %d default fail, errno=%d\n", i, i4_ret);
            return i4_ret;
        }
    }

    return APP_CFGR_OK;
}

INT32 u_acfg_factory_reset(VOID)
{
    INT32 i4_ret;
    char *ps_reset_acfg = "rm -rf "ACFG_CONFIG_SAVE_PATH;
    char *ps_reset_acfg_backup = "rm -rf "ACFG_CONFIG_SAVE_PATH_BACKUP;
    char *ps_sync = "sync";
    char *ps_reboot = "reboot -f";
    char *ps_reset_smartbox = "rm -rf "SMARTBOX_AP_SAVE_PATH;
    char *ps_reset_update = "rm -rf /data/update.zip";

#if 0
    i4_ret = u_acfg_clean_ap();
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("_cmd_clean_ap error(%d)\n", i4_ret);
        //return APP_CFGR_CANT_INIT;
    }
    i4_ret = u_acfg_reset_factory_default();
    if (0 != i4_ret)
    {
        DBG_ACFG_ERR("u_acfg_reset_factory_default error(%d)\n", i4_ret);
        return APP_CFGR_CANT_INIT;
    }

    i4_ret = system(ps_reset_smartbox);
    if (-1 == i4_ret)
    {
        DBG_ACFG_ERR("exec %s error(%d)\n", ps_reset_smartbox, i4_ret);
        //return APP_CFGR_CANT_INIT;
    }
#endif

    acfg_set_b_g_app_cfg_init(FALSE);

#if CONFIG_SUPPORT_BT_APP
    i4_ret = u_bluetooth_clear_bluetooth_data();
    if (0 != i4_ret)
    {
        DBG_ACFG_ERR("u_bluetooth_clear_bluetooth_data error(%d)\n", i4_ret);
        //return APP_CFGR_CANT_INIT;
    }
#endif

    i4_ret = system(ps_sync);
    if (-1 == i4_ret)
    {
        DBG_ACFG_ERR("exec %s error(%d)\n", ps_sync, i4_ret);
        //return APP_CFGR_CANT_INIT;
    }


    i4_ret = system(ps_reset_acfg);
    if (-1 == i4_ret)
    {
        DBG_ACFG_ERR("exec %s error(%d)\n", ps_reset_acfg, i4_ret);
        //return APP_CFGR_CANT_INIT;
    }
    i4_ret = system(ps_reset_acfg_backup);
    if (-1 == i4_ret)
    {
        DBG_ACFG_ERR("exec %s error(%d)\n", ps_reset_acfg_backup, i4_ret);
        //return APP_CFGR_CANT_INIT;
    }
    i4_ret = system(ps_reset_update);
    if (-1 == i4_ret)
    {
        DBG_ACFG_ERR("exec %s error(%d)\n", ps_reset_update, i4_ret);
        //return APP_CFGR_CANT_INIT;
    }
    i4_ret = system(ps_sync);
    if (-1 == i4_ret)
    {
        DBG_ACFG_ERR("exec %s error(%d)\n", ps_sync, i4_ret);
        //return APP_CFGR_CANT_INIT;
    }

    DBG_ACFG_INFO("factory_reset success, system will reboot!\n");

    i4_ret = system(ps_reboot);
    if (-1 == i4_ret)
    {
        DBG_ACFG_ERR("exec %s error(%d)\n", ps_reboot, i4_ret);
        return APP_CFGR_CANT_INIT;
    }

    return APP_CFGR_OK;
}

/***********************************************************************
 * fn static INT32 acfg_grp_system_init(VOID)
 * brief this function is to initialize the records
 *retval INT32 APP_CFGR_OK if successful
***********************************************************************/
INT32 acfg_grp_system_init(VOID)
{

    SETUP_FIELD(TRUE,
                IDX_VOLUME,                  /*ac_name */
                CFG_GRPID_SYSTEM,            /* gid of setting ID */
                CFG_RECID_SYSTEM_VOLUME,     /* rid of setting ID */
                CFG_8BIT_T,                  /* type of element */
                sizeof(ui1_volume_def),      /* number of elements */
                &ui1_volume_def,             /* default value */
                sizeof(ui1_volume_def),      /* size of default value */
                _acfg_volume_validate,       /* validate function, if any */
                _acfg_volume_update);        /* update hook fucntion, if any */

    SETUP_FIELD(TRUE,
                IDX_MUTE,                    /*ac_name */
                CFG_GRPID_SYSTEM,            /* gid of setting ID */
                CFG_RECID_SYSTEM_MUTE,       /* rid of setting ID */
                CFG_8BIT_T,                  /* type of element */
                sizeof(ui1_mute_def),        /* number of elements */
                &ui1_mute_def,               /* default value */
                sizeof(ui1_mute_def),        /* size of default value */
                _acfg_mute_validate,         /* validate function, if any */
                _acfg_mute_update);          /* update hook fucntion, if any */

    SETUP_FIELD(TRUE,
                IDX_SOURCE,                  /*ac_name */
                CFG_GRPID_SYSTEM,            /* gid of setting ID */
                CFG_RECID_SYSTEM_SOURCE,     /* rid of setting ID */
                CFG_8BIT_T,                  /* type of element */
                sizeof(ui1_source_def),      /* number of elements */
                &ui1_source_def,             /* default value */
                sizeof(ui1_source_def),      /* size of default value */
                _acfg_source_validate,       /* validate function, if any */
                _acfg_source_update);        /* update hook fucntion, if any */

    SETUP_FIELD(TRUE,
                IDX_HW_SPEAKER_CHMAP,          /*ac_name */
                CFG_GRPID_SYSTEM,            /* gid of setting ID */
                CFG_RECID_SYSTEM_HW_SPEAK,   /* rid of setting ID */
                CFG_8BIT_T,                  /* type of element */
                sizeof(mas_chmap_desc_t),    /* number of elements */
                &g_t_hw_speaker,               /* default value */
                sizeof(mas_chmap_desc_t),    /* size of default value */
                _acfg_hw_speaker_validate,     /* validate function, if any */
                _acfg_hw_speaker_update);      /* update hook fucntion, if any */

    SETUP_FIELD(TRUE,
                IDX_USER_SPEAKER_CHMAP,          /*ac_name */
                CFG_GRPID_SYSTEM,            /* gid of setting ID */
                CFG_RECID_SYSTEM_USER_SPEAK,   /* rid of setting ID */
                CFG_8BIT_T,                  /* type of element */
                sizeof(mas_chmap_desc_t),    /* number of elements */
                &g_t_user_speaker,               /* default value */
                sizeof(mas_chmap_desc_t),    /* size of default value */
                _acfg_user_speaker_validate,     /* validate function, if any */
                _acfg_user_speaker_update);      /* update hook fucntion, if any */

    return APP_CFGR_OK;
}



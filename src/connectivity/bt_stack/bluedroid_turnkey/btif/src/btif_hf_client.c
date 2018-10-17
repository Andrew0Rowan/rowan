/******************************************************************************
 *
 *  Copyright (c) 2014 The Android Open Source Project
 *  Copyright (C) 2009-2012 Broadcom Corporation
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

/************************************************************************************
 *
 *  Filename:      btif_hf_client.c
 *
 *  Description:   Handsfree Profile (HF role) Bluetooth Interface
 *
 *
 ***********************************************************************************/

#define LOG_TAG "bt_btif_hfc"

#include <stdlib.h>
#include <string.h>
#include <hardware/bluetooth.h>
#include <hardware/bt_hf_client.h>

#include "bt_utils.h"
#include "bta_hf_client_api.h"
#include "btcore/include/bdaddr.h"
#include "btif_common.h"
#include "btif_profile_queue.h"
#include "btif_util.h"
#include "osi/include/properties.h"

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
#include "mtk_bt_hf_client.h"
#endif

/************************************************************************************
**  Constants & Macros
************************************************************************************/

#ifndef BTIF_HF_CLIENT_SERVICE_NAME
#define BTIF_HF_CLIENT_SERVICE_NAME ("Handsfree")
#endif

#ifndef BTIF_HF_CLIENT_SECURITY
#define BTIF_HF_CLIENT_SECURITY    (BTA_SEC_AUTHENTICATE | BTA_SEC_ENCRYPT)
#endif

#ifndef BTIF_HF_CLIENT_FEATURES
#define BTIF_HF_CLIENT_FEATURES   ( BTA_HF_CLIENT_FEAT_ECNR  | \
                                    BTA_HF_CLIENT_FEAT_3WAY  | \
                                    BTA_HF_CLIENT_FEAT_CLI   | \
                                    BTA_HF_CLIENT_FEAT_VREC  | \
                                    BTA_HF_CLIENT_FEAT_VOL   | \
                                    BTA_HF_CLIENT_FEAT_ECS   | \
                                    BTA_HF_CLIENT_FEAT_ECC   | \
                                    BTA_HF_CLIENT_FEAT_CODEC)
#endif

/************************************************************************************
**  Local type definitions
************************************************************************************/

/************************************************************************************
**  Static variables
************************************************************************************/
static bthf_client_callbacks_t *bt_hf_client_callbacks = NULL;
#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
static bthf_client_ex_callbacks_t *bt_hf_client_ex_callbacks = NULL;
static void  cleanup_ex(void);
#endif
static UINT32 btif_hf_client_features = 0;

char btif_hf_client_version[PROPERTY_VALUE_MAX];

#define CHECK_BTHF_CLIENT_INIT() if (bt_hf_client_callbacks == NULL)\
    {\
        BTIF_TRACE_WARNING("BTHF CLIENT: %s: not initialized", __FUNCTION__);\
        return BT_STATUS_NOT_READY;\
    }\
    else\
    {\
        BTIF_TRACE_EVENT("BTHF CLIENT: %s", __FUNCTION__);\
    }

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
#define CHECK_BTHF_CLIENT_EX_INIT() if (bt_hf_client_ex_callbacks == NULL)\
    {\
        BTIF_TRACE_WARNING("BTHF CLIENT EX: %s: not initialized", __FUNCTION__);\
        return BT_STATUS_NOT_READY;\
    }\
    else\
    {\
        BTIF_TRACE_EVENT("BTHF CLIENT EX: %s", __FUNCTION__);\
    }
#endif

#define CHECK_BTHF_CLIENT_SLC_CONNECTED() if (bt_hf_client_callbacks == NULL)\
    {\
        BTIF_TRACE_WARNING("BTHF CLIENT: %s: not initialized", __FUNCTION__);\
        return BT_STATUS_NOT_READY;\
    }\
    else if (btif_hf_client_cb.state != BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED)\
    {\
        BTIF_TRACE_WARNING("BTHF CLIENT: %s: SLC connection not up. state=%s",\
                           __FUNCTION__, \
                           dump_hf_conn_state(btif_hf_client_cb.state));\
        return BT_STATUS_NOT_READY;\
    }\
    else\
    {\
        BTIF_TRACE_EVENT("BTHF CLIENT: %s", __FUNCTION__);\
    }

/* BTIF-HF control block to map bdaddr to BTA handle */
typedef struct
{
    UINT16                          handle;
    bt_bdaddr_t                     connected_bda;
    bthf_client_connection_state_t  state;
#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
    bthf_client_audio_state_t       audio_state;
    bthf_client_disable_status_t    disable_state;
#endif
    bthf_client_vr_state_t          vr_state;
    tBTA_HF_CLIENT_PEER_FEAT        peer_feat;
    tBTA_HF_CLIENT_CHLD_FEAT        chld_feat;
} btif_hf_client_cb_t;

static btif_hf_client_cb_t btif_hf_client_cb;

/************************************************************************************
**  Static functions
************************************************************************************/

/*******************************************************************************
**
** Function        btif_in_hf_client_generic_evt
**
** Description     Processes generic events to be sent to JNI that are not triggered from the BTA.
**                 Always runs in BTIF context
**
** Returns          void
**
*******************************************************************************/
static void btif_in_hf_client_generic_evt(UINT16 event, char *p_param)
{
    UNUSED(p_param);

    BTIF_TRACE_EVENT("%s: event=%d", __FUNCTION__, event);
    switch (event) {
        case BTIF_HF_CLIENT_CB_AUDIO_CONNECTING:
        {
#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
            btif_hf_client_cb.audio_state = BTHF_CLIENT_AUDIO_STATE_CONNECTING;
#endif
            HAL_CBACK(bt_hf_client_callbacks, audio_state_cb, (bthf_client_audio_state_t)BTHF_CLIENT_AUDIO_STATE_CONNECTING,
                      &btif_hf_client_cb.connected_bda);
        }
        break;
        default:
        {
            BTIF_TRACE_WARNING("%s : Unknown event 0x%x", __FUNCTION__, event);
        }
        break;
    }
}

/************************************************************************************
**  Externs
************************************************************************************/

/************************************************************************************
**  Functions
************************************************************************************/

static void clear_state(void)
{
    memset(&btif_hf_client_cb, 0, sizeof(btif_hf_client_cb_t));
}

static BOOLEAN is_connected(bt_bdaddr_t *bd_addr)
{
    if (((btif_hf_client_cb.state == BTHF_CLIENT_CONNECTION_STATE_CONNECTED) ||
            (btif_hf_client_cb.state == BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED))&&
        ((bd_addr == NULL) || (bdcmp(bd_addr->address, btif_hf_client_cb.connected_bda.address) == 0)))
        return TRUE;
    return FALSE;
}

/*****************************************************************************
**   Section name (Group of functions)
*****************************************************************************/

/*****************************************************************************
**
**   btif hf api functions (no context switch)
**
*****************************************************************************/

/*******************************************************************************
**
** Function         btif_hf_client_init
**
** Description     initializes the hf interface
**
** Returns         bt_status_t
**
*******************************************************************************/
static bt_status_t init( bthf_client_callbacks_t* callbacks )
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);

    bt_hf_client_callbacks = callbacks;

    btif_enable_service(BTA_HFP_HS_SERVICE_ID);

    clear_state();

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         connect
**
** Description     connect to audio gateway
**
** Returns         bt_status_t
**
*******************************************************************************/
static bt_status_t connect_int( bt_bdaddr_t *bd_addr, uint16_t uuid )
{
    if (is_connected(bd_addr))
        return BT_STATUS_BUSY;

    btif_hf_client_cb.state = BTHF_CLIENT_CONNECTION_STATE_CONNECTING;
    bdcpy(btif_hf_client_cb.connected_bda.address, bd_addr->address);

    BTA_HfClientOpen(btif_hf_client_cb.handle, btif_hf_client_cb.connected_bda.address,
               BTIF_HF_CLIENT_SECURITY);

    return BT_STATUS_SUCCESS;
}

static bt_status_t connect( bt_bdaddr_t *bd_addr )
{
    BTIF_TRACE_EVENT("HFP Client version is  %s", btif_hf_client_version);
    CHECK_BTHF_CLIENT_INIT();
    return btif_queue_connect(UUID_SERVCLASS_HF_HANDSFREE, bd_addr, connect_int);

}

/*******************************************************************************
**
** Function         disconnect
**
** Description      disconnect from audio gateway
**
** Returns         bt_status_t
**
*******************************************************************************/
static bt_status_t disconnect( bt_bdaddr_t *bd_addr )
{
    CHECK_BTHF_CLIENT_INIT();

    if (is_connected(bd_addr))
    {
        BTA_HfClientClose(btif_hf_client_cb.handle);
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}

/*******************************************************************************
**
** Function         connect_audio
**
** Description     create an audio connection
**
** Returns         bt_status_t
**
*******************************************************************************/
static bt_status_t connect_audio( bt_bdaddr_t *bd_addr )
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    if (is_connected(bd_addr))
    {
        if ((BTIF_HF_CLIENT_FEATURES & BTA_HF_CLIENT_FEAT_CODEC) &&
                (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_CODEC))
        {
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BCC, 0, 0, NULL);
        }
        else
        {
            BTA_HfClientAudioOpen(btif_hf_client_cb.handle);
        }

        /* Inform the application that the audio connection has been initiated successfully */
        btif_transfer_context(btif_in_hf_client_generic_evt, BTIF_HF_CLIENT_CB_AUDIO_CONNECTING,
                              (char *)bd_addr, sizeof(bt_bdaddr_t), NULL);
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}

/*******************************************************************************
**
** Function         disconnect_audio
**
** Description      close the audio connection
**
** Returns         bt_status_t
**
*******************************************************************************/
static bt_status_t disconnect_audio( bt_bdaddr_t *bd_addr )
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    if (is_connected(bd_addr))
    {
        BTA_HfClientAudioClose(btif_hf_client_cb.handle);
        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_FAIL;
}

/*******************************************************************************
**
** Function         start_voice_recognition
**
** Description      start voice recognition
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t start_voice_recognition()
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    if (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_FEAT_VREC)
    {
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BVRA, 1, 0, NULL);

        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_UNSUPPORTED;
}

/*******************************************************************************
**
** Function         stop_voice_recognition
**
** Description      stop voice recognition
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t stop_voice_recognition()
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    if (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_FEAT_VREC)
    {
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BVRA, 0, 0, NULL);

        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_UNSUPPORTED;
}

/*******************************************************************************
**
** Function         volume_control
**
** Description      volume control
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t volume_control(bthf_client_volume_type_t type, int volume)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    switch (type)
    {
        case BTHF_CLIENT_VOLUME_TYPE_SPK:
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_VGS, volume, 0, NULL);
            break;
        case BTHF_CLIENT_VOLUME_TYPE_MIC:
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_VGM, volume, 0, NULL);
            break;
        default:
            return BT_STATUS_UNSUPPORTED;
    }

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         dial
**
** Description      place a call
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t dial(const char *number)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    if (number)
    {
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_ATD, 0, 0, number);
    }
    else
    {
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BLDN, 0, 0, NULL);
    }

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         dial_memory
**
** Description      place a call with number specified by location (speed dial)
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t dial_memory(int location)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_ATD, location, 0, NULL);

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         handle_call_action
**
** Description      handle specified call related action
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t handle_call_action(bthf_client_call_action_t action, int idx)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    switch (action)
    {
    case BTHF_CLIENT_CALL_ACTION_CHLD_0:
        if (btif_hf_client_cb.chld_feat & BTA_HF_CLIENT_CHLD_REL)
        {
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 0, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case BTHF_CLIENT_CALL_ACTION_CHLD_1:
        // CHLD 1 is mandatory for 3 way calling
        if (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_FEAT_3WAY)
        {
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 1, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case BTHF_CLIENT_CALL_ACTION_CHLD_2:
        // CHLD 2 is mandatory for 3 way calling
        if (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_FEAT_3WAY)
        {
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 2, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case BTHF_CLIENT_CALL_ACTION_CHLD_3:
        if (btif_hf_client_cb.chld_feat & BTA_HF_CLIENT_CHLD_MERGE)
        {
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 3, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case BTHF_CLIENT_CALL_ACTION_CHLD_4:
        if (btif_hf_client_cb.chld_feat & BTA_HF_CLIENT_CHLD_MERGE_DETACH)
        {
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 4, 0, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case BTHF_CLIENT_CALL_ACTION_CHLD_1x:
        if (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_ECC)
        {
            if (idx < 1)
            {
                return BT_STATUS_FAIL;
            }
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 1, idx, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case BTHF_CLIENT_CALL_ACTION_CHLD_2x:
        if (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_ECC)
        {
            if (idx < 1)
            {
                return BT_STATUS_FAIL;
            }
            BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHLD, 2, idx, NULL);
            break;
        }
        return BT_STATUS_UNSUPPORTED;
    case BTHF_CLIENT_CALL_ACTION_ATA:
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_ATA, 0, 0, NULL);
        break;
    case BTHF_CLIENT_CALL_ACTION_CHUP:
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CHUP, 0, 0, NULL);
        break;
    case BTHF_CLIENT_CALL_ACTION_BTRH_0:
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BTRH, 0, 0, NULL);
        break;
    case BTHF_CLIENT_CALL_ACTION_BTRH_1:
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BTRH, 1, 0, NULL);
        break;
    case BTHF_CLIENT_CALL_ACTION_BTRH_2:
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BTRH, 2, 0, NULL);
        break;
    default:
        return BT_STATUS_FAIL;

    }

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         query_current_calls
**
** Description      query list of current calls
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t query_current_calls(void)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    if (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_ECS)
    {
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CLCC, 0, 0, NULL);

        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_UNSUPPORTED;
}

/*******************************************************************************
**
** Function         query_current_operator_name
**
** Description      query current selected operator name
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t query_current_operator_name(void)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_COPS, 0, 0, NULL);

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         retieve_subscriber_info
**
** Description      retrieve subscriber number information
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t retrieve_subscriber_info(void)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CNUM, 0, 0, NULL);

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         send_dtmf
**
** Description      send dtmf
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t send_dtmf(char code)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_VTS, code, 0, NULL);

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         request_last_voice_tag_number
**
** Description      Request number from AG for VR purposes
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t request_last_voice_tag_number(void)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();

    if (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_VTAG)
    {
        BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_BINP, 1, 0, NULL);

        return BT_STATUS_SUCCESS;
    }

    return BT_STATUS_UNSUPPORTED;
}

/*******************************************************************************
**
** Function         cleanup
**
** Description      Closes the HFP Client interface
**
** Returns          bt_status_t
**
*******************************************************************************/
static void  cleanup( void )
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    if (bt_hf_client_callbacks)
    {
        btif_disable_service(BTA_HFP_HS_SERVICE_ID);
        bt_hf_client_callbacks = NULL;
    }
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    BTIF_TRACE_EVENT("release connect_queue when hfp client cleanup");
    btif_queue_release();
#endif
}

/*******************************************************************************
**
** Function         send_at_cmd
**
** Description      Send requested AT command to remote device.
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t send_at_cmd(int cmd,int val1,int val2,const char *arg)
{
    CHECK_BTHF_CLIENT_SLC_CONNECTED();
    BTIF_TRACE_EVENT("%s Cmd %d val1 %d val2 %d arg %s",
            __FUNCTION__,cmd,val1,val2,arg);
    BTA_HfClientSendAT(btif_hf_client_cb.handle, cmd, val1, val2, arg);

    return BT_STATUS_SUCCESS;
}

static const bthf_client_interface_t bthfClientInterface = {
    sizeof(bthf_client_interface_t),
    .init = init,
    .connect = connect,
    .disconnect = disconnect,
    .connect_audio = connect_audio,
    .disconnect_audio = disconnect_audio,
    .start_voice_recognition = start_voice_recognition,
    .stop_voice_recognition = stop_voice_recognition,
    .volume_control = volume_control,
    .dial = dial,
    .dial_memory = dial_memory,
    .handle_call_action = handle_call_action,
    .query_current_calls = query_current_calls,
    .query_current_operator_name = query_current_operator_name,
    .retrieve_subscriber_info = retrieve_subscriber_info,
    .send_dtmf = send_dtmf,
    .request_last_voice_tag_number = request_last_voice_tag_number,
    .cleanup = cleanup,
    .send_at_cmd = send_at_cmd,
};

static void process_ind_evt(tBTA_HF_CLIENT_IND *ind)
{
    switch (ind->type)
    {
        case BTA_HF_CLIENT_IND_CALL:
            HAL_CBACK(bt_hf_client_callbacks, call_cb, ind->value);
            break;

        case BTA_HF_CLIENT_IND_CALLSETUP:
            HAL_CBACK(bt_hf_client_callbacks, callsetup_cb, ind->value);
            break;
        case BTA_HF_CLIENT_IND_CALLHELD:
            HAL_CBACK(bt_hf_client_callbacks, callheld_cb, ind->value);
            break;

        case BTA_HF_CLIENT_IND_SERVICE:
            HAL_CBACK(bt_hf_client_callbacks, network_state_cb, ind->value);
            break;

        case BTA_HF_CLIENT_IND_SIGNAL:
            HAL_CBACK(bt_hf_client_callbacks, network_signal_cb, ind->value);
            break;

        case BTA_HF_CLIENT_IND_ROAM:
            HAL_CBACK(bt_hf_client_callbacks, network_roaming_cb, ind->value);
            break;

        case BTA_HF_CLIENT_IND_BATTCH:
            HAL_CBACK(bt_hf_client_callbacks, battery_level_cb, ind->value);
            break;

        default:
            break;
    }
}

/*******************************************************************************
**
** Function         btif_hf_client_upstreams_evt
**
** Description      Executes HF CLIENT UPSTREAMS events in btif context
**
** Returns          void
**
*******************************************************************************/
static void btif_hf_client_upstreams_evt(UINT16 event, char* p_param)
{
    tBTA_HF_CLIENT *p_data = (tBTA_HF_CLIENT *)p_param;
    bdstr_t bdstr;

    BTIF_TRACE_DEBUG("%s: event=%s (%u)", __FUNCTION__, dump_hf_client_event(event), event);
    switch (event)
    {
        case BTA_HF_CLIENT_ENABLE_EVT:
#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
            HAL_CBACK(bt_hf_client_ex_callbacks, ability_cb, BTHF_CLIENT_ENABLE);
            break;
#endif
        case BTA_HF_CLIENT_DISABLE_EVT:
#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
            if (bt_hf_client_ex_callbacks)
            {
                if (BTHF_CLIENT_DISABLE_CLEANUP == btif_hf_client_cb.disable_state)
                {
                    BTIF_TRACE_DEBUG("%s:%d, disable:cleanup done, notify disable state", __FUNCTION__, __LINE__);
                    HAL_CBACK(bt_hf_client_ex_callbacks, ability_cb, BTHF_CLIENT_DISABLE);
                    btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_COMPLETE;
                }
                cleanup_ex();
            }
#endif
            break;

        case BTA_HF_CLIENT_REGISTER_EVT:
            btif_hf_client_cb.handle = p_data->reg.handle;
            break;

        case BTA_HF_CLIENT_OPEN_EVT:
            if (p_data->open.status == BTA_HF_CLIENT_SUCCESS)
            {
                bdcpy(btif_hf_client_cb.connected_bda.address, p_data->open.bd_addr);
                btif_hf_client_cb.state = BTHF_CLIENT_CONNECTION_STATE_CONNECTED;
                btif_hf_client_cb.peer_feat = 0;
                btif_hf_client_cb.chld_feat = 0;
                //clear_phone_state();
            }
            else if (btif_hf_client_cb.state == BTHF_CLIENT_CONNECTION_STATE_CONNECTING)
            {
                btif_hf_client_cb.state = BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED;
            }
            else
            {
                BTIF_TRACE_WARNING("%s: HF CLient open failed, but another device connected. status=%d state=%d connected device=%s",
                        __FUNCTION__, p_data->open.status, btif_hf_client_cb.state, bdaddr_to_string(&btif_hf_client_cb.connected_bda, bdstr, sizeof(bdstr)));
                break;
            }

            HAL_CBACK(bt_hf_client_callbacks, connection_state_cb, btif_hf_client_cb.state,
                        0, 0, &btif_hf_client_cb.connected_bda);

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
            if (BTHF_CLIENT_CONNECTION_STATE_CONNECTED == btif_hf_client_cb.state)
            {
                if (BTHF_CLIENT_DISABLE_START == btif_hf_client_cb.disable_state)
                {
                    BTIF_TRACE_DEBUG("%s:%d, disable:RFC Connected", __FUNCTION__, __LINE__);
                    btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_DISCONNECT;
                    disconnect(btif_hf_client_cb.connected_bda.address);
                }
            }
#endif

            if (btif_hf_client_cb.state == BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED)
            {
                bdsetany(btif_hf_client_cb.connected_bda.address);
#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
                if (BTHF_CLIENT_DISABLE_DISCONNECT == btif_hf_client_cb.disable_state ||
                    BTHF_CLIENT_DISABLE_START == btif_hf_client_cb.disable_state)
                {
                    BTIF_TRACE_DEBUG("%s:%d, disable:RFC/SLC Disconnected", __FUNCTION__, __LINE__);
                    btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_CLEANUP;
                    cleanup();
                    //break;
                }
#endif
            }

            if (p_data->open.status != BTA_HF_CLIENT_SUCCESS)
                btif_queue_advance();
            break;

        case BTA_HF_CLIENT_CONN_EVT:
            btif_hf_client_cb.peer_feat = p_data->conn.peer_feat;
            btif_hf_client_cb.chld_feat = p_data->conn.chld_feat;
            btif_hf_client_cb.state = BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED;

            HAL_CBACK(bt_hf_client_callbacks, connection_state_cb, btif_hf_client_cb.state,
                        btif_hf_client_cb.peer_feat, btif_hf_client_cb.chld_feat,
                        &btif_hf_client_cb.connected_bda);

            /* Inform the application about in-band ringtone */
            if (btif_hf_client_cb.peer_feat & BTA_HF_CLIENT_PEER_INBAND)
            {
                HAL_CBACK(bt_hf_client_callbacks, in_band_ring_tone_cb, BTHF_CLIENT_IN_BAND_RINGTONE_PROVIDED);
            }

            btif_queue_advance();

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
            if (BTHF_CLIENT_DISABLE_START == btif_hf_client_cb.disable_state)
            {
                BTIF_TRACE_DEBUG("%s:%d, disable:SLC Connected", __FUNCTION__, __LINE__);
                btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_DISCONNECT;
                disconnect(btif_hf_client_cb.connected_bda.address);
            }
#endif

            break;

        case BTA_HF_CLIENT_CLOSE_EVT:
            btif_hf_client_cb.state = BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED;
            HAL_CBACK(bt_hf_client_callbacks, connection_state_cb,  btif_hf_client_cb.state,
                        0, 0, &btif_hf_client_cb.connected_bda);
            bdsetany(btif_hf_client_cb.connected_bda.address);
            btif_hf_client_cb.peer_feat = 0;
            btif_hf_client_cb.chld_feat = 0;
            btif_queue_advance();

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
            if (BTHF_CLIENT_DISABLE_DISCONNECT == btif_hf_client_cb.disable_state ||
                BTHF_CLIENT_DISABLE_START == btif_hf_client_cb.disable_state)
            {
                BTIF_TRACE_DEBUG("%s:%d, disable:SLC Disconnected", __FUNCTION__, __LINE__);
                btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_CLEANUP;
                cleanup();
            }
#endif
            break;

        case BTA_HF_CLIENT_IND_EVT:
            process_ind_evt(&p_data->ind);
            break;

        case BTA_HF_CLIENT_MIC_EVT:
            HAL_CBACK(bt_hf_client_callbacks, volume_change_cb, BTHF_CLIENT_VOLUME_TYPE_MIC, p_data->val.value);
            break;

        case BTA_HF_CLIENT_SPK_EVT:
            HAL_CBACK(bt_hf_client_callbacks, volume_change_cb, BTHF_CLIENT_VOLUME_TYPE_SPK, p_data->val.value);
            break;

        case BTA_HF_CLIENT_VOICE_REC_EVT:
            HAL_CBACK(bt_hf_client_callbacks, vr_cmd_cb, p_data->val.value);
            break;

        case BTA_HF_CLIENT_OPERATOR_NAME_EVT:
            HAL_CBACK(bt_hf_client_callbacks, current_operator_cb, p_data->operator.name);
            break;

        case BTA_HF_CLIENT_CLIP_EVT:
            HAL_CBACK(bt_hf_client_callbacks, clip_cb, p_data->number.number);
            break;

        case BTA_HF_CLIENT_BINP_EVT:
            HAL_CBACK(bt_hf_client_callbacks, last_voice_tag_number_callback, p_data->number.number);
            break;

        case BTA_HF_CLIENT_CCWA_EVT:
            HAL_CBACK(bt_hf_client_callbacks, call_waiting_cb, p_data->number.number);
            break;

        case BTA_HF_CLIENT_AT_RESULT_EVT:
            HAL_CBACK(bt_hf_client_callbacks, cmd_complete_cb, p_data->result.type, p_data->result.cme);
            break;

        case BTA_HF_CLIENT_CLCC_EVT:
            HAL_CBACK(bt_hf_client_callbacks, current_calls_cb, p_data->clcc.idx,
                        p_data->clcc.inc ? BTHF_CLIENT_CALL_DIRECTION_INCOMING : BTHF_CLIENT_CALL_DIRECTION_OUTGOING,
                        p_data->clcc.status,
                        p_data->clcc.mpty ? BTHF_CLIENT_CALL_MPTY_TYPE_MULTI : BTHF_CLIENT_CALL_MPTY_TYPE_SINGLE,
                        p_data->clcc.number_present ? p_data->clcc.number : NULL);
            break;

        case BTA_HF_CLIENT_CNUM_EVT:
            if (p_data->cnum.service == 4)
            {
                HAL_CBACK(bt_hf_client_callbacks, subscriber_info_cb, p_data->cnum.number, BTHF_CLIENT_SERVICE_VOICE);
            }
            else if (p_data->cnum.service == 5)
            {
                HAL_CBACK(bt_hf_client_callbacks, subscriber_info_cb, p_data->cnum.number, BTHF_CLIENT_SERVICE_FAX);
            }
            else
            {
                HAL_CBACK(bt_hf_client_callbacks, subscriber_info_cb, p_data->cnum.number, BTHF_CLIENT_SERVICE_UNKNOWN);
            }
            break;

        case BTA_HF_CLIENT_BTRH_EVT:
            if (p_data->val.value <= BTRH_CLIENT_RESP_AND_HOLD_REJECT)
            {
                HAL_CBACK(bt_hf_client_callbacks, resp_and_hold_cb, p_data->val.value);
            }
            break;

        case BTA_HF_CLIENT_BSIR_EVT:
            if (p_data->val.value != 0)
            {
                HAL_CBACK(bt_hf_client_callbacks, in_band_ring_tone_cb, BTHF_CLIENT_IN_BAND_RINGTONE_PROVIDED);
            }
            else
            {
                HAL_CBACK(bt_hf_client_callbacks, in_band_ring_tone_cb, BTHF_CLIENT_IN_BAND_RINGTONE_NOT_PROVIDED);
            }
            break;

        case BTA_HF_CLIENT_AUDIO_OPEN_EVT:
            HAL_CBACK(bt_hf_client_callbacks, audio_state_cb, BTHF_CLIENT_AUDIO_STATE_CONNECTED, &btif_hf_client_cb.connected_bda);
#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
            btif_hf_client_cb.audio_state = BTHF_CLIENT_AUDIO_STATE_CONNECTED;
            if (BTHF_CLIENT_DISABLE_START == btif_hf_client_cb.disable_state)
            {
                BTIF_TRACE_DEBUG("%s, disable:eSCO/SCO link Connected", __FUNCTION__);
                btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_AUDIO_DISCONNECT;
                disconnect_audio(btif_hf_client_cb.connected_bda.address);
            }
#endif
            break;

        case BTA_HF_CLIENT_AUDIO_MSBC_OPEN_EVT:
            HAL_CBACK(bt_hf_client_callbacks, audio_state_cb, BTHF_CLIENT_AUDIO_STATE_CONNECTED_MSBC, &btif_hf_client_cb.connected_bda);
#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
            btif_hf_client_cb.audio_state = BTHF_CLIENT_AUDIO_STATE_CONNECTED_MSBC;
            if (BTHF_CLIENT_DISABLE_START == btif_hf_client_cb.disable_state)
            {
                BTIF_TRACE_DEBUG("%s, disable:eSCO/SCO link Connected", __FUNCTION__);
                btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_AUDIO_DISCONNECT;
                disconnect_audio(btif_hf_client_cb.connected_bda.address);
            }
#endif
            break;

        case BTA_HF_CLIENT_AUDIO_CLOSE_EVT:
            HAL_CBACK(bt_hf_client_callbacks, audio_state_cb, BTHF_CLIENT_AUDIO_STATE_DISCONNECTED, &btif_hf_client_cb.connected_bda);

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
            btif_hf_client_cb.audio_state = BTHF_CLIENT_AUDIO_STATE_DISCONNECTED;
            if (BTHF_CLIENT_DISABLE_AUDIO_DISCONNECT == btif_hf_client_cb.disable_state ||
                BTHF_CLIENT_DISABLE_START == btif_hf_client_cb.disable_state)
            {
                BTIF_TRACE_DEBUG("%s, disable:eSCO/SCO link Disconnected", __FUNCTION__);
                btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_DISCONNECT;
                disconnect(btif_hf_client_cb.connected_bda.address);
            }
#endif
            break;
        case BTA_HF_CLIENT_RING_INDICATION:
            HAL_CBACK(bt_hf_client_callbacks, ring_indication_cb);
            break;
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
        case BTA_HF_CLIENT_CPBS_EVT:
            HAL_CBACK(bt_hf_client_ex_callbacks, cpbs_cb, (int*)p_data->cpbs.storage_lookup);
            break;
        case BTA_HF_CLIENT_CPBR_COUNT_EVT:
            HAL_CBACK(bt_hf_client_ex_callbacks, cpbr_count_cb, p_data->cpbr.count.idx_max);
            break;
        case BTA_HF_CLIENT_CPBR_ENTRY_EVT:
            HAL_CBACK(bt_hf_client_ex_callbacks, cpbr_entry_cb, p_data->cpbr.entry.idx,
                        p_data->cpbr.entry.number,
                        p_data->cpbr.entry.type,
                        p_data->cpbr.entry.name);
            break;
        case BTA_HF_CLIENT_CPBR_COMPLETE_EVT:
            HAL_CBACK(bt_hf_client_ex_callbacks, cpbr_complete_cb);
            break;
#endif
        default:
            BTIF_TRACE_WARNING("%s: Unhandled event: %d", __FUNCTION__, event);
            break;
    }
}

/*******************************************************************************
**
** Function         bte_hf_client_evt
**
** Description      Switches context from BTE to BTIF for all HF Client events
**
** Returns          void
**
*******************************************************************************/

static void bte_hf_client_evt(tBTA_HF_CLIENT_EVT event, tBTA_HF_CLIENT *p_data)
{
    bt_status_t status;

    /* switch context to btif task context (copy full union size for convenience) */
    status = btif_transfer_context(btif_hf_client_upstreams_evt, (uint16_t)event, (void*)p_data, sizeof(*p_data), NULL);

    /* catch any failed context transfers */
    ASSERTC(status == BT_STATUS_SUCCESS, "context transfer failed", status);
}

/*******************************************************************************
**
** Function         btif_hf_client_execute_service
**
** Description      Initializes/Shuts down the service
**
** Returns          BT_STATUS_SUCCESS on success, BT_STATUS_FAIL otherwise
**
*******************************************************************************/
bt_status_t btif_hf_client_execute_service(BOOLEAN b_enable)
{
    BTIF_TRACE_EVENT("%s enable:%d", __FUNCTION__, b_enable);

#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    osi_property_get("ro.bluetooth.hfp.ver", btif_hf_client_version, "1.6");
#else
    osi_property_get("ro.bluetooth.hfp.ver", btif_hf_client_version, "1.5");
#endif

     if (b_enable)
     {
          /* Enable and register with BTA-HFClient */
          BTA_HfClientEnable(bte_hf_client_evt);
          if (strcmp(btif_hf_client_version, "1.6") == 0)
          {
              BTIF_TRACE_EVENT("Support Codec Nego. %d ", BTIF_HF_CLIENT_FEATURES);
              BTA_HfClientRegister(BTIF_HF_CLIENT_SECURITY, BTIF_HF_CLIENT_FEATURES,
                      BTIF_HF_CLIENT_SERVICE_NAME);
          }
          else
          {
              BTIF_TRACE_EVENT("No Codec Nego Supported");
              btif_hf_client_features = BTIF_HF_CLIENT_FEATURES;
              btif_hf_client_features = btif_hf_client_features & (~BTA_HF_CLIENT_FEAT_CODEC);
              BTIF_TRACE_EVENT("btif_hf_client_features is   %d", btif_hf_client_features);
              BTA_HfClientRegister(BTIF_HF_CLIENT_SECURITY, btif_hf_client_features,
                      BTIF_HF_CLIENT_SERVICE_NAME);
          }

     }
     else
     {
         BTA_HfClientDeregister(btif_hf_client_cb.handle);
         BTA_HfClientDisable();
     }
     return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         btif_hf_get_interface
**
** Description      Get the hf callback interface
**
** Returns          bthf_interface_t
**
*******************************************************************************/
const bthf_client_interface_t *btif_hf_client_get_interface(void)
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &bthfClientInterface;
}

#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
/*******************************************************************************
**
** Function         select_pb_storage
**
** Description
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t select_pb_storage()
{
    CHECK_BTHF_CLIENT_EX_INIT();

    BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CPBS, BT_HFCLIENT_PB_STORAGE_COUNT, 0, NULL);
    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         set_pb_storage
**
** Description
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t set_pb_storage(int storage_idx)
{
    CHECK_BTHF_CLIENT_EX_INIT();

    BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CPBS, storage_idx, 0, NULL);
    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         Set charset
**
** Description
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t set_charset(const char* charset)
{
    CHECK_BTHF_CLIENT_EX_INIT();

    BTIF_TRACE_ERROR("%s", charset);
    BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CSCS, 0, 0, charset);
    return BT_STATUS_SUCCESS;
}


/*******************************************************************************
**
** Function         read_pb_entry
**
** Description
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t read_pb_entry(int idx_min, int idx_max)
{
    CHECK_BTHF_CLIENT_EX_INIT();

    BTA_HfClientSendAT(btif_hf_client_cb.handle, BTA_HF_CLIENT_AT_CMD_CPBR, idx_min, idx_max, NULL);
    return BT_STATUS_SUCCESS;
}
#endif

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
/*******************************************************************************
**
** Function         init_ex
**
** Description
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t init_ex(bthf_client_ex_callbacks_t* callbacks )
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);

    bt_hf_client_ex_callbacks = callbacks;

    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         cleanup_ex
**
** Description      Closes HFP Client Ex Interface
**
** Returns          bt_status_t
**
*******************************************************************************/
static void  cleanup_ex(void)
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    if (bt_hf_client_ex_callbacks)
    {
        bt_hf_client_ex_callbacks = NULL;
    }
}

/*******************************************************************************
**
** Function         enable
**
** Description      enable HFP Client Interface
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t enable(bthf_client_callbacks_t* callbacks)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_NONE;
    if (bt_hf_client_callbacks)
    {
        BTIF_TRACE_DEBUG("%s, already init!", __FUNCTION__);
        HAL_CBACK(bt_hf_client_ex_callbacks, ability_cb, BTHF_CLIENT_ENABLE);
        return BT_STATUS_SUCCESS;
    }
    ret = init(callbacks);
    return ret;
}

/*******************************************************************************
**
** Function         enable_ex
**
** Description      enable HFP Client Ex Interface
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t enable_ex(bthf_client_ex_callbacks_t* callbacks)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    //init_ex aleady done
    if (NULL != bt_hf_client_ex_callbacks)
        return BT_STATUS_SUCCESS;

    ret = init_ex(callbacks);
    return ret;
}

/*******************************************************************************
**
** Function         disable
**
** Description      disable HFP Client and HFP Client Ex Interface
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t disable(void)
{
    BTIF_TRACE_DEBUG("%s begin", __FUNCTION__);
    bt_status_t ret = BT_STATUS_SUCCESS;
    btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_START;

    //Audio Connected
    if ((btif_hf_client_cb.audio_state == BTHF_CLIENT_AUDIO_STATE_CONNECTED) ||
            (btif_hf_client_cb.audio_state == BTHF_CLIENT_AUDIO_STATE_CONNECTED_MSBC))
    {
        BTIF_TRACE_DEBUG("%s, eSco/Sco link connected!", __FUNCTION__);
        btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_AUDIO_DISCONNECT;
        ret = disconnect_audio(btif_hf_client_cb.connected_bda.address);
        if (BT_STATUS_SUCCESS != ret)
        {
            BTIF_TRACE_ERROR("%s, disconnect eSco/Sco failed!", __FUNCTION__);
            return ret;
        }
        return ret;
    }
    //Audio Connecting
    else if (btif_hf_client_cb.audio_state == BTHF_CLIENT_AUDIO_STATE_CONNECTING)
    {
        BTIF_TRACE_DEBUG("%s, eSco/Sco link connecting!", __FUNCTION__);
        return BT_STATUS_BUSY;
    }
    //Audio Disconnected
    else if (btif_hf_client_cb.audio_state == BTHF_CLIENT_AUDIO_STATE_DISCONNECTED)
    {
        BTIF_TRACE_DEBUG("%s, eSco/Sco link disconnected!", __FUNCTION__);
    }

    //SLC connected
    if ((btif_hf_client_cb.state == BTHF_CLIENT_CONNECTION_STATE_CONNECTED) ||
            (btif_hf_client_cb.state == BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED))
    {
        BTIF_TRACE_DEBUG("%s, RFC/SLC connected!", __FUNCTION__);
        btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_DISCONNECT;
        ret = disconnect(btif_hf_client_cb.connected_bda.address);
        if (BT_STATUS_SUCCESS != ret)
        {
            BTIF_TRACE_ERROR("%s, disconnect slc failed!", __FUNCTION__);
            return ret;
        }
        return ret;
    }
    //SLC connecting
    else if (btif_hf_client_cb.state == BTHF_CLIENT_CONNECTION_STATE_CONNECTING)
    {
        BTIF_TRACE_DEBUG("%s, RFC/SLC connecting!", __FUNCTION__);
        return BT_STATUS_BUSY;
    }
    //SLC disconnecting
    else if (btif_hf_client_cb.state == BTHF_CLIENT_CONNECTION_STATE_DISCONNECTING)
    {
        BTIF_TRACE_DEBUG("%s, RFC/SLC disconnecting!", __FUNCTION__);
        return BT_STATUS_BUSY;
    }
    //SLC disconnected
    else if (btif_hf_client_cb.state == BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED)
    {
        BTIF_TRACE_DEBUG("%s, RFC/SLC disconnected!", __FUNCTION__);
    }

    //only init
    if (bt_hf_client_callbacks)
    {
        BTIF_TRACE_DEBUG("%s, only init hf_client callbacks, no connection", __FUNCTION__);
        btif_hf_client_cb.disable_state = BTHF_CLIENT_DISABLE_CLEANUP;
        cleanup();
        return BT_STATUS_SUCCESS;
    }
    return BT_STATUS_SUCCESS;
}

static const bthf_client_ex_interface_t bthfClientExInterface = {
    sizeof(bthf_client_ex_interface_t),
    .init_ex = init_ex,
    .cleanup_ex = cleanup_ex,
    .enable = enable,
    .enable_ex = enable_ex,
    .disable = disable,
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
    .select_pb_storage = select_pb_storage,
    .set_pb_storage = set_pb_storage,
    .set_charset = set_charset,
    .read_pb_entry = read_pb_entry,
#endif
};

const bthf_client_ex_interface_t *btif_hf_ex_client_get_interface(void)
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &bthfClientExInterface;
}
#endif

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
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
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

#include <string.h>
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_avrcp_tg_if.h"

#include "mtk_bt_service_avrcp_wrapper.h"

#define BTMW_RPC_TEST_CMD_KEY_AVRCP_TG     "MW_RPC_AVRCP_TG"

extern CHAR g_avrcp_addr_test[18];

static int btmw_rpc_test_rc_tg_update_volume_cmd_handler(int argc, char *argv[])
{
    UINT8 u1volume = 0;
    u1volume = atoi(argv[0]);
    BTMW_RPC_TEST_Logi("[AVRCP] %s() volume=%d\n", __func__, u1volume);
    a_mtkapi_avrcp_update_absolute_volume(u1volume);
    return 0;
}

static int btmw_rpc_test_rc_tg_update_mediaInfo_cmd_handler(int argc, char *argv[])
{
    BT_AVRCP_PLAYER_MEDIA_INFO player_media_info;
    BOOL track_selected = 0;
    BOOL large_element = 0;
    track_selected = atoi(argv[0]);
    large_element = atoi(argv[1]);

    memset(&player_media_info, 0, sizeof(player_media_info));
    if (large_element)
    {
        memset(&player_media_info.media_info.title, 'a', sizeof(player_media_info.media_info.title));
        player_media_info.media_info.title[BT_AVRCP_MAX_NAME_LEN-1] = 0;
        memset(&player_media_info.media_info.artist, 'a', sizeof(player_media_info.media_info.artist));
        player_media_info.media_info.artist[BT_AVRCP_MAX_NAME_LEN-1] = 0;
    }
    else
    {
        strcpy(player_media_info.media_info.title, "rpc_media_title");
        strcpy(player_media_info.media_info.artist, "rpc_media_artist");
    }
    strcpy(player_media_info.media_info.album, "rpc_media_album");
    player_media_info.media_info.current_track_number = 1;
    player_media_info.media_info.number_of_tracks = 10;
    strcpy(player_media_info.media_info.genre, "rpc_media_genre");
    player_media_info.media_info.position = 10000;

    if (track_selected)
    {
        player_media_info.track[0] = track_selected-1;
    }
    else
    {
        memset(&player_media_info.track, 0xFF, sizeof(player_media_info.track));
    }

    BTMW_RPC_TEST_Logi("media info: \n", __func__);
    BTMW_RPC_TEST_Logi("Title: %s\n", player_media_info.media_info.title);
    BTMW_RPC_TEST_Logi("artist: %s\n", player_media_info.media_info.artist);
    BTMW_RPC_TEST_Logi("album: %s\n", player_media_info.media_info.album);
    BTMW_RPC_TEST_Logi("current_track_number: %d\n", player_media_info.media_info.current_track_number);
    BTMW_RPC_TEST_Logi("number_of_tracks: %d\n", player_media_info.media_info.number_of_tracks);
    BTMW_RPC_TEST_Logi("genre: %s\n", player_media_info.media_info.genre);
    BTMW_RPC_TEST_Logi("position: %d\n", player_media_info.media_info.position);
    BTMW_RPC_TEST_Logi("track: 0x%x, 0x%x\n", player_media_info.track[0],
        player_media_info.track[BT_AVRCP_UID_SIZE-1]);
    a_mtkapi_avrcp_update_player_media_info(&player_media_info);
    return 0;
}

static int btmw_rpc_test_rc_tg_update_play_status_cmd_handler(int argc, char *argv[])
{
    BT_AVRCP_PLAYER_STATUS player_status;

    player_status.song_len = atoi(argv[0]);
    player_status.song_pos = atoi(argv[1]);
    player_status.play_status = atoi(argv[2]);
    BTMW_RPC_TEST_Logi("song_length: %d\n", player_status.song_len);
    BTMW_RPC_TEST_Logi("song_position: %d\n", player_status.song_pos);
    BTMW_RPC_TEST_Logi("play_status: %d\n", player_status.play_status);
    a_mtkapi_avrcp_update_player_status(&player_status);

    return 0;
}


static BTMW_RPC_TEST_CLI btmw_rpc_test_rc_tg_cli_commands[] =
{
    {"update_media_info",   btmw_rpc_test_rc_tg_update_mediaInfo_cmd_handler,     " = update_media_info <track_selected:0/1> <large:0/1>"},
    {"update_play_status",  btmw_rpc_test_rc_tg_update_play_status_cmd_handler,   " = update_play_status <song_len> <song_pos> <status>"},
    {"update_volume",       btmw_rpc_test_rc_tg_update_volume_cmd_handler,        " = update_volume <volume:0~100> "},
    {NULL, NULL, NULL},
};

int btmw_rpc_test_rc_tg_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count = 0;

    BTMW_RPC_TEST_Logd("[AVRCP] argc: %d, argv[0]: %s\n", argc, argv[0]);

    cmd = btmw_rpc_test_rc_tg_cli_commands;
    while (cmd->cmd)
    {
        if (!strcmp(cmd->cmd, argv[0]))
        {
            match = cmd;
            count = 1;
            break;
        }
        cmd++;
    }

    if (count == 0)
    {
        BTMW_RPC_TEST_Logd("[AVRCP] Unknown command '%s'\n", argv[0]);
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_AVRCP_TG, btmw_rpc_test_rc_tg_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}


int btmw_rpc_test_rc_tg_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD rc_mod = {0};

    BTMW_RPC_TEST_Logd("[AVRCP] %s() \n", __func__);

    rc_mod.mod_id = BTMW_RPC_TEST_MOD_AVRCP_TG;
    strncpy(rc_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_AVRCP_TG, sizeof(rc_mod.cmd_key));
    rc_mod.cmd_handler = btmw_rpc_test_rc_tg_cmd_handler;
    rc_mod.cmd_tbl = btmw_rpc_test_rc_tg_cli_commands;

    ret = btmw_rpc_test_register_mod(&rc_mod);
    BTMW_RPC_TEST_Logd("[AVRCP] btmw_rpc_test_register_mod() for TG returns: %d\n", ret);


    return ret;
}

int btmw_rpc_test_rc_tg_deinit()
{
    BTMW_RPC_TEST_Logd("[AVRCP] %s() \n", __func__);

    return 0;
}

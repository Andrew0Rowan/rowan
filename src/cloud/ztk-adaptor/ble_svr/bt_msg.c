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

#include <stddef.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "adaptor_log.h"
#include "bt_msg.h"
#include "gattc_handler.h"
#include "gatts_handler.h"
#include "gatt_handler.h"


static pthread_t  g_bt_app_msg_thread;
static int      g_bt_app_msg_queue_id = 0;
static int      g_bt_app_msg_thread_exit = 0;

static void bt_app_msg_handle(BT_APP_MSG *p_msg)
{
    APDATOR_LOG(INFO, "event:%d", p_msg->hdr.event);

    switch(p_msg->hdr.event) {
        case BT_APP_GATTC_EVENT:
            bt_app_gattc_msg_handle(&p_msg->data.gattc_msg);
            break;
        case BT_APP_GATTS_EVENT:
            bt_app_gatts_msg_handle(&p_msg->data.gatts_msg);
            break;
        case BT_APP_GATT_EVENT:
            bt_app_gatt_msg_handle(&p_msg->data.gatts_msg);
            break;
        default:
            APDATOR_LOG(ERROR, "unknown: event:%d", p_msg->hdr.event);
            break;
    }
}

BOOL bt_app_send_msg(BT_APP_MSG *msg)
{
    APDATOR_LOG(INFO,"send msg(queue_id=%d)",g_bt_app_msg_queue_id);

    int i4Ret;
    BT_APP_IPC_MSG msg_struct = {0};
    size_t tx_size = 0;
    msg_struct.tMsgType = 1;
    memcpy(&(msg_struct.body), msg, sizeof(BT_APP_MSG));

    if (msg->hdr.len != 0) {
        tx_size = msg->hdr.len + sizeof(msg->hdr);
    } else {
        tx_size = sizeof(BT_APP_MSG);
    }

    i4Ret = msgsnd(g_bt_app_msg_queue_id, &msg_struct, tx_size, 0 );
    if (-1 == i4Ret) {
        APDATOR_LOG(ERROR,"send %lu failed %s(%ld)!", tx_size, strerror(errno), (long)errno);
        return FALSE;
    } else {
        APDATOR_LOG(INFO,"send %lu bytes!", tx_size);
    }
    return TRUE;
}

void bt_app_msg_queue_destroy()
{
    APDATOR_LOG(INFO,"!!!");
    g_bt_app_msg_thread_exit = 1;
    return;
}

static void *bt_app_msg_recv_thread(void * args)
{
    APDATOR_LOG(INFO,"Enter %s", __FUNCTION__);

    int ret = 0;
    BT_APP_IPC_MSG msg_struct;
    static BT_APP_MSG *t_gatt_msg;

    prctl(PR_SET_NAME, "bt_app_msg_recv_thread", 0, 0, 0);

    while(1)
    {
        if (g_bt_app_msg_thread_exit) {
           APDATOR_LOG(INFO, "recv msg size: %ld ", (long)ret);
           if (-1 == msgctl(g_bt_app_msg_queue_id, IPC_RMID, NULL)) {
               APDATOR_LOG(ERROR,"msg destroy failed!");
           }
           break;
        }
        memset(&msg_struct, 0, sizeof(msg_struct));
        ret = msgrcv(g_bt_app_msg_queue_id, &msg_struct, sizeof(BT_APP_MSG), 0, 0);
        APDATOR_LOG(INFO, "recv msg(queue_id=%d) size: %ld ", g_bt_app_msg_queue_id,(long)ret);

        t_gatt_msg = &(msg_struct.body);
        if ( ret > 0 ) {
            bt_app_msg_handle(t_gatt_msg);
        } else if ( ret == 0 ) {
            /* do nothing */
        } else {
            APDATOR_LOG(INFO, "receive message failed %d", errno);
        }
    }
    APDATOR_LOG(ERROR,"bt_app_msg_recv_thread exit!");

    return NULL;
}

int bt_app_queue_init_new(void)
{
    APDATOR_LOG(INFO,"Enter %s", __FUNCTION__);

    key_t key;
    unsigned int ui4_que_id;
    int i4_ret = 0;

    /**
     * Create message queue
     */
    //remember to set the path
    key = ftok(".", 1);
    if (-1 == key)
    {
        APDATOR_LOG(ERROR, "ftok failed!\n");
        return -1;
    }

    ui4_que_id = msgget(key, IPC_CREAT);
    if ((-1) == ui4_que_id) {
        APDATOR_LOG(ERROR, " get queue id failed!!");
        return -1;
    }
    g_bt_app_msg_queue_id = ui4_que_id;

    APDATOR_LOG(ERROR, "++++++++++  queue_id=%d",g_bt_app_msg_queue_id);
    pthread_attr_t attr;
    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret) {
        APDATOR_LOG(ERROR, "pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return i4_ret;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret) {
        if (0 != (i4_ret = pthread_create(&g_bt_app_msg_thread,
                                          &attr,
                                          bt_app_msg_recv_thread,
                                          NULL))) {
            APDATOR_LOG(ERROR, "pthread_create i4_ret:%ld", (long)i4_ret);
            assert(0);
        }
    } else {
        APDATOR_LOG(ERROR, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }

    g_bt_app_msg_thread_exit = 0;

    pthread_attr_destroy(&attr);
    return 0;
}


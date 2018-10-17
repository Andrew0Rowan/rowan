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

#include "u_common.h"

#include "ble.h"
#include "ble_msg.h"
#include "ble_gatt.h"

static pthread_t  g_ble_msg_thread;
static INT32      g_ble_msg_queue_id = 0;
static INT32      g_ble_msg_thread_exit = 0;

static VOID ble_msg_handle(BLE_MSG *p_msg)
{
    BLE_LOG(BLE_LOG_INFO, "event:%d", p_msg->hdr.event);

    switch(p_msg->hdr.event) {
        case BLE_SERVER_EVENT:
            ble_gatt_server_msg_handle(&p_msg->data.svr_msg);
            break;
        case BLE_CLIENT_EVENT:
            BLE_LOG(BLE_LOG_INFO, "ble client event:%d", p_msg->hdr.event);
            break;
        default:
            BLE_LOG(BLE_LOG_ERR, "unknown: event:%d", p_msg->hdr.event);
            break;
    }
}

INT32 ble_send_msg(BLE_MSG *msg)
{
    BLE_LOG(BLE_LOG_INFO,"send msg(queue_id=%d)",g_ble_msg_queue_id);

    INT32 i4Ret;
    BLE_IPC_MSG msg_struct = {0};
    size_t tx_size = 0;
    msg_struct.tMsgType = 1;
    memcpy(&(msg_struct.body), msg, sizeof(BLE_MSG));

    if (msg->hdr.len != 0) {
        tx_size = msg->hdr.len + sizeof(msg->hdr);
    } else {
        tx_size = sizeof(BLE_MSG);
    }

    i4Ret = msgsnd(g_ble_msg_queue_id, &msg_struct, tx_size, 0 );
    if (-1 == i4Ret) {
        BLE_LOG(BLE_LOG_ERR,"send %lu failed %s(%ld)!", tx_size, strerror(errno), (long)errno);
        return BLE_FAIL;
    } else {
        BLE_LOG(BLE_LOG_INFO,"send %lu bytes!", tx_size);
    }
    return BLE_OK;
}

VOID ble_msg_queue_destroy()
{
    BLE_LOG(BLE_LOG_INFO,"!!!");
    g_ble_msg_thread_exit = 1;
    return;
}

static VOID *ble_msg_recv_thread(VOID * args)
{
    BLE_LOG(BLE_LOG_INFO,"Enter");

    INT32 ret = 0;
    BLE_IPC_MSG *msg_struct = NULL;
    static BLE_MSG *ble_msg;

    msg_struct = (BLE_IPC_MSG *)malloc(sizeof (BLE_IPC_MSG));

    if (msg_struct == NULL) {
        BLE_LOG(BLE_LOG_INFO,"msg_struct is NULL");
        return NULL;
    }

    prctl(PR_SET_NAME, "ble_msg_recv_thread", 0, 0, 0);

    while(1)
    {
        if (g_ble_msg_thread_exit) {
           BLE_LOG(BLE_LOG_INFO, "recv msg size: %ld ", (long)ret);
           if (-1 == msgctl(g_ble_msg_queue_id, IPC_RMID, NULL)) {
               BLE_LOG(BLE_LOG_ERR,"msg destroy failed!");
           }
           break;
        }
        memset(msg_struct, 0, sizeof(msg_struct));
        ret = msgrcv(g_ble_msg_queue_id, msg_struct, 1024, 0, 0);
        BLE_LOG(BLE_LOG_INFO, "recv msg(queue_id=%d) size: %ld ", g_ble_msg_queue_id,(long)ret);

        ble_msg = &(msg_struct->body);
        if ( ret > 0 ) {
            ble_msg_handle(ble_msg);
        } else if ( ret == 0 ) {
            /* do nothing */
        } else {
            BLE_LOG(BLE_LOG_INFO, "receive message failed %d", errno);
        }
    }
    BLE_LOG(BLE_LOG_ERR,"ble_msg_recv_thread exit!");

    free(msg_struct);

    return NULL;
}

INT32 ble_queue_init_new(VOID)
{
    BLE_LOG(BLE_LOG_INFO,"Enter %s", __FUNCTION__);

    key_t key;
    UINT32 ui4_que_id;
    INT32 i4_ret = 0;

    /**
     * Create message queue
     */
    //remember to set the path
    key = ftok(".", 1);
    if (-1 == key)
    {
        BLE_LOG(BLE_LOG_ERR, "ftok failed!\n");
        return -1;
    }

    ui4_que_id = msgget(key, IPC_CREAT);
    if ((-1) == ui4_que_id) {
        BLE_LOG(BLE_LOG_ERR, " get queue id failed!!");
        return -1;
    }
    g_ble_msg_queue_id = ui4_que_id;

    BLE_LOG(BLE_LOG_ERR, "++++++++++  queue_id=%d",g_ble_msg_queue_id);
    pthread_attr_t attr;
    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret) {
        BLE_LOG(BLE_LOG_ERR, "pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return i4_ret;
    }
    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 == i4_ret) {
        if (0 != (i4_ret = pthread_create(&g_ble_msg_thread,
                                          &attr,
                                          ble_msg_recv_thread,
                                          NULL))) {
            BLE_LOG(BLE_LOG_ERR, "pthread_create i4_ret:%ld", (long)i4_ret);
            assert(0);
        }
    } else {
        BLE_LOG(BLE_LOG_ERR, "pthread_attr_setdetachstate i4_ret:%ld", (long)i4_ret);
    }

    g_ble_msg_thread_exit = 0;

    pthread_attr_destroy(&attr);
    return 0;
}


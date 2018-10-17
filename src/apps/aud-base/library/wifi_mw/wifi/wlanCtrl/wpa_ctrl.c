/*
 * wpa_supplicant/hostapd control interface library
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include "wpa_ctrl.h"
#include <stdio.h>
#include "_sys_config.h"
#include "wifi_log.h"

#include <sys/un.h>

#include "u_common.h"
#include "unistd.h"
#include "u_net_wlan.h"
#include <malloc.h>



/**
 * struct wpa_ctrl - Internal structure for control interface library
 *
 * This structure is used by the wpa_supplicant/hostapd control interface
 * library to store internal data. Programs using the library should not touch
 * this data directly. They can only use the pointer to the data structure as
 * an identifier for the control interface connection and use this as one of
 * the arguments for most of the control interface library functions.
 */
struct wpa_ctrl
{
    int s;
    struct sockaddr_un local;
    struct sockaddr_un dest;
};

#define AID_WIFI            1008

extern BOOL g_flagCastForAudioEnable;

static int wpa_show_errno(void)
{
    WIFI_LOG(WIFI_ERROR,("errno: %d, %s\n", errno, strerror(errno)));
    return(0);
}



struct wpa_ctrl * wpa_ctrl_open(const char *ctrl_path, const char *bind_path)
{
    struct wpa_ctrl *ctrl;

    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Enter wpa_ctrl_open  ctrl_path = %s.\n", ctrl_path));

    ctrl = x_mem_alloc(sizeof(*ctrl));
    if (ctrl == NULL)
        return NULL;
    x_memset(ctrl, 0, sizeof(*ctrl));

    ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (ctrl->s < 0)
    {
        WIFI_LOG(WIFI_DEBUG,("[ERR] wpa_ctrl_open.socket(PF_UNIX, SOCK_STREAM, 0) fail ctrl->s %d.\n", ctrl->s));
        x_mem_free(ctrl);
        return NULL;
    }

    ctrl->local.sun_family = AF_UNIX;

    x_snprintf(ctrl->local.sun_path, sizeof(ctrl->local.sun_path), "%s", bind_path);

    WIFI_LOG(WIFI_DEBUG,("{unlink} wpa_ctrl_open.bind() ctrl->s %d, ctrl->local.sun_path : %s\n",
               ctrl->s, ctrl->local.sun_path));
    unlink(bind_path);
    int i4Bend = 0;
    i4Bend = bind(ctrl->s, (struct sockaddr *) &ctrl->local, sizeof(ctrl->local));
    if (i4Bend < 0)
    {
        wpa_show_errno();
        close(ctrl->s);
        x_mem_free(ctrl);
        WIFI_LOG(WIFI_DEBUG,("[ERR] wpa_ctrl_open.bind() fail.\n"));
        return NULL;
    }

    WIFI_LOG(WIFI_DEBUG,("wpa_ctrl_open.bind() i4bind %d.\n", i4Bend));

	if ( g_flagCastForAudioEnable ) {
        int ret = -1;
        ret = chmod(ctrl->local.sun_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if ( ret < 0 ) {
            WIFI_LOG(WIFI_ERROR,("Chmod fail, errno: %d, %s\n", 
                errno, strerror(errno)));
        }

        gid_t wpas_gid = AID_WIFI;

        WIFI_LOG(WIFI_ERROR,("wpas_gid = %d\n", wpas_gid));

        ret = chown(ctrl->local.sun_path, -1, wpas_gid);
        if ( ret < 0 ) {
            WIFI_LOG(WIFI_ERROR,("Chown fail, errno: %d, %s\n", 
                errno, strerror(errno)));
        }
    }

    ctrl->dest.sun_family = AF_UNIX;
    x_snprintf(ctrl->dest.sun_path, sizeof(ctrl->dest.sun_path), "%s", ctrl_path);

    WIFI_LOG(WIFI_DEBUG,("wpa_ctrl_open.connect() ctrl->s %d, ctrl->dest.sun_path : %s\n",
               ctrl->s, ctrl->dest.sun_path));

    int conn = 0;
    conn = (int)(connect(ctrl->s, (struct sockaddr *) &ctrl->dest, sizeof(ctrl->dest)));
    if (conn < 0)
    {
        wpa_show_errno();

        close(ctrl->s);
        unlink(ctrl->local.sun_path);
        x_mem_free(ctrl);
        WIFI_LOG(WIFI_DEBUG,("[ERR] wpa_ctrl_open.connect() fail, connect = %d.\n", conn));
        return NULL;
    }
    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Leave wpa_ctrl_open(),conn = %d.\n", conn));

    return ctrl;
}


void wpa_ctrl_close(struct wpa_ctrl *ctrl)
{
    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Enter wpa_ctrl_close() ctrl->s: %d\n", ctrl->s));

    unlink(ctrl->local.sun_path);
    close(ctrl->s);
    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Leave wpa_ctrl_close() ctrl->s: %d\n", ctrl->s));
    x_mem_free(ctrl);
}


int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
                     char *reply, size_t *reply_len,
                     void (*msg_cb)(void *msg, size_t len))
{
    struct timeval tv;
    int res;
    fd_set rfds;
    const char *_cmd;
    char *cmd_buf = NULL;
    size_t _cmd_len;
    int bitidx = ctrl->s;
    WIFI_LOG_ENTRY(WIFI_API, ("wpa_ctrl_request"));
    _cmd = cmd;

    _cmd_len = cmd_len;

    WIFI_LOG(WIFI_DEBUG,("[HQ]%d[_cmd=%s]%d.\n", ctrl->s, _cmd, _cmd_len));
    if (send(bitidx, _cmd, _cmd_len, 0) < 0)
    {
        WIFI_LOG(WIFI_DEBUG,("send() fail, errorno=%d, @%s()\n", errno, __FUNCTION__));
        x_mem_free(cmd_buf);
        return -1;
    }
    x_mem_free(cmd_buf);

    for (;;)
    {
        tv.tv_sec = 5;
        tv.tv_usec = 0;
		
		WIFI_LOG(WIFI_INFO,("set select timeout %d, @%s()\n", tv.tv_sec,__FUNCTION__));
        FD_ZERO(&rfds);
        FD_SET(bitidx, &rfds);
        res = select(bitidx + 1, &rfds, NULL, NULL, &tv);
        if (FD_ISSET(bitidx, &rfds))
        {
            res = recv(bitidx, reply, *reply_len, 0);
            if (res < 0)
            {
                WIFI_LOG(WIFI_DEBUG,("recv() fail, @%s()\n", __FUNCTION__));
                return res;
            }
            if (res > 0 && reply[0] == '<')
            {
                WIFI_LOG(WIFI_DEBUG,("unsolicited message\n"));
                /* This is an unsolicited message from
                 * wpa_supplicant, not the reply to the
                 * request. Use msg_cb to report this to the
                 * caller. */
                if (msg_cb)
                {
                    /* Make sure the message is nul
                     * terminated. */
                    if ((size_t) res == *reply_len)
                        res = (*reply_len) - 1;
                    reply[res] = '\0';
                    msg_cb(reply, res);
                }
                continue;
            }
            *reply_len = res;
            break;
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG,("select() timeout, @%s()\n", __FUNCTION__));
            return -2;
        }
    }
    return 0;
}


static int wpa_ctrl_attach_helper(struct wpa_ctrl *ctrl, int attach)
{
    char buf[10];
    int ret;
    size_t len = 10;
    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Enter wpa_ctrl_attach_helper.\n"));

    ret = wpa_ctrl_request(ctrl, attach ? "ATTACH" : "DETACH", 6,
                           buf, &len, NULL);
    if (ret < 0)
        return ret;
    if (len == 3 && x_memcmp(buf, "OK\n", 3) == 0)
        return 0;
    return -1;
}


int wpa_ctrl_attach(struct wpa_ctrl *ctrl)
{
    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Enter wpa_ctrl_attach.\n"));

    return wpa_ctrl_attach_helper(ctrl, 1);
}


int wpa_ctrl_detach(struct wpa_ctrl *ctrl)
{
    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Enter wpa_ctrl_detach.\n"));

    return wpa_ctrl_attach_helper(ctrl, 0);
}



int wpa_ctrl_recv(struct wpa_ctrl *ctrl, void *reply, size_t *reply_len)
{
    int res;

    res = recv(ctrl->s, reply, *reply_len, 0);
    if (res < 0)
    {
        WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Leave wpa_ctrl_recv(), fail, res = %d.\n", res));
        return res;
    }
    *reply_len = res;
    return 0;
}


int wpa_ctrl_send_nfy_ret(struct wpa_ctrl *ctrl, void *cmd, size_t cmd_len)
{
    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] wpa_ctrl_send_nfy_ret, Before send() ctrl->s %d, i4NotifykId %d, i4Ret %d, cmd_len %d.\n",
               ctrl->s, (int)((NET_802_11_NFY_T *)cmd)->i4NotifykId, (int)((NET_802_11_NFY_T *)cmd)->i4Ret, cmd_len));
    int i4send = 0;
    i4send = (int)(send(ctrl->s, cmd, cmd_len, 0));
    if (i4send < 0)
    {
        wpa_show_errno();
        WIFI_LOG(WIFI_DEBUG,("[ERR] wpa_ctrl_send_nfy_ret, send fail i4send %d.\n", i4send));
        return -1;
    }
    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] wpa_ctrl_send_nfy_ret, After send() i4send %d.\n", i4send));
    return 0;
}


int wpa_ctrl_pending(struct wpa_ctrl *ctrl)
{
    if (ctrl == NULL)
    {
        WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Enter wpa_ctrl_pending ctrl = null.\n"));
        return -1;
    }
    struct timeval tv;
    fd_set rfds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(ctrl->s, &rfds);
    select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
    return FD_ISSET(ctrl->s, &rfds);
}


int wpa_ctrl_get_fd(struct wpa_ctrl *ctrl)
{
    WIFI_LOG(WIFI_DEBUG,("[WPA_CTRL] Enter wpa_ctrl_get_fd(CTRL_IFACE_SOCKET).\n"));

    return ctrl->s;
}



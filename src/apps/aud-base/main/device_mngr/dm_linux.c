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
 * $RCSfile: dm_linux.c,v $
 * $Revision:
 * $Date:
 * $SWAuthor: Yan Wang $
 *
 * Description:
 *         This file contains all the transition effect interface APIs
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <net/if.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "dm.h"
#include "dm_linux.h"
#include "dmh.h"
#include "dmhlib.h"
#include "u_os.h"
#include "u_handle.h"
#include "u_amb.h"
#include "u_fm.h"
#include "u_rwlock.h"
#include "u_assert.h"
#include "u_appman.h"
#include "u_wifi_utils.h"

struct
{
    FM_MNT_INFO_T t_mnt_info;
    HANDLE_T h_mnt_rwlock;
} g_t_dm_mnt_info;

BOOL _fgDescNotFree = FALSE;
BOOL g_callback_ctr = TRUE;
DM_CORE_T t_dm_core;
DM_DEV_DESC_T *pt_dev_desc_tmp = NULL;

static INT32 _dm_chrdev_get_size(INT32  i4_fd, UINT32 *pui4_size)
{
    INT32 i4_ret;
    struct stat t_stat;

    if (NULL == pui4_size)
    {
        return DMR_INVALID_PARM;
    }

    i4_ret = fstat(i4_fd, &t_stat);

    if (i4_ret != 0)
    {
        return DMR_FM_ERR;
    }

    *pui4_size = (UINT32)(t_stat.st_size);

    return DMR_OK;
}

static BOOL _dm_dev_approve(DM_COND_T *pt_cond,DM_HW_TYPE_T t_hw_type,DM_DEV_TYPE_T t_dev_type)
{
    BOOL b_hw = FALSE;
    BOOL b_dev = FALSE;

    if ((DM_HW_TYPE_UNKNOWN == pt_cond->t_hw_type)||(t_hw_type == pt_cond->t_hw_type))
    {
        b_hw = TRUE;
    }

    if ((DM_DEV_UKW_TYPE_UNKNOWN == pt_cond->t_dev_type)||
        ((DM_DEV_SKT_TYPE_UNKNOWN == pt_cond->t_dev_type)&&(DM_DEV_CHR_SOCKET == DM_DEV_CHR(t_dev_type)))||
        ((DM_DEV_MED_TYPE_UNKNOWN == pt_cond->t_dev_type)&&(DM_DEV_CHR_MEDIUM == DM_DEV_CHR(t_dev_type)))||
        (t_dev_type == pt_cond->t_dev_type))
    {
        b_dev = TRUE;
    }

    return (b_hw && b_dev);
}

static DM_DEV_TYPE_T _dm_get_dev_type_from_dmh(DMH_DEV_TYPE_T t_dev_type)
{
    switch (t_dev_type)
    {
        case DMH_DEV_MED_TYPE_USBSTORAGE:       return DM_DEV_MED_TYPE_MASS_STRG;
        case DMH_DEV_MED_TYPE_ATA_HDD:          return DM_DEV_MED_TYPE_HDD;
        case DMH_DEV_MED_TYPE_USBPARTI:         return DM_DEV_MED_TYPE_MASS_STRG_PARTITION;
        case DMH_DEV_SKT_TYPE_FAKEHUB:          return DM_DEV_SKT_TYPE_HUB;
        case DMH_DEV_SKT_TYPE_REALHUB:          return DM_DEV_SKT_TYPE_HUB;
        case DMH_DEV_SKT_TYPE_ROOTHUB:          return DM_DEV_SKT_TYPE_HUB;
        case DMH_DEV_MED_TYPE_ATA_HDD_PARTI:    return DM_DEV_MED_TYPE_HDD_PARTITION;
        case DMH_DEV_SKT_TYPE_OPTI_DRV:         return DM_DEV_SKT_TYPE_OPTI_DRV;
        case DMH_DEV_UKW_TYPE_UNKNOWN:          return DM_DEV_UKW_TYPE_UNKNOWN;
        case DMH_DEV_SKT_TYPE_RTSTA:            return DM_DEV_SKT_TYPE_RTSTA;
        case DMH_DEV_SKT_TYPE_7650:             return DM_DEV_SKT_TYPE_7650;
        case DMH_DEV_SKT_TYPE_7632:             return DM_DEV_SKT_TYPE_7632;
        case DMH_DEV_MED_TYPE_USB_EP:           return DM_DEV_MED_TYPE_USB_EP;
        case DMH_DEV_SKT_TYPE_UNSUPPORT_WIFI:   return DM_DEV_SKT_TYPE_UNSUPPORT_WIFI;
        case DMH_DEV_SKT_TYPE_BLUETOOTH:        return DM_DEV_SKT_TYPE_BLUETOOTH;
        default:                                ASSERT(0);
    }

    return DM_CB_DEV_UNSUPPORT;
}

static DM_CB_TYPE_T _dm_get_cb_type_from_dmh(DEV_STATUS_T t_dev_stat)
{
    switch (t_dev_stat)
    {
        case DEV_STATUS_HUB_ATTACH:      return DM_CB_DEV_ATTACH;
        case DEV_STATUS_MED_ATTACH:      return DM_CB_DEV_ATTACH;
        case DEV_STATUS_HUB_DETACH:      return DM_CB_DEV_DETACH;
        case DEV_STATUS_MED_DETACH:      return DM_CB_DEV_DETACH;
        case DEV_STATUS_PART_DETECTED:   return DM_CB_DEV_PART_DETECTED;
        case DEV_STATUS_PART_REMOVED:    return DM_CB_DEV_PART_REMOVED;
        case DEV_STATUS_DEV_FAIL:        return DM_CB_DEV_FAIL;
        case DEV_STATUS_DEV_UNSUPPORT:   return DM_CB_DEV_UNSUPPORT;
        default:                         ASSERT(0);
    }

    return DM_CB_DEV_UNSUPPORT;
}

static VOID _dm_broadcast_mnt_msg(DM_EVENT_E e_dm_event)
{
    INT32 i4_ret;
    DM_MNT_MSG_T t_dm_mnt_msg;
    AMB_BROADCAST_MSG_T t_msg;

    t_dm_mnt_msg.e_dm_event = e_dm_event;

    t_msg.ui4_type = E_APP_MSG_TYPE_USB_DEV;
    t_msg.z_msg_len = sizeof(t_dm_mnt_msg);
    ASSERT(t_msg.z_msg_len <= BROADCAST_MSG_MAX_LEN);
    memcpy(t_msg.uc_msg, &t_dm_mnt_msg, t_msg.z_msg_len);

    DM_ERR("dm broadcast usb %s msg\n", DM_DEV_EVT_UNMOUNTED == e_dm_event ? "umnt" : "mnt");

    i4_ret = u_amb_broadcast_msg(&t_msg, TRUE);
    if(FMR_OK != i4_ret)
    {
        DM_ERR("u_amb_broadcast_msg return %ld\n", i4_ret);
    }
}

static VOID _dm_broadcast_network_msg(DM_NETWORK_EVENT_E e_network_event)
{
    INT32 i4_ret;
    DM_NETWORK_MSG_T t_network_msg;
    AMB_BROADCAST_MSG_T t_msg;

    t_network_msg.e_network_event = e_network_event;

    t_msg.ui4_type = E_APP_MSG_TYPE_NETWORK;
    t_msg.z_msg_len = sizeof(t_network_msg);
    ASSERT(t_msg.z_msg_len <= BROADCAST_MSG_MAX_LEN);
    memcpy(t_msg.uc_msg, &t_network_msg, t_msg.z_msg_len);

    DM_ERR("dm broadcast network %s msg\n", DM_NETWORK_DISCONNECT == e_network_event ? "disconnect" : "connect");

    i4_ret = u_amb_broadcast_msg(&t_msg, TRUE);
    if(FMR_OK != i4_ret)
    {
        DM_ERR("u_amb_broadcast_msg return %ld\n", i4_ret);
    }

}

static VOID _dm_send_mnt_msg_to_nfy_thread(BOOL b_mnt)
{
    INT32 i4_ret;
    DM_NFY_REQ_T t_nfy_req;

    t_nfy_req.e_msgtype = DM_MSG_TYPE_NORMAL;
    t_nfy_req.e_event = (TRUE == b_mnt) ? DM_DEV_EVT_MOUNTED : DM_DEV_EVT_UNMOUNTED;

    i4_ret = x_msg_q_send(t_dm_core.h_nfy_req_q,
                          (const VOID *) &t_nfy_req,
                          sizeof(DM_NFY_REQ_T),
                          DM_NFY_REQ_PRIORITY);
    if (OSR_OK != i4_ret)
    {
        DM_ERR("send msg to close nfy thread failed!\n");
    }
}

static VOID _dm_modify_mnt_info(BOOL b_mnt, FM_MNT_INFO_T *pt_mnt_info)
{
    DM_INFO("IN\n");

    if (RWLR_OK != u_rwl_write_lock(g_t_dm_mnt_info.h_mnt_rwlock, RWL_OPTION_WAIT))
    {
        DM_ERR("<write lock> mnt info's rwlock failed!\n");
        return;
    }

    if (b_mnt)
    {
        g_t_dm_mnt_info.t_mnt_info = *pt_mnt_info;
    }
    g_t_dm_mnt_info.t_mnt_info.b_mnt = b_mnt;

    u_rwl_release_lock(g_t_dm_mnt_info.h_mnt_rwlock);

    _dm_send_mnt_msg_to_nfy_thread(b_mnt);

    DM_INFO("OUT\n");
}
static VOID _dm_automnt_mnt_thread(VOID *pv_data)
{
    INT32    i4_ret;
    UINT32   ui4_count =0;
    UINT32   ui4_mount_loop = 0;
    FM_MNT_TYPE_E e_mnt_type;
    HANDLE_T h_mnt_root     = NULL_HANDLE;
    HANDLE_T h_dev_root_lbl = NULL_HANDLE;
    CHAR     *ps_dev_name   = (CHAR *)pv_data;
    FM_PART_INFO_T t_part_info = {0};

    i4_ret = u_fm_get_part_ns(h_dev_root_lbl,(const CHAR *)ps_dev_name,&ui4_count);
    if (FMR_OK != i4_ret)
    {
        DM_ERR("u_fm_get_part_ns:%s error.", ps_dev_name);
        goto exit;
    }

    DM_INFO("u_fm_get_part_ns: ui4_count=%ld.\n", ui4_count);
    if (1 < ui4_count)
    {
        goto exit;
    }

    i4_ret = u_fm_set_dir_path(FM_ROOT_HANDLE, "/data", &h_mnt_root);
    if (FMR_OK != i4_ret)
    {
        DM_ERR("u_fm_set_dir_path /data error.");
        goto exit;
    }

    i4_ret = u_fm_set_dir_path(FM_ROOT_HANDLE, "/dev", &h_dev_root_lbl);
    if (FMR_OK != i4_ret)
    {
        DM_ERR("u_fm_set_dir_path /dev/block error.");
        goto exit;
    }

    /*************** this function need hs_tianming help to edit **********************/
    i4_ret = u_fm_get_part_info(h_dev_root_lbl,
                               (const CHAR *)ps_dev_name,
                               0,
                               &t_part_info);
    if (FMR_OK != i4_ret)
    {
        DM_ERR("u_fm_get_part_info:%s error.", ps_dev_name);
        goto exit;
    }

    if (FALSE == t_part_info.b_mnt)
    {
        FM_MNT_INFO_T t_mnt_info;

        i4_ret =  u_fm_create_dir_fdr(h_mnt_root,
                                     (const CHAR *)t_part_info.ps_part_name,
                                     0666);
        if ((FMR_OK != i4_ret) && (FMR_EXIST != i4_ret))
        {
            DM_ERR("u_fm_create_dir_fdr:%s error.", t_part_info.ps_part_name);
            goto exit;
        }

        do
        {
            if (4 < ui4_mount_loop)
            {
                DM_ERR("Has try 5 times");
                break;
            }

            i4_ret = u_fm_mount(h_dev_root_lbl,
                                (const CHAR *)t_part_info.ps_part_name,
                                h_mnt_root,
                                (const CHAR *)t_part_info.ps_part_name,
                                &t_mnt_info
                                );

            if (FMR_OK != i4_ret)
            {
                DM_ERR("mount %s error. retry...", t_part_info.ps_part_name);
            }

            ui4_mount_loop ++;
        }
        while(FMR_OK != i4_ret);

        if (FMR_OK != i4_ret)
        {
            u_fm_delete_dir_fdr(h_mnt_root,(const CHAR *)t_part_info.ps_part_name);
            DM_ERR("auto mount %s error.", t_part_info.ps_part_name);
        }
        else
        {
            /*send mount sucess message to dm_nfy_thead*/
            _dm_modify_mnt_info(TRUE, &t_mnt_info);
        }
    }

exit:

    if (NULL_HANDLE != h_mnt_root)
    {
        u_fm_close(h_mnt_root);
    }

    if (NULL_HANDLE != h_dev_root_lbl)
    {
        u_fm_close(h_dev_root_lbl);
    }

    u_thread_exit();
}

static VOID _dm_automnt_umnt_thread(VOID *pv_data)
{
    INT32    i4_ret;
    HANDLE_T h_dev_root_lbl    = NULL_HANDLE;
    CHAR     *ps_dev_name      = (CHAR *)pv_data;
    FM_PART_INFO_T t_part_info = {0};

#if 1//CONFIG_MW_GOOGLE_CAST_AUDIO
    i4_ret = u_fm_set_dir_path(FM_ROOT_HANDLE, "/dev", &h_dev_root_lbl);
    if (FMR_OK != i4_ret)
    {
        DM_ERR("u_fm_set_dir_path /dev/block error.");
        goto exit;
    }
#else
    i4_ret = u_fm_set_dir_path(FM_ROOT_HANDLE, "/dev", &h_dev_root_lbl);
    if (i4_ret != FMR_OK)
    {
        DM_ERR("u_fm_set_dir_path /dev error.");
        goto exit;
    }
#endif

    i4_ret = u_fm_get_part_info(h_dev_root_lbl,
                                (const CHAR *)ps_dev_name,
                                0,
                                &t_part_info);
    if (FMR_OK != i4_ret)
    {
        DM_ERR("u_fm_get_part_info:%s error.", ps_dev_name);
        goto exit;
    }

#if 0 //Mark Out for BDP00125712. In case mount fail 'cos removing usb quickly but
      //mount list is built already. If that case also need free
      // mount list.
    if (t_part_info.b_mnt == TRUE)
#endif
    {
        i4_ret = u_fm_umount(FM_ROOT_HANDLE,(const CHAR *)t_part_info.ps_mnt_path);
        if (FMR_OK != i4_ret)
        {
            DM_ERR("auto unmount %s error.", t_part_info.ps_part_name);
        }

        i4_ret = u_fm_delete_dir_fdr(FM_ROOT_HANDLE,(const CHAR *)t_part_info.ps_mnt_path);
        if (FMR_OK != i4_ret)
        {
            DM_ERR("u_fm_delete_dir_fdr:%s error.", t_part_info.ps_mnt_path);
        }
    }

exit:
    _dm_modify_mnt_info(FALSE, NULL);

    DM_INFO("h_dev_root_lbl:%p\n", (void *)h_dev_root_lbl);
    if (NULL_HANDLE != h_dev_root_lbl)
    {
        u_fm_close(h_dev_root_lbl);
    }

    u_thread_exit();

}


static VOID _dm_check_automnt(DM_DEV_DESC_T *pt_dev_desc,BOOL b_mnt)
{
    UINT32 ui4_cnt;
    BOOL b_approve = FALSE;
    CHAR ps_thrd_name[32];

    memset(ps_thrd_name,0,32);
    if (FALSE == t_dm_core.b_automnt_init)
    {
        DM_ERR("[DM]%s(%d):b_automnt_init exit\n",__FUNCTION__,__LINE__);
        return;
    }

    ASSERT(NULL != pt_dev_desc);

    for (ui4_cnt = 0; ui4_cnt < t_dm_core.ui4_automnt_cond_ns; ui4_cnt++)
    {
        DM_COND_T *pt_cond = &(t_dm_core.pt_automnt_cond[ui4_cnt]);

        if (TRUE == _dm_dev_approve(pt_cond,pt_dev_desc->t_hw_type,pt_dev_desc->t_dev_type))
        {
            b_approve = TRUE;
            DM_INFO("[%s:%d]_dm_dev_approve\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    if (TRUE == b_approve)
    {
        INT32 i4_ret;
        HANDLE_T h_automnt_thrd;

        if (TRUE == b_mnt)
        {
            DM_INFO("[%s:%d] b_mnt = TRUE\n", __FUNCTION__, __LINE__);
            snprintf(ps_thrd_name,32,"mnt_%lu_%s",pt_dev_desc->h_dev,"m");

            i4_ret = u_thread_create(&h_automnt_thrd,
                                     ps_thrd_name,
                                     DM_AUTOMNT_THREAD_DEFAULT_STACK_SIZE,
                                     DM_AUTOMNT_THREAD_DEFAULT_PRIORITY,
                                     _dm_automnt_mnt_thread,
                                     (sizeof(CHAR) * DM_MAX_LABEL_NAME_LEN),
                                     pt_dev_desc->ps_dev_name);
            if (OSR_OK != i4_ret)
            {
                DM_ERR("u_thread_create: %s error.", pt_dev_desc->ps_dev_name);
            }
        }
        else
        {
            DM_INFO("[%s:%d] b_mnt = FLASE\n", __FUNCTION__, __LINE__);
            snprintf(ps_thrd_name,32,"mnt_%lu_%s",pt_dev_desc->h_dev,"u");

            i4_ret = u_thread_create(&h_automnt_thrd,
                                     ps_thrd_name,
                                     DM_AUTOMNT_THREAD_DEFAULT_STACK_SIZE,
                                     DM_AUTOMNT_THREAD_DEFAULT_PRIORITY,
                                     _dm_automnt_umnt_thread,
                                     (sizeof(CHAR) * DM_MAX_LABEL_NAME_LEN),
                                     pt_dev_desc->ps_dev_name);

            if (OSR_OK != i4_ret)
            {
                DM_ERR("u_thread_create: %s error.", pt_dev_desc->ps_dev_name);
            }
        }
    }
}

INT32 dm_init_automnt(void)
{
    DM_DEV_DESC_T *pt_dev_desc = NULL;

    DM_INFO("%s,%d\n",__FUNCTION__,__LINE__);

    if (RWLR_OK != u_rwl_read_lock(t_dm_core.h_dev_rwlock, RWL_OPTION_WAIT))
    {
        return DMR_OS;
    }

    t_dm_core.b_automnt_init = TRUE;

    SLIST_FOR_EACH(pt_dev_desc, &(t_dm_core.t_dev_list), t_link)
    {
        if ((DM_DEV_CHR_MEDIUM == DM_DEV_CHR(pt_dev_desc->t_dev_type)) && (TRUE == pt_dev_desc->b_avail))
        {
            _dm_check_automnt(pt_dev_desc, TRUE);
        }
    }

    u_rwl_release_lock(t_dm_core.h_dev_rwlock);

    return DMR_OK;
}

static VOID _dm_dev_nfy_fct(VOID         *pv_nfy_tag,
                                DEV_STATUS_T  e_nfy_st,
                                VOID         *pv_data)
{
    DMH_CB_DESC_T *pt_dmh_info = (DMH_CB_DESC_T *) pv_data;
    DM_CB_MSG_T t_cb_msg;
    INT32 i4_ret;

    ASSERT(NULL != pt_dmh_info);
    ASSERT(e_nfy_st == pt_dmh_info->e_status);

    DM_INFO("-----------------_dm_dev_nfy_fct------------------.\n");
    DM_INFO("e_nfy_st=%d .\n", e_nfy_st);
    DM_INFO("pt_dmh_info->e_status=%d .\n", pt_dmh_info->e_status);
    DM_INFO("pt_dmh_info->ps_dev_name=%s .\n", pt_dmh_info->ps_dev_name);
    DM_INFO("pt_dmh_info->t_dev_type=%d .\n", pt_dmh_info->t_dev_type);
    DM_INFO("pt_dmh_info->ui4_dev_flag=%ld .\n", pt_dmh_info->ui4_dev_flag);
    DM_INFO("pt_dmh_info->ui4_major=%ld .\n", pt_dmh_info->ui4_major);
    DM_INFO("pt_dmh_info->ui4_minor=%ld .\n", pt_dmh_info->ui4_minor);
    DM_INFO("pt_dmh_info->ui4_skt_no=%ld .\n", pt_dmh_info->ui4_skt_no);
    DM_INFO("pt_dmh_info->ps_product=%ld .\n", pt_dmh_info->ui4_product);
    DM_INFO("pt_dmh_info->ps_vendor=%ld .\n", pt_dmh_info->ui4_vendor);
    DM_INFO("---------------------------------------------------.\n");

    //for handle special device ,CQ:BDP00055889, First patition haven't file system.
    if (((4704 == pt_dmh_info->ui4_product) || (4617 == pt_dmh_info->ui4_product) ||
          (4618 == pt_dmh_info->ui4_product))&& (1452 == pt_dmh_info->ui4_vendor))
    {
        if((0 == strncmp(pt_dmh_info->ps_dev_name, "sda1", 4)) ||
           (0 == strncmp(pt_dmh_info->ps_dev_name, "sdb1", 4)) ||
           (0 == strncmp(pt_dmh_info->ps_dev_name, "sdc1", 4)) ||
           (0 == strncmp(pt_dmh_info->ps_dev_name, "sdd1", 4)) ||
           (0 == strncmp(pt_dmh_info->ps_dev_name, "sde1", 4)) ||
           (0 == strncmp(pt_dmh_info->ps_dev_name, "sdf1", 4)))
        {
            DM_ERR("[DM] This is a special device.\n");
            DM_ERR("pt_dmh_info->ui2_product=%ld .\n", pt_dmh_info->ui4_product);
            DM_ERR("pt_dmh_info->ui2_vendor=%ld .\n", pt_dmh_info->ui4_vendor);
            return;
        }
    }

    t_cb_msg.e_msgtype = DM_MSG_TYPE_NORMAL;
    t_cb_msg.pv_tag    = pv_nfy_tag;

    if (DEV_STATUS_UNKNOWN == e_nfy_st)
    {
        return;
    }

    t_cb_msg.t_cb_type    = _dm_get_cb_type_from_dmh(e_nfy_st);
    t_cb_msg.ui4_unit_id  = pt_dmh_info->ui4_minor;
    t_cb_msg.ui4_skt_no   = pt_dmh_info->ui4_skt_no;
    t_cb_msg.ui4_major    = pt_dmh_info->ui4_major;
    t_cb_msg.ui4_minor    = pt_dmh_info->ui4_minor;
    t_cb_msg.t_dev_type   = _dm_get_dev_type_from_dmh(pt_dmh_info->t_dev_type);
    t_cb_msg.ui4_dev_flag = pt_dmh_info->ui4_dev_flag;
    strncpy(t_cb_msg.ps_dev_name, pt_dmh_info->ps_dev_name, sizeof(t_cb_msg.ps_dev_name));
    t_cb_msg.pv_data = NULL;
    t_cb_msg.ui4_data_sz = 0;

   if(1)//(t_cb_msg.t_dev_type!=DM_DEV_MED_TYPE_NAND)
    {
        DM_INFO("******[DM]_dm_dev_nfy_fct: _event:0x%x device_type:0x%x socket #:%ld,M:N=[%ld:%ld]******\n",
               t_cb_msg.t_cb_type,t_cb_msg.t_dev_type,t_cb_msg.ui4_skt_no,
               pt_dmh_info->ui4_major,pt_dmh_info->ui4_minor);
    }

    i4_ret = x_msg_q_send(t_dm_core.h_cb_msg_q,
                          (const VOID *) &t_cb_msg,
                          sizeof(DM_CB_MSG_T),
                          DM_CB_MSG_PRIORITY);

    if (OSR_OK != i4_ret)
    {
        DM_ERR("_dm_dev_nfy_fct: Driver delivers callback error.");
    }
}

static BOOL _dm_handle_free_fct(HANDLE_T        h_handle,
                                     HANDLE_TYPE_T   e_type,
                                     VOID            *pv_obj,
                                     VOID            *pv_tag,
                                     BOOL            b_req_handle)
{
    DM_INFO("will close fd\n");
    if (TRUE == b_req_handle)
    {
        switch (e_type)
        {
            case DMT_DEV_HANDLE:
            {
                DM_DEV_DESC_T *pt_dev_desc = (DM_DEV_DESC_T *) pv_obj;

                if (NULL != pt_dev_desc)
                {
                    if (NULL_HANDLE != pt_dev_desc->h_dev_root_lbl)
                    {
                        u_fm_close(pt_dev_desc->h_dev_root_lbl);
                    }

                    if ((INT32)pt_dev_desc->h_drv >= 0)
                    {
                        DM_INFO("close fd:%lu\n", pt_dev_desc->h_drv);
                        close(pt_dev_desc->h_drv);
                    }

                    if (NULL != pt_dev_desc->pv_data)
                    {
                        free(pt_dev_desc->pv_data);
                    }

                    if (NULL_HANDLE != pt_dev_desc->h_automnt_sema)
                    {
                        u_sema_delete(pt_dev_desc->h_automnt_sema);
                    }

                    if (NULL_HANDLE != pt_dev_desc->h_mnt_rwlock)
                    {
                        u_rwl_delete_lock(pt_dev_desc->h_mnt_rwlock);
                    }

                    if (DM_DEV_CHR_MEDIUM == DM_DEV_CHR(pt_dev_desc->t_dev_type))
                    {
#if 0 //we will del this unregister callback flow,because  we use  broadcast ..
                        x_fm_unreg_nfy_fct(pt_dev_desc->h_mnt_nfy);
#endif
                    }

                    free(pt_dev_desc);
                    //free(pv_obj);
                }

                break;
            }
            default:
                ASSERT(0);
        }
    }

    return TRUE;
}

static DM_DEV_TYPE_T _dm_get_dev_type(DRV_TYPE_T ui2_drv_type)
{
    switch (ui2_drv_type)
    {
        case DRVT_USB_MASS_STORAGE: return DM_DEV_MED_TYPE_MASS_STRG;
        case DRVT_HW_USB:           return DM_DEV_SKT_TYPE_USB_HW;
        case DRVT_HW_IDE:           return DM_DEV_SKT_TYPE_IDE_HW;
        case DRVT_HUB:              return DM_DEV_SKT_TYPE_HUB;
        default:                    return DM_DEV_UKW_TYPE_UNKNOWN;
    }

    return DM_DEV_UKW_TYPE_UNKNOWN;
}

static DM_HW_TYPE_T _dm_get_hw_type(DRV_TYPE_T ui2_drv_type)
{
    switch (ui2_drv_type)
    {
        case DRVT_HW_USB:           return DM_HW_TYPE_USB;
        case DRVT_HW_IDE:           return DM_HW_TYPE_IDE;
        default:                    ASSERT(0);
    }

    return DM_HW_TYPE_UNKNOWN;
}

static INT32 _dm_init_hw_comp(DRV_TYPE_T t_root_dev_type,UINT32 ui4_unit_id)
{
    DM_DEV_DESC_T *pt_dev_desc = NULL;
    DEV_DEV_ST_NFY_T t_nfy;
    INT32 i4_ret;
    DMH_ROOT_DEV_T t_root_reg_type = DMH_ROOT_DEV_UNSUPPORT;

    if (RWLR_OK != u_rwl_write_lock(t_dm_core.h_dev_rwlock, RWL_OPTION_WAIT))
    {
        DM_ERR("_dm_init_hw_comp lock error.");
        i4_ret = DMR_OS;
        goto skip;
    }

    pt_dev_desc = (DM_DEV_DESC_T *)malloc(sizeof(DM_DEV_DESC_T));
    if (NULL == pt_dev_desc)
    {
        DM_ERR("_dm_init_hw_comp alloc mem error.");
        i4_ret = DMR_OS;
        goto fail;
    }

    memset((VOID *) pt_dev_desc, 0, sizeof(DM_DEV_DESC_T));
    pt_dev_desc->ui4_dev_flag  = DMH_FLAG_SOCKET_DEV;
    pt_dev_desc->pt_prnt_dev   = NULL;
    pt_dev_desc->pt_root_dev   = pt_dev_desc;
    pt_dev_desc->t_dev_type    = _dm_get_dev_type(t_root_dev_type);
    pt_dev_desc->t_hw_type     = _dm_get_hw_type(t_root_dev_type);
    pt_dev_desc->ui4_skt_no    = 0;
    pt_dev_desc->ui4_unit_id   = ui4_unit_id;
    pt_dev_desc->ui4_major     = _dm_get_dev_type(t_root_dev_type);
    pt_dev_desc->ui4_minor     = ui4_unit_id;
    pt_dev_desc->pv_data       = NULL;
    pt_dev_desc->ui4_data_size = 0;
    pt_dev_desc->h_dev_root_lbl= NULL_HANDLE;
    memset(pt_dev_desc->ps_dev_name, 0, DM_MAX_LABEL_NAME_LEN);
    pt_dev_desc->h_drv         = (HANDLE_T)-1;
    pt_dev_desc->b_avail       = TRUE;

    /* Register call back */
    switch (t_root_dev_type)
    {
        case DRVT_HW_IDE:
        {
            if(0 == ui4_unit_id)
            {
                t_root_reg_type = DMH_ROOT_DEV_ATA1;
            }
            else if(1 == ui4_unit_id)
            {
                t_root_reg_type = DMH_ROOT_DEV_ATA2;
            }
            else if(2 == ui4_unit_id)
            {
                t_root_reg_type = DMH_ROOT_DEV_ATA3;
            }
            break;
        }
        case DRVT_HW_USB:
        {
            if (0 == ui4_unit_id)
            {
                t_root_reg_type = DMH_ROOT_DEV_USB1;
            }
            else if (1 == ui4_unit_id)
            {
                t_root_reg_type = DMH_ROOT_DEV_USB2;
            }
            else
            {
                t_root_reg_type = DMH_ROOT_DEV_USB3;
            }
            break;
        }
        default:
        {
            t_root_reg_type = DMH_ROOT_DEV_UNSUPPORT;
            break;
        }
    }

    t_nfy.pf_cb_nfy_fct = _dm_dev_nfy_fct;
    t_nfy.pv_nfy_tag    = pt_dev_desc;

    i4_ret = dmh_reg_root_nfy_fct(t_root_reg_type, &t_nfy);
    if (0 != i4_ret)
    {
        DM_ERR("[DM] %s: #%d: _dm_init_hw_comp rm_set device error.\n", __FUNCTION__, __LINE__);
        i4_ret = DMR_RM_ERR;
        goto fail;
    }

    SLIST_INSERT_HEAD(pt_dev_desc, &(t_dm_core.t_dev_list), t_link);

    u_rwl_release_lock(t_dm_core.h_dev_rwlock);

    /* do not need to fire event for root hardware. */
    return DMR_OK;

fail:

    if (pt_dev_desc )
    {
        free(pt_dev_desc);
    }

    u_rwl_release_lock(t_dm_core.h_dev_rwlock);

skip:

    DM_INFO("[DM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);

    return i4_ret;

}

static INT32 _dm_init_hw(VOID)
{
    INT32 i4_ret;
    DM_INFO("[DM] %s: #%d: \n", __FUNCTION__, __LINE__);

    /* Trial and error to test the hw components:  IDE 0/1/2, USB 0/1/2  */
    i4_ret = _dm_init_hw_comp(DRVT_HW_IDE, 0);
    if (DMR_OK != i4_ret)
    {
        if (DMR_DEVICE_ERR == i4_ret)
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_IDE 0 error.");
        }
        else
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_IDE 0 error.");
            goto fail;
        }
    }
    else
    {
        DM_INFO("_dm_init_hw init DRVT_HW_IDE 0 ok.\n");
    }

    i4_ret = _dm_init_hw_comp(DRVT_HW_IDE, 1);
    if (DMR_OK != i4_ret)
    {
        if (DMR_DEVICE_ERR == i4_ret)
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_IDE 1 error.");
        }
        else
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_IDE 1 error.");
            goto fail;
        }
    }
    else
    {
        DM_INFO("_dm_init_hw init DRVT_HW_IDE 1 ok.\n");
    }

    i4_ret = _dm_init_hw_comp(DRVT_HW_IDE, 2);
    if (DMR_OK != i4_ret)
    {
        if (DMR_DEVICE_ERR == i4_ret)
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_IDE 2 error.");
        }
        else
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_IDE 2 error.");
            goto fail;
        }
    }
    else
    {
        DM_INFO("_dm_init_hw init DRVT_HW_IDE 2 ok.\n");
    }


    i4_ret = _dm_init_hw_comp(DRVT_HW_USB, 0);
    if (DMR_OK != i4_ret)
    {
        if (DMR_DEVICE_ERR == i4_ret)
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_USB 0 error.");
        }
        else
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_USB 0 error.");
            goto fail;
        }
    }
    else
    {
        DM_INFO("_dm_init_hw init DRVT_HW_USB 0 ok.\n");
    }

    i4_ret = _dm_init_hw_comp(DRVT_HW_USB, 1);
    if (DMR_OK != i4_ret)
    {
        if (DMR_DEVICE_ERR == i4_ret)
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_USB 1 error.");
        }
        else
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_USB 1 error.");
            goto fail;
        }
    }
    else
    {
        DM_INFO("_dm_init_hw init DRVT_HW_USB 1 ok.\n");
    }

    i4_ret = _dm_init_hw_comp(DRVT_HW_USB, 2);
    if (DMR_OK != i4_ret)
    {
        if (DMR_DEVICE_ERR == i4_ret)
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_USB 2 error.");
        }
        else
        {
            DM_ERR("[DM] %s: #%d: %s\n", __FUNCTION__, __LINE__, "_dm_init_hw init DRVT_HW_USB 2 error.");
            goto fail;
        }
    }
    else
    {
        DM_INFO("_dm_init_hw init DRVT_HW_USB 2 ok.\n");
    }

fail:

    return i4_ret;
}


static VOID _dm_nfy_req_thread(VOID *pv_data)
{
    INT32        i4_ret;
    UINT16       ui2_q_idx=0;
    SIZE_T       z_msg_size;
    DM_NFY_REQ_T t_nfy_req;

    while (1)
    {
        z_msg_size = sizeof(DM_NFY_REQ_T);
        i4_ret = x_msg_q_receive(&ui2_q_idx,
                                 &t_nfy_req,
                                 &z_msg_size,
                                 &t_dm_core.h_nfy_req_q,
                                 1,
                                 X_MSGQ_OPTION_WAIT);
        if (OSR_OK != i4_ret)
        {
            DM_ERR("_dm_nfy_req_thread receive msg error.");
            goto fail;
        }

        if (DM_MSG_TYPE_SHUTDOWN == t_nfy_req.e_msgtype)
        {
            t_dm_core.b_nfy_thrd_exit = TRUE;
            u_thread_exit();
        }

        ASSERT(ui2_q_idx == 0);
        ASSERT(z_msg_size == sizeof(DM_NFY_REQ_T));

        switch(t_nfy_req.e_event)
        {
            case DM_DEV_EVT_MOUNTED:
            case DM_DEV_EVT_UNMOUNTED:
                _dm_broadcast_mnt_msg(t_nfy_req.e_event);
                break;
            default:
                break;
        }
    }

fail:
    u_thread_exit();
}


static VOID _dm_cb_msg_thread(VOID *pv_data)
{
    INT32  i4_ret;
    UINT16 ui2_q_idx=0;
    SIZE_T z_msg_size;
    DM_CB_MSG_T  t_cb_msg;
    static INT32 i4_count=0;

    while (1)
    {
        CHAR   *ps_dev_name;
        VOID   *pv_data;
        UINT32 ui4_unit_id;
        UINT32 ui4_skt_no;
        UINT32 ui4_major;
        UINT32 ui4_minor;
        UINT32 ui4_dev_flag;
        UINT32 ui4_data_size;
        DM_CB_TYPE_T  t_cb_type;
        DM_DEV_TYPE_T t_dev_type;
        DM_DEV_DESC_T *pt_cb_dev = NULL;

        z_msg_size = sizeof(DM_CB_MSG_T);

        i4_ret = x_msg_q_receive(&ui2_q_idx,
                                 &t_cb_msg,
                                 &z_msg_size,
                                 &t_dm_core.h_cb_msg_q,
                                 1,
                                 X_MSGQ_OPTION_WAIT);
        if (OSR_OK != i4_ret)
        {
            DM_ERR("_dm_cb_msg_thread receive msg error.");
            goto fail;
        }

        i4_count++;

        if (DM_MSG_TYPE_SHUTDOWN == t_cb_msg.e_msgtype)
        {
            t_dm_core.b_cb_thrd_exit = TRUE;
            u_thread_exit();
        }

        ASSERT(0 == ui2_q_idx);
        ASSERT(sizeof(DM_CB_MSG_T) == z_msg_size);

        t_cb_type    = t_cb_msg.t_cb_type;
        t_dev_type   = t_cb_msg.t_dev_type;
        ui4_unit_id  = t_cb_msg.ui4_unit_id;
        ui4_skt_no   = t_cb_msg.ui4_skt_no;
        ui4_major    = t_cb_msg.ui4_major;
        ui4_minor    = t_cb_msg.ui4_minor;
        ps_dev_name  = t_cb_msg.ps_dev_name;
        ui4_dev_flag = t_cb_msg.ui4_dev_flag;
        pv_data      = t_cb_msg.pv_data;
        ui4_data_size= t_cb_msg.ui4_data_sz;

        pt_cb_dev = (DM_DEV_DESC_T *) t_cb_msg.pv_tag;

        if (_fgDescNotFree && (NULL != pt_dev_desc_tmp))
        {
            if (SLIST_IS_EMPTY(&(pt_dev_desc_tmp->t_mnt_list)))
            {

                u_handle_free(pt_dev_desc_tmp->h_dev, FALSE);
                _fgDescNotFree = FALSE;
                pt_dev_desc_tmp = NULL;
            }
        }

        DM_INFO("%s(%d)ps_dev_name = %s!\n", __FUNCTION__,__LINE__,ps_dev_name);
        DM_INFO("%s(%d)ui4_major = %d, ui4_minor = %d,t_cb_type = %d!\n", __FUNCTION__,__LINE__,(int)ui4_major,(int)ui4_minor,(int)t_cb_type);
        ASSERT(NULL != pt_cb_dev);
        if (DM_CB_CAT_DEV == DM_CB_CAT(t_cb_type))
        {
            /*The event call-backer is the parent device. */
            if (DM_CB_DEV_ATTACH == t_cb_type)
            {
                DM_INFO("%s(%d)!\n", __FUNCTION__,__LINE__);

                DM_DEV_DESC_T *pt_dev_desc = NULL;
                DM_DEV_DESC_T *pt_dev_desc_lst = NULL;
                BOOL b_found = FALSE;
                BOOL b_avail = TRUE;

                DM_INFO("[DM][%ld] %s: #%d: DM_CB_DEV_ATTACH\n", i4_count,__FUNCTION__, __LINE__);
                ASSERT(DM_DEV_CHR_UNKNOWN != DM_DEV_CHR(t_dev_type));

                if (RWLR_OK != u_rwl_write_lock(t_dm_core.h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    DM_ERR("_dm_cb_msg_thread lock error.");
                    goto fail;
                }

                /*Scan the list to check if the device is already attached. */
                SLIST_FOR_EACH(pt_dev_desc, &(t_dm_core.t_dev_list), t_link)
                {
                    if ((pt_dev_desc->pt_prnt_dev == pt_cb_dev) &&
                        (pt_dev_desc->ui4_skt_no  == ui4_skt_no) &&
                        (pt_dev_desc->ui4_unit_id == ui4_unit_id) &&
                        (pt_dev_desc->t_dev_type  == t_dev_type) &&
                        (pt_dev_desc->pt_root_dev == pt_cb_dev->pt_root_dev)&&
                        (pt_dev_desc->t_hw_type   == pt_cb_dev->t_hw_type))
                    {
                        ASSERT(pt_dev_desc->pt_root_dev == pt_cb_dev->pt_root_dev);
                        ASSERT(pt_dev_desc->t_hw_type == pt_cb_dev->t_hw_type);
                        ASSERT(pt_dev_desc->t_dev_type == t_dev_type);
                        ASSERT(pt_dev_desc->ui4_unit_id == ui4_unit_id);

                        b_found = TRUE;
                        DM_INFO("[%s:%d] b_found = TRUE;!\n", __FUNCTION__,__LINE__);
                        break;
                    }

                    pt_dev_desc_lst = pt_dev_desc;
                }

                if (b_found == FALSE)
                {
                    pt_dev_desc = (DM_DEV_DESC_T *)malloc(sizeof(DM_DEV_DESC_T));
                    if (NULL == pt_dev_desc)
                    {
                        DM_ERR("_dm_cb_msg_thread alloc mem error.");
                        goto error_1;
                    }
                    memset((VOID *) pt_dev_desc, 0, sizeof(DM_DEV_DESC_T));

                    i4_ret = u_handle_alloc((HANDLE_TYPE_T)DMT_DEV_HANDLE,
                                             pt_dev_desc,
                                             NULL,
                                             _dm_handle_free_fct,
                                             &(pt_dev_desc->h_dev));
                    if (HR_OK != i4_ret)
                    {
                        DM_ERR("_dm_cb_msg_thread alloc handle error.");
                        goto error_1;
                    }

                    pt_dev_desc->ui4_dev_flag = ui4_dev_flag;
                    pt_dev_desc->pt_prnt_dev = pt_cb_dev;
                    pt_dev_desc->pt_root_dev = pt_cb_dev->pt_root_dev;
                    pt_dev_desc->t_dev_type = t_dev_type;
                    pt_dev_desc->t_hw_type = pt_cb_dev->t_hw_type;
                    pt_dev_desc->ui4_skt_no = ui4_skt_no;
                    pt_dev_desc->ui4_unit_id = ui4_unit_id;
                    pt_dev_desc->ui4_major = ui4_major;
                    pt_dev_desc->ui4_minor = ui4_minor;
                    pt_dev_desc->pv_data = pv_data;
                    pt_dev_desc->ui4_data_size = ui4_data_size;
                    pt_dev_desc->h_dev_root_lbl = NULL_HANDLE;
                    memset(pt_dev_desc->ps_dev_name, 0, DM_MAX_LABEL_NAME_LEN);
                    pt_dev_desc->h_drv = (HANDLE_T)-1;

                    /* just not DM_DEV_MED_TYPE_PTP_MTP */
                    if (0 != strlen(ps_dev_name))
                    {
                        UINT32 ui4_flag;
                        CHAR ps_dev_path[DM_MAX_LABEL_NAME_LEN];
                        memset(ps_dev_path,0,DM_MAX_LABEL_NAME_LEN);
                        strncpy(pt_dev_desc->ps_dev_name, ps_dev_name, strlen(ps_dev_name));

#if 0
                        if((pt_dev_desc->t_dev_type!=DM_DEV_SKT_TYPE_CIFS) &&
                           (pt_dev_desc->t_dev_type!=DM_DEV_SKT_TYPE_NFS) &&
                           (pt_dev_desc->t_dev_type!=DM_DEV_SKT_TYPE_USB_SOUND_CARD))
#endif
                        {
                        #if 0//CONFIG_MW_GOOGLE_CAST_AUDIO
                            strncpy(ps_dev_path, "/dev/block/",11);
                        #else
                            strncpy(ps_dev_path, "/dev/",5);
                        #endif
                        }

                        strncat(ps_dev_path, pt_dev_desc->ps_dev_name,strlen(ps_dev_name));

                        if (DM_DEV_SKT_TYPE_OPTI_DRV == pt_dev_desc->t_dev_type)
                        {
                            DM_INFO("[DM][%ld] DM_DEV_SKT_TYPE_OPTI_DRV\n",i4_count);
                            ui4_flag = O_RDONLY | O_NONBLOCK | O_CLOEXEC;
                        }
                        else
                        {
                            DM_INFO("[DM][%ld] No DM_DEV_SKT_TYPE_OPTI_DRV\n",i4_count);
                            //ui4_flag = O_RDWR | O_CLOEXEC;
                            /**PATCH FOR WRITE PROTECTED USB DEVICE @20091215*/
                            ui4_flag = O_RDONLY | O_CLOEXEC;
                            /**PATCH END*/
                        }

                        /* we do not open optical drive when attached:
                        1. io control functions uses no such file descriptor
                        2. if we keep opening the optica7/31/2014 1:33:53 PMl drive, we can not get the
                        disc status updated even after disc tray in / tray out */

                        if (DM_DEV_SKT_TYPE_OPTI_DRV != pt_dev_desc->t_dev_type)
                        {
#if 1 //in case open fail
                            UINT8 ui1_cnt = 0;
                            DM_INFO("[DM][%ld] %s %d ps_dev_path = %s\n", i4_count,__FUNCTION__, __LINE__, ps_dev_path);

                            while (1)
                            {
                                i4_ret = open(ps_dev_path, ui4_flag);
                                DM_INFO("<wj follow> open i4_ret:%ld\n", i4_ret);
                                if (i4_ret >= 0)
                                {
                                    pt_dev_desc->h_drv = (HANDLE_T)i4_ret;
                                    break;
                                }

                                usleep(10000);
                                ui1_cnt++;

                                if (ui1_cnt >= 10)
                                {
                                    DM_ERR("_dm_cb_msg_thread open device error. %s", ps_dev_path);
                                    b_avail = FALSE;
                                    break;
                                }
                            }
#endif
                        }

                    }
                    else if (DM_DEV_SKT_TYPE_HUB == pt_dev_desc->t_dev_type)
                    {
                        /* fake hub / real hub may have no device node, but we still need to register call back for it */
                        b_avail = TRUE;
                    }

                    /* *  Register call back (socket device) or create FM "dev" entry   (medium device) */
                    if (b_avail == TRUE)
                    {
                        if (DM_DEV_CHR_SOCKET == DM_DEV_CHR(t_dev_type))
                        {
                            DEV_DEV_ST_NFY_T t_nfy;

                            DM_INFO("[DM][%ld] %s: #%d: DM_DEV_CHR_SOCKET\n",i4_count, __FUNCTION__, __LINE__);
                            t_nfy.pf_cb_nfy_fct = _dm_dev_nfy_fct;
                            t_nfy.pv_nfy_tag =    pt_dev_desc;

                            i4_ret = dmh_reg_nfy_fct(pt_dev_desc->ui4_major,
                                                     pt_dev_desc->ui4_minor,
                                                     &t_nfy);
                            if (0 != i4_ret)
                            {
                                DM_ERR("_dm_cb_msg_thread rm_set device error.");
                                b_avail = FALSE;
                            }
                        }
                        else if (DM_DEV_CHR_MEDIUM == DM_DEV_CHR(t_dev_type))
                        {
                            DM_INFO("[DM][%ld] %s: #%d: DM_DEV_CHR_MEDIUM\n",i4_count, __FUNCTION__, __LINE__);

#if 0 //we will del this unregister callback flow,becuase we usel  broadcast.
                            i4_ret = x_fm_reg_nfy_fct(_dm_fm_mnt_nfy_fct,
                                                      pt_dev_desc->ps_dev_name,
                                                      (VOID *) pt_dev_desc,
                                                      &(pt_dev_desc->h_mnt_nfy));
                            if (i4_ret != FMR_OK)
                            {
                                DM_ERR("_dm_cb_msg_thread register FM mount nfy error.");
                                b_avail = FALSE;
                            }
#endif
                            SLIST_INIT(&(pt_dev_desc->t_mnt_list));

                            i4_ret = u_rwl_create_lock(&(pt_dev_desc->h_mnt_rwlock));
                            if (RWLR_OK != i4_ret)
                            {
                                DM_ERR("_dm_cb_msg_thread OS error.");
                                goto error_1;
                            }
                            #if 0
                            if (t_dev_type != DM_DEV_MED_TYPE_DISC &&
                               t_dev_type != DM_DEV_MED_TYPE_ISO)
                            #endif
                            {
                                 i4_ret = u_sema_create(&(pt_dev_desc->h_automnt_sema),
                                                        X_SEMA_TYPE_BINARY,
                                                        X_SEMA_STATE_UNLOCK);
                                 if (OSR_OK != i4_ret)
                                 {
                                      DM_ERR("_dm_cb_msg_thread OS error.");
                                      goto error_1;
                                 }
                            }

                            i4_ret = u_fm_set_dir_path(FM_ROOT_HANDLE,
                                                       DM_DEV_ROOT_DEV_LABEL,
                                                       &(pt_dev_desc->h_dev_root_lbl));
                            if (FMR_OK != i4_ret)
                            {
                                DM_ERR("_dm_cb_msg_thread FM error.");
                                goto error_1;
                            }

                            /* if the medium is not disc, perhaps there will be partitions */
                            #if 0
                            if (t_dev_type != DM_DEV_MED_TYPE_DISC  &&
                                t_dev_type != DM_DEV_MED_TYPE_ISO)
                            #endif
                            {
                                DEV_DEV_ST_NFY_T t_nfy;

                                t_nfy.pf_cb_nfy_fct = _dm_dev_nfy_fct;
                                if (0)//(t_dev_type == DM_DEV_MED_TYPE_MEM_CARD)
                                {
                                    t_nfy.pv_nfy_tag = pt_dev_desc->pt_prnt_dev;
                                }
                                else
                                {
                                    t_nfy.pv_nfy_tag = pt_dev_desc;
                                }

                                i4_ret = dmh_reg_nfy_fct(pt_dev_desc->ui4_major,
                                                         pt_dev_desc->ui4_minor,
                                                         &t_nfy);
                                if (i4_ret != 0)
                                {
                                    DM_ERR("_dm_cb_msg_thread rm_set device error.");
                                    b_avail = FALSE;
                                }
                            }
                        }
                    }

                    pt_dev_desc->b_avail = b_avail;

                    DM_INFO("pt_dev_desc_lst:%p\n", pt_dev_desc_lst);
                    if (NULL == pt_dev_desc_lst)
                    {
                        DM_INFO("pt_dev_desc link to t_dm_core.t_dev_list!\n");
                        SLIST_INSERT_HEAD(pt_dev_desc, &(t_dm_core.t_dev_list), t_link);
                    }
                    else
                    {
                        DM_INFO("pt_dev_desc link to pt_dev_desc_lst, devname:%s!\n", pt_dev_desc_lst->ps_dev_name);
                        SLIST_INSERT_AFTER(pt_dev_desc, pt_dev_desc_lst, t_link);
                    }
                }

                DM_INFO("%s(%d)!\n", __FUNCTION__,__LINE__);
                u_rwl_release_lock(t_dm_core.h_dev_rwlock);

                if (TRUE == b_found)
                {
                    continue;
                }

                if ((DM_DEV_CHR_MEDIUM == DM_DEV_CHR(t_dev_type)) && (TRUE == b_avail))
                {
                    _dm_check_automnt(pt_dev_desc, TRUE);
                    DM_INFO("AUTO mnt @Line %d\n", __LINE__);
                }

                continue;

            error_1:

                if (NULL != pt_dev_desc)
                {
                    if (NULL_HANDLE != pt_dev_desc->h_dev)
                    {
                        u_handle_free(pt_dev_desc->h_dev, TRUE);
                    }

                    if ((INT32)pt_dev_desc->h_drv >= 0)
                    {
                        close(pt_dev_desc->h_drv);
                    }
                    if (NULL_HANDLE != pt_dev_desc->h_dev_root_lbl)
                    {
                        u_fm_close(pt_dev_desc->h_dev_root_lbl);
                    }

                    if (NULL != pt_dev_desc->pv_data)
                    {
                        free(pt_dev_desc->pv_data);
                    }

                    if (NULL_HANDLE != pt_dev_desc->h_automnt_sema)
                    {
                        u_sema_delete(pt_dev_desc->h_automnt_sema);
                    }

                    if (NULL_HANDLE != pt_dev_desc->h_mnt_rwlock)
                    {
                        u_rwl_delete_lock(pt_dev_desc->h_mnt_rwlock);
                    }

                    free(pt_dev_desc);
                }

                u_rwl_release_lock(t_dm_core.h_dev_rwlock);

                goto fail;
            }
            else if (DM_CB_DEV_DETACH == t_cb_type)
            {
                DM_INFO("%s(%d)!\n", __FUNCTION__,__LINE__);

                DM_DEV_DESC_T *pt_dev_desc = NULL;
                BOOL b_found = FALSE;

                DM_INFO("[DM][%ld] %s: #%d: DM_CB_DEV_DETACH\n",i4_count, __FUNCTION__, __LINE__);
                if (RWLR_OK != u_rwl_write_lock(t_dm_core.h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    DM_ERR("_dm_cb_msg_thread lock error.");
                    goto fail;
                }

                /*Scan the list to check if the device is already attached. */
                SLIST_FOR_EACH(pt_dev_desc, &(t_dm_core.t_dev_list), t_link)
                {
                    if ((pt_dev_desc->pt_prnt_dev == pt_cb_dev) &&
                        (pt_dev_desc->ui4_skt_no == ui4_skt_no) &&
                        (pt_dev_desc->ui4_unit_id == ui4_unit_id) &&
                        (pt_dev_desc->t_dev_type == t_dev_type))
                    {
                        ASSERT(pt_dev_desc->pt_root_dev == pt_cb_dev->pt_root_dev);
                        ASSERT(pt_dev_desc->t_hw_type == pt_cb_dev->t_hw_type);
                        b_found = TRUE;
                        SLIST_REMOVE(pt_dev_desc, t_link);
                        break;
                    }
                }

                u_rwl_release_lock(t_dm_core.h_dev_rwlock);

                /* Raise event request. */
                if (TRUE == b_found)
                {
                    DM_INFO("[DM %s umount delay 1s\n.",pt_dev_desc->ps_dev_name);
                    if ((DM_DEV_CHR_MEDIUM == DM_DEV_CHR(t_dev_type)) && (TRUE == pt_dev_desc->b_avail))
                    {
                        DM_INFO("[DM] %s: #%d: DM_CB_DEV_DETACH\n", __FUNCTION__, __LINE__);
                        _dm_check_automnt(pt_dev_desc, FALSE);
                    }
                }

                if (TRUE == b_found)
                {
                    UINT8 ui1_i = 0;

                    /* Wait until unmount procedure is completed and then we may  release the pt_dev_desc.  Kind of dirty...*/
                    if(1)//if(!((pt_dev_desc->t_dev_type==DM_DEV_SKT_TYPE_CIFS)||(pt_dev_desc->t_dev_type==DM_DEV_SKT_TYPE_IPOD) || (pt_dev_desc->t_dev_type==DM_DEV_SKT_TYPE_NFS) || (pt_dev_desc->t_dev_type==DM_DEV_SKT_TYPE_USB_SOUND_CARD)))
                    {
                        for (;;)
                        {
                            if (SLIST_IS_EMPTY(&(pt_dev_desc->t_mnt_list)))
                            {
                                DM_INFO("umount comming: mnt desc released %s\n",pt_dev_desc->ps_dev_name);
                                break;
                            }
                            else
                            {
                                u_thread_delay(10);
                                ui1_i ++;
                                if (ui1_i > 200)
                                {
                                    DM_INFO("umount waiting: but mnt desc not released %s\n",pt_dev_desc->ps_dev_name);
                                    break;
                                }
                            }
                        }
                    }

                    if (ui1_i <= 100)
                    {
                        i4_ret = u_handle_free(pt_dev_desc->h_dev, TRUE);
                        if (i4_ret != HR_OK)
                        {
                            DM_ERR("_dm_cb_msg_thread free handle error.");
                            goto fail;
                        }
                    }
                    else
                    {
                        DM_INFO("[DM] not free %s\n", pt_dev_desc->ps_dev_name);
                        _fgDescNotFree = TRUE;
                        pt_dev_desc_tmp = pt_dev_desc;
                    }
                }
            }
            else if ((DM_CB_DEV_FAIL == t_cb_type) || (DM_CB_DEV_UNSUPPORT == t_cb_type))
            {
                DM_DEV_DESC_T *pt_dev_desc = NULL;
                DM_DEV_DESC_T *pt_dev_desc_lst = NULL;
                BOOL b_found = FALSE;

                ASSERT(((DM_CB_DEV_FAIL==t_cb_type)&&(DM_DEV_UKW_TYPE_UNKNOWN!=t_dev_type)) ||
                       ((DM_CB_DEV_UNSUPPORT==t_cb_type)&&(DM_DEV_UKW_TYPE_UNKNOWN==t_dev_type)) ||
                       ((DM_CB_DEV_UNSUPPORT==t_cb_type)&&(DM_DEV_MED_TYPE_USB_EP==t_dev_type)) ||
                       ((DM_CB_DEV_UNSUPPORT==t_cb_type)&&(DM_DEV_SKT_TYPE_UNSUPPORT_WIFI==t_dev_type)));

                if (RWLR_OK != u_rwl_write_lock(t_dm_core.h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    DM_ERR("_dm_cb_msg_thread lock error.");
                    goto fail;
                }

                /*Scan the list to check if the device is already detected. */
                SLIST_FOR_EACH(pt_dev_desc, &(t_dm_core.t_dev_list), t_link)
                {
                    if ((pt_dev_desc->pt_prnt_dev == pt_cb_dev) &&
                        (pt_dev_desc->ui4_skt_no == ui4_skt_no) &&
                        (pt_dev_desc->t_dev_type == t_dev_type) &&
                        (pt_dev_desc->ui4_unit_id == ui4_unit_id))
                    {
                        ASSERT(pt_dev_desc->pt_root_dev == pt_cb_dev->pt_root_dev);
                        ASSERT(pt_dev_desc->t_hw_type   == pt_cb_dev->t_hw_type);
                        ASSERT(pt_dev_desc->t_dev_type  == t_dev_type);
                        ASSERT(pt_dev_desc->ui4_unit_id == ui4_unit_id);
                        if (DM_DEV_SKT_TYPE_OPTI_DRV != t_dev_type)
                        {
                            ASSERT(FALSE == pt_dev_desc->b_avail);
                        }

                        b_found = TRUE;

                        break;
                    }

                    pt_dev_desc_lst = pt_dev_desc;
                }

                if (FALSE == b_found)
                {
                    pt_dev_desc = (DM_DEV_DESC_T *)malloc(sizeof(DM_DEV_DESC_T));
                    if (NULL == pt_dev_desc)
                    {
                        DM_ERR("_dm_cb_msg_thread mem alloc error.");
                        goto error_2;
                    }
                    memset((VOID *) pt_dev_desc, 0, sizeof(DM_DEV_DESC_T));

                    i4_ret = u_handle_alloc((HANDLE_TYPE_T)DMT_DEV_HANDLE,
                                            pt_dev_desc,
                                            NULL,
                                            _dm_handle_free_fct,
                                            &(pt_dev_desc->h_dev));

                    if (HR_OK != i4_ret)
                    {
                        DM_ERR("_dm_cb_msg_thread alloc handle error.");
                        goto error_2;
                    }

                    pt_dev_desc->ui4_dev_flag = ui4_dev_flag;
                    pt_dev_desc->h_drv = (HANDLE_T)-1;
                    pt_dev_desc->pt_prnt_dev = pt_cb_dev;
                    pt_dev_desc->pt_root_dev = pt_cb_dev->pt_root_dev;
                    pt_dev_desc->t_dev_type = t_dev_type;
                    pt_dev_desc->t_hw_type = pt_cb_dev->t_hw_type;
                    pt_dev_desc->ui4_skt_no = ui4_skt_no;
                    pt_dev_desc->ui4_unit_id = ui4_unit_id;
                    pt_dev_desc->ui4_major = ui4_major;
                    pt_dev_desc->ui4_minor = ui4_minor;
                    pt_dev_desc->pv_data = pv_data;
                    pt_dev_desc->ui4_data_size = ui4_data_size;
                    pt_dev_desc->b_avail = FALSE;
                    pt_dev_desc->h_dev_root_lbl = NULL_HANDLE;
                    memset(pt_dev_desc->ps_dev_name,0,DM_MAX_LABEL_NAME_LEN);

                    if(1)//if ((pt_dev_desc->t_dev_type != DM_DEV_SKT_TYPE_CIFS) &&  (pt_dev_desc->t_dev_type != DM_DEV_SKT_TYPE_NFS) && (pt_dev_desc->t_dev_type != DM_DEV_SKT_TYPE_USB_SOUND_CARD))
                    {
                        if (NULL == pt_dev_desc_lst)
                        {
                            SLIST_INSERT_HEAD(pt_dev_desc, &(t_dm_core.t_dev_list), t_link);
                        }
                        else
                        {
                            SLIST_INSERT_AFTER(pt_dev_desc, pt_dev_desc_lst, t_link);
                        }
                    }
                }

                u_rwl_release_lock(t_dm_core.h_dev_rwlock);

                continue;

            error_2:

                if (NULL != pt_dev_desc)
                {
                    if (NULL_HANDLE != pt_dev_desc->h_dev)
                    {
                        u_handle_free(pt_dev_desc->h_dev, TRUE);
                    }

                    if (NULL != pt_dev_desc->pv_data)
                    {
                        free(pt_dev_desc->pv_data);
                    }

                    free(pt_dev_desc);
                }

                u_rwl_release_lock(t_dm_core.h_dev_rwlock);

                goto fail;
            }
            else if (DM_CB_DEV_PART_DETECTED == t_cb_type)
            {
                DM_DEV_DESC_T *pt_dev_desc = NULL;
                DM_DEV_DESC_T *pt_dev_desc_lst = NULL;
                BOOL b_found = FALSE;
                BOOL b_avail = TRUE;
                DM_INFO("[%ld]%s(%d): partion detected.\n",i4_count,__FUNCTION__,__LINE__);
                ASSERT(DM_DEV_CHR(t_dev_type) != DM_DEV_CHR_UNKNOWN);

                if (RWLR_OK != u_rwl_write_lock(t_dm_core.h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    DM_ERR("_dm_cb_msg_thread lock error.");
                    goto fail;
                }

                /*Scan the list to check if the device is already attached.*/
                SLIST_FOR_EACH(pt_dev_desc, &(t_dm_core.t_dev_list), t_link)
                {
                    if ((pt_dev_desc->pt_prnt_dev == pt_cb_dev) &&
                        (pt_dev_desc->ui4_skt_no == ui4_skt_no) &&
                        (pt_dev_desc->ui4_unit_id == ui4_unit_id))
                    {
                        ASSERT(pt_dev_desc->pt_root_dev == pt_cb_dev->pt_root_dev);
                        ASSERT(pt_dev_desc->t_hw_type   == pt_cb_dev->t_hw_type);
                        ASSERT(pt_dev_desc->t_dev_type  == t_dev_type);
                        ASSERT(pt_dev_desc->ui4_unit_id == ui4_unit_id);

                        b_found = TRUE;

                        break;
                    }

                    pt_dev_desc_lst = pt_dev_desc;
                }

                if (FALSE == b_found)
                {
                    pt_dev_desc = (DM_DEV_DESC_T *)malloc(sizeof(DM_DEV_DESC_T));
                    if (NULL == pt_dev_desc)
                    {
                        DM_ERR("_dm_cb_msg_thread alloc mem error.");
                        goto error_3;
                    }
                    memset((VOID *) pt_dev_desc, 0, sizeof(DM_DEV_DESC_T));

                    i4_ret = u_handle_alloc((HANDLE_TYPE_T)DMT_DEV_HANDLE,
                                             pt_dev_desc,
                                             NULL,
                                             _dm_handle_free_fct,
                                             &(pt_dev_desc->h_dev));
                    if (HR_OK != i4_ret)
                    {
                        DM_ERR("_dm_cb_msg_thread alloc handle error.");
                        goto error_3;
                    }

                    pt_dev_desc->ui4_dev_flag = ui4_dev_flag;
                    pt_dev_desc->pt_prnt_dev = pt_cb_dev;
                    pt_dev_desc->pt_root_dev = pt_cb_dev->pt_root_dev;
                    pt_dev_desc->t_dev_type = t_dev_type;
                    pt_dev_desc->t_hw_type = pt_cb_dev->t_hw_type;
                    pt_dev_desc->ui4_skt_no = ui4_skt_no;
                    pt_dev_desc->ui4_unit_id = ui4_unit_id;
                    pt_dev_desc->ui4_major = ui4_major;
                    pt_dev_desc->ui4_minor = ui4_minor;
                    pt_dev_desc->pv_data = pv_data;
                    pt_dev_desc->ui4_data_size = ui4_data_size;

                    i4_ret = u_fm_set_dir_path(FM_ROOT_HANDLE,
                                               DM_DEV_ROOT_DEV_LABEL,
                                               &(pt_dev_desc->h_dev_root_lbl));
                    if (FMR_OK != i4_ret)
                    {
                        DM_ERR("_dm_cb_msg_thread FM error.");
                        goto error_3;
                    }
                    memset(pt_dev_desc->ps_dev_name, 0, DM_MAX_LABEL_NAME_LEN);
                    strncpy(pt_dev_desc->ps_dev_name, ps_dev_name, DM_MAX_LABEL_NAME_LEN);
                    DM_INFO("[%ld]%s(%d) Enter pt_dev_desc->ps_dev_name=%s\n ",i4_count,__FUNCTION__,__LINE__,pt_dev_desc->ps_dev_name);
                    pt_dev_desc->h_drv = (HANDLE_T)-1;
                    pt_dev_desc->b_avail = b_avail;

                    if (NULL == pt_dev_desc_lst)
                    {
                        SLIST_INSERT_HEAD(pt_dev_desc, &(t_dm_core.t_dev_list), t_link);
                    }
                    else
                    {
                        SLIST_INSERT_AFTER(pt_dev_desc, pt_dev_desc_lst, t_link);
                    }
                }

                u_rwl_release_lock(t_dm_core.h_dev_rwlock);

                if (TRUE == b_found)
                {
                    continue;
                }

                _dm_check_automnt(pt_dev_desc, TRUE);

                continue;

            error_3:
#if 1
                if (NULL != pt_dev_desc)
                {
                    if (NULL_HANDLE != pt_dev_desc->h_dev)
                    {
                        u_handle_free(pt_dev_desc->h_dev, TRUE);
                    }
                    if (NULL_HANDLE != pt_dev_desc->h_dev_root_lbl)
                    {
                        u_fm_close(pt_dev_desc->h_dev_root_lbl);
                    }
                    if (NULL != pt_dev_desc->pv_data)
                    {
                        free(pt_dev_desc->pv_data);
                    }

                    if (NULL_HANDLE != pt_dev_desc->h_automnt_sema)
                    {
                        u_sema_delete(pt_dev_desc->h_automnt_sema);
                    }

                    if (NULL_HANDLE != pt_dev_desc->h_mnt_rwlock)
                    {
                        u_rwl_delete_lock(pt_dev_desc->h_mnt_rwlock);
                    }

                    free(pt_dev_desc);
                }
#endif
                goto fail;
            }
            else if (DM_CB_DEV_PART_REMOVED == t_cb_type)
            {
                DM_DEV_DESC_T *pt_dev_desc = NULL;
                BOOL b_found = FALSE;
                if (RWLR_OK != u_rwl_write_lock(t_dm_core.h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    DM_ERR("_dm_cb_msg_thread lock error.");
                    goto fail;
                }

                /*Scan the list to check if the device is already attached.*/
                SLIST_FOR_EACH(pt_dev_desc, &(t_dm_core.t_dev_list), t_link)
                {
                    if ((pt_dev_desc->pt_prnt_dev == pt_cb_dev) &&
                        (pt_dev_desc->ui4_skt_no == ui4_skt_no) &&
                        (pt_dev_desc->ui4_unit_id == ui4_unit_id))
                    {
                        ASSERT(pt_dev_desc->pt_root_dev == pt_cb_dev->pt_root_dev);
                        ASSERT(pt_dev_desc->t_hw_type   == pt_cb_dev->t_hw_type);

                        b_found = TRUE;

                        SLIST_REMOVE(pt_dev_desc, t_link);

                        break;
                    }
                }
                u_rwl_release_lock(t_dm_core.h_dev_rwlock);

                /*Raise event request.*/
                if (TRUE == b_found)
                {
                    if ((DM_DEV_CHR_MEDIUM == DM_DEV_CHR(t_dev_type)) && (TRUE == pt_dev_desc->b_avail))
                    {
                        _dm_check_automnt(pt_dev_desc, FALSE);
                    }
                }

                if (TRUE == b_found)
                {
                    ASSERT(SLIST_IS_EMPTY(&(pt_dev_desc->t_mnt_list)));

                    i4_ret = u_handle_free(pt_dev_desc->h_dev, TRUE);

                    if (HR_OK != i4_ret)
                    {
                        DM_ERR("_dm_cb_msg_thread free handle error.");
                        goto fail;
                    }
                }
            }
        }
    }

fail:
    u_thread_exit();

}

static BOOL _dm_is_cable_network_connect(VOID)
{
    INT32 skfd = 0;
    struct ifreq ifr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0)
    {
        DM_ERR("[DM_LINUX] cable network socket failed!\n");
        return FALSE;
    }

    strcpy(ifr.ifr_name, "eth0");
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 )
    {

        //DM_ERR("[DM_LINUX] cable network ioctl SIOCGIFFLAGS failed!\n");
        close(skfd);
        return FALSE;
    }

    close(skfd);

    return !!(ifr.ifr_flags & IFF_RUNNING);
}

VOID *_dm_network_monitor_thread(VOID *arg)
{
    BOOL b_network_connect = FALSE;
    BOOL b_previous_connect = FALSE;

    prctl(PR_SET_NAME,"network_monitor");

    while(1)
    {
        if (!_dm_is_cable_network_connect())
        {
            b_network_connect = FALSE;
        }
        else
        {
            b_network_connect = TRUE;
        }

        if (b_network_connect != b_previous_connect)
        {
            b_previous_connect = b_network_connect;
            _dm_broadcast_network_msg(b_previous_connect);
        }

        sleep(1);
    }
}

static VOID dm_deinit(VOID)
{
    INT32 i4_ret = DMR_OK;

    if (NULL_HANDLE != t_dm_core.h_cb_msg_thrd)
    {
        DM_CB_MSG_T t_cb_msg;

        t_cb_msg.e_msgtype = DM_MSG_TYPE_SHUTDOWN;

        i4_ret = x_msg_q_send(t_dm_core.h_cb_msg_q,
                          (const VOID *) &t_cb_msg,
                          sizeof(DM_CB_MSG_T),
                          DM_CB_MSG_PRIORITY);
        if (OSR_OK != i4_ret)
        {
            DM_ERR("send msg to close cb_msg thread failed!\n");
        }
    }

    if (NULL_HANDLE != t_dm_core.h_nfy_req_thrd)
    {
        DM_NFY_REQ_T t_nfy_req;

        t_nfy_req.e_msgtype = DM_MSG_TYPE_SHUTDOWN;

        i4_ret = x_msg_q_send(t_dm_core.h_nfy_req_q,
                          (const VOID *) &t_nfy_req,
                          sizeof(DM_NFY_REQ_T),
                          DM_NFY_REQ_PRIORITY);
        if (OSR_OK != i4_ret)
        {
            DM_ERR("send msg to close nfy thread failed!\n");
        }
    }

    if (NULL_HANDLE != t_dm_core.h_cb_msg_q)
    {
        x_msg_q_delete(t_dm_core.h_cb_msg_q);
        t_dm_core.h_cb_msg_q = NULL_HANDLE;
    }

    if (NULL_HANDLE != t_dm_core.h_nfy_req_q)
    {
        x_msg_q_delete(t_dm_core.h_nfy_req_q);
        t_dm_core.h_nfy_req_q = NULL_HANDLE;
    }

    if (NULL_HANDLE != t_dm_core.h_dev_rwlock)
    {
        u_rwl_delete_lock(t_dm_core.h_dev_rwlock);
        t_dm_core.h_dev_rwlock = NULL_HANDLE;
    }

    if (NULL_HANDLE != g_t_dm_mnt_info.h_mnt_rwlock)
    {
        u_rwl_delete_lock(g_t_dm_mnt_info.h_mnt_rwlock);
        g_t_dm_mnt_info.h_mnt_rwlock = NULL_HANDLE;
    }

    if (NULL != t_dm_core.pt_automnt_cond)
    {
        free((VOID *) t_dm_core.pt_automnt_cond);
        t_dm_core.pt_automnt_cond = NULL;
    }

    t_dm_core.b_init = FALSE;
}

INT32 dm_init(DM_INIT_PARM_T *pt_init)
{
    INT32 i4_ret=0;
    DM_INFO("[DM] %s: #%d: \n", __FUNCTION__, __LINE__);

    if (TRUE == t_dm_core.b_init)
    {
        i4_ret = DMR_ALREADY_INIT;
        goto fail;
    }

    memset(&t_dm_core, 0, sizeof(DM_CORE_T));
    memset(&g_t_dm_mnt_info, 0, sizeof(g_t_dm_mnt_info));

    i4_ret = dmh_init();
    if (DMR_OK != i4_ret)
    {
        DM_ERR("dmh_init error.");
        goto fail;
    }

    t_dm_core.b_automnt_init = FALSE;

    i4_ret = x_msg_q_create(&t_dm_core.h_cb_msg_q,
                            DM_CB_MSG_Q_NAME,
                            sizeof(DM_CB_MSG_T),
                            DM_CB_MSG_Q_NUM);
    if (OSR_OK != i4_ret)
    {
        i4_ret = DMR_OS;
        goto fail;
    }

    i4_ret = x_msg_q_create(&t_dm_core.h_nfy_req_q,
                            DM_NFY_REQ_Q_NAME,
                            sizeof(DM_NFY_REQ_T),
                            DM_NFY_REQ_Q_NUM);
    if (OSR_OK != i4_ret)
    {
        i4_ret = DMR_OS;
        goto fail;
    }

    i4_ret = u_rwl_create_lock(&t_dm_core.h_dev_rwlock);
    if (RWLR_OK != i4_ret)
    {
        i4_ret = DMR_OS;
        goto fail;
    }

    i4_ret = u_rwl_create_lock(&g_t_dm_mnt_info.h_mnt_rwlock);
    if (RWLR_OK != i4_ret)
    {
        i4_ret = DMR_OS;
        goto fail;
    }

    SLIST_INIT(&(t_dm_core.t_dev_list));

    i4_ret = u_thread_create(&t_dm_core.h_cb_msg_thrd,
                             DM_CB_MSG_THREAD_NAME,
                             DM_CB_MSG_THREAD_DEFAULT_STACK_SIZE,
                             DM_CB_MSG_THREAD_DEFAULT_PRIORITY,
                             _dm_cb_msg_thread,
                             0,
                             NULL);
    if (OSR_OK != i4_ret)
    {
        i4_ret = DMR_OS;
        goto fail;
    }

    i4_ret = u_thread_create(&t_dm_core.h_nfy_req_thrd,
                             DM_NFY_REQ_THREAD_NAME,
                             DM_NFY_REQ_THREAD_DEFAULT_STACK_SIZE,
                             DM_NFY_REQ_THREAD_DEFAULT_PRIORITY,
                             _dm_nfy_req_thread,
                             0,
                             NULL);
    if (OSR_OK != i4_ret)
    {
        i4_ret = DMR_OS;
        goto fail;
    }

    if (NULL != pt_init)
    {
        t_dm_core.ui4_automnt_cond_ns = pt_init->ui4_automnt_cond_ns;
        t_dm_core.pt_automnt_cond = (DM_COND_T *)malloc(pt_init->ui4_automnt_cond_ns *sizeof(DM_COND_T));

        if (NULL == t_dm_core.pt_automnt_cond)
        {
            i4_ret = DMR_OS;
            goto fail;
        }

        memcpy((VOID *) t_dm_core.pt_automnt_cond,
               (const VOID *) pt_init->pt_automnt_cond,
                pt_init->ui4_automnt_cond_ns * sizeof(DM_COND_T));
    }

    i4_ret = _dm_init_hw();

    if (DMR_OK != i4_ret)
    {
        DM_ERR("[DM] %s: #%d: _dm_init init hw error.\n", __FUNCTION__, __LINE__);
        goto fail;
    }

    t_dm_core.b_init = TRUE;

    return DMR_OK;

fail:
    dm_deinit();

    DM_INFO("[DM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
    return i4_ret;
}
INT32 dm_multi_thread_init(VOID)
{
    INT32          i4_ret=0;
    pthread_t      t_monitor_thread;
    DM_COND_T      pt_cond[4]={0};
    DM_INIT_PARM_T pt_init={0};
    DM_INFO("[DM]******%s:IN******#%d:\n", __FUNCTION__, __LINE__);

    /*init pt_connd*/
    pt_cond[0].t_hw_type  = DM_HW_TYPE_IDE;
    pt_cond[0].t_dev_type = DM_DEV_MED_TYPE_HDD;
    pt_cond[1].t_hw_type  = DM_HW_TYPE_IDE;
    pt_cond[1].t_dev_type = DM_DEV_MED_TYPE_HDD_PARTITION;
    pt_cond[2].t_hw_type  = DM_HW_TYPE_USB;
    pt_cond[2].t_dev_type = DM_DEV_MED_TYPE_UNKNOWN;
    pt_cond[3].t_hw_type  = DM_HW_TYPE_FCI;
    pt_cond[3].t_dev_type = DM_DEV_MED_TYPE_UNKNOWN;

    /*init pt_init*/
    pt_init.ui4_automnt_cond_ns = 4;
    pt_init.pt_automnt_cond     = pt_cond;

    /*init fm for mount USB device */
    i4_ret=u_fm_init();
    if(0 != i4_ret)
    {
        DM_ERR("[DM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
        return i4_ret;
    }

    /*start device manager  */
    i4_ret=dm_init(&pt_init);
    if(0 != i4_ret)
    {
        DM_ERR("[DM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
        return i4_ret;
    }

    /*init automnt*/
    i4_ret=dm_init_automnt();
    if(0 != i4_ret)
    {
        DM_ERR("[DM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
        return i4_ret;
    }

    /*start network monitor thread*/
    i4_ret = pthread_create(&t_monitor_thread, NULL, _dm_network_monitor_thread, NULL);
    if (0 != i4_ret)
    {
        DM_ERR("[DM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
        return i4_ret;
    }

    DM_INFO("[DM]******%s:OUT******#%d:\n", __FUNCTION__, __LINE__);

    return 0;
}

INT32 u_dm_get_device(DM_DEV_SPEC_T *pt_spec, HANDLE_T *ph_dev)
{
    DM_DEV_DESC_T *pt_dev_desc = NULL;
    BOOL b_found = FALSE;

    if ((pt_spec == NULL) || (ph_dev == NULL))
    {
        return DMR_INVALID_PARM;
    }

    if ((pt_spec->t_hw_type == DM_HW_TYPE_UNKNOWN) ||
        (pt_spec->t_dev_type == DM_DEV_UKW_TYPE_UNKNOWN) ||
        (pt_spec->t_dev_type == DM_DEV_SKT_TYPE_UNKNOWN) ||
        (pt_spec->t_dev_type == DM_DEV_MED_TYPE_UNKNOWN))
    {
        return DMR_INVALID_PARM;
    }

    if (u_rwl_read_lock(t_dm_core.h_dev_rwlock, RWL_OPTION_WAIT) != RWLR_OK)
    {
        return DMR_OS;
    }

    SLIST_FOR_EACH(pt_dev_desc, &(t_dm_core.t_dev_list), t_link)
    {
        if ((pt_dev_desc->t_hw_type == pt_spec->t_hw_type) &&
            (pt_dev_desc->t_dev_type == pt_spec->t_dev_type) &&
            (pt_dev_desc->ui4_unit_id == pt_spec->ui4_unit_id))
        {
            b_found = TRUE;
            *ph_dev = pt_dev_desc->h_dev;
            break;
        }
    }

    u_rwl_release_lock(t_dm_core.h_dev_rwlock);

    if (b_found == FALSE)
    {
        return DMR_ERR;
    }

    return DMR_OK;
}

INT32 u_dm_get_device_info(HANDLE_T h_dev, DM_DEV_INF_T *pt_inf)
{
    INT32 i4_ret;
    DM_DEV_DESC_T *pt_dev_desc = NULL;
    VOID          *pv_obj      = NULL;
    SIZE_T z_size;
    HANDLE_TYPE_T t_handle_type;

    if (pt_inf == NULL)
    {
        return DMR_INVALID_PARM;
    }

    if (u_rwl_read_lock(t_dm_core.h_dev_rwlock, RWL_OPTION_WAIT) != RWLR_OK)
    {
        return DMR_OS;
    }

    i4_ret = u_handle_get_type_obj(h_dev,
                                 &t_handle_type,
                                 &pv_obj);

    if ((i4_ret != HR_OK) ||
        (pv_obj == NULL) ||
        (t_handle_type != DMT_DEV_HANDLE))
    {
        i4_ret = DMR_INVALID_HANDLE;
        goto fail;
    }

    pt_dev_desc = (DM_DEV_DESC_T *)pv_obj;

    memset((VOID *) pt_inf, 0, sizeof(DM_DEV_INF_T));

    /*
     *  General information
     */

    pt_inf->t_hw_type = pt_dev_desc->t_hw_type;
    pt_inf->t_dev_type = pt_dev_desc->t_dev_type;

    pt_inf->ui4_unit_id = pt_dev_desc->ui4_unit_id;
    pt_inf->ui4_skt_no = pt_dev_desc->ui4_skt_no;

    pt_inf->pv_data = pt_dev_desc->pv_data;
    pt_inf->ui4_data_size = pt_dev_desc->ui4_data_size;

    //DM_INFO("[DM_TEST] Get Device Info: pt_inf->t_dev_type = %d\n", pt_inf->t_dev_type);

    /*
     *  Root device information
     */

    ASSERT(pt_dev_desc->pt_root_dev != NULL);
    ASSERT(pt_dev_desc->pt_root_dev->b_avail == TRUE);

    pt_inf->t_root_stat.t_hw_type = pt_dev_desc->pt_root_dev->t_hw_type;
    pt_inf->t_root_stat.t_dev_type = pt_dev_desc->pt_root_dev->t_dev_type;
    pt_inf->t_root_stat.ui4_unit_id = pt_dev_desc->pt_root_dev->ui4_unit_id;
    pt_inf->t_root_stat.ui4_skt_no = pt_dev_desc->pt_root_dev->ui4_skt_no;
    pt_inf->t_root_stat.b_avail = pt_dev_desc->pt_root_dev->b_avail;
    pt_inf->t_root_stat.h_dev = pt_dev_desc->pt_root_dev->h_dev;

    /*
     *  Parent device information
     */

    if (pt_dev_desc->pt_prnt_dev != NULL)
    {
       // DM_ASSERT(pt_dev_desc->pt_prnt_dev->b_avail == TRUE);
        if(pt_dev_desc->b_avail !=TRUE)
            {
                u_rwl_release_lock(t_dm_core.h_dev_rwlock);
                DM_ERR("pt_dev_desc->pt_prnt_dev->b_avail is FALSE .\n");
                return DMR_RM_ERR;
            }
        pt_inf->t_prnt_stat.t_hw_type = pt_dev_desc->pt_prnt_dev->t_hw_type;
        pt_inf->t_prnt_stat.t_dev_type = pt_dev_desc->pt_prnt_dev->t_dev_type;
        pt_inf->t_prnt_stat.ui4_unit_id = pt_dev_desc->pt_prnt_dev->ui4_unit_id;
        pt_inf->t_prnt_stat.ui4_skt_no = pt_dev_desc->pt_prnt_dev->ui4_skt_no;
        pt_inf->t_prnt_stat.b_avail = pt_dev_desc->pt_prnt_dev->b_avail;
        pt_inf->t_prnt_stat.h_dev = pt_dev_desc->pt_prnt_dev->h_dev;
    }
    else
    {
        pt_inf->t_prnt_stat.t_hw_type = DM_HW_TYPE_UNKNOWN;
    }

    pt_inf->b_avail = pt_dev_desc->b_avail;
    if(pt_dev_desc->t_dev_type == DM_DEV_SKT_TYPE_CIFS)
    {
       i4_ret = dmh_blkdev_get_usb_device_path(
                            pt_dev_desc->ui4_major,
                             pt_dev_desc->ui4_minor,
                             pt_inf->aui1_device_path);
       pt_inf->t_hw_type = pt_dev_desc->t_hw_type;
       pt_inf->t_dev_type = pt_dev_desc->t_dev_type;
       pt_inf->ui4_unit_id = pt_dev_desc->ui4_unit_id;
       pt_inf->pv_resv = pt_dev_desc->pv_resv;

       pt_inf->u.t_med_inf.u.t_blk_inf.ui4_sec_size = 512;
       pt_inf->u.t_med_inf.u.t_blk_inf.ui4_best_tx_sec_ns = 1;

       u_rwl_release_lock(t_dm_core.h_dev_rwlock);
       return DMR_OK;
    }

    if(pt_dev_desc->t_dev_type == DM_DEV_SKT_TYPE_NFS)
    {
       i4_ret = dmh_blkdev_get_usb_device_path(
                            pt_dev_desc->ui4_major,
                             pt_dev_desc->ui4_minor,
                             pt_inf->aui1_device_path);
       pt_inf->t_hw_type = pt_dev_desc->t_hw_type;
       pt_inf->t_dev_type = pt_dev_desc->t_dev_type;
       pt_inf->ui4_unit_id = pt_dev_desc->ui4_unit_id;
       pt_inf->pv_resv = pt_dev_desc->pv_resv;

       pt_inf->u.t_med_inf.u.t_blk_inf.ui4_sec_size = 512;
       pt_inf->u.t_med_inf.u.t_blk_inf.ui4_best_tx_sec_ns = 1;

       u_rwl_release_lock(t_dm_core.h_dev_rwlock);
       return DMR_OK;
    }
    if (pt_dev_desc->b_avail == TRUE)
    {
        /**** PATCH for usb device port address (device type)*/
        if (pt_dev_desc->t_hw_type == DM_HW_TYPE_USB)
        {
            i4_ret = dmh_blkdev_get_usb_device_path(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            pt_inf->aui1_device_path);
        }
        /**** END */
        if (pt_dev_desc->ui4_dev_flag & DMH_FLAG_BLOCK_DEV)
        {
            /* just not DM_DEV_MED_TYPE_PTP_MTP */

            if (pt_dev_desc->t_dev_type == DM_DEV_MED_TYPE_PTP_MTP)
            {
               pt_inf->t_hw_type = pt_dev_desc->t_hw_type;
               pt_inf->t_dev_type = pt_dev_desc->t_dev_type;
               pt_inf->ui4_unit_id = pt_dev_desc->ui4_unit_id;
               pt_inf->pv_resv = pt_dev_desc->pv_resv;

               pt_inf->u.t_med_inf.u.t_blk_inf.ui4_sec_size = 512;
               pt_inf->u.t_med_inf.u.t_blk_inf.ui4_best_tx_sec_ns = 1;

               i4_ret = dmh_blkdev_get_size(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            &(pt_inf->u.t_med_inf.ui8_total_size));

               DM_INFO("dmh_blkdev_get_size = %lld .\n", pt_inf->u.t_med_inf.ui8_total_size);

               i4_ret = dmh_blkdev_get_freesize(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            &(pt_inf->u.t_med_inf.ui8_free_size));

               DM_INFO("dmh_blkdev_get_freesize = %lld .\n", pt_inf->u.t_med_inf.ui8_free_size);

               i4_ret = dmh_blkdev_get_model(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            (CHAR *)pt_inf->aui1_model_str);
               DM_INFO("dmh_blkdev_get_model = %s .\n", pt_inf->aui1_model_str);


               u_rwl_release_lock(t_dm_core.h_dev_rwlock);
               return DMR_OK;
            }

            ASSERT(DM_DEV_CHR(pt_dev_desc->t_dev_type) == DM_DEV_CHR_MEDIUM);

            /*
             *  General information
             */

            /* BLKDEV_GET_TYPE_MAX_IO_REQUEST yjg */
            pt_inf->ui4_max_io_request = 1;

            i4_ret = dmh_blkdev_get_model(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            (CHAR *)pt_inf->aui1_model_str);
            if (i4_ret != DMH_OK)
            {
                i4_ret = DMR_RM_ERR;
                goto fail;
            }

            /* BLKDEV_GET_TYPE_ALIGNMENT yjg */
            pt_inf->ui4_mem_align = 32;

            /*
             *  Medium device specific information
             */

            pt_inf->u.t_med_inf.t_tx_type = DM_DEV_MED_TX_TYPE_BLKDEV;

            pt_inf->u.t_med_inf.h_dev_root_lbl = pt_dev_desc->h_dev_root_lbl;
            memcpy(pt_inf->u.t_med_inf.ps_dev_name,
                     pt_dev_desc->ps_dev_name,
                     DM_MAX_LABEL_NAME_LEN);
            //get mnt parttion

            DM_INFO("[DM] %s: #%d:pt_inf->u.t_med_inf.ps_dev_name=%s \n", __FUNCTION__, __LINE__,pt_inf->u.t_med_inf.ps_dev_name);
            DM_MNT_DESC_T *pt_mnt_desc = NULL;
             SLIST_FOR_EACH(pt_mnt_desc, &(pt_dev_desc->t_mnt_list), t_link)
            {

                DM_INFO("[DM] %s: #%d:part index=%lu,ps_part_name=%s \n", __FUNCTION__, __LINE__,pt_mnt_desc->ui4_part_idx, pt_mnt_desc->ps_part_name);
                if ( !strncmp(pt_mnt_desc->ps_part_name, pt_dev_desc->ps_dev_name, strlen(pt_dev_desc->ps_dev_name)))
                {
                    DM_INFO("[DM] %s: #%d: found\n", __FUNCTION__, __LINE__);
                    pt_inf->ui4_part_idx=pt_mnt_desc->ui4_part_idx;
                    break;
                }
            }

            i4_ret = dmh_blkdev_get_size(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            &(pt_inf->u.t_med_inf.ui8_total_size));
            if (i4_ret != DMH_OK)
            {
                i4_ret = DMR_RM_ERR;
                goto fail;
            }

            /*
             *  Block device specific information
             */

            i4_ret = dmh_blkdev_get_sector_ns(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            &(pt_inf->u.t_med_inf.u.t_blk_inf.ui8_sec_ns));
            if (i4_ret != DMH_OK)
            {
                i4_ret = DMR_RM_ERR;
                goto fail;
            }

            i4_ret = dmh_blkdev_get_sector_size(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            &(pt_inf->u.t_med_inf.u.t_blk_inf.ui4_sec_size));
            if (i4_ret != DMH_OK)
            {
                i4_ret = DMR_RM_ERR;
                goto fail;
            }

            if (pt_dev_desc->t_dev_type == DM_DEV_MED_TYPE_DISC)
            {
            }
            else
            {
                pt_inf->u.t_med_inf.u.t_blk_inf.ui4_max_tx_sec_ns = 1;
            }

            if (pt_dev_desc->t_dev_type == DM_DEV_MED_TYPE_DISC)
            {
            }
            else
            {
                pt_inf->u.t_med_inf.u.t_blk_inf.ui4_best_tx_sec_ns = 1;
            }
        }
        else if (pt_dev_desc->ui4_dev_flag & DMH_FLAG_CHAR_DEV)
        {
            UINT32 ui4_value = 0;

            ASSERT(DM_DEV_CHR(pt_dev_desc->t_dev_type) == DM_DEV_CHR_MEDIUM);

            /*
                     *  General information
                     */

            pt_inf->ui4_max_io_request = 1;
            pt_inf->ui4_mem_align = 1;

            /*
                     *  Medium device specific information
                     */

            pt_inf->u.t_med_inf.t_tx_type = DM_DEV_MED_TX_TYPE_CHRDEV;

            pt_inf->u.t_med_inf.h_dev_root_lbl = pt_dev_desc->h_dev_root_lbl;
            memcpy(pt_inf->u.t_med_inf.ps_dev_name,
                     pt_dev_desc->ps_dev_name,
                     DM_MAX_LABEL_NAME_LEN);
            if(pt_dev_desc->t_dev_type != DM_DEV_MED_TYPE_ISO)
            {
                i4_ret = _dm_chrdev_get_size(
                                pt_dev_desc->h_drv,
                                &(ui4_value));
                if (i4_ret != DMR_OK)
                {
                    i4_ret = DMR_RM_ERR;
                    goto fail;
                }
            }

            pt_inf->u.t_med_inf.ui8_total_size = (UINT64) ui4_value;

       /*
             *  Character device specific information
            */

        }
        else if (pt_dev_desc->ui4_dev_flag & DMH_FLAG_SOCKET_DEV)
        {
            UINT32 ui4_cnt = 0;
            DM_DEV_DESC_T *pt_tmp_dev = NULL;

            ASSERT(DM_DEV_CHR(pt_dev_desc->t_dev_type) == DM_DEV_CHR_SOCKET);

            /*
             *  General information
             */

            i4_ret = dmh_sktdev_get_model(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            pt_inf->aui1_model_str);
            if (i4_ret != DMH_OK)
            {
                i4_ret = DMR_RM_ERR;
                goto fail;
            }

            if (pt_dev_desc->t_dev_type == DM_DEV_SKT_TYPE_OPTI_DRV)
            {
                z_size = sizeof(UINT32);

                /* SKTDEV_GET_TYPE_MAX_IO_REQUEST yjg */
                pt_inf->ui4_max_io_request = 1;

                /* SKTDEV_GET_TYPE_ALIGNMENT yjg */
                pt_inf->ui4_mem_align = 32;

                i4_ret = dmh_blkdev_get_model(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            (CHAR *)pt_inf->aui1_model_str);
                if (i4_ret != DMH_OK)
                {
                    i4_ret = DMR_RM_ERR;
                    goto fail;
                }

            }
            else
            {
                pt_inf->ui4_max_io_request = 0;
                pt_inf->ui4_mem_align = 0;
            }

            /*
             *  Socket specific information
             */

            i4_ret = dmh_sktdev_get_skt_ns(
                            pt_dev_desc->ui4_major,
                            pt_dev_desc->ui4_minor,
                            &(pt_inf->u.t_skt_inf.ui4_total_slot_ns));
            if (i4_ret != DMH_OK)
            {
                i4_ret = DMR_RM_ERR;
                goto fail;
            }

            ASSERT(pt_inf->u.t_skt_inf.ui4_total_slot_ns <= DM_DEV_SKT_MAX_SLOT_NS);

            SLIST_FOR_EACH(pt_tmp_dev, &(t_dm_core.t_dev_list), t_link)
            {
                if (pt_tmp_dev->pt_prnt_dev == pt_dev_desc)
                {
                    DM_DEV_SLOT_INF_T *pt_slot_inf;

                    ASSERT(pt_dev_desc->pt_root_dev == pt_tmp_dev->pt_root_dev);
                    ASSERT(pt_dev_desc->t_hw_type == pt_tmp_dev->t_hw_type);

                    pt_slot_inf = &(pt_inf->u.t_skt_inf.at_slot_inf[ui4_cnt]);

                    pt_slot_inf->t_dev_stat.t_hw_type = pt_tmp_dev->t_hw_type;
                    pt_slot_inf->t_dev_stat.t_dev_type = pt_tmp_dev->t_dev_type;
                    pt_slot_inf->t_dev_stat.ui4_unit_id = pt_tmp_dev->ui4_unit_id;
                    pt_slot_inf->t_dev_stat.ui4_skt_no = pt_tmp_dev->ui4_skt_no;
                    pt_slot_inf->t_dev_stat.b_avail = pt_tmp_dev->b_avail;
                    pt_slot_inf->t_dev_stat.h_dev = pt_tmp_dev->h_dev;

                    pt_slot_inf->ui4_slot_no = pt_tmp_dev->ui4_skt_no;

                    ui4_cnt++;
                }
            }

            pt_inf->u.t_skt_inf.ui4_used_slot_ns = ui4_cnt;

            ASSERT(pt_inf->u.t_skt_inf.ui4_used_slot_ns <= DM_DEV_SKT_MAX_SLOT_NS);

        }
        else
        {
            ASSERT(0);
        }
    }
    else
    {
        pt_inf->u.t_uns_inf.t_uns_type = DM_DEV_UNS_TYPE_DEV;
    }

    u_rwl_release_lock(t_dm_core.h_dev_rwlock);

    return DMR_OK;

fail:

    u_rwl_release_lock(t_dm_core.h_dev_rwlock);

    return i4_ret;
}


INT32 u_dm_get_mnt_info(FM_MNT_INFO_T *pt_mnt_info)
{
    if (NULL == pt_mnt_info)
    {
        DM_ERR("pt_mnt_info can't be null!\n");
        return DMR_INVALID_HANDLE;
    }

    if (RWLR_OK != u_rwl_read_lock(g_t_dm_mnt_info.h_mnt_rwlock, RWL_OPTION_WAIT))
    {
        DM_ERR("<read lock> mnt info's rwlock failed!\n");
        return DMR_OS;
    }

    if (FALSE == g_t_dm_mnt_info.t_mnt_info.b_mnt)
    {
        DM_ERR("mnt info is not mount!\n");
        u_rwl_release_lock(g_t_dm_mnt_info.h_mnt_rwlock);
        return DMR_WRONG_DEV_STATUS;
    }

    *pt_mnt_info = g_t_dm_mnt_info.t_mnt_info;

    u_rwl_release_lock(g_t_dm_mnt_info.h_mnt_rwlock);

    return DMR_OK;
}


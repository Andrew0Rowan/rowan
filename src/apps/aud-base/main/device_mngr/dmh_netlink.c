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


#include <unistd.h>
#include <strings.h>
#include "dm.h"
#include "dmhlib.h"
#include "u_os.h"
#include "u_rwlock.h"
#include "u_ipcd.h"

BOOL  g_kernelInfo   = TRUE;

#define MAX_PAYLOAD        4096

extern INT32 g_root_usb_num;
extern BOOL _fgDmh_mount_usbfs;
extern DMH_DEV_T  *t_dmh_dev;

typedef struct _UEVENT_MSG_T
{
    UINT32      ui4_major;
    UINT32      ui4_minor;
    UINT32      ui4_seqnum;
    UINT32      ui4_devnum;
    UINT32      p_EVENT_MEDIA_DISAPPEAR;
    UINT8       ui1_busnum;
    UINT8       ui1_switch_state;
    CHAR *      p_dev_type;
    CHAR *      p_dev_path;
    CHAR *      p_driver;
    CHAR *      p_action;
    CHAR *      p_subsystem;
    CHAR *      p_product;
    CHAR *      p_type;
    CHAR *      p_interface;
    CHAR *      p_modalias;

}UEVENT_MSG_T;

INT32 dmh_init_hotplug_sock(void)
{
    struct sockaddr_nl dmh_snl;
    INT32 dmh_retval;
    INT32 dmh_hotplug_sock;
    const INT32 dmh_buffersize = 16 * 1024;

    memset(&dmh_snl, 0, sizeof(struct sockaddr_nl));
    dmh_snl.nl_family  = AF_NETLINK;
    dmh_snl.nl_pid     = getpid();
    dmh_snl.nl_groups  = 1;

    dmh_hotplug_sock   = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (-1 == dmh_hotplug_sock)
    {
        DM_ERR("[DMH]error getting socket: %s", strerror(errno));
        return DMH_ERR;
    }

    /* set receive buffersize */
    setsockopt(dmh_hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &dmh_buffersize, sizeof(dmh_buffersize));

    /* bind server */
    dmh_retval = bind(dmh_hotplug_sock, (struct sockaddr *) &dmh_snl, sizeof(struct sockaddr_nl));
    if (dmh_retval < 0)
    {
        DM_ERR("[DMH]bind failed: %s", strerror(errno));
        close(dmh_hotplug_sock);
        dmh_hotplug_sock = -1;
        return DMH_ERR;
    }

    _fgDmh_mount_usbfs = TRUE;

    return dmh_hotplug_sock;
}

struct _UEVENT_MSG_T *dmh_get_netlink_dev(CHAR *p_msg_data)
{
    size_t bufpos;
    struct _UEVENT_MSG_T *pt_uevent_msg;

    if (NULL == p_msg_data)
    {
        return NULL;
    }

    /* skip header */
    bufpos = strlen(p_msg_data) + 1;
    if (bufpos < sizeof("a@/d") || bufpos >= 4096)
    {
        DM_ERR("[DMH]invalid message length \n");
        return NULL;
    }

    /* check message header */
    if (strstr(p_msg_data, "@/") == NULL)
    {
        DM_ERR("[DMH]unrecognized message header \n");
        return NULL;
    }

    pt_uevent_msg = malloc(sizeof(UEVENT_MSG_T));
    if (pt_uevent_msg == NULL)
    {
        return NULL;
    }
    memset(pt_uevent_msg,0,sizeof(UEVENT_MSG_T));

    while (bufpos < 4096)
    {
        CHAR *key;
        size_t keylen;
        key = &p_msg_data[bufpos];
        keylen = strlen(key);
        if (0 == keylen)
        {
            break;
        }

        bufpos += keylen + 1;

        if (0 == strncmp(key, "DEVPATH=", 8))
        {
            if(NULL != pt_uevent_msg->p_dev_path)
            {
                free(pt_uevent_msg->p_dev_path);
            }
            pt_uevent_msg->p_dev_path = strdup(&key[8]);
        }
        else if (0 == strncmp(key, "SUBSYSTEM=", 10))
        {
            if (NULL == pt_uevent_msg->p_subsystem)
            {
                pt_uevent_msg->p_subsystem = strdup(&key[10]);
            }
        }
        else if (0 == strncmp(key, "DEVTYPE=", 8))
        {
            if(NULL != pt_uevent_msg->p_dev_type)
            {
                free(pt_uevent_msg->p_dev_type);
            }
            pt_uevent_msg->p_dev_type = strdup(&key[8]);
        }
        else if (0 == strncmp(key, "MAJOR=", 6))
        {
            pt_uevent_msg->ui4_major = strtoull(&key[6], NULL, 10);
        }
        else if (0 == strncmp(key, "MINOR=", 6))
        {
            pt_uevent_msg->ui4_minor = strtoull(&key[6], NULL, 10);
        }
        else if (0 == strncmp(key, "SEQNUM=", 7))
        {
            pt_uevent_msg->ui4_seqnum = strtoull(&key[7], NULL, 10);
        }
        else if (0 == strncmp(key, "DEVNUM=", 7))
        {
            pt_uevent_msg->ui4_devnum = strtoull(&key[7], NULL, 10);
        }
        else if (0 == strncmp(key, "BUSNUM=", 7))
        {
            pt_uevent_msg->ui1_busnum = strtoull(&key[7], NULL, 10);
        }
        else if (0 == strncmp(key, "SWITCH_STATE=", 13))
        {
            pt_uevent_msg->ui1_switch_state = strtoull(&key[13], NULL, 10);
        }
        else if (0 == strncmp(key, "ACTION=", 7))
        {
            if(NULL != pt_uevent_msg->p_action)
            {
                free(pt_uevent_msg->p_action);
            }
            pt_uevent_msg->p_action = strdup(&key[7]);
        }
        else if (0 == strncmp(key, "DRIVER=", 7))
        {
            if (NULL != pt_uevent_msg->p_driver)
            {
                free(pt_uevent_msg->p_driver);
            }
            pt_uevent_msg->p_driver    = strdup(&key[7]);
        }
        else if (0 == strncmp(key, "TYPE=", 5))
        {
            if (NULL != pt_uevent_msg->p_type)
            {
                free(pt_uevent_msg->p_type);
            }
            pt_uevent_msg->p_type = strdup(&key[5]);
        }
        else if (0 == strncmp(key, "INTERFACE=", 10))
        {
            if (NULL != pt_uevent_msg->p_interface)
            {
                free(pt_uevent_msg->p_interface);
            }
            pt_uevent_msg->p_interface = strdup(&key[10]);
        }
        else if (0 == strncmp(key, "MODALIAS=", 9))
        {
            if (NULL != pt_uevent_msg->p_modalias)
            {
                free(pt_uevent_msg->p_modalias);
            }
            pt_uevent_msg->p_modalias = strdup(&key[9]);
        }
        else if (0 == strncmp(key, "PRODUCT=", 8))
        {
            if (NULL != pt_uevent_msg->p_product)
            {
                free(pt_uevent_msg->p_product);
            }
            pt_uevent_msg->p_product= strdup(&key[8]);
        }
        else if (0 == strncmp(key, "DISK_EVENT_MEDIA_DISAPPEAR=", 27))
        {
            pt_uevent_msg->p_EVENT_MEDIA_DISAPPEAR= strtoull(&key[27], NULL, 10);
            DM_INFO("[DMH]%10s(%d):pt_uevent_msg->p_EVENT_MEDIA_DISAPPEAR == %ld\n",__FUNCTION__,__LINE__,pt_uevent_msg->p_EVENT_MEDIA_DISAPPEAR);
        }
    }

    return pt_uevent_msg;

}


static INT32 dmh_free_uevent_msg(struct _UEVENT_MSG_T *pt_uevent_msg)
{
    if(NULL == pt_uevent_msg)
    {
        return DMH_INVALID_PARM;
    }

    if(NULL != pt_uevent_msg->p_dev_type)
    {
        free(pt_uevent_msg->p_dev_type);
        pt_uevent_msg->p_dev_type = NULL;
    }

    if(NULL != pt_uevent_msg->p_dev_path)
    {
        free(pt_uevent_msg->p_dev_path);
        pt_uevent_msg->p_dev_path = NULL;
    }

    if(NULL != pt_uevent_msg->p_driver)
    {
        free(pt_uevent_msg->p_driver);
        pt_uevent_msg->p_driver = NULL;
    }

    if(NULL != pt_uevent_msg->p_action)
    {
        free(pt_uevent_msg->p_action);
        pt_uevent_msg->p_action = NULL;
    }

    if(NULL != pt_uevent_msg->p_subsystem)
    {
        free(pt_uevent_msg->p_subsystem);
        pt_uevent_msg->p_subsystem = NULL;
    }

    if(NULL != pt_uevent_msg->p_product)
    {
        free(pt_uevent_msg->p_product);
        pt_uevent_msg->p_product = NULL;
    }

    if(NULL != pt_uevent_msg->p_type)
    {
        free(pt_uevent_msg->p_type);
        pt_uevent_msg->p_type = NULL;
    }

    if(NULL != pt_uevent_msg->p_interface)
    {
        free(pt_uevent_msg->p_interface);
        pt_uevent_msg->p_interface = NULL;
    }

    if(NULL != pt_uevent_msg->p_modalias)
    {
        free(pt_uevent_msg->p_modalias);
        pt_uevent_msg->p_modalias = NULL;
    }

    free(pt_uevent_msg);
    pt_uevent_msg = NULL;

    return DMH_OK;

}

INT32 dmh_cb_msg_thread(VOID *pv_data)
{
    struct sockaddr_nl dest_addr;
    static BOOL b_loop_flag = FALSE;
    static BOOL b_find_hub  = FALSE;
    static BOOL b_add_change_message = FALSE;
    struct _UEVENT_MSG_T * pt_uevent_msg = NULL;
    CHAR *kernelmsg = NULL;
    struct iovec iov;
    struct msghdr msg;
    INT32 i4_ret = 0;
    kernelmsg = (CHAR *)malloc(MAX_PAYLOAD);
    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(kernelmsg, 0, MAX_PAYLOAD);
    memset(&iov, 0, sizeof(iov));
    iov.iov_base = (void *)kernelmsg;
    iov.iov_len = MAX_PAYLOAD;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
    CHAR* dmh_dev_path = malloc(256);
    memset(dmh_dev_path,0,256);
    while(1)
    {
        memset(dmh_dev_path,0,256);
        memset(kernelmsg, 0, MAX_PAYLOAD);
        DM_INFO("[DMH] start recv netlink msg .\n");
        DM_INFO("dmh_cb_msg_thread sock_fd = %ld\n",*(INT32 *)pv_data);
        i4_ret = recvmsg(*(INT32 *)pv_data, &msg, 0);
        if (i4_ret < 0)
        {
            DM_ERR("[USER] Socket recvmsg failed., err message %s \n", strerror(errno));
            continue;
            //break;
        }

        if(g_kernelInfo)
        {
            int cnt;

            DM_INFO("\n");

            DM_INFO("[Msg from Kernel]\n");

            for (cnt = 0; cnt < i4_ret; cnt++)
            {
                if (kernelmsg[cnt] == 0)
                {
                  DM_MSG("\n");
                }
                else
                {
                  DM_MSG("%c", kernelmsg[cnt]);
                }
            }
        }

        pt_uevent_msg = dmh_get_netlink_dev(kernelmsg);
        if(NULL == pt_uevent_msg)
        {
            return DMH_ERR;
        }

        if(NULL == pt_uevent_msg->p_subsystem)
        {
            dmh_free_uevent_msg(pt_uevent_msg);
            continue;
        }

        if(0 == strncmp(pt_uevent_msg->p_subsystem,"module",strlen("module")))
        {
            dmh_free_uevent_msg(pt_uevent_msg);
            continue;
        }

        /* USB ENDPOINT NOT SUPPORT*/
        DM_INFO("pt_uevent_msg->p_subsystem = %s\n", pt_uevent_msg->p_subsystem);
        if(strncmp(pt_uevent_msg->p_subsystem,"usb_endpoint",strlen("usb_endpoint")) ==0 &&
          (strstr(pt_uevent_msg->p_dev_path,"epee")!= NULL ||
           strstr(pt_uevent_msg->p_dev_path,"epef")!= NULL ||
           strstr(pt_uevent_msg->p_dev_path,"eped")!= NULL))
        {
            struct _DMH_DEV_T * pt_tmp_dev;
            int index = 0;

            memset(dmh_dev_path,0,256);
            strncpy(dmh_dev_path,"/sys",strlen("/sys"));
            strncat(dmh_dev_path,pt_uevent_msg->p_dev_path,strlen(pt_uevent_msg->p_dev_path)-strlen(rindex(pt_uevent_msg->p_dev_path,':')));

            DM_INFO("%s   line: %d dmh_dev_path =%s\n", __FUNCTION__, __LINE__, dmh_dev_path);
            index = (int)strlen(dmh_dev_path) - 1;
            while(index >= 0)
            {
                if (dmh_dev_path[index] == '/')
                {
                   dmh_dev_path[index] = 0;
                   break;
                }
                index--;
            }

            DM_INFO("%s   line: %d dmh_dev_path =%s\n", __FUNCTION__, __LINE__, dmh_dev_path);

            i4_ret = dmh_add_fake_hub_dev(dmh_dev_path,&pt_tmp_dev);
            if(i4_ret != DMH_OK)
            {
               DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
               dmh_free_uevent_msg(pt_uevent_msg);
               continue;
            }
            pt_tmp_dev->t_device_type = DMH_DEV_MED_TYPE_USB_EP;

            i4_ret = dmh_nfy_func(pt_tmp_dev,pt_tmp_dev->ui1_dev_level,DEV_STATUS_DEV_UNSUPPORT);
            if(DMH_OK != i4_ret)
            {
               DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
               dmh_free_uevent_msg(pt_uevent_msg);
               continue;
            }

            i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
            if(DMH_OK != i4_ret)
            {
               DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
               continue;
            }

            continue;

        }

        if(NULL == pt_uevent_msg->p_dev_type)
        {
            dmh_free_uevent_msg(pt_uevent_msg);
            continue;
        }

        if(0 == strncmp(pt_uevent_msg->p_action,"add",3))
        {
            /* root hub */
            if(pt_uevent_msg->ui4_devnum == 1)
            {
                DM_INFO(" the root hub can't be hotplugged \n");
                CHAR temp_node_name[26];
                dev_t t_tmp;
                mode_t mode = 0660;

                snprintf(temp_node_name,sizeof(temp_node_name), "/dev/usbdev%d.%d", (int)pt_uevent_msg->ui1_busnum, (int)pt_uevent_msg->ui4_devnum);
                t_tmp = (pt_uevent_msg->ui4_major<<8)|(pt_uevent_msg->ui4_minor);
                i4_ret = mknod(temp_node_name,S_IFCHR|mode,t_tmp);

                dmh_free_uevent_msg(pt_uevent_msg);

                continue;
            }
            else if(0 == strncmp(pt_uevent_msg->p_dev_type,"usb_device",strlen("usb_device")) &&
                    0 == strncmp(pt_uevent_msg->p_subsystem,"usb",strlen("usb")))
            {
                /** PATCH FOR MOUNT USBFS 20091110*/
                if (_fgDmh_mount_usbfs)
                {
                    CHAR ps_cmd[64];
                    INT32 i4_sys_ret;

                    snprintf(ps_cmd, 64,"mount -t usbfs none /proc/bus/usb");
                    i4_sys_ret = system(ps_cmd);
                    DM_INFO("[DMH] mount usbfs i4_sys_ret = %d\n", (int)i4_sys_ret);

                    if (-1 == i4_sys_ret)
                    {
                        snprintf(ps_cmd, 64,"mount -t usbdevfs /proc/bus/usb");
                        i4_sys_ret = system(ps_cmd);
                        DM_INFO("[DMH] mount usbdevfs i4_sys_ret = %d\n", (int)i4_sys_ret);
                    }
                    _fgDmh_mount_usbfs = FALSE;
                }

                /** PATCH END*/
                struct _DMH_DEV_T * pt_tmp_dev;

                if (RWLR_OK != u_rwl_read_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_OS;
                }

                SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
                {
                    if((pt_tmp_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major)&&
                       (pt_tmp_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                    {
                        dmh_free_uevent_msg(pt_uevent_msg);
                        DM_INFO("the device already in \n");
                        b_loop_flag = TRUE;
                        b_find_hub = TRUE;
                        break;
                    }
                }

                u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

                if(b_loop_flag)
                {
                    b_loop_flag = FALSE;
                    dmh_free_uevent_msg(pt_uevent_msg);
                    continue;
                }
                dmh_free_uevent_msg(pt_uevent_msg);

            }

            else if(0 == strncmp(pt_uevent_msg->p_dev_type,"usb_interface",strlen("usb_interface")) &&
                    0 == strncmp(pt_uevent_msg->p_subsystem,"usb",strlen("usb")))
            {
                UINT8 ui1_loop = 0;
                struct _DMH_DEV_T * pt_tmp_dev;
                BOOL  fgFoundDriver = FALSE;
                if(b_find_hub)
                {
                    b_find_hub = FALSE;
                    i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                    if(i4_ret != DMH_OK)
                    {
                        DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                        continue;
                    }
                    continue;
                }

                struct _BLK_UEVENT_ATTR_T    t_blk_uevent_attr;
                memset((VOID *)&t_blk_uevent_attr, 0, sizeof(BLK_UEVENT_ATTR_T));
                strncpy(dmh_dev_path,"/sys",strlen("/sys"));
                strncat(dmh_dev_path,pt_uevent_msg->p_dev_path,strlen(pt_uevent_msg->p_dev_path));
/* Can't directly read driver value from uEvent file, loop to read it */
#if 1 // patched for BDP00122708
                ui1_loop = 0;
                while(1)
                {
                    i4_ret = dmh_parse_uevent_file(dmh_dev_path, &t_blk_uevent_attr);
                    if (NULL == t_blk_uevent_attr.ps_driver)
                    {
                       ui1_loop++;
                       usleep(10000);
                    }
                    else
                    {
                       fgFoundDriver = TRUE;
                       break;
                    }

                    if(ui1_loop > 20 && FALSE == fgFoundDriver)
                    {
                        DM_ERR("Can't read driver type %10s(%d):\n",__FUNCTION__,__LINE__);
                        if (DMH_OK != i4_ret)
                        {
                            DM_INFO("Can't read driver type %10s(%d): 1d6b\n",__FUNCTION__,__LINE__);
                            fgFoundDriver = TRUE;

                            i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                            if(DMH_OK != i4_ret)
                            {
                               DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                               continue;
                            }

                            i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                            if(DMH_OK != i4_ret)
                            {
                               DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                               continue;
                            }
                        }
                        break;
                    }
                }

                if(ui1_loop > 20 && TRUE == fgFoundDriver)
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    continue;
                }
#endif
                memset(dmh_dev_path,0,256);
                strncpy(dmh_dev_path,"/sys",strlen("/sys"));
                strncat(dmh_dev_path,pt_uevent_msg->p_dev_path,strlen(pt_uevent_msg->p_dev_path)-strlen(rindex(pt_uevent_msg->p_dev_path,'/')));

                if(FALSE == fgFoundDriver || 0 == strncmp(t_blk_uevent_attr.ps_driver,"usb-storage",strlen("usb-storage")))
                {
                        UINT16 ui2_product_id;
                        UINT16 ui2_vendor_id;

                        ui2_product_id = (UINT16) dmh_get_attr_product_id(dmh_dev_path);
                        ui2_vendor_id  = (UINT16) dmh_get_attr_vendor_id(dmh_dev_path);

                        DM_INFO("ui2_product_id is %d \n",ui2_product_id);
                        DM_INFO("ui2_vendor_id is %d \n",ui2_vendor_id);

                        if ((1423 == ui2_vendor_id) && (25444 == ui2_product_id))//for BDP00072266.
                        {
                            if (FALSE == fgFoundDriver)
                            {
                                DM_INFO("[DMH]%s(%d): don't send unsupport message for special device \n",__FUNCTION__,__LINE__);
                                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                                i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                                continue;
                            }
                        }

                        if (NULL == t_blk_uevent_attr.ps_driver)
                        {
                            if(DMH_OK == i4_ret || DMH_INVALID_DEV_TYPE == i4_ret)
                            {
                                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                                i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                                if(DMH_OK != i4_ret)
                                {
                                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                                    continue;
                                }
                                continue;
                             }
                        }
                }

                if(FALSE == fgFoundDriver)
                {
                    UINT16 ui2_product_id;
                    UINT16 ui2_vendor_id;
                    ui2_product_id = (UINT16) dmh_get_attr_product_id(dmh_dev_path);
                    ui2_vendor_id  = (UINT16) dmh_get_attr_vendor_id(dmh_dev_path);
                    DM_INFO("ui2_product_id is %d \n",ui2_product_id);
                    DM_INFO("ui2_vendor_id is %d \n",ui2_vendor_id);

                    if (((ui2_vendor_id == 0x0CF3) && (ui2_product_id == 0x7011))
                       ||((ui2_vendor_id == 0x045E) && (ui2_product_id == 0x02A7))
                       ||((ui2_vendor_id == 0x0411) && (ui2_product_id == 0x017F))
                       ||((ui2_vendor_id == 0x148F) && (ui2_product_id == 0x2870))
                       ||((ui2_vendor_id == 0x148F) && (ui2_product_id == 0x3370))
                       ||((ui2_vendor_id == 0x0489) && (ui2_product_id == 0x0006))
                       ||((ui2_vendor_id == 0x471) && (ui2_product_id == 0x20dd))
                       ||((ui2_vendor_id == 0x471) && (ui2_product_id == 0x209e))
                       ||((ui2_vendor_id == 0x0) && (ui2_product_id == 0x0)))
                    {
                        //g_root_usb_num record the root number,it will be reference by app.
                        if(strstr(dmh_dev_path,"usb1"))
                        {
                            g_root_usb_num=1;
                        }
                        else if (strstr(dmh_dev_path,"usb2"))
                        {
                            g_root_usb_num=2;
                        }
                        else if(strstr(dmh_dev_path,"usb3"))
                        {
                            g_root_usb_num=3;
                        }
                        else
                        {
                            DM_ERR("[DMH]: not invalid path:dmh_dev_path=%s\n",dmh_dev_path);
                        }
                        //Don't notify unsupport message. for special wifi dongle.
                        DM_INFO("[DMH]  Don't notify unsupport message for special wifi dongle .\n");
                        i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                        i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);

                        continue;
                    }

                    i4_ret = dmh_add_fake_hub_dev(dmh_dev_path,&pt_tmp_dev);
                    if(DMH_OK != i4_ret)
                    {
                         i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                         i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                         continue;
                    }

                    pt_tmp_dev->t_device_type = DM_DEV_UKW_TYPE_UNKNOWN;
                    i4_ret = dmh_nfy_func(pt_tmp_dev,pt_tmp_dev->ui1_dev_level,DEV_STATUS_DEV_UNSUPPORT);
                    if(DMH_OK != i4_ret)
                    {
                        dmh_free_uevent_msg(pt_uevent_msg);
                        return DMH_ERR;
                    }

                    i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                    i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                    if(i4_ret != DMH_OK)
                    {
                        return DMH_ERR;
                    }

                    fgFoundDriver = FALSE;

                    continue;
                }

                if(0 == strncmp(t_blk_uevent_attr.ps_driver,"hub",strlen("hub")))
                {
                     DM_INFO(" strncmp t_blk_uevent_attr.ps_driver  hub \n");
                     i4_ret = dmh_add_real_hub_dev(dmh_dev_path,&pt_tmp_dev);
                     if(DMH_OK != i4_ret)
                     {
                         i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                         i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                         continue;
                     }

                     i4_ret = dmh_nfy_func(pt_tmp_dev,pt_tmp_dev->ui1_dev_level,DEV_STATUS_HUB_ATTACH);
                     i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                     i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                     if(DMH_OK != i4_ret)
                     {
                         DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                         continue;
                     }
                     continue;

                 }
                 else if(0 == strncmp(t_blk_uevent_attr.ps_driver,"usb-storage",strlen("usb-storage")))
                 {
                     DM_INFO(" strncmp t_blk_uevent_attr.ps_driver  usb-storage \n");
                     i4_ret = dmh_add_fake_hub_dev(dmh_dev_path,&pt_tmp_dev);
                     if(DMH_OK != i4_ret)
                     {
                         i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                         i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                         continue;
                     }

                     i4_ret = dmh_nfy_func(pt_tmp_dev,pt_tmp_dev->ui1_dev_level,DEV_STATUS_HUB_ATTACH);
                     if(DMH_OK != i4_ret)
                     {
                         DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                         dmh_free_uevent_msg(pt_uevent_msg);
                         continue;
                     }

                     i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                     i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                     if(DMH_OK != i4_ret)
                     {
                         DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                         continue;
                     }
                     continue;
                 }

                 i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                 i4_ret = dmh_free_uevent_file(&t_blk_uevent_attr);
                 DM_INFO("%10s(%d):\n",__FUNCTION__,__LINE__);
                 if(DMH_OK != i4_ret)
                 {
                     DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                     continue;
                 }
                 continue;

            }

            /*usb med */
            else if(0 == strncmp(pt_uevent_msg->p_dev_type,"disk",strlen("disk")) &&
                    0 == strncmp(pt_uevent_msg->p_subsystem,"block",strlen("block")))
            {
                /** Wei Yu Patch for BDP00125235*/
                if (11 == pt_uevent_msg->ui4_major && NULL != pt_uevent_msg->p_dev_path)
                {
                    DM_INFO("[DMH] %s %d device path = %s\n", __FUNCTION__, __LINE__, pt_uevent_msg->p_dev_path);
                    if (NULL != strstr(pt_uevent_msg->p_dev_path,"usb") ||
                        NULL != strstr(pt_uevent_msg->p_dev_path,"virtual"))
                    {
                        i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                        if(DMH_OK != i4_ret)
                        {
                            DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                            continue;
                        }
                        continue;
                    }
                }
                /** End*/

                if (NULL != pt_uevent_msg->p_dev_path && NULL != strstr(pt_uevent_msg->p_dev_path,"virtual"))
                {
                    i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                    if(DMH_OK != i4_ret)
                    {
                        DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                        continue;
                    }
                    continue;
                }

                struct _DMH_DEV_T * pt_tmp_dev;
                if (RWLR_OK != u_rwl_read_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_OS;
                }

                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
                {
                    if((pt_tmp_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major)&&
                       (pt_tmp_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                    {
                        b_loop_flag = TRUE;
                        break;
                    }
                }

                u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

                if(b_loop_flag)
                {
                    b_loop_flag = FALSE;
                    dmh_free_uevent_msg(pt_uevent_msg);
                    continue;
                }
                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);

                strncpy(dmh_dev_path,"/sys",strlen("/sys"));
                strncat(dmh_dev_path,pt_uevent_msg->p_dev_path,strlen(pt_uevent_msg->p_dev_path));
                DM_INFO("[DMH]%10s(%d): dmh_dev_path = %s\n",__FUNCTION__,__LINE__, dmh_dev_path);

                if(NULL != strstr(dmh_dev_path,"mmcblk"))
                {
                    DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                    //i4_ret = dmh_add_mmc_medium_dev(dmh_dev_path);
                    DM_INFO("[DMH]%10s(%d): return %d\n",__FUNCTION__,__LINE__, (int)i4_ret);
                }
                else
                {
                    i4_ret = dmh_add_usb_storage_dev(dmh_dev_path);
                }

                if(DMH_OK != i4_ret)
                {
                    if(DMH_INVALID_PARM == i4_ret)
                    {
                        DM_ERR("[DMH]%10s(%d): err, but continue\n",__FUNCTION__,__LINE__);
                        if (DMH_OK != dmh_free_uevent_msg(pt_uevent_msg))
                        {
                            return DMH_ERR;
                        }

                        continue;
                    }

                    DM_ERR("[DMH]%10s(%d): err, return\n",__FUNCTION__,__LINE__);
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_ERR;
                }

                if (RWLR_OK != u_rwl_read_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_OS;
                }

                SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
                {
                    if((pt_tmp_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major)&&
                       (pt_tmp_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                    {
                        DM_INFO("[DMH]%10s(%d) ui1_dev_level %d\n",__FUNCTION__,__LINE__,pt_tmp_dev->ui1_dev_level);
                        if (11 != pt_uevent_msg->ui4_major )
                        {
                            u_thread_delay(1000);
                        }
                        else
                        {
                            DM_INFO("[DMH]%10s(%d) Major %ld:\n",__FUNCTION__,__LINE__,pt_uevent_msg->ui4_major);
                        }

                        i4_ret = dmh_nfy_func(pt_tmp_dev,pt_tmp_dev->ui1_dev_level,DEV_STATUS_MED_ATTACH);
                        if(DMH_OK != i4_ret)
                        {
                            break;;
                        }
                    }
                }

                u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                    continue;
                }

                continue;
            }
            /*usb partition */
            else if(0 == strncmp(pt_uevent_msg->p_dev_type,"partition",strlen("partition")) &&
                    0 == strncmp(pt_uevent_msg->p_subsystem,"block",strlen("block")))
            {
                CHAR temp_node_name[20];
                struct _DMH_DEV_T * pt_tmp_dev;
                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);

                if(RWLR_OK != u_rwl_read_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_OS;
                }

                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
                {
                    if((pt_tmp_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major)&&
                       (pt_tmp_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                    {
                        b_loop_flag = TRUE;
                        break;
                    }
                }

                u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

                if(b_loop_flag)
                {
                    b_loop_flag = FALSE;
                    dmh_free_uevent_msg(pt_uevent_msg);
                    continue;
                }

                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                strncpy(dmh_dev_path,"/sys",strlen("/sys"));
                strncat(dmh_dev_path,pt_uevent_msg->p_dev_path,strlen(pt_uevent_msg->p_dev_path));

                strncpy(temp_node_name,"/dev/",sizeof(temp_node_name));
                strncat(temp_node_name, rindex(dmh_dev_path, 's'), sizeof(temp_node_name) - strlen(temp_node_name));

                DM_INFO("[DMH]%10s(%d) dmh_dev_path%s temp_node_name%s:\n",__FUNCTION__,__LINE__, dmh_dev_path, temp_node_name);
                if(NULL != strstr(dmh_dev_path,"mmcblk"))
                {
                    DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                    //i4_ret = dmh_add_mmc_medium_dev(dmh_dev_path);
                    DM_INFO("[DMH]%10s(%d): return %d\n",__FUNCTION__,__LINE__, (int)i4_ret);
                }
                else
                {
                    i4_ret = dmh_add_usb_partition_dev(dmh_dev_path,temp_node_name);
                }

                if(DMH_OK != i4_ret)
                {
                    if(DMH_INVALID_PARM == i4_ret)
                    {
                        DM_ERR("[DMH]%10s(%d): err, but continue\n",__FUNCTION__,__LINE__);
                        if (DMH_OK != dmh_free_uevent_msg(pt_uevent_msg))
                        {
                            return DMH_ERR;
                        }

                        continue;
                    }
                    DM_ERR("[DMH]%10s(%d): err, return\n",__FUNCTION__,__LINE__);
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_ERR;
                }

                if (RWLR_OK != u_rwl_read_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_OS;
                }

                SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
                {
                    if((pt_tmp_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major)&&
                    (pt_tmp_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                    {
                        DM_INFO("[DMH]%10s(%d): major:%lu minor:%lu\n",__FUNCTION__,__LINE__, pt_uevent_msg->ui4_major, pt_uevent_msg->ui4_minor);
                        i4_ret = dmh_nfy_func(pt_tmp_dev,pt_tmp_dev->ui1_dev_level,(pt_tmp_dev->t_device_type == DMH_DEV_MED_TYPE_MMC_STORAGE) ? DEV_STATUS_MED_ATTACH : DEV_STATUS_PART_DETECTED);
                        if(DMH_OK != i4_ret)
                        {
                            break;
                        }
                    }
                }

                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                    continue;
                }
                continue;

            }
            else
            {
                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                    continue;
                }
                continue;
            }

        }
        else if(0 == strncmp(pt_uevent_msg->p_action,"remove",6))
        {
            /* root hub */
            if(1 == pt_uevent_msg->ui4_devnum)
            {
                dmh_free_uevent_msg(pt_uevent_msg);
                DM_INFO("[DMH]Receive msg\n");
            }
            /*hub */
            else if(0 == strncmp(pt_uevent_msg->p_dev_type,"usb_device",strlen("usb_device")) &&
                    0 == strncmp(pt_uevent_msg->p_subsystem,"usb",strlen("usb")))
            {
                struct _DMH_DEV_T * pt_tmp_dev;
                UINT8 ui1_level;
                strncpy(dmh_dev_path,"/sys",strlen("/sys"));
                strncat(dmh_dev_path,pt_uevent_msg->p_dev_path,strlen(pt_uevent_msg->p_dev_path));

                DM_INFO("%10s(%d):\n",__FUNCTION__,__LINE__);

                if (RWLR_OK != u_rwl_write_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_OS;
                }

                SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
                {
                    if((pt_tmp_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major)&&
                       (pt_tmp_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                    {
                        ui1_level = pt_tmp_dev->ui1_dev_level;
                        i4_ret = dmh_nfy_func(pt_tmp_dev,ui1_level,DEV_STATUS_HUB_DETACH);
                        if(DMH_OK != i4_ret)
                        {
                             DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                             continue;
                        }

                        SLIST_REMOVE(pt_tmp_dev,t_link);
                        i4_ret = dmh_free_dev(pt_tmp_dev);
                        if(DMH_OK != i4_ret)
                        {
                            DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                            continue;
                        }

                        pt_tmp_dev = SLIST_FIRST(&(t_dmh_dev->t_dev_list));
                    }
                }

                SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
                {
                    if((pt_tmp_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major)&&
                    (pt_tmp_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                    {
                        DM_INFO("%10s(%d):\n",__FUNCTION__,__LINE__);
                        ui1_level = pt_tmp_dev->ui1_dev_level;
                        i4_ret = dmh_nfy_func(pt_tmp_dev,ui1_level,DEV_STATUS_HUB_DETACH);
                        if(DMH_OK != i4_ret)
                        {
                             DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                             continue;
                        }

                        SLIST_REMOVE(pt_tmp_dev,t_link);
                        i4_ret = dmh_free_dev(pt_tmp_dev);
                        if(DMH_OK != i4_ret)
                        {
                            DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                            continue;
                        }
                        break;
                    }
                }

                u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

                DM_INFO("%10s(%d):\n",__FUNCTION__,__LINE__);
                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                    continue;
                }
                continue;

            }

            /*usb med */
            else if(0 == strncmp(pt_uevent_msg->p_dev_type,"disk",strlen("disk"))&&
                    0 == strncmp(pt_uevent_msg->p_subsystem,"block",strlen("block")))
            {
                /** Wei Yu Patch for BDP00125235*/
#if 1           //!DM_USB_EXT_LOADER_SUPPORT
                if (11 == pt_uevent_msg->ui4_major && NULL != pt_uevent_msg->p_dev_path)
                {
                    if (NULL != strstr(pt_uevent_msg->p_dev_path,"usb") ||
                        NULL != strstr(pt_uevent_msg->p_dev_path,"virtual"))
                    {
                        i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                        if(DMH_OK != i4_ret)
                        {
                            DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                            continue;
                        }
                        continue;
                    }
                }
#endif
                /** End*/

                if (NULL != pt_uevent_msg->p_dev_path && strstr(pt_uevent_msg->p_dev_path,"virtual") != NULL)
                {
                    i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                    if(DMH_OK != i4_ret)
                    {
                        return DMH_ERR;
                    }
                    continue;
                }

                struct _DMH_DEV_T * pt_tmp_dev;
                UINT8 ui1_level=0;
                strncpy(dmh_dev_path,"/sys",strlen("/sys"));
                strncat(dmh_dev_path,pt_uevent_msg->p_dev_path,strlen(pt_uevent_msg->p_dev_path));
                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);

                if (RWLR_OK != u_rwl_write_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_OS;
                }

                INT32 i=0;
                for(i=0; i<2; i++)
                {
                    SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
                    {
                        if((pt_tmp_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major)&&
                           (pt_tmp_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                        {
                            ui1_level = pt_tmp_dev->ui1_dev_level;

                            i4_ret = dmh_nfy_func(pt_tmp_dev,ui1_level,DEV_STATUS_MED_DETACH);
                            if(DMH_OK != i4_ret)
                            {
                               DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                               continue;
                            }

                            SLIST_REMOVE(pt_tmp_dev,t_link);
                            i4_ret = dmh_free_dev(pt_tmp_dev);
                            if(DMH_OK != i4_ret)
                            {
                                break;
                            }

                            break;
                        }
                    }
                }

                u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                    continue;
                }
                continue;
            }
            /*usb partition */
            else if(0 == strncmp(pt_uevent_msg->p_dev_type,"partition",strlen("partition")) &&
                    0 == strncmp(pt_uevent_msg->p_subsystem,"block",strlen("block")))
            {
                struct _DMH_DEV_T * pt_tmp_dev;
                UINT8 ui1_level=0;
                strncpy(dmh_dev_path,"/sys",strlen("/sys"));
                strncat(dmh_dev_path,pt_uevent_msg->p_dev_path,strlen(pt_uevent_msg->p_dev_path));

                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);

                if (RWLR_OK != u_rwl_write_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_OS;
                }

                SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
                {
                    if((pt_tmp_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major)&&
                    (pt_tmp_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                    {

                        DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                        ui1_level = pt_tmp_dev->ui1_dev_level;

                        DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);

                        if(!pt_tmp_dev->b_remove_card)
                        {
                            i4_ret = dmh_nfy_func(pt_tmp_dev,ui1_level,(pt_tmp_dev->t_device_type == DMH_DEV_MED_TYPE_MMC_STORAGE) ? DEV_STATUS_MED_DETACH : DEV_STATUS_PART_REMOVED);
                            if(DMH_OK != i4_ret)
                            {
                                break;
                            }
                        }

                        if(pt_tmp_dev->b_remove_card)
                        {
                            pt_tmp_dev->b_remove_card = FALSE;
                        }

                        SLIST_REMOVE(pt_tmp_dev,t_link);

                        i4_ret = dmh_free_dev(pt_tmp_dev);
                        if(DMH_OK != i4_ret)
                        {
                            break;
                        }
                        break;
                    }
                }

                u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                    continue;
                }
                continue;
            }
            else
            {
                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                    continue;
                }
                continue;
            }
            /* add other usb device */

        }
        else if(0 == strncmp(pt_uevent_msg->p_action,"change",6)) /* Change for memory card insert or plug out  tray in and tray out for BD*/
        {
            if((0 == strncmp(pt_uevent_msg->p_dev_type,"disk",strlen("disk"))) &&
               (0 == strncmp(pt_uevent_msg->p_subsystem,"block",strlen("block"))))
            {
                struct _DMH_DEV_T * pt_dmh_dev;

                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                if (strstr(pt_uevent_msg->p_dev_path,"loop")!=NULL)
                {
                    i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                    continue;
                }

                b_add_change_message = TRUE;

                strncpy(dmh_dev_path,"/sys",strlen("/sys"));
                strncat(dmh_dev_path,pt_uevent_msg->p_dev_path,strlen(pt_uevent_msg->p_dev_path));

                if (RWLR_OK != u_rwl_read_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
                {
                    dmh_free_uevent_msg(pt_uevent_msg);
                    return DMH_OS;
                }

                SLIST_FOR_EACH(pt_dmh_dev, &(t_dmh_dev->t_dev_list), t_link)
                {
                    if ((pt_dmh_dev->t_uevent_attr.ui4_major == pt_uevent_msg->ui4_major) &&
                        (pt_dmh_dev->t_uevent_attr.ui4_minor == pt_uevent_msg->ui4_minor))
                    {
                        b_add_change_message = TRUE;

                        UINT64 ui8_size = 0;
                        ui8_size = dmh_get_attr_num(pt_dmh_dev->ps_full_dev_path,"size");
                        pt_dmh_dev->ui8_size = ui8_size;
                        DM_INFO("[DMH]%10s(%d)ui8_size%lld:\n",__FUNCTION__,__LINE__,ui8_size);
                        i4_ret = dmh_nfy_func(pt_dmh_dev,pt_dmh_dev->ui1_dev_level,DEV_STATUS_MED_ATTACH);
                        if(DMH_OK != i4_ret)
                        {
                            break;
                        }
                        break;
                    }
                }

                u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

                if (NULL != pt_dmh_dev)
                {
                    i4_ret = dmh_nfy_func(pt_dmh_dev,pt_dmh_dev->ui1_dev_level,DEV_STATUS_MED_ATTACH);
                    if(DMH_OK != i4_ret)
                    {
                        DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                        dmh_free_uevent_msg(pt_uevent_msg);
                        continue;
                    }
                }

                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                    continue;
                }
                continue;
            }
            else
            {
                i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                    continue;
                }
                continue;
            }
        }
        else
        {
            i4_ret = dmh_free_uevent_msg(pt_uevent_msg);
            if(DMH_OK != i4_ret)
            {
                DM_ERR("[DMH]%s(%d): get error information \n",__FUNCTION__,__LINE__);
                continue;
            }

            continue;
        }

    }
    
    dmh_free_uevent_msg(pt_uevent_msg);
    return DMH_OK;

}


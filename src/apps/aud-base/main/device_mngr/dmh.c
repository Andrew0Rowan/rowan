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


#include "dm.h"
#include "dmh.h"
#include "dmhlib.h"
#include "dm_linux.h"
#include "u_os.h"
#include "u_rwlock.h"

BOOL _fgDmh_open_dev_thread = TRUE;
BOOL _fgDmh_open_dev_thread_exited = FALSE;
DMH_DEV_T *t_dmh_dev;

extern INT32 dmh_init_hotplug_sock(void);
extern INT32 dmh_scan_path(char *ps_path);

INT32 dmh_blkdev_get_size(UINT32 ui4_major,UINT32 ui4_minor,UINT64 *pui8_size)
{
     struct _DMH_DEV_T * pt_tmp_dev;

     SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
     {
         if ((pt_tmp_dev->t_uevent_attr.ui4_major == ui4_major) &&
             (pt_tmp_dev->t_uevent_attr.ui4_minor== ui4_minor))
         {
             *pui8_size = pt_tmp_dev->ui8_size;
             return DMH_OK;
         }
     }
     return DMH_ERR;

}

INT32 dmh_blkdev_get_freesize(UINT32 ui4_major,UINT32 ui4_minor,UINT64 *pui8_freesize)
{
     struct _DMH_DEV_T * pt_tmp_dev;

     SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
     {
         if ((pt_tmp_dev->t_uevent_attr.ui4_major == ui4_major) &&
             (pt_tmp_dev->t_uevent_attr.ui4_minor== ui4_minor))
         {
             *pui8_freesize = pt_tmp_dev->ui8_freesize;
             return DMH_OK;
         }
     }
     return DMH_ERR;

}

INT32 dmh_blkdev_get_model(UINT32 ui4_major,UINT32 ui4_minor,CHAR  *ps_model_str)
{
    struct _DMH_DEV_T * pt_tmp_dev;
    CHAR * ps_path = NULL;
    BOOL found = FALSE;

    if (ui4_major == 11 && ui4_minor == 0)
    {
        SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
        {
            if (pt_tmp_dev->t_device_type == DMH_DEV_SKT_TYPE_OPTI_DRV)
            {
                if (pt_tmp_dev->ps_full_dev_path != NULL)
                {
                    ps_path = malloc(strlen(pt_tmp_dev->ps_full_dev_path) + 1);
                     if(ps_path==NULL)
                         {
                            DM_ERR("[DM_TEST] ps_path malloc failed\n");
                            break;
                         }
                    memset(ps_path, 0, strlen(pt_tmp_dev->ps_full_dev_path) + 1);
                    strncpy(ps_path, pt_tmp_dev->ps_full_dev_path, strlen(pt_tmp_dev->ps_full_dev_path));
                    found = TRUE;
                    DM_INFO("[DM_TEST] ps_path = %s\n", ps_path);
                    break;
                }
            }
        }

        SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
        {
            DM_INFO("[DMH]%10s(%d): found = %d\n",__FUNCTION__,__LINE__,found);
            if (found == FALSE)
                break;
            DM_INFO("[DM_TEST] pt_tmp_dev->ps_full_dev_path = %s\n", pt_tmp_dev->ps_full_dev_path);

             if(ps_path==NULL)
             {
            //   dmh_printf("[DM_TEST] ps_path is null\n");
               break;
             }
            if ((pt_tmp_dev->t_device_type == DMH_DEV_SKT_TYPE_FAKEHUB) &&
                (strstr(ps_path, pt_tmp_dev->ps_full_dev_path) != NULL))
            {
                if (pt_tmp_dev->ps_model != NULL)
                {
                    snprintf(ps_model_str,DM_DEV_MAX_MODEL_NAME_LEN,"%s %s",pt_tmp_dev->ps_model,pt_tmp_dev->ps_rev);
                }
                if (ps_path != NULL)
                {
                    free(ps_path);
                    ps_path = NULL;
                }

                return DMH_OK;
            }
        }
    }

    SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
    {
        if ((pt_tmp_dev->t_uevent_attr.ui4_major == ui4_major) &&
            (pt_tmp_dev->t_uevent_attr.ui4_minor== ui4_minor))
        {
            if (pt_tmp_dev->ps_model != NULL)
            {
                snprintf(ps_model_str,DM_DEV_MAX_MODEL_NAME_LEN,"%s %s",pt_tmp_dev->ps_model,pt_tmp_dev->ps_rev);

            }

            if (ps_path != NULL)
            {
                free(ps_path);
                ps_path = NULL;
            }

            return DMH_OK;
        }
    }
    if (ps_path != NULL)
    {
        free(ps_path);
        ps_path = NULL;
    }
    return DMH_ERR;
}

INT32 dmh_blkdev_get_sector_ns(UINT32 ui4_major,UINT32 ui4_minor,UINT64 *pui8_blocks)
{
     struct _DMH_DEV_T * pt_tmp_dev;

     SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
     {
         if ((pt_tmp_dev->t_uevent_attr.ui4_major == ui4_major) &&
             (pt_tmp_dev->t_uevent_attr.ui4_minor== ui4_minor))
         {
             *pui8_blocks = pt_tmp_dev->ui8_size;
             return DMH_OK;
         }
     }
     return DMH_ERR;
}

INT32 dmh_blkdev_get_sector_size(UINT32 ui4_major,UINT32 ui4_minor,UINT32 *pui4_size)
{
    struct _DMH_DEV_T * pt_tmp_dev;
    int    i_sec_size = 0;
    SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
    {
        if ((pt_tmp_dev->t_uevent_attr.ui4_major == ui4_major) &&
            (pt_tmp_dev->t_uevent_attr.ui4_minor== ui4_minor))
        {
            if (DMH_DEV_MED_TYPE_USBSTORAGE == pt_tmp_dev->t_device_type ||
                DMH_DEV_MED_TYPE_USBPARTI  == pt_tmp_dev->t_device_type)
            {
                i_sec_size = (int)pt_tmp_dev->ui4_hw_sector_size;
                *pui4_size = (i_sec_size <= 0) ? 512 : pt_tmp_dev->ui4_hw_sector_size;
            }
            else
                *pui4_size = pt_tmp_dev->ui4_hw_sector_size;
            return DMH_OK;
        }
    }
    return DMH_OK;
}


INT32 dmh_blkdev_get_usb_device_path(UINT32 ui4_major,UINT32 ui4_minor,UINT8  *ps_path_str)
{
     struct _DMH_DEV_T * pt_tmp_dev;
     int i_path_cnt = 0;
     int i_cnt = 0;
     int i_start_cpy = 0;

     SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
     {
         if ((pt_tmp_dev->t_uevent_attr.ui4_major == ui4_major) &&
             (pt_tmp_dev->t_uevent_attr.ui4_minor== ui4_minor))
         {
            // dmh_printf("[DM_TEST] dmh_blkdev_get_usb_device_path pt_tmp_dev->ps_full_dev_path = %s\n", pt_tmp_dev->ps_full_dev_path);
             while((UINT32)i_path_cnt < strlen(pt_tmp_dev->ps_full_dev_path))
             {
                 if (pt_tmp_dev->ps_full_dev_path[i_path_cnt] == ':')
                 {
                     ps_path_str[i_cnt] = 0;
                     break;
                 }
                 if (i_start_cpy == 1)
                 {
                     ps_path_str[i_cnt] = pt_tmp_dev->ps_full_dev_path[i_path_cnt];
                     i_cnt ++;
                     i_path_cnt ++;
                     continue;
                 }
                 if (pt_tmp_dev->ps_full_dev_path[i_path_cnt] == 'M')
                 {
                     i_start_cpy = 1;
                     ps_path_str[i_cnt] = pt_tmp_dev->ps_full_dev_path[i_path_cnt];
                     i_cnt ++;
                     i_path_cnt ++;
                     continue;
                 }
                 i_path_cnt ++;
             }
            // dmh_printf("[DM_TEST] dmh_blkdev_get_usb_device_path ps_path_str = %s\n", ps_path_str);
             return DMH_OK;
         }
     }
     return DMH_ERR;
}

INT32 dmh_sktdev_get_model(UINT32 ui4_major,UINT32 ui4_minor, UINT8  *ps_model_str)
{
    return dmh_blkdev_get_model(ui4_major,ui4_minor,(CHAR *)ps_model_str);
}

INT32 dmh_sktdev_get_skt_ns(UINT32 ui4_major,UINT32 ui4_minor, UINT32 *pui4_slot_ns)
{
    struct _DMH_DEV_T * pt_tmp_dev;

    SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
    {
        if ((pt_tmp_dev->t_uevent_attr.ui4_major == ui4_major) &&
            (pt_tmp_dev->t_uevent_attr.ui4_minor== ui4_minor))
        {
            *pui4_slot_ns = pt_tmp_dev->ui1_slot_num;
            return DMH_OK;
        }
    }
    return DMH_ERR;
}

INT32 dmh_reg_root_nfy_fct(DMH_ROOT_DEV_T ui4_root_dev_id, DEV_DEV_ST_NFY_T *pt_nfy)
{
    struct _DMH_DEV_T *     pt_tmp_dmh_dev;
    struct _DMH_NFY_DESC_T* pt_nfy_desc;
    struct _DMH_CB_DESC_T*  pt_cb_desc;

    pt_nfy_desc = (DMH_NFY_DESC_T *)malloc(sizeof(DMH_NFY_DESC_T));
    if(NULL == pt_nfy_desc)
    {
        return DMH_NO_MEM;
    }
    memset(pt_nfy_desc,0,sizeof(DMH_NFY_DESC_T));

    DM_INFO("[DMH]%10s(%d): ui4_root_dev_id:%d\n",__FUNCTION__,__LINE__, ui4_root_dev_id);

    if (RWLR_OK != u_rwl_write_lock(t_dmh_dev->h_nfy_rwlock, RWL_OPTION_WAIT))
    {
         free(pt_nfy_desc);
        return DMH_OS;
    }

    pt_nfy_desc->ui4_root_dev_id = ui4_root_dev_id;
    pt_nfy_desc->pf_nfy = pt_nfy->pf_cb_nfy_fct;
    pt_nfy_desc->pv_tag = pt_nfy->pv_nfy_tag;
    SLIST_INSERT_HEAD(pt_nfy_desc,&(t_dmh_dev->t_dev_nfy_list),t_link);

    u_rwl_release_lock(t_dmh_dev->h_nfy_rwlock);

    if((ui4_root_dev_id == DMH_ROOT_DEV_USB1) ||(ui4_root_dev_id == DMH_ROOT_DEV_USB2) ||
       (ui4_root_dev_id == DMH_ROOT_DEV_USB3) ||(ui4_root_dev_id == DMH_ROOT_DEV_ATA1) ||
       (ui4_root_dev_id == DMH_ROOT_DEV_ATA2) ||(ui4_root_dev_id == DMH_ROOT_DEV_ATA3) ||
       (ui4_root_dev_id == DMH_ROOT_DEV_NAND1)||(ui4_root_dev_id == DMH_ROOT_DEV_MMC))
    {
        if (DMH_ROOT_DEV_MMC == ui4_root_dev_id)
        {
            u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);
            return DMH_OK;
        }

        if (RWLR_OK != u_rwl_read_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
        {
            return DMH_OS;
        }

        SLIST_FOR_EACH(pt_tmp_dmh_dev, &(t_dmh_dev->t_dev_list), t_link)
        {
            if ((pt_tmp_dmh_dev->ui4_root_dev_id == ui4_root_dev_id) &&
                (pt_tmp_dmh_dev->ui1_dev_level == 2))
            {
                pt_cb_desc = malloc(sizeof(struct _DMH_CB_DESC_T));
                if(NULL == pt_cb_desc)
                {
                    return DMH_NO_MEM;
                }
                memset(pt_cb_desc,0,sizeof(struct _DMH_CB_DESC_T));

                /*need to call the callback function to notify DM */
                if(DMH_ROOT_DEV_NAND1 == ui4_root_dev_id || DMH_ROOT_DEV_MMC == ui4_root_dev_id)
                {
                    pt_cb_desc->e_status = DEV_STATUS_MED_ATTACH;
                    strncpy(pt_cb_desc->ps_dev_name,(&pt_tmp_dmh_dev->ps_dev_node[5]),sizeof(pt_cb_desc->ps_dev_name));

                }
                else if(DMH_DEV_UKW_TYPE_UNKNOWN == pt_tmp_dmh_dev->t_device_type)
                {
                    pt_cb_desc->e_status = DEV_STATUS_DEV_UNSUPPORT;
                }
                else
                {
                    pt_cb_desc->e_status = DEV_STATUS_HUB_ATTACH;
                }
                pt_cb_desc->ui4_skt_no   = pt_tmp_dmh_dev->ui1_slot_num;
                pt_cb_desc->t_dev_type   = pt_tmp_dmh_dev->t_device_type;
                pt_cb_desc->ui4_major    = pt_tmp_dmh_dev->t_uevent_attr.ui4_major;
                pt_cb_desc->ui4_minor    = pt_tmp_dmh_dev->t_uevent_attr.ui4_minor;
                pt_cb_desc->ui4_dev_flag = pt_tmp_dmh_dev->ui4_dev_flag;

                DM_INFO("%10s %d :\n",__FUNCTION__,__LINE__);
                pt_nfy_desc->pf_nfy(pt_nfy_desc->pv_tag,pt_cb_desc->e_status,pt_cb_desc);

                free(pt_cb_desc);
            }
        }

        u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

        return DMH_OK;
    }

    if(DMH_ROOT_DEV_UNSUPPORT == ui4_root_dev_id)
    {
        return DMH_ERR;
    }

    return DMH_INVALID_PARM;
}


INT32 dmh_reg_nfy_fct(UINT32 ui4_major,UINT32 ui4_minor, DEV_DEV_ST_NFY_T *pt_nfy)
{
    struct _DMH_DEV_T      *pt_tmp_dev;
    struct _DMH_CB_DESC_T  *pt_cb_desc;
    struct _DMH_NFY_DESC_T *pt_nfy_desc;

    DM_INFO("[DMH]%10s %d ui4_major:%ld ui1_minor:%ld\n",__FUNCTION__,__LINE__,ui4_major,ui4_minor);

    pt_nfy_desc = (DMH_NFY_DESC_T *)malloc(sizeof(DMH_NFY_DESC_T));
    if(pt_nfy_desc == NULL)
    {
        return DMH_NO_MEM;
    }
    memset(pt_nfy_desc,0,sizeof(DMH_NFY_DESC_T));

    if (u_rwl_write_lock(t_dmh_dev->h_nfy_rwlock, RWL_OPTION_WAIT) != RWLR_OK)
    {
        free(pt_nfy_desc);
        return DMH_OS;
    }

    pt_nfy_desc->ui4_major = ui4_major;
    pt_nfy_desc->ui4_minor = ui4_minor;
    pt_nfy_desc->pf_nfy = pt_nfy->pf_cb_nfy_fct;
    pt_nfy_desc->pv_tag = pt_nfy->pv_nfy_tag;
    SLIST_INSERT_HEAD(pt_nfy_desc,&(t_dmh_dev->t_dev_nfy_list),t_link);

    u_rwl_release_lock(t_dmh_dev->h_nfy_rwlock);

    if (RWLR_OK != u_rwl_read_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
    {
        return DMH_OS;
    }

    if(0x555 == ui4_major)//if this  is virtual device ,don't deal with this case.
    {
         DM_INFO("[DMH]%10s(%d):visual device ,not deal with!\n",__FUNCTION__,__LINE__);
         u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);
         return DMH_OK;
    }

    SLIST_FOR_EACH(pt_tmp_dev, &(t_dmh_dev->t_dev_list), t_link)
    {
        if ((pt_tmp_dev->t_uevent_attr.ui4_major == ui4_major) && (pt_tmp_dev->t_uevent_attr.ui4_minor== ui4_minor))
        {
            if(DMH_DEV_MED_TYPE_ATA_BD == pt_tmp_dev->t_device_type)
            {
                continue;
            }

            struct _DMH_DEV_T * pt_next_usb_dev;
            SLIST_FOR_EACH(pt_next_usb_dev, &(t_dmh_dev->t_dev_list), t_link)
            {
                if( (pt_next_usb_dev->ui1_dev_level == pt_tmp_dev->ui1_dev_level+1) &&
                    (strstr(pt_next_usb_dev->ps_full_dev_path,pt_tmp_dev->ps_full_dev_path)!=NULL) )
                {
                    DM_INFO("%10s linux %d t_device_type %d:\n",__FUNCTION__,__LINE__,pt_next_usb_dev->t_device_type);
                    DM_INFO("%10s linux %d t_device_type %lld:\n",__FUNCTION__,__LINE__,pt_next_usb_dev->ui8_size);

                    if((DMH_DEV_MED_TYPE_USBSTORAGE == pt_next_usb_dev->t_device_type) && (0 == pt_next_usb_dev->ui8_size))
                    {
                        DM_INFO("%10s linux %d t_device_type %d:\n",__FUNCTION__,__LINE__,pt_next_usb_dev->t_device_type);
                        break;
                    }

                    if((DMH_DEV_MED_TYPE_USBSTORAGE == pt_next_usb_dev->t_device_type)&&
                       (DMH_DEV_SKT_TYPE_FAKEHUB == pt_tmp_dev->t_device_type)&&
                       (TRUE == pt_next_usb_dev->b_send))
                    {
                        DM_INFO("%s(%d)fakehub startup too later!:\n",__FUNCTION__,__LINE__);
                        break;
                    }

                    if((DMH_DEV_MED_TYPE_ATA_BD == pt_next_usb_dev->t_device_type) &&
                       (DMH_DEV_SKT_TYPE_OPTI_DRV == pt_tmp_dev->t_device_type))
                    {
                        DM_INFO("[DMH]ignore the BD notify\n");
                        break;
                    }

                    if((DMH_DEV_MED_TYPE_USBSTORAGE == pt_next_usb_dev->t_device_type )&&
                       (DMH_DEV_SKT_TYPE_CIFS == pt_tmp_dev->t_device_type))
                    {
                        DM_INFO("[DMH]cifs issue\n");
                        break;
                    }
                    if((DMH_DEV_MED_TYPE_USBSTORAGE == pt_next_usb_dev->t_device_type)&&
                       (DMH_DEV_SKT_TYPE_USB_SOUND_CARD == pt_tmp_dev->t_device_type))
                    {
                        DM_INFO("[DMH]USB sound card issue\n");
                        break;
                    }

                    if (DMH_DEV_SKT_TYPE_OPTI_DRV == pt_next_usb_dev->t_device_type)
                    {
                        DM_INFO("%10s linux %d device typer :DMH_DEV_SKT_TYPE_OPTI_DRV IGNORE!\n",__FUNCTION__,__LINE__);
                        break;
                    }

                    if((DMH_DEV_MED_TYPE_USBPARTI == pt_next_usb_dev->t_device_type) && (pt_next_usb_dev->ui8_size <=10))
                    {
                        DM_INFO("%10s linux %d t_device_type %d: pt_next_usb_dev->ui8_size: %lld\n",__FUNCTION__,__LINE__,pt_next_usb_dev->t_device_type, pt_next_usb_dev->ui8_size);
                        continue;
                    }

                    pt_cb_desc = malloc(sizeof(struct _DMH_CB_DESC_T));
                    if(pt_cb_desc == NULL)
                    {
                        return DMH_NO_MEM;
                    }
                    memset(pt_cb_desc,0,sizeof(struct _DMH_CB_DESC_T));

                    pt_cb_desc->ui4_skt_no   = pt_next_usb_dev->ui1_slot_num;
                    pt_cb_desc->t_dev_type   = pt_next_usb_dev->t_device_type;
                    pt_cb_desc->ui4_dev_flag = pt_next_usb_dev->ui4_dev_flag;

                    if( (DMH_DEV_SKT_TYPE_REALHUB == pt_next_usb_dev->t_device_type)||
                        (DMH_DEV_SKT_TYPE_FAKEHUB == pt_next_usb_dev->t_device_type))
                    {
                        pt_cb_desc->e_status = DEV_STATUS_HUB_ATTACH;
                    }
                    else if((pt_next_usb_dev->t_device_type == DMH_DEV_MED_TYPE_USBSTORAGE)||
                            (pt_next_usb_dev->t_device_type == DMH_DEV_MED_TYPE_ATA_HDD)   ||
                            (pt_next_usb_dev->t_device_type == DMH_DEV_SKT_TYPE_OPTI_DRV)  ||
                            (pt_next_usb_dev->t_device_type == DMH_DEV_MED_TYPE_ATA_BD)    ||
                            (pt_next_usb_dev->t_device_type == DMH_DEV_MED_TYPE_NAND)      ||
                            (pt_next_usb_dev->t_device_type == DMH_DEV_MED_TYPE_MMC_STORAGE))
                    {
                        pt_cb_desc->e_status = DEV_STATUS_MED_ATTACH;
                        strncpy(pt_cb_desc->ps_dev_name,(&pt_next_usb_dev->ps_dev_node[5]),sizeof(pt_cb_desc->ps_dev_name));
                    }
                    else if((DMH_DEV_MED_TYPE_USBPARTI == pt_next_usb_dev->t_device_type) ||
                            (DMH_DEV_MED_TYPE_ATA_HDD_PARTI == pt_next_usb_dev->t_device_type))
                    {
                        pt_cb_desc->e_status = DEV_STATUS_PART_DETECTED;
                        strncpy(pt_cb_desc->ps_dev_name,(&pt_next_usb_dev->ps_dev_node[5]),sizeof(pt_cb_desc->ps_dev_name));
                    }

                    pt_cb_desc->ui4_major  = pt_next_usb_dev->t_uevent_attr.ui4_major;
                    pt_cb_desc->ui4_minor  = pt_next_usb_dev->t_uevent_attr.ui4_minor;
                    pt_cb_desc->ui4_product= pt_next_usb_dev->ui4_product;
                    pt_cb_desc->ui4_vendor = pt_next_usb_dev->ui4_vendor;

                    DM_INFO("%10s linux %d t_device_type %d:\n",__FUNCTION__,__LINE__,pt_next_usb_dev->t_device_type);
                    pt_nfy_desc->pf_nfy(pt_nfy_desc->pv_tag,pt_cb_desc->e_status,pt_cb_desc);

                    if((pt_next_usb_dev->t_device_type == DMH_DEV_MED_TYPE_USBSTORAGE)&&
                       (pt_tmp_dev->t_device_type==DMH_DEV_SKT_TYPE_FAKEHUB)&&
                       (FALSE == pt_next_usb_dev->b_send))
                    {
                             pt_next_usb_dev->b_send = TRUE;
                             pt_next_usb_dev->pf_nptify = pt_nfy_desc;
                             DM_INFO("%s(%d)fakehub startup too later!:\n",__FUNCTION__,__LINE__);
                             DM_INFO("(%s)%d USB attach pf_nptify = %d!\n",__FUNCTION__,__LINE__,(int)(pt_next_usb_dev->pf_nptify));
                    }

                    free(pt_cb_desc); /* free pt_cb_desc */
                }
            }

            break;
        }
    }

    u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

    return DMH_OK;
}

INT32 dmh_init_netlink_services()
{
    INT32    i4_ret = 0;
    INT32    sock_fd = 0;
    HANDLE_T h_cb_uevent_thrd = 0;

    static BOOL fg_dmh_init_netlink_services = FALSE;

    if (fg_dmh_init_netlink_services)
    {
        return DMH_OK;
    }

    sock_fd = dmh_init_hotplug_sock();
    DM_INFO("dmh_init_hotplug_sock creat sock_fd=%ld\n",sock_fd);

    i4_ret = dmh_scan_path("/sys/devices/virtual/block");
    if(DMH_OK != i4_ret)
    {
        DM_ERR("[DMH]scan failed");
        close(sock_fd);
        return DMH_ERR;
    }

    i4_ret = dmh_scan_path("/sys/devices/platform");
    if(DMH_OK != i4_ret)
    {
        DM_ERR("[DMH]scan failed");
        close(sock_fd);
        return DMH_ERR;
    }

    i4_ret = u_thread_create(&h_cb_uevent_thrd,
                             DMH_CB_UEVENT_THREAD_NAME,
                             DMH_CB_UEVENT_DEFAULT_STACK_SIZE,
                             DMH_CB_UEVENT_DEFAULT_PRIORITY,
                             dmh_cb_msg_thread,
                             sizeof(INT32),
                             &sock_fd);
    if (DMH_OK != i4_ret)
    {
        DM_ERR("[DMH]dmh_cb_msg_thread  create failed");
        close(sock_fd);
        return DMH_OS;
    }

    _fgDmh_open_dev_thread = TRUE;
    _fgDmh_open_dev_thread_exited = FALSE;

    fg_dmh_init_netlink_services = TRUE;

    return i4_ret;
}


INT32 dmh_init(VOID)
{
    INT32 i4_ret = 0;
    DM_INFO("[DM] %s: #%d: \n", __FUNCTION__, __LINE__);

    t_dmh_dev = malloc(sizeof(DMH_DEV_T));
    if(NULL == t_dmh_dev)
    {
        return DMH_NO_MEM;
    }
    memset(t_dmh_dev,0,sizeof(DMH_DEV_T));

    SLIST_INIT(&(t_dmh_dev->t_dev_list));
    SLIST_INIT(&(t_dmh_dev->t_dev_nfy_list));

    i4_ret = u_rwl_create_lock(&(t_dmh_dev->h_nfy_rwlock));
    if (DMH_OK != i4_ret)
    {
        i4_ret = DMH_OS;

        if (NULL_HANDLE != t_dmh_dev->h_nfy_rwlock)
        {
            u_rwl_delete_lock(t_dmh_dev->h_nfy_rwlock);
        }
        return i4_ret;
    }

    i4_ret = u_rwl_create_lock(&t_dmh_dev->h_dev_rwlock);
    if (DMH_OK != i4_ret)
    {
        i4_ret = DMH_OS;
        if (NULL_HANDLE != t_dmh_dev->h_dev_rwlock)
        {
            u_rwl_delete_lock(t_dmh_dev->h_dev_rwlock);
        }
        return i4_ret;
    }

    DM_INFO("Listing raw device(s)\n");

    i4_ret = dmh_init_netlink_services();

    DM_INFO("[DMH] DMH init Success  \n");

    return i4_ret;

}


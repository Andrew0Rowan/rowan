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


#ifndef _DMH_H_
#define _DMH_H_

#include"u_common.h"
#include"u_dm.h"
#include"u_lnk_list.h"

#define DMH_CB_UEVENT_THREAD_NAME         "dmh_cb_msg_thrd"
#define DMH_CB_UEVENT_DEFAULT_PRIORITY    ((UINT8)  (100+20+0))
#define DMH_CB_UEVENT_DEFAULT_STACK_SIZE  ((SIZE_T) 16*4096)

#define DMH_NOTSUPPORT_DEVNODE_RDONLY 0

#define DMH_OK                          ((INT32)  0)
#define DMH_ERR                         ((INT32)  -1)
#define DMH_INVALID_PARM                ((INT32)  -2)
#define DMH_INVALID_DEV_TYPE            ((INT32)  -3)
#define DMH_INVALID_DEV_CHAR            ((INT32)  -4)
#define DMH_INVALID_HANDLE              ((INT32)  -5)
#define DMH_NO_MEM                      ((INT32)  -6)
#define DMH_WRONG_DEV_STATUS            ((INT32)  -7)
#define DMH_INTERNAL                    ((INT32)  -8)
#define DMH_ALREADY_INIT                ((INT32)  -9)
#define DMH_NOT_IMPLEMETED              ((INT32)  -10)
#define DMH_HANDLE                      ((INT32)  -11)
#define DMH_OS                          ((INT32)  -12)
#define DMH_DEVICE_ERR                  ((INT32)  -13)
#define DMH_FM_ERR                      ((INT32)  -14)
#define DMH_RM_ERR                      ((INT32)  -15)
#define DMH_HW_INIT_ERR                 ((INT32)  -16)
#define DMH_CLI_ERR                     ((INT32)  -17)

#define DMH_FLAG_BLOCK_DEV              ((UINT32)1 << 0)
#define DMH_FLAG_CHAR_DEV               ((UINT32)1 << 1)
#define DMH_FLAG_SOCKET_DEV             ((UINT32)1 << 2)

typedef enum
{
    DEV_STATUS_UNKNOWN,
    DEV_STATUS_HUB_ATTACH,
    DEV_STATUS_HUB_DETACH,
    DEV_STATUS_MED_ATTACH,
    DEV_STATUS_MED_DETACH,
    DEV_STATUS_PART_DETECTED,
    DEV_STATUS_PART_REMOVED,
    DEV_STATUS_DEV_FAIL,
    DEV_STATUS_DEV_UNSUPPORT
} DEV_STATUS_T;

typedef VOID (* x_dev_st_nfy_fct)(VOID *pv_nfy_tag, DEV_STATUS_T e_nfy_st,VOID *pv_data);

typedef enum _DMH_DEV_TYPE_T
{
    DMH_DEV_UKW_TYPE_UNKNOWN     = 0,
    DMH_DEV_SKT_TYPE_ROOTHUB     ,
    DMH_DEV_SKT_TYPE_REALHUB,
    DMH_DEV_SKT_TYPE_FAKEHUB,
    DMH_DEV_SKT_TYPE_OPTI_DRV,
    DMH_DEV_SKT_TYPE_USB_KB,
    DMH_DEV_SKT_TYPE_USB_MOUSE,
    DMH_DEV_SKT_TYPE_IPOD,
    DMH_DEV_SKT_TYPE_AOA,
    DMH_DEV_SKT_TYPE_CIFS,
    DMH_DEV_SKT_TYPE_WIFI_RT3370,
    DMH_DEV_SKT_TYPE_BLUETOOTH,
    DMH_DEV_SKT_TYPE_INVISIBLE_IPOD,
    DMH_DEV_SKT_TYPE_CAMERA,
    DMH_DEV_SKT_TYPE_MICROPHONE,
    DMH_DEV_SKT_TYPE_PL2303,
    DMH_DEV_SKT_TYPE_USB_LOADER,
    DMH_DEV_SKT_TYPE_USB_SOUND_CARD,
    DMH_DEV_SKT_TYPE_ATHMAG,              //FOXCONN  MAGPIE
    DMH_DEV_SKT_TYPE_MARVELL,
    DMH_DEV_SKT_TYPE_MAGPIE,              //SONY MAGPIE
    DMH_DEV_SKT_TYPE_RT3370,
    DMH_DEV_SKT_TYPE_RT5370,
    DMH_DEV_SKT_TYPE_RTL8150,
    DMH_DEV_SKT_TYPE_NFS,
    DMH_DEV_SKT_TYPE_RTSTA,
    DMH_DEV_SKT_TYPE_7650,
    DMH_DEV_SKT_TYPE_7632,
    DMH_DEV_SKT_TYPE_ATHEROS_AR9271,
    DMH_DEV_SKT_TYPE_SIO,
    DMH_DEV_SKT_TYPE_UNSUPPORT_WIFI,
    DMH_DEV_MED_TYPE_USBSTORAGE,
    DMH_DEV_MED_TYPE_USBPARTI,
    DMH_DEV_MED_TYPE_ATA_HDD,
    DMH_DEV_MED_TYPE_ATA_HDD_PARTI,
    DMH_DEV_MED_TYPE_ATA_BD,
    DMH_DEV_MED_TYPE_NAND,
    DMH_DEV_MED_TYPE_PTP_MTP,
    DMH_DEV_MED_TYPE_USB_EP,
    DMH_DEV_MED_TYPE_MMC_STORAGE,
    DMH_DEV_MED_TYPE_MMC_PARTI,
    DMH_DEV_MED_TYPE_IPOD,
    DMH_DEV_MED_TYPE_ISO,
    DMH_DEV_MED_TYPE_ATHMAG
} DMH_DEV_TYPE_T;

typedef struct _BLK_UEVENT_ATTR_T
{
    UINT32              ui4_major;
    UINT32              ui4_minor;
    CHAR *              ps_dev_type;
    CHAR *              ps_phys_dev_path;
    CHAR *              ps_phys_dev_driver;
    CHAR *              ps_driver;
    CHAR *              ps_product;
    CHAR *              ps_interface;
}BLK_UEVENT_ATTR_T;

typedef enum _DMH_ROOT_DEV_T
{
    DMH_ROOT_DEV_ATA1     = 1,
    DMH_ROOT_DEV_ATA2,
    DMH_ROOT_DEV_ATA3,
    DMH_ROOT_DEV_USB1,
    DMH_ROOT_DEV_USB2,
    DMH_ROOT_DEV_USB3,
    DMH_ROOT_DEV_NAND1,
    DMH_ROOT_DEV_MMC,
    DMH_ROOT_DEV_UNSUPPORT
} DMH_ROOT_DEV_T;

typedef struct _DMH_CB_DESC_T
{
    DMH_DEV_TYPE_T              t_dev_type;
    DEV_STATUS_T                e_status;
    UINT32                      ui4_skt_no;
    UINT32                      ui4_major;
    UINT32                      ui4_minor;
    UINT32                      ui4_dev_flag;
    CHAR                        ps_dev_name[DM_MAX_LABEL_NAME_LEN];
    VOID                        *pv_resv;
    UINT32                      ui4_product;
    UINT32                      ui4_vendor;
} DMH_CB_DESC_T;

typedef struct _DEV_DEV_ST_NFY_T
{
    VOID                *       pv_nfy_tag;
    x_dev_st_nfy_fct            pf_cb_nfy_fct;
} DEV_DEV_ST_NFY_T;


typedef struct _DMH_NFY_DESC_T
{
    UINT32                          ui4_major;
    UINT32                          ui4_minor;
    DMH_ROOT_DEV_T                  ui4_root_dev_id;
    VOID *                          pv_tag;
    x_dev_st_nfy_fct                pf_nfy;
    SLIST_ENTRY_T(_DMH_NFY_DESC_T)  t_link;
} DMH_NFY_DESC_T;

typedef struct _DMH_DEV_T
{
    CHAR *                      ps_manufacturer;
    CHAR *                      ps_product;
    CHAR *                      ps_version;
    CHAR *                      ps_vendor;
    CHAR *                      ps_model;
    CHAR *                      ps_state;
    CHAR *                      ps_stat;
    CHAR *                      ps_rev;
    CHAR *                      ps_capability;
    CHAR *                      ps_dev_node;
    CHAR *                      ps_block_name;
    CHAR *                      ps_full_dev_path;

    UINT8                       ui1_slot_num;

    UINT8                       ui1_ro;
    UINT8                       ui1_removable;
    UINT8                       ui1_dev_level;

    UINT32                      ui4_hw_sector_size;
    UINT32                      ui4_range;
    UINT32                      ui4_scsi_level;
    UINT32                      ui4_dev_num;
    UINT32                      ui4_max_child_num;
    UINT32                      ui4_product_id;
    UINT32                      ui4_vendor_id;
    UINT32                      ui4_speed_num;
    UINT32                      ui4_dev_flag;
    UINT32                      ui4_verify_count;
    BOOL                        b_send;
    VOID *                      pf_nptify;

    BOOL                        b_remove_card;
    BOOL                        b_support_get_partial_obj;
    BOOL                        b_in_upgrade;

    UINT32                      ui4_start;
    UINT64                      ui8_size;
    UINT64                      ui8_freesize;

    DMH_DEV_TYPE_T              t_device_type;
    BLK_UEVENT_ATTR_T           t_uevent_attr;
    DMH_ROOT_DEV_T              ui4_root_dev_id;


    HANDLE_T                    h_nfy_rwlock;
    HANDLE_T                    h_dev_rwlock;


    SLIST_T(_DMH_NFY_DESC_T)    t_dev_nfy_list;
    SLIST_T(_DMH_DEV_T)         t_dev_list;
    SLIST_ENTRY_T(_DMH_DEV_T)   t_link;
    UINT32                      ui4_product;
    UINT32                      ui4_vendor;
}DMH_DEV_T;

extern INT32 dmh_blkdev_get_size(UINT32 ui4_major,UINT32 ui4_minor,UINT64 *pui8_size);
extern INT32 dmh_blkdev_get_freesize(UINT32 ui4_major,UINT32 ui4_minor,UINT64 *pui8_freesize);
extern INT32 dmh_blkdev_get_model(UINT32 ui4_major,UINT32 ui4_minor,CHAR  *ps_model_str);
extern INT32 dmh_blkdev_get_sector_ns(UINT32 ui4_major,UINT32 ui4_minor,UINT64 *pui8_blocks);
extern INT32 dmh_blkdev_get_sector_size(UINT32 ui4_major,UINT32 ui4_minor,UINT32 *pui4_size);
extern INT32 dmh_blkdev_get_usb_device_path(UINT32 ui4_major,UINT32 ui4_minor,UINT8  *ps_path_str);
extern INT32 dmh_sktdev_get_model(UINT32 ui4_major,UINT32 ui4_minor, UINT8  *ps_model_str);
extern INT32 dmh_sktdev_get_skt_ns(UINT32 ui4_major,UINT32 ui4_minor, UINT32 *pui4_slot_ns);
extern INT32 dmh_reg_nfy_fct(UINT32 ui4_major,UINT32 ui4_minor, DEV_DEV_ST_NFY_T *pt_nfy);
extern INT32 dmh_init(VOID);

#endif


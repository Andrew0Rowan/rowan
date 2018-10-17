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
 * $RCSfile: u_dm.h,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 * $SWAuthor: $
 *
 * Description:
 *         This file contains all the transition effect interface APIs
 *---------------------------------------------------------------------------*/

#ifndef _U_DM_H_
#define _U_DM_H_
#include "u_common.h"
#include "u_handle.h"
#include "u_fm.h"

#define DM_DEV_SKT_MAX_SLOT_NS        6
#define DM_DEV_MED_DISC_MAX_LAYER_NS  5
#define DM_DEV_MAX_MODEL_NAME_LEN     64
#define DM_MAX_LABEL_NAME_LEN         32

/*Return values.*/
#define DMR_OK                    ((INT32)  0)
#define DMR_ERR                   ((INT32)  -1)
#define DMR_INVALID_PARM          ((INT32)  -2)
#define DMR_INVALID_DEV_TYPE      ((INT32)  -3)
#define DMR_INVALID_DEV_CHAR      ((INT32)  -4)
#define DMR_INVALID_HANDLE        ((INT32)  -5)
#define DMR_NO_MEM                ((INT32)  -6)
#define DMR_WRONG_DEV_STATUS      ((INT32)  -7)
#define DMR_INTERNAL              ((INT32)  -8)
#define DMR_ALREADY_INIT          ((INT32)  -9)
#define DMR_NOT_IMPLEMETED        ((INT32)  -10)
#define DMR_HANDLE                ((INT32)  -11)
#define DMR_OS                    ((INT32)  -12)
#define DMR_DEVICE_ERR            ((INT32)  -13)
#define DMR_FM_ERR                ((INT32)  -14)
#define DMR_RM_ERR                ((INT32)  -15)
#define DMR_HW_INIT_ERR           ((INT32)  -16)
#define DMR_CLI_ERR               ((INT32)  -17)
#define DMR_IOC_FAIL              ((INT32)  -32)
#define DMR_IOC_ERR_DEV_BUSY      ((INT32)  -33)
#define DMR_IOC_ERR_ILL_PARM      ((INT32)  -34)
#define DMR_IOC_ERR_CMD_TIMEOUT   ((INT32)  -35)
#define DMR_IOC_ERR_CMD_ABORT     ((INT32)  -36)
#define DMR_IOC_ERR_NOT_SUPPORT   ((INT32)  -37)
#define DMR_IOC_ERR_PROPRIETARY   ((INT32)  -38)
#define DMR_IOC_ERR_BLANK_SECTOR  ((INT32)  -39)

#define DM_CB_CAT(x)  (x & 0xFF00)
#define DM_DEV_CHR(x) (x & 0xFF00)
#define DM_EVT_CAT(x) (x & 0xFF00)

#if 0//(CONFIG_MW_GOOGLE_CAST_AUDIO)
#define DM_DEV_ROOT_DEV_LABEL         "/dev/block"
#else
#define DM_DEV_ROOT_DEV_LABEL         "/dev"
#endif

/*Supported device event types.*/
typedef enum _DM_EVENT_E
{
    DM_DEV_EVT_UNMOUNTED = 0,
    DM_DEV_EVT_MOUNTED,

    DM_DEV_EVT_DRV_MAX
} DM_EVENT_E;

/*network event types.*/
typedef enum _DM_NETWORK_EVENT_E
{
    DM_NETWORK_DISCONNECT = 0,
    DM_NETWORK_CONNECT,

    DM_NETWORK_MAX
} DM_NETWORK_EVENT_E;

/*Device characteristic (socket device or medium device)*/
typedef enum _DM_DEV_CHR_T
{
    DM_DEV_CHR_UNKNOWN  = 0x0,
    DM_DEV_CHR_SOCKET   = 0x100,
    DM_DEV_CHR_MEDIUM   = 0x200
} DM_DEV_CHR_T;

/*
 *  Unsupport device type.
 */
typedef enum _DM_DEV_UNS_TYPE_T
{
    DM_DEV_UNS_TYPE_DEV = 0,
    DM_DEV_UNS_TYPE_HUB = 1
} DM_DEV_UNS_TYPE_T;


/*
 *  Supported device types.
 */
typedef enum _DM_DEV_TYPE_T
{
    DM_DEV_UKW_TYPE_UNKNOWN     = DM_DEV_CHR_UNKNOWN,

    DM_DEV_SKT_TYPE_UNKNOWN     = DM_DEV_CHR_SOCKET,
    DM_DEV_SKT_TYPE_EEPROM_HW,
    DM_DEV_SKT_TYPE_NAND_HW,
    DM_DEV_SKT_TYPE_NOR_HW,
    DM_DEV_SKT_TYPE_USB_HW,
    DM_DEV_SKT_TYPE_IDE_HW,
    DM_DEV_SKT_TYPE_FCI_HW,
    DM_DEV_SKT_TYPE_1394_HW,
    DM_DEV_SKT_TYPE_HUB,
    DM_DEV_SKT_TYPE_CARD_READER,
    DM_DEV_SKT_TYPE_OPTI_DRV,
    DM_DEV_SKT_TYPE_HID,
    DM_DEV_SKT_TYPE_IPOD,
    DM_DEV_SKT_TYPE_AOA,
    DM_DEV_SKT_TYPE_USB_SOUND_CARD,
    DM_DEV_SKT_TYPE_INVISIBLE_IPOD,
    DM_DEV_SKT_TYPE_USBKB,
    DM_DEV_SKT_TYPE_USBMOUSE,
    DM_DEV_SKT_TYPE_USBPS,
    DM_DEV_SKT_TYPE_WIFI_RT3370,
    DM_DEV_SKT_TYPE_BLUETOOTH,
    DM_DEV_SKT_TYPE_ATHMAG,
    DM_DEV_SKT_TYPE_MARVELL,
    DM_DEV_SKT_TYPE_MAGPIE,
    DM_DEV_SKT_TYPE_RT3370,
    DM_DEV_SKT_TYPE_RT5370,
    DM_DEV_SKT_TYPE_RTL8150,     //277
    DM_DEV_SKT_TYPE_RTSTA,
    DM_DEV_SKT_TYPE_7650,
    DM_DEV_SKT_TYPE_7632,
    DM_DEV_SKT_TYPE_SIO,
    DM_DEV_SKT_TYPE_UNSUPPORT_WIFI,
    DM_DEV_SKT_TYPE_CAMERA,
    DM_DEV_SKT_TYPE_MICROPHONE,
    DM_DEV_SKT_TYPE_PL2303,
    DM_DEV_SKT_TYPE_USB_LOADER,
    DM_DEV_SKT_TYPE_ATHEROS_AR9271,
    DM_DEV_MED_TYPE_UNKNOWN     = DM_DEV_CHR_MEDIUM,
    DM_DEV_MED_TYPE_EEPROM,
    DM_DEV_MED_TYPE_NAND,
    DM_DEV_MED_TYPE_NOR,
    DM_DEV_MED_TYPE_HDD,
    DM_DEV_MED_TYPE_HDD_PARTITION,
    DM_DEV_MED_TYPE_DISC,
    DM_DEV_MED_TYPE_ISO,
    DM_DEV_MED_TYPE_MEM_CARD,
    DM_DEV_MED_TYPE_MEM_CARD_PARTI,
    DM_DEV_MED_TYPE_MASS_STRG,
    DM_DEV_MED_TYPE_MASS_STRG_PARTITION,
    DM_DEV_MED_TYPE_PTP_MTP,
    DM_DEV_MED_TYPE_IPOD,
    DM_DEV_MED_TYPE_USB_EP,
    DM_DEV_SKT_TYPE_CIFS,
    DM_DEV_SKT_TYPE_NFS,
} DM_DEV_TYPE_T;

/* Supported types for root hardware.*/
typedef enum _DM_HW_TYPE_T
{
    DM_HW_TYPE_UNKNOWN  = 0,
    DM_HW_TYPE_EEPROM   = 1,
    DM_HW_TYPE_NAND     = 2,
    DM_HW_TYPE_NOR      = 3,
    DM_HW_TYPE_USB      = 4,
    DM_HW_TYPE_IDE      = 5,
    DM_HW_TYPE_FCI      = 6,
    DM_HW_TYPE_1394     = 7,
    DM_HW_TYPE_MMC      = 8,
    DM_HW_TYPE_UART     = 9,
} DM_HW_TYPE_T;

/*
 *  Medium device trasfer type (block device or character device).
 */
typedef enum _DM_DEV_MED_TX_TYPE_T
{
    DM_DEV_MED_TX_TYPE_BLKDEV = 0,
    DM_DEV_MED_TX_TYPE_CHRDEV = 1
} DM_DEV_MED_TX_TYPE_T;

/*
 *  Supported optical disc types.
 */
typedef enum _DM_DEV_MED_DISC_TYPE_T
{
    DM_DEV_MED_DISC_TYPE_UNKNOWN = 0,
    DM_DEV_MED_DISC_TYPE_CD_ROM  = 1,
    DM_DEV_MED_DISC_TYPE_CD_R    = 2,
    DM_DEV_MED_DISC_TYPE_CD_RW   = 3,
    DM_DEV_MED_DISC_TYPE_DVD_ROM = 4,
    DM_DEV_MED_DISC_TYPE_DVD_R   = 5,
    DM_DEV_MED_DISC_TYPE_DVD_RW  = 6,
    DM_DEV_MED_DISC_TYPE_DVD_PR  = 7,
    DM_DEV_MED_DISC_TYPE_DVD_PRW = 8,
    DM_DEV_MED_DISC_TYPE_DVD_RAM = 9,
    DM_DEV_MED_DISC_TYPE_BD_ROM  = 10,
    DM_DEV_MED_DISC_TYPE_BD_RE   = 11,
    DM_DEV_MED_DISC_TYPE_BD_RW   = 12,
    DM_DEV_MED_DISC_TYPE_NODISC  = 13
} DM_DEV_MED_DISC_TYPE_T;

/*
 *  Optical disc status.
 */
typedef enum _DM_DEV_MED_DISC_STATUS_T
{
    DM_DEV_MED_DISC_STATUS_INVALID     = 0,
    DM_DEV_MED_DISC_STATUS_RDONLY      = 1,
    DM_DEV_MED_DISC_STATUS_BLANK       = 2,
    DM_DEV_MED_DISC_STATUS_FORMATTED   = 3,
    DM_DEV_MED_DISC_STATUS_APPENDABLE  = 4
} DM_DEV_MED_DISC_STATUS_T;

/*
 *  Supported memory card types.
 */
typedef enum _DM_DEV_MED_CARD_TYPE_T
{
    DM_DEV_MED_CARD_TYPE_UNKNOWN  = 0,
    DM_DEV_MED_CARD_TYPE_CF       = 1,
    DM_DEV_MED_CARD_TYPE_SD       = 2,
    DM_DEV_MED_CARD_TYPE_MMC      = 3,
    DM_DEV_MED_CARD_TYPE_MS       = 4,
    DM_DEV_MED_CARD_TYPE_MS_PRO   = 5,
    DM_DEV_MED_CARD_TYPE_SM       = 6,
    DM_DEV_MED_CARD_TYPE_XD       = 7,
    DM_DEV_MED_CARD_TYPE_USB      = 100
} DM_DEV_MED_CARD_TYPE_T;

typedef struct _DM_COND_T
{
    /* If the t_dev_type is specified as DM_HW_TYPE_UNKNOWN, allhardware type match the condition. */
    DM_HW_TYPE_T  t_hw_type;
    /*If the t_dev_type is specified as DM_DEV_UKW_TYPE_UNKNOWN, all device
       *  match the condition.  If the value is DM_DEV_SKT_TYPE_UNKNOWN, all socket
       *  device match the condition.  If the value is DM_DEV_MED_TYPE_UNKNWON,
       *  all medium device match the condition.*/
    DM_DEV_TYPE_T t_dev_type;
} DM_COND_T;

typedef struct _DM_INIT_PARM_T
{
    DM_COND_T  *pt_automnt_cond;

    UINT32     ui4_automnt_cond_ns;
} DM_INIT_PARM_T;

/*
 *  Type for simplified device information
 */
typedef struct _DM_DEV_STAT_T
{
    /*
     *  no such device if the value is DM_HW_TYPE_UNKNOWN
     */

    DM_HW_TYPE_T  t_hw_type;

    /*
     *  Un-supported device if the value is DM_DEV_UKW_TYPE_UNKNOWN
     */

    DM_DEV_TYPE_T t_dev_type;

    UINT32  ui4_unit_id;

    /*
     *  Attached socket number on the parent device
     */

    UINT32  ui4_skt_no;

    /*
     *  FALSE if the device is unsupported or failed
     */

    BOOL  b_avail;

    /*
     *  valid if the value is not DM_INVALID_HANDLE
     */

    HANDLE_T  h_dev;
} DM_DEV_STAT_T;


typedef struct _DM_DEV_UNS_INF_T
{
    DM_DEV_UNS_TYPE_T t_uns_type;
} DM_DEV_UNS_INF_T;

/*
 *  Slot information for socket device.
 */
typedef struct _DM_DEV_SLOT_INF_T
{
    UINT32 ui4_slot_no;
    DM_DEV_STAT_T t_dev_stat;
} DM_DEV_SLOT_INF_T;

/*
 *  Socket device information.
 */
typedef struct _DM_DEV_SKT_INF_T
{
    UINT32  ui4_total_slot_ns;
    UINT32  ui4_used_slot_ns;

    DM_DEV_SLOT_INF_T at_slot_inf[DM_DEV_SKT_MAX_SLOT_NS];
} DM_DEV_SKT_INF_T;

/*
 *  Optical disc layer information.
 */
typedef struct _DM_DEV_MED_DISC_LAYER_SIZE_T
{
    UINT32 ui4_layer_ns;
    UINT32 aui4_layer_sec_ns[DM_DEV_MED_DISC_MAX_LAYER_NS];
} DM_DEV_MED_DISC_LAYER_SIZE_T;

/*
 *  Optical disc proprietary information.
 */
typedef struct _DM_DEV_MED_DISC_INF_T
{
    DM_DEV_MED_DISC_TYPE_T        t_disc_type;
    DM_DEV_MED_DISC_STATUS_T      t_disc_status;
    DM_DEV_MED_DISC_LAYER_SIZE_T  t_disc_layer;
} DM_DEV_MED_DISC_INF_T;

/*
 *  Memory card proprietary information.
 */
typedef struct _DM_DEV_MED_CARD_INF_T
{
    DM_DEV_MED_CARD_TYPE_T t_card_type;
} DM_DEV_MED_CARD_INF_T;


/*
 *  Block device information.
 */
typedef struct _DM_DEV_BLK_INF_T
{
    UINT64 ui8_sec_ns;
    UINT32 ui4_sec_size;

    UINT32 ui4_max_tx_sec_ns;
    UINT32 ui4_best_tx_sec_ns;

    union
    {
        DM_DEV_MED_DISC_INF_T t_disc_inf;
        DM_DEV_MED_CARD_INF_T t_card_inf;
    } u;
} DM_DEV_BLK_INF_T;

/*
 *  Character device information.
 */
typedef struct _DM_DEV_CHR_INF_T
{
    UINT32 ui4_max_sync_read_sz;
    UINT32 ui4_max_sync_write_sz;
    UINT32 ui4_sec_ns;
} DM_DEV_CHR_INF_T;

/*
 *  Medium device information.
 */
typedef struct _DM_DEV_MED_INF_T
{
    DM_DEV_MED_TX_TYPE_T t_tx_type;

    UINT64 ui8_total_size;  /*  general for medium type */
    UINT64 ui8_free_size;   /*  general for medium type */

    union
    {
        DM_DEV_BLK_INF_T t_blk_inf;
        DM_DEV_CHR_INF_T t_chr_inf;
    } u;

    HANDLE_T  h_dev_root_lbl;  /*  "dev" entry in FM */

    CHAR ps_dev_name[DM_MAX_LABEL_NAME_LEN];
} DM_DEV_MED_INF_T;

/*
 *  Device information.
 */
typedef struct _DM_DEV_INF_T
{
    DM_HW_TYPE_T  t_hw_type;

    /*
     *  Un-supported device if the value is DM_DEV_UKW_TYPE_UNKNOWN
     */

    DM_DEV_TYPE_T t_dev_type;

    DM_DEV_STAT_T  t_root_stat;
    DM_DEV_STAT_T  t_prnt_stat;

    UINT32 ui4_unit_id;
    UINT32 ui4_skt_no;

    VOID *pv_data;
    UINT32 ui4_data_size;

    UINT32  ui4_max_io_request;
    UINT8   aui1_model_str[DM_DEV_MAX_MODEL_NAME_LEN];
    UINT8   aui1_device_path[DM_MAX_LABEL_NAME_LEN];
    UINT32  ui4_mem_align;
    UINT32  ui4_part_idx;
/* add the mtp pv_resv */
    VOID *pv_resv;

    /*
     *  FALSE if the device is unsupported or failed
     */

    BOOL  b_avail;

    union
    {
        DM_DEV_UNS_INF_T  t_uns_inf;
        DM_DEV_SKT_INF_T  t_skt_inf;
        DM_DEV_MED_INF_T  t_med_inf;
    } u;
} DM_DEV_INF_T;

typedef struct _DM_DEV_SPEC_T
{
    DM_HW_TYPE_T  t_hw_type;
    DM_DEV_TYPE_T t_dev_type;
    UINT32  ui4_unit_id;
} DM_DEV_SPEC_T;

typedef struct _DM_MNT_MSG_T
{
    DM_EVENT_E e_dm_event;
} DM_MNT_MSG_T;

typedef struct _DM_NETWORK_MSG_T
{
    DM_NETWORK_EVENT_E e_network_event;
} DM_NETWORK_MSG_T;

extern INT32 dm_multi_thread_init(VOID);
extern INT32 u_dm_get_mnt_info(FM_MNT_INFO_T *pt_mnt_info);
extern INT32 u_dm_get_device_info(HANDLE_T h_dev, DM_DEV_INF_T *pt_inf);
extern INT32 u_dm_get_mnt_info(FM_MNT_INFO_T *pt_mnt_info);

#endif


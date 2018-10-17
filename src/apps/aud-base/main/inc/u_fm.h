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


/*----------------------------------------------------------------------------*
 * $RCSfile: u_fm.h,v $
 * $Revision: #1 $
 * $Date: 2016/06/06 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/1 $
 * $SWAuthor: Yuchien Chen $
 * $MD5HEX: 748ca9ac1805fcf33d028a9d2b82d57a $
 *
 * Description:
 *         This header file contains File Manager exported constants, macros,
 *         and types.
 *---------------------------------------------------------------------------*/

#ifndef _U_FM_H_
#define _U_FM_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"
#include "u_handle.h"
#include "u_lnk_list.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/*Return values.*/
#define FMR_OK                  ((INT32)  0)
#define FMR_ARG                 ((INT32) -1)
#define FMR_HANDLE              ((INT32) -2)
#define FMR_INVAL               ((INT32) -3)
#define FMR_CORE                ((INT32) -4)
#define FMR_EXIST               ((INT32) -5)
#define FMR_NAME_TOO_LONG       ((INT32) -10)

#define FM_MODE_PERM_MASK       ((UINT32) 0777)

#if 0
#define FM_MAX_FILE_LEN         (256 - 1) /* Unicode not including trailing NULL */
#define FM_MAX_PATH_LEN         (1152 - 1)
#else
#define FM_MAX_FILE_LEN         (64) /* Unicode not including trailing NULL */
#define FM_MAX_PATH_LEN         (96)
#endif

#define FM_ROOT_HANDLE          NULL_HANDLE

/* Handle types supported by the File Manager. */
#define HT_GROUP_FM     ((HANDLE_TYPE_T) ( 5 * HT_GROUP_SIZE))  /* File Manager */
#define FMT_FILE_DESC   (HT_GROUP_FM + ((HANDLE_TYPE_T) 0))
#define FMT_DIR_LABEL   (HT_GROUP_FM + ((HANDLE_TYPE_T) 1))
#define FMT_ASYNC_READ  (HT_GROUP_FM + ((HANDLE_TYPE_T) 2))
#define FMT_ASYNC_WRITE (HT_GROUP_FM + ((HANDLE_TYPE_T) 3))

/*Asynchronous notification conditions.*/
typedef enum
{
    FM_ASYNC_COND_READ_PARTIAL_FAIL = -11,
    FM_ASYNC_COND_READ_FAIL = -10,
    FM_ASYNC_COND_BLANK_SECTOR  = -8,
    FM_ASYNC_COND_CMD_TIMEOUT   = -4,
    FM_ASYNC_COND_READ_TIMEOUT  = -3,   // to notify network http timeout
    FM_ASYNC_COND_ABORT_FAIL = -2,
    FM_ASYNC_COND_FAIL      = -1,
    FM_ASYNC_COND_READ_OK   =  1,
    FM_ASYNC_COND_WRITE_OK  =  2,
    FM_ASYNC_COND_ABORT_OK  =   3,
    FM_ASYNC_COND_STREAMING_EOF = 4,
    FM_ASYNC_COND_DTCP_UPDATE = 5,
    FM_ASYNC_COND_NOT_FOUND  = 6,
    FM_ASYNC_COND_SERVER_DISCONNECT = 7,
    FM_ASYNC_COND_BAD_REQUEST  = 8,
    FM_ASYNC_COND_READ_CONTINUE = 9,
    FM_ASYNC_COND_UNSUPPORT_URL = 10,
    FM_ASYNC_COND_STATUS_UNKNOW = 0xFF
} FM_ASYNC_COND_T;

typedef VOID (*fm_obj_free_fct)(VOID *pv_obj);

typedef enum
{
    FM_TYPE_XENTRY = 1,
    FM_TYPE_DIRLBL,
    FM_TYPE_SOCKET          /* reserved for further extension */
} FM_ENTRY_TYPE_T;

struct _FM_MTX_OBJ_T
{
    HANDLE_T        h_mtx;
    HANDLE_T        h_ref_handle;
    UINT16          ui2_lock_cnt;
    UINT16          ui2_padding;
    UINT32          ui4_status;
    fm_obj_free_fct pf_free;
    FM_ENTRY_TYPE_T e_type;
    DLIST_ENTRY_T(_FM_MTX_OBJ_T) t_obj_link;
};

typedef enum
{
    BLKDEV_COND_OK          =  0,
    BLKDEV_COND_READ_OK     =  1,
    BLKDEV_COND_WRITE_OK    =  2,
    BLKDEV_COND_ERASE_OK    =  3,
    BLKDEV_COND_FLUSH_OK    =  4,
    BLKDEV_COND_FAIL        = -1,
    BLKDEV_ERR_CMD_TIMEOUT  = -4,
    BLKDEV_OP_IN_PROGRESS   = -7,
    BLKDEV_ERR_BLANK_CHK    = -8
} BLKDEV_COND_T;

typedef VOID (*x_blkdev_nfy_fct)(VOID          *pv_nfy_tag,
                                   BLKDEV_COND_T e_nfy_cond,
                                   VOID        *pv_data);


typedef struct _BLKDEV_NFY_INFO_T
{
    VOID                        *pv_nfy_tag;
    x_blkdev_nfy_fct            pf_cb_nfy_fct;
} BLKDEV_NFY_INFO_T;

typedef enum
{
    CHRDEV_COND_OK          =  0,
    CHRDEV_COND_FAIL        = -1,
    CHRDEV_COND_READ_OK     =  1,
    CHRDEV_COND_WRITE_OK    =  2,
    CHRDEV_COND_ERASE_OK    =  3,
    CHRDEV_OP_IN_PROGRESS   = -7
} CHRDEV_COND_T;

typedef VOID (*x_chrdev_nfy_fct)(VOID          *pv_nfy_tag,
                                  CHRDEV_COND_T e_nfy_cond,
                                  VOID         *pv_data);

typedef struct _CHRDEV_NFY_INFO_T
{
    VOID                      *pv_nfy_tag;
    x_chrdev_nfy_fct          pf_cb_nfy_fct;
} CHRDEV_NFY_INFO_T;

typedef struct _MTPDEV_IO_DATA_T
{
    UINT32              ui4_object_handle;
    UINT32              ui4_storage_id;
    UINT32              ui4_offset;
    UINT32              ui4_length;
    VOID*               pv_data;
    UINT8*              pui1_data;
} MTPDEV_IO_DATA_T;

typedef struct _BLKDEV_IO_DATA_T
{
    UINT64            ui8_blk_num;
    UINT32            ui4_count;
    UINT32            ui4_trans_count;
    BOOL              b_ext_buf;
    VOID              *pv_data;
    UINT32            ui4_handle;         /* for abort function, default is 0x00 */
    BLKDEV_NFY_INFO_T t_nfy_info;         /* for individual notification */
} BLKDEV_IO_DATA_T;

typedef struct _BLKDEV_SECTOR_RANGE_INFO_T
{
    UINT32            ui4_start_sector;
    UINT32            ui4_number_of_sectors;
    UINT32            ui4_handle;         /* for abort function, default is 0x00 */
    BLKDEV_NFY_INFO_T t_nfy_info;         /* for individual notification */
} BLKDEV_SECTOR_RANGE_INFO_T;

typedef struct _CHRDEV_LONG_DATA_INFO_T
{
    UINT32            ui4_offset;
    UINT32            ui4_len;
    VOID              *pv_data;
    UINT32            ui4_handle;         /* for abort function, default is 0x00 */
    CHRDEV_NFY_INFO_T t_nfy_info;         /* for individual notification */
} CHRDEV_LONG_DATA_INFO_T;

typedef struct _CHRDEV_SHORT_DATA_INFO_T
{
    UINT32            ui4_offset;
    UINT8             ui1_data;
    UINT32            ui4_handle;         /* for abort function, default is 0x00 */
    CHRDEV_NFY_INFO_T t_nfy_info;         /* for individual notification */
} CHRDEV_SHORT_DATA_INFO_T;

typedef struct _CHRDEV_SECTOR_RANGE_INFO_T
{
    UINT32            ui4_start_sector;
    UINT32            ui4_number_of_sectors;
    UINT32            ui4_handle;         /* for abort function, default is 0x00 */
    CHRDEV_NFY_INFO_T t_nfy_info;         /* for individual notification */
} CHRDEV_SECTOR_RANGE_INFO_T;

typedef VOID (*x_fm_async_fct)(HANDLE_T    h_req,
                                VOID            *pv_tag,
                                FM_ASYNC_COND_T e_async_cond,
                                UINT32          ui4_data);

typedef struct _FM_IO_REQ_T
{
    union
    {
        MTPDEV_IO_DATA_T            t_mio;
        BLKDEV_IO_DATA_T            t_bio;
        CHRDEV_LONG_DATA_INFO_T     t_cio_long;
        CHRDEV_SHORT_DATA_INFO_T    t_cio_short;
        BLKDEV_SECTOR_RANGE_INFO_T  t_brng;
        CHRDEV_SECTOR_RANGE_INFO_T  t_crng;
    } u_io;

    SIZE_T          z_len;
    UINT32          ui4_done_byte;
    UINT8           ui1_type;
    #define FM_DEV_REQ_TYPE_BLK     ((UINT8) 0x01)
    #define FM_DEV_REQ_TYPE_CHRL    ((UINT8) 0x02)
    #define FM_DEV_REQ_TYPE_CHRS    ((UINT8) 0x04)
    #define FM_DEV_REQ_TYPE_ERASE   ((UINT8) 0x08)
    #define FM_DEV_REQ_DATA_MASK    ((UINT8) 0x0f)
    #define FM_DEV_REQ_TYPE_GET     ((UINT8) 0x10)
    #define FM_DEV_REQ_TYPE_SET     ((UINT8) 0x20)
    #define FM_DEV_REQ_OP_MASK      ((UINT8) 0x30)
    #define FM_DEV_REQ_READ         ((UINT8) 0x40)
    #define FM_DEV_REQ_ABORT        ((UINT8)0x80)

    UINT16          ui2_hold_cnt;
    UINT32          ui4_op;
    FM_ASYNC_COND_T e_cond;
    UINT8           ui1_pri;
    VOID            *pv_tag;
    x_fm_async_fct  pf_nfy_fct;

    DLIST_ENTRY_T(_FM_IO_REQ_T) t_qlink;

    HANDLE_T    h_rw_req;
    BOOL        b_sync;
    /* XXX, watch dog */
} FM_IO_REQ_T;

typedef struct _FLASH_SECTOR_SEG_INFO_T
{
    UINT32 ui4_num_sectors;
    UINT32 ui4_sector_size;
} FLASH_SECTOR_SEG_INFO_T;

typedef struct _FLASH_SECTOR_SEG_RANGE_T
{
    UINT64 ui8_start;
    UINT64 ui8_end;
} FLASH_SECTOR_SEG_RANGE_T;

typedef struct _FLASH_SECTOR_TBL_INFO_T
{
    UINT32 ui4_num_entries;
    FLASH_SECTOR_SEG_INFO_T  *pt_sector_info;
    FLASH_SECTOR_SEG_RANGE_T *pt_sector_rng;
} FLASH_SECTOR_TBL_INFO_T;

typedef UINT16 DRV_TYPE_T;

struct _FM_DEVICE_T
{
    HANDLE_T        h_driver;
    DRV_TYPE_T      e_drv_type;
    UINT16          ui2_flags;
    #define         FM_DEV_FLAG_ASYNC   ((UINT16) 0x0001)
    UINT16          ui2_unit;
    UINT16          ui2_ref_cnt;
    union
    {
        struct blk_param_t
        {
            UINT64 ui8_dev_size;
            UINT64 ui8_sec_num;
            UINT32 ui4_sec_size;
            UINT32 ui4_max_trans_sec;
            UINT32 ui4_max_io_req;
            UINT32 ui4_align_bytes;
            BLKDEV_NFY_INFO_T        t_nfy_info;
        } t_blkdev;

        struct chr_param_t
        {
            UINT32 ui4_dev_size;
            UINT32 ui4_sec_num;
            UINT32 ui4_max_rsync;
            UINT32 ui4_max_wsync;
            CHRDEV_NFY_INFO_T        t_nfy_info;
        } t_chrdev;
    } u_param;

    #define t_chr u_param.t_chrdev
    #define t_blk u_param.t_blkdev

    UINT64          ui8_space;
    HANDLE_T        h_thread;
    HANDLE_T        h_async_thrd_mtx;
    UINT8           ui1_thread_state;
    #define FM_DEV_THREAD_INIT      ((UINT8) 0)
    #define FM_DEV_THREAD_RUN       ((UINT8) 1)
    #define FM_DEV_THREAD_KILL      ((UINT8) 2)
    #define FM_DEV_THREAD_OVER      ((UINT8) 3)

    HANDLE_T        h_done_msgq;
    HANDLE_T        h_queue_mtx;
    DLIST_T(_FM_IO_REQ_T) t_pend_req_q;
    DLIST_T(_FM_IO_REQ_T) t_busy_req_q;
    DLIST_T(_FM_IO_REQ_T) t_done_req_q;
    DLIST_T(_FM_IO_REQ_T) t_abort_req_q;

    SLIST_ENTRY_T(_FM_DEVICE_T) t_llink;

    /* used for NOR/NAND Flash */
    FLASH_SECTOR_TBL_INFO_T *pt_sector_tbl;
    DLIST_T(_FLASH_SECTOR_CACHE_T) t_flash_cache; /* For flash I/O cache */
    HANDLE_T    h_flash_wait_flush;
    HANDLE_T    h_flash_wait_io;
    UINT32      ui4_cached_size;
    BOOL        b_flush_dirty;
    /* added by riccardo */
    HANDLE_T    h_ctrl_mtx;
};


typedef struct _FM_DEVICE_T FM_DEVICE_T;

typedef struct _FLASH_SECTOR_CACHE_T
{
    FM_DEVICE_T *pt_dev;
    UINT32  ui4_sector_num;     /* Sector number, index of the table */
    UINT32  ui4_sector_size;    /* Size of this sector */
    UINT64  ui8_start_addr;     /* Address of this sector on device */
    VOID    *pv_data;           /* Data */
    UINT16  ui2_flags;
    #define FM_FLASH_CACHE_NEED_ERASE ((UINT32) 0x0001)
    #define FM_FLASH_CACHE_PURGE      ((UINT32) 0x0002)
    #define FM_FLASH_CACHE_DIRTY      ((UINT32) 0x0004)
    #define FM_FLASH_CACHE_SYNC       ((UINT32) 0x0008)

    DLIST_ENTRY_T(_FLASH_SECTOR_CACHE_T) t_link;
} FLASH_SECTOR_CACHE_T;

typedef INT32 (*fm_ufs_fct)(VOID *pv_data);

typedef struct _FM_UFS_OP_T
{
    fm_ufs_fct      pf_newfs;
    fm_ufs_fct      pf_chkfs;
    fm_ufs_fct      pf_mount;
    fm_ufs_fct      pf_umount;
} FM_UFS_OP_T;

typedef enum _FM_MNT_TYPE_E
{
    FM_MNT_TYPE_VFAT = 0,
    FM_MNT_TYPE_NTFS,
    FM_MNT_TYPE_EXT3,
    FM_MNT_TYPE_TEXFAT,

    FM_MNT_TYPE_UNKNOW
} FM_MNT_TYPE_E;

/*File system types.*/
typedef enum
{
    FM_TYPE_INVAL  = 0,
    FM_TYPE_ROOTFS,
    FM_TYPE_FAT,
    FM_TYPE_FAT12,
    FM_TYPE_FAT16,
    FM_TYPE_FAT32,
    FM_TYPE_UDF,
    FM_TYPE_ISO9660,
    FM_TYPE_MEMFS,
    FM_TYPE_SFS,
    FM_TYPE_MTP,
    FM_TYPE_NTFS,
    FM_TYPE_EXT3
} FM_FS_TYPE_T;

/* File system descriptor*/
typedef struct _FM_FS_DESC_T
{
    CHAR             ps_name[16];
    FM_FS_TYPE_T     e_fs_type;
    INT32            i4_ref_cnt;
    FM_UFS_OP_T      *pt_ufs_op;
    SLIST_ENTRY_T(_FM_FS_DESC_T) t_link;
} FM_FS_DESC_T;

/*UDF file system version*/
typedef enum
{
    FM_UDF_UNKNOW =0,
    FM_UDF_100,
    FM_UDF_102,
    FM_UDF_150,
    FM_UDF_200,
    FM_UDF_201,
    FM_UDF_250,
    FM_UDF_260
}FM_UDF_VERSION;

/* File system information.*/
typedef struct _FM_FS_INFO_T
{
    FM_FS_TYPE_T    e_type;
    UINT64          ui8_blk_size;
    UINT64          ui8_total_blk;
    UINT64          ui8_free_blk;
    UINT64          ui8_avail_blk;
    UINT64          ui8_files;
    UINT16          ui2_max_name_len;

    UINT32              ui4_alignment;
    UINT32              ui4_min_blk_size;

    UINT64              ui8_first_free_blk;

    //Martin_20081208: Get device volume label for use
    UINT8       s_volume_label[32];

    //MingHuang_2009_6_30  Get UDF Version
    FM_UDF_VERSION       e_udf_version;
    //MingHuang_2010_8-21  Get JOLIET information for ISO9600 Disc.
    BOOL                 b_joliet;
} FM_FS_INFO_T;

typedef enum
{
    FM_MNT_OK,
    FM_MNT_UMNT,
    FM_MNT_FAIL
} FM_MNT_COND_T;

typedef VOID (*x_fm_mnt_fct)(FM_MNT_COND_T t_cond,
                              CHAR  *ps_part_name,
                              UINT32  ui4_part_idx,
                              CHAR *ps_mnt_path,
                              VOID *pv_tag);


typedef struct _FM_MNT_CB_T
{
    x_fm_mnt_fct pf_mnt_fct;
    VOID *pv_tag;
} FM_MNT_CB_T;


/* Partition structure.*/
typedef struct _FM_PART_T
{
    UINT8   ui1_drv;
    UINT8   ui1_start_chs[3]; /* starting CHS for INT 13 */
    UINT8   ui1_type;
    UINT8   ui1_end_chs[3];   /* skip ending CHS for INT 13 */
    UINT32  ui4_start_lba;
    UINT32  ui4_sectors;

    UINT32  ui4_part_idx;
    BOOL    b_entried;

    BOOL    b_try_mnt;
    BOOL    b_mnt;

    CHAR    ps_part_name[32];
    CHAR    ps_part_path[32];
    CHAR    ps_mnt_path[32];

    FM_MNT_CB_T t_mnt_cb;
} FM_PART_T;

/*Xentry operations*/
typedef struct _FM_XENTRY_OP_T
{
    fm_ufs_fct      pf_lookup;
    fm_ufs_fct      pf_read;
    fm_ufs_fct      pf_write;
    fm_ufs_fct      pf_read_async;
    fm_ufs_fct      pf_write_async;
    fm_ufs_fct      pf_lseek;
    fm_ufs_fct      pf_close;
    fm_ufs_fct      pf_get_info;
    fm_ufs_fct      pf_set_info;
    fm_ufs_fct      pf_get_dir_info;
    fm_ufs_fct      pf_make_entry;
    fm_ufs_fct      pf_open_dir;
    fm_ufs_fct      pf_close_dir;
    fm_ufs_fct      pf_read_dir;
    fm_ufs_fct      pf_delete;
    fm_ufs_fct      pf_rename;
    fm_ufs_fct      pf_sync;
    fm_ufs_fct      pf_seek_dir;
    fm_ufs_fct      pf_seek_dir_ex;
    fm_ufs_fct      pf_free_fs_node;
} FM_XENTRY_OP_T;


typedef struct _FM_MOUNT_T  FM_MOUNT_T;

/*Xentry type and flags */
struct _FM_XENTRY_T
{
    HANDLE_T        h_mtx;
    FM_DEVICE_T     *pt_dev;
    UINT32          ui4_mode;
    FM_MOUNT_T      *pt_mount;
    FM_MOUNT_T      *pt_mounted;
    FM_UFS_OP_T     *pt_ufs_ops;
    FM_XENTRY_OP_T  *pt_xent_ops;
    VOID            *pv_fs_node;
    UINT32          ui4_flags;
    UINT32          ui4_ref_cnt; /* if it is 0, purge it to LRU */
    UINT16          ui2_lock_cnt;
    UINT16          ui2_want_cnt;
    HANDLE_T        h_rw_lock;
    DLIST_ENTRY_T(_FM_XENTRY_T) t_qlink;

    DLIST_T(_FM_BUF_T) t_buf_list;

    /* Fields for name cache */
    struct _FM_XENTRY_T *pt_prnt_xent;
    CHAR                *ps_name;
    SIZE_T              z_name_size;
    SLIST_ENTRY_T(_FM_XENTRY_T) t_hlink;
};

typedef struct _FM_XENTRY_T FM_XENTRY_T;

/* Mount structure*/
struct _FM_MOUNT_T
{
    FM_FS_DESC_T    *pt_fs_desc;
    FM_XENTRY_T     *pt_mp_xent;
    FM_XENTRY_T     *pt_root_xent;
    FM_XENTRY_T     *pt_dev_xent;
    SLIST_ENTRY_T(_FM_MOUNT_T) t_link;

    FM_FS_INFO_T    t_fs_info;
    FM_PART_T       t_part_info;
    VOID            *pv_priv;
    UINT32          ui4_flags;
    #define FM_MNT_FLAG_ROOTFS 0x00000001
    HANDLE_T        h_sync_sema;
    HANDLE_T        h_lock_mtx;

    /* Opened file desc and dir label in this file system */
    DLIST_T(_FM_MTX_OBJ_T) t_opened_obj_q;
};

typedef struct _FM_RW_ARG_T
{
    FM_XENTRY_T     *pt_xent;
    VOID            *pv_data;
    UINT32          ui4_count;
    UINT64          ui8_offset;
    UINT32          ui4_done_bytes;
    UINT16          ui2_flag;
#define FM_IO_SYNC          ((UINT16) 0x0001)
#define FM_IO_DIRECT        ((UINT16) 0x0002)
#define FM_IO_INVAL         ((UINT16) 0x8000)
#define FM_IO_RW_END    ((UINT16) 0x0800)
    /* for sync */
    HANDLE_T        h_bin_sema;

    /* for async */
    UINT8           ui1_pri;
    x_fm_async_fct  pf_nfy_fct;
    VOID            *pv_tag;
    FM_ASYNC_COND_T e_async_cond;
    HANDLE_T    h_rw_req;
    HANDLE_T    h_file;

    UINT8       ui1_all_req_sent;
    UINT32      ui4_req_cnt;
    UINT32      ui4_req_ok_cnt;
    UINT32      ui4_req_fail_cnt;
    UINT8       ui1_aborted;

} FM_RW_ARG_T;

typedef struct _FM_BUF_T
{
    DLIST_ENTRY_T(_FM_BUF_T) t_hlink;   /* link to next buffer */
    DLIST_ENTRY_T(_FM_BUF_T) t_vlink;   /* link to state chain */
    VOID            *pv_alloc;      /* buffer allocated memroy */
    VOID            *pv_data;       /* buffer data, fit to alignment */
    UINT32          ui4_size;       /* buffer size */
    UINT32          ui4_align;      /* buffer alignment */
    UINT64          ui8_lbn;        /* logic block number into file */
    UINT32          ui4_sec_size;   /* block device sector size */
    UINT32          ui4_resid;      /* remaining I/O for this buffer */

    UINT16          ui2_flags;
#define FM_BUF_FLAG_INVAL       ((UINT16)0x0001)    /* Invalid buffer */
#define FM_BUF_FLAG_IO          ((UINT16)0x0002)    /* Performing I/O */
#define FM_BUF_FLAG_DIRTY       ((UINT16)0x0004)    /* Data is dirty */
#define FM_BUF_FLAG_RETRY       ((UINT16)0x0008)    /* Data to be reflush */
#define FM_BUF_FLAG_LOCKED      ((UINT16)0x0010)    /* It is being locked */
#define FM_BUF_FLAG_READ        ((UINT16)0x0020)    /* To read from device */
#define FM_BUF_FLAG_DROP        ((UINT16)0x0040)    /* Drop after I/O done */
#define FM_BUF_FLAG_ASYNC       ((UINT16)0x0100)    /* Async operation */
#define FM_BUF_FLAG_COPY_IN     ((UINT16)0x0200)    /* Async write */
#define FM_BUF_FLAG_IO_DONE     ((UINT16)0x0400)    /* No performing I/O */

    UINT16          ui2_want;
    UINT8           ui1_pri;

    FM_XENTRY_T     *pt_xent; /* device xent */
    /*FM_DEVICE_T     *pt_dev;*/
    HANDLE_T        h_mtx;
    HANDLE_T        h_sync;

    /* For buffer read done callback */
    FM_RW_ARG_T     *pt_rw_req;
    VOID            *pv_copy_addr;  /* User space data address */
    UINT32          ui4_copy_off;   /* The starting offset to copy in buffer */
    UINT32          ui4_copy_size;
} FM_BUF_T;

typedef struct _FM_FILE_DESC_T FM_FILE_DESC_T;

typedef INT32 (*fm_file_op_fct)(VOID *pv_data);

typedef INT32 (*fm_file_rw_fct)(FM_FILE_DESC_T  *pt_desc,
                                 VOID            *pv_data,
                                 UINT32          ui4_count,
                                 UINT32          *pui4_done);

typedef INT32 (*fm_file_arw_fct)(FM_FILE_DESC_T  *pt_desc,
                                  VOID            *pv_data,
                                  UINT32          ui4_count,
                                  UINT8           ui1_pri,
                                  x_fm_async_fct  pf_nfy_fct,
                                  VOID            *pv_tag,
                                  HANDLE_T        *ph_req,
                                  HANDLE_T        h_file);

typedef INT32 (*fm_file_lock_fct)(FM_FILE_DESC_T *pt_desc,UINT32 ui4_lock_op);

typedef struct _FM_FILE_OP_T
{
    fm_file_rw_fct   pf_read;
    fm_file_rw_fct   pf_write;
    fm_file_arw_fct  pf_async_read;
    fm_file_arw_fct  pf_async_write;
    fm_file_op_fct   pf_close;
    fm_file_lock_fct pf_flock;
} FM_FILE_OP_T;

/* File descriptor and directory label*/
struct _FM_FILE_DESC_T
{
    HANDLE_T        h_mtx;
    HANDLE_T        h_ref_handle;
    UINT16          ui2_lock_cnt;
    UINT16          ui2_padding;
    UINT32          ui4_status;
    fm_obj_free_fct pf_free;
    FM_ENTRY_TYPE_T e_type;
    DLIST_ENTRY_T(_FM_MTX_OBJ_T) t_obj_link;

    /* Cannot modify above entries. It must be the same as FM_MTX_OBJ_T */
    FM_XENTRY_T     *pt_xent;
    FM_FILE_OP_T    *pt_file_ops;
    UINT32          ui4_flags;
    UINT64          ui8_offset;
    HANDLE_T        h_sync_sema; /* for synchronous I/O */
    UINT32          ui4_flock_cnt;
    BOOL            b_open_ex_enabled;
};

/* Directory label */
typedef struct _FM_DIR_LABEL_T
{
    HANDLE_T        h_mtx;
    HANDLE_T        h_ref_handle;
    UINT16          ui2_lock_cnt;
    UINT16          ui2_padding;
    UINT32          ui4_status;
    fm_obj_free_fct pf_free;
    FM_ENTRY_TYPE_T e_type;
    DLIST_ENTRY_T(_FM_MTX_OBJ_T) t_obj_link;

    /* Cannot modify above entries. It must be the same as FM_MTX_OBJ_T */
    FM_XENTRY_T     *pt_xent;
    CHAR            *ps_path; /* includes endding '/' */
    UINT16          ui2_len;  /* strlen(ps_path) */
} FM_DIR_LABEL_T;

typedef struct _FM_AIO_ARG_T
{
    INT32           fd;
    VOID            *pv_aiocb;
    x_fm_async_fct  pf_nfy_fct;
    VOID            *pv_tag;
    HANDLE_T        h_rw_req;
} FM_AIO_ARG_T;

typedef struct _FM_PART_INFO_T
{
    BOOL    b_try_mnt;
    BOOL    b_mnt;

    CHAR    ps_part_name[FM_MAX_FILE_LEN];
    CHAR    ps_part_path[FM_MAX_PATH_LEN];
    CHAR    ps_mnt_path[FM_MAX_PATH_LEN];
} FM_PART_INFO_T;

typedef struct _FM_PART_INFO_DESC_T
{
    FM_PART_INFO_T t_part_info;

    SLIST_ENTRY_T(_FM_PART_INFO_DESC_T) t_link;
} FM_PART_INFO_DESC_T;

typedef struct _FM_MNT_INFO_T
{
    BOOL            b_mnt;
    FM_MNT_TYPE_E   e_mnt_type;
    UCHAR           s_mnt_path[FM_MAX_PATH_LEN];
} FM_MNT_INFO_T;

extern INT32 u_fm_init(VOID);
extern INT32 u_fm_close(HANDLE_T h_file);
extern INT32 u_fm_mount(HANDLE_T h_dev_dir,const CHAR *ps_dev_path,HANDLE_T h_mp_dir,const CHAR *ps_mp_path, FM_MNT_INFO_T  *pt_mnt_info);
extern INT32 u_fm_umount(HANDLE_T  h_dir,const CHAR *ps_path);
extern INT32 u_fm_get_part_ns(HANDLE_T h_dir,const CHAR *ps_path,UINT32 *pui4_count);
extern INT32 u_fm_set_dir_path(HANDLE_T h_dir,const CHAR *ps_path,HANDLE_T *ph_dir);
extern INT32 u_fm_get_part_info(HANDLE_T h_dir,const CHAR *ps_path,UINT32  ui4_part_idx,FM_PART_INFO_T *pt_part_info);
extern INT32 u_fm_create_dir_fdr(HANDLE_T h_dir,const CHAR *ps_path,UINT32 ui4_mode);
extern INT32 u_fm_delete_dir_fdr(HANDLE_T h_dir,const CHAR *ps_path);

#endif /* _U_FM_H */

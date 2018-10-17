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
 * $RCSfile: u_fm_linux.c,v $
 * $Revision:
 * $Date:
 * $CCRevision: $
 * $SWAuthor:$
 *
 * Description:
 *         This file contains File Manager exported API's.
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include <sys/mount.h>
#include <sys/vfs.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <asm/unistd.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utime.h>
#include <fcntl.h>
#include <linux/magic.h>
#include <linux/udf_fs_i.h>
#include <linux/iso_fs.h>
#include <linux/msdos_fs.h>

#include "u_os.h"
#include "u_handle.h"
#include "u_fm.h"
#include "u_lnk_list.h"
#include "u_rwlock.h"
#include "u_assert.h"
#include "u_amb.h"
#include "u_ipcd.h"
#include "dm.h"

HANDLE_T         h_fm_mnt_lock;
static HANDLE_T  h_fm_part_info_rwlock;

static struct
{
    struct _FM_PART_INFO_DESC_T *pt_head;
}t_fm_part_info_list;

static VOID _fm_record_mnt_path(const char *s_mnt_path, FM_MNT_INFO_T  *pt_mnt_info)
{
    INT32 i4_str_len = strlen(s_mnt_path);
    INT32 i4_max_len = i4_str_len <= (FM_MAX_PATH_LEN - 1) ? i4_str_len : (FM_MAX_PATH_LEN - 1);

    strncpy((char *)pt_mnt_info->s_mnt_path, s_mnt_path, i4_max_len);
    pt_mnt_info->s_mnt_path[i4_max_len] = '\0';
}

INT32 u_fm_init(VOID)
{
    INT32 i4_ret;
    DM_INFO("[FM] %s: #%d: \n", __FUNCTION__, __LINE__);

    i4_ret = u_sema_create(&h_fm_mnt_lock, X_SEMA_TYPE_MUTEX, 1);
    if (OSR_OK != i4_ret)
    {
        return FMR_CORE;
    }

    i4_ret = u_rwl_create_lock(&h_fm_part_info_rwlock);
    if (RWLR_OK != i4_ret)
    {
        return FMR_CORE;
    }

    SLIST_INIT(&t_fm_part_info_list);

    return i4_ret;
}

INT32 u_fm_close(HANDLE_T h_file)
{
    VOID            *pv_obj = NULL;
    HANDLE_TYPE_T   e_obj;
    INT32           i4_ret;

    DM_INFO("[FM] %s: ""h_file %lu\n", __FUNCTION__, h_file);

    if (HR_OK != u_handle_get_type_obj(h_file, &e_obj, &pv_obj))
    {
        DM_ERR("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_HANDLE);
        return FMR_HANDLE;
    }

    if (FMT_FILE_DESC == e_obj)
    {
        FM_FILE_DESC_T *pt_desc = (FM_FILE_DESC_T *)pv_obj;
        DM_INFO("[FM] %s: #%d: close FMT_FILE_DESC\n", __FUNCTION__, __LINE__);

        if (FM_TYPE_XENTRY == pt_desc->e_type)  /* file */
        {
            INT32 fd = (INT32)pt_desc->h_ref_handle;
            CHAR *dir_path = (CHAR *)pt_desc->ui4_status;

            DM_INFO("[FM] %s: #%d: close file\n", __FUNCTION__, __LINE__);
            i4_ret = close(fd);
            if (-1 == i4_ret)
            {
                DM_ERR("[FM] %s: #%d: close file fail!!!\n", __FUNCTION__, __LINE__);
            }
            else
            {
                i4_ret = u_handle_free(h_file, TRUE);
                free(dir_path);
                free(pt_desc);
            }
        }
        else if (FM_TYPE_DIRLBL == pt_desc->e_type) /* directory */
        {
            CHAR *dir_path = (CHAR *)pt_desc->ui4_status;
            if (TRUE == pt_desc->b_open_ex_enabled)
            {
                INT32 i4_dir_fd = pt_desc->h_ref_handle;
                DM_INFO("[FM] %s: #%d: close directory\n", __FUNCTION__, __LINE__);
                i4_ret = close(i4_dir_fd);
            }
            else
            {
                DIR *dir = (DIR *)pt_desc->h_ref_handle;
                DM_INFO("[FM] %s: #%d: close directory\n", __FUNCTION__, __LINE__);
                i4_ret = closedir(dir);
            }

            if (-1 == i4_ret)
            {
                DM_ERR("[FM] %s: #%d: close directory fail!!!\n", __FUNCTION__, __LINE__);
            }
            else
            {
                i4_ret = u_handle_free(h_file, TRUE);
                free(dir_path);
                free(pt_desc);
            }
        }
        else
        {
            i4_ret = FMR_HANDLE;
        }

    }
    else if (FMT_DIR_LABEL == e_obj)
    {
        FM_DIR_LABEL_T *pt_lbl = (FM_DIR_LABEL_T *)pv_obj;

        DM_INFO("[FM] %s: #%d: close FMT_DIR_LABEL\n", __FUNCTION__, __LINE__);
        i4_ret = u_handle_free(h_file, TRUE);
        free(pt_lbl->ps_path);
        free(pt_lbl);
    }
    else
    {
        i4_ret = FMR_HANDLE;
    }

    DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);

    return i4_ret;
}

/*-----------------------------------------------------------------------------
 * Name: fm_get_dir_label
 *
 * Description: Get the direcotry label with lock.
 *
 * Inputs:  h_dir
 *              Handle of a directory label.
 *
 * Outputs: ppt_lbl
 *              The reference to address of FM_DIR_LABEL_T object.
 *
 * Returns: FMR_OK
 *              Success.
 *          FMR_HANDLE
 *              Fail to get the handle object.
 *          FMR_INVAL
 *              Invalid object type or object is being closed.
 *          FMR_CORE
 *              OSAI error.
 ----------------------------------------------------------------------------*/
INT32 fm_get_dir_label(HANDLE_T h_dir,FM_DIR_LABEL_T **ppt_lbl)
{
    HANDLE_TYPE_T   e_obj;

    ASSERT(NULL != ppt_lbl);

    if (FM_ROOT_HANDLE == h_dir)
    {
        *ppt_lbl = NULL;
        return FMR_OK;
    }

    if (HR_OK != u_handle_get_type_obj(h_dir, &e_obj, (VOID**)ppt_lbl))
    {
        return FMR_HANDLE;
    }

    if (FMT_DIR_LABEL != e_obj)
    {
        return FMR_INVAL;
    }

    return FMR_OK; /*modify for dtv temporary*/
}

/*-----------------------------------------------------------------------------
 * Name: fm_get_absolute_path
 *
 * Description:
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 ----------------------------------------------------------------------------*/
INT32 fm_get_absolute_path(FM_DIR_LABEL_T *pt_dir_lbl,const CHAR *ps_path,CHAR **pps_dir_path)
{
    UINT32          ui4_strlen = 0;
    /* lenth of pt_dir_lbl is 0 */
    if((NULL == pt_dir_lbl)||(NULL == pt_dir_lbl->ps_path)||(0 == pt_dir_lbl->ui2_len))
    {
        if ((NULL == ps_path) || (0 == strlen(ps_path)))
        {
            return FMR_INVAL;
        }

        *pps_dir_path = (CHAR *)malloc(sizeof(CHAR) * (FM_MAX_PATH_LEN + 2));
        if (NULL == *pps_dir_path)
        {
            return FMR_CORE;
        }
        memset(*pps_dir_path, 0, sizeof(CHAR) * (FM_MAX_PATH_LEN + 2));

        strncpy(*pps_dir_path, ps_path, strlen(ps_path)+1);
    }
    else
    {
        ui4_strlen = (ps_path == NULL) ? 0 : strlen(ps_path);

        if ((pt_dir_lbl->ui2_len +  ui4_strlen) > FM_MAX_PATH_LEN)
        {
            return FMR_NAME_TOO_LONG;
        }

        *pps_dir_path = (CHAR *)malloc(sizeof(CHAR) * (FM_MAX_PATH_LEN + 2));
        if (NULL == *pps_dir_path)
        {
            return FMR_CORE;
        }
        memset(*pps_dir_path, 0, sizeof(CHAR) * (FM_MAX_PATH_LEN + 2));

        (*pps_dir_path)[FM_MAX_PATH_LEN] = '\0';
        strncpy(*pps_dir_path, pt_dir_lbl->ps_path, strlen(pt_dir_lbl->ps_path)+1);

        if ((NULL != ps_path) && (strlen(ps_path) > 0))
        {
            if(('/' != (*pps_dir_path)[strlen(*pps_dir_path) - 1])&& ('/' != ps_path[0]))
            {
                if((strlen(*pps_dir_path)+strlen("/")+1) >= (FM_MAX_PATH_LEN + 1))
                {
                    return FMR_NAME_TOO_LONG;
                }
                strncat(*pps_dir_path, "/", strlen("/")+1);
            }

            if((strlen(*pps_dir_path)+strlen(ps_path)+1) >= (FM_MAX_PATH_LEN + 1))
            {
                return FMR_NAME_TOO_LONG;
            }
            strncat(*pps_dir_path, ps_path, strlen(ps_path)+1);
        }
    }

    if ((strlen(*pps_dir_path)+1) > (FM_MAX_PATH_LEN + 2))
    {
        return FMR_NAME_TOO_LONG;
    }

    DM_INFO("[FM] %s: %s\n", __FUNCTION__, (NULL == *pps_dir_path) ? "NULL" : *pps_dir_path);

    return FMR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: fm_handle_free_fct
 *
 * Description: Free allocated resources.
 *
 * Inputs:   h_handle
 *           e_type
 *          *pv_obj
 *          *pv_tag
 *           b_req_handle
 *
 * Outputs: -
 *
 * Returns: Always return TRUE.
 ----------------------------------------------------------------------------*/
BOOL fm_handle_free_fct(HANDLE_T        h_handle,
                            HANDLE_TYPE_T   e_type,
                            VOID            *pv_obj,
                            VOID            *pv_tag,
                            BOOL            b_req_handle)
{
    if (TRUE == b_req_handle)
    {
        if ((FMT_ASYNC_WRITE == e_type) || (FMT_ASYNC_READ == e_type))
        {
            FM_AIO_ARG_T *pt_aio_arg = (FM_AIO_ARG_T *) pv_obj;
            if (NULL != pt_aio_arg)
            {
                struct aiocb64 *pt_aiocb = (struct aiocb64 *)(pt_aio_arg->pv_aiocb);
                if (NULL != pt_aiocb)
                {
                    free(pt_aiocb);
                    pt_aiocb = NULL;
                }

                free(pt_aio_arg);
                pt_aio_arg = NULL;
            }
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * Name: u_fm_set_dir_path
 *
 * Description: Set a label to the specified directory. One can use this
 *              function to set a reference to a directory, so that the
 *              underneath UFS can start finding a file from this directory
 *              without re-searching from the root. It hence provides the
 *              similar ability to the current working directory, but it's
 *              more flexible.
 *
 * Inputs:  h_dir
 *              Handle of a directory label.
 *          ps_path
 *              Path name of a directory to be set.
 *
 * Outputs: ph_dir
 *              Reference to a handle of directory label.
 *
 * Returns: FMR_OK
 *              Success.
 *          FMR_ARG
 *              ps_path is NULL or ph_dir is NULL.
 *          FMR_HANDLE
 *              h_dir is an invalid handle.
 *          FMR_INVAL
 *              ps_path is an invalid path.
 *          FMR_NO_ENTRY
 *              ps_path is not found.
 *          FMR_NOT_DIR
 *              A component name in ps_path is not a directory.
 *          FMR_NAME_TOO_LONG
 *              Length of ps_path is too long.
 *          FMR_CORE
 *              OSAI error.
 ----------------------------------------------------------------------------*/
INT32 u_fm_set_dir_path(HANDLE_T h_dir,const CHAR *ps_path,HANDLE_T *ph_dir)
{
    CHAR            *s_name      = NULL;
    FM_DIR_LABEL_T  *pt_new_lbl  = NULL;
    FM_DIR_LABEL_T  *pt_dir_lbl  = NULL;
    CHAR            *ps_dir_path = NULL;
    INT32 i4_ret;

    DM_INFO("[FM] %s: ""h_dir %lu, ""ps_path %s\n",__FUNCTION__,h_dir,(ps_path == NULL) ? "NULL" : ps_path);
    if (NULL == ph_dir)
    {
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_ARG);
        return FMR_ARG;
    }

    i4_ret = fm_get_dir_label(h_dir, &pt_dir_lbl);
    if (FMR_OK != i4_ret)
    {
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
        return i4_ret;
    }

    i4_ret = fm_get_absolute_path(pt_dir_lbl, ps_path, &ps_dir_path);
    if (FMR_OK != i4_ret)
    {
        if (NULL != ps_dir_path)
        {
            free(ps_dir_path);
        }
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
        return i4_ret;
    }

    s_name = (CHAR *)malloc(sizeof(CHAR) * 4 * (FM_MAX_FILE_LEN + 1));
    if (NULL == s_name)
    {
        free(ps_dir_path);
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_CORE);
        return FMR_CORE;
    }

    pt_new_lbl = (FM_DIR_LABEL_T *)malloc(sizeof(FM_DIR_LABEL_T));
    if (NULL == pt_new_lbl)
    {
        free(ps_dir_path);
        free(s_name);
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_CORE);
        return FMR_CORE;
    }
    memset(pt_new_lbl, 0, sizeof(FM_DIR_LABEL_T));

    strncpy(s_name, ps_dir_path, sizeof(CHAR) * 4 * (FM_MAX_FILE_LEN + 1));
    if('/' != s_name[strlen(s_name) - 1])
    {
        if ((strlen(s_name)+strlen("/")+1) >= ((FM_MAX_FILE_LEN + 1) * 4))
        {
            free(s_name);
            free(pt_new_lbl);
            free(ps_dir_path);
            DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_NAME_TOO_LONG);
            return FMR_NAME_TOO_LONG;
        }
        strncat(s_name, "/", strlen("/")+1);
    }

    pt_new_lbl->h_ref_handle = (NULL == pt_dir_lbl) ? (HANDLE_T)NULL : (HANDLE_T)(pt_dir_lbl->h_ref_handle);
    pt_new_lbl->ui2_len = strlen(s_name);
    pt_new_lbl->ps_path = s_name;

    free(ps_dir_path);

    i4_ret = u_handle_alloc((HANDLE_TYPE_T)FMT_DIR_LABEL,
                           pt_new_lbl,
                           NULL,
                           fm_handle_free_fct,
                           ph_dir);

    if (HR_OK != i4_ret)
    {
        free(pt_new_lbl);
        i4_ret = FMR_HANDLE;
    }

    DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
    return i4_ret;
 }

 /*-----------------------------------------------------------------------------
  * Name: fm_get_part_ns
  *
  * Description:
  *
  * Inputs:
  *
  * Outputs:
  *
  * Returns:
  ----------------------------------------------------------------------------*/
 INT32 fm_get_part_ns(const CHAR *ps_dir_path,UINT32 *pui4_count,UINT32 *pui4_minimum)
 {
     FILE   *fp;
     CHAR   *ps_last;
     CHAR   *ps_name     = NULL;
     CHAR   *read_buf    = NULL;
     CHAR   *ps_dev_name = NULL;
     UINT32 ui4_tmp      = 0;
     UINT32 ui4_part_idx = 0;
     UINT32 ui4_part_tmp = 1;
     BOOL   b_self       = FALSE;
     BOOL   b_init       = FALSE;

     DM_INFO("[FM] %s: #%d: ps_name = %s\n", __FUNCTION__, __LINE__, ps_dir_path);

     if ((NULL == ps_dir_path) || (NULL == pui4_count) || (NULL == pui4_minimum))
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_INVAL);
         return FMR_INVAL;
     }

     read_buf = (CHAR *)malloc(sizeof(CHAR)*1024);
     if (NULL == read_buf)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_CORE);
         return FMR_CORE;
     }
     memset(read_buf, 0, (sizeof(CHAR) * 1024));

     ps_name = (CHAR *)malloc(sizeof(CHAR)*256);
     if (NULL == ps_name)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_CORE);
         free(read_buf);
         return FMR_CORE;
     }
     memset(ps_name, 0, (sizeof(CHAR) * 256));

     ps_dev_name = (CHAR *)malloc(sizeof(CHAR)*256);
     if (NULL == ps_dev_name)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_CORE);
         free(read_buf);
         free(ps_name);
         return FMR_CORE;
     }
     memset(ps_dev_name, 0, (sizeof(CHAR) * 256));

     /* get dev short name */
     ps_last = strrchr(ps_dir_path, '/');
     if (0 == ps_last)
     {
         strncpy(ps_name, ps_dir_path, 255);
     }
     else
     {
         strncpy(ps_name, (ps_last + 1), 255);
     }
     DM_INFO("[FM] %s: #%d: ps_name = %s\n", __FUNCTION__, __LINE__, ps_name);

     fp = fopen("/proc/diskstats", "r");
     if (NULL == fp)
     {
        DM_ERR("[FM] %s: #%d: open file fail .\n", __FUNCTION__, __LINE__);
        if (NULL != ps_dev_name)
        {
            free(ps_dev_name);
            ps_dev_name = NULL;
        }
        if (NULL != read_buf)
        {
           free(read_buf);
           read_buf = NULL;
        }
        return FMR_CORE;
     }
     rewind(fp);

     *pui4_count = 0;
     *pui4_minimum = 0;

     while(!feof(fp))
     {
         if(!fgets(read_buf, 1023 , fp))
         {
             break;
         }

         memset(ps_dev_name, 0, (sizeof(CHAR) * 256));
         sscanf(read_buf, "%*d %*d %s %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u",ps_dev_name);
         DM_INFO("[FM] %s: #%d: ps_dev_name = %s\n", __FUNCTION__, __LINE__, ps_dev_name);

         if(0 == strncmp(ps_dev_name, ps_name ,strlen(ps_name)))
         {
             DM_INFO("[FM] %s: #%d: (match)\n", __FUNCTION__, __LINE__);
             if(strlen(ps_name) == strlen(ps_dev_name))
             {
                 DM_INFO("[FM] %s: #%d: (self)\n", __FUNCTION__, __LINE__);
                 b_self = TRUE;
             }
             else
             {
                 /* get partition index */
                 ui4_tmp      = strlen(ps_dev_name) - 1;
                 ui4_part_idx = 0;
                 ui4_part_tmp = 1;

                 while (((ps_dev_name[ui4_tmp] >= '0') && (ps_dev_name[ui4_tmp] <= '9')))
                 {
                     ui4_part_idx += ((ps_dev_name[ui4_tmp] - '0') * ui4_part_tmp);
                     ui4_part_tmp *= 10;
                     ui4_tmp --;
                 }

                 if (FALSE == b_init)
                 {
                     *pui4_minimum = ui4_part_idx;
                     b_init = TRUE;
                 }
                 else if(*pui4_minimum > ui4_part_idx)
                 {
                     *pui4_minimum = ui4_part_idx;
                 }

                 if (b_self)
                 {
                     *pui4_minimum = 0;
                     DM_INFO("[FM] %s: #%d: b_self is true so set minmum to zero.\n", __FUNCTION__, __LINE__);
                 }
                 DM_INFO("[FM] %s: #%d: ui4_part_idx: %lu\n", __FUNCTION__, __LINE__, ui4_part_idx);
             }

             *pui4_count  += 1;
             if(((TRUE == b_self) && (strlen(ps_name) > 3) && (0 == strncmp(ps_name, "sd",strlen("sd"))))  //If we match the sda1 we will break the while loop.
                 ||((TRUE == b_self) && (strlen(ps_name) > 8) && (0 == strncmp(ps_name, "mmcblk",strlen("mmcblk")))))  // For SD CARD.
             {
                 break;
             }
         }
     }

     fclose(fp);

     DM_INFO("[FM] %s: #%d: ui4_count = %ld, b_self = %d, ui4_minimum = %lu\n", __FUNCTION__, __LINE__, *pui4_count, b_self, *pui4_minimum);

#if 1
     if ((0 == strncmp(ps_dir_path, "/dev/sr0", strlen("/dev/sr0"))) && (0 == *pui4_count))
     {
         *pui4_count = 1;
     }
#endif

     free(read_buf);
     free(ps_name);
     free(ps_dev_name);

     DM_INFO("[FM] %s: #%d: ui4_count = %lu final\n", __FUNCTION__, __LINE__, *pui4_count);
     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_OK);

     return FMR_OK;
 }

 /*-----------------------------------------------------------------------------
  * Name: x_fm_get_partition_ns
  *
  * Description:
  *
  * Inputs:
  *          h_partition
  *              handle of partition table
  *
  * Outputs: -
  *
  * Returns: FMR_OK
  *              Success.
  *          FMR_ARG
  *              Invalid argument.
  *          FMR_CORE
  *              OSAI error.
  ----------------------------------------------------------------------------*/
 INT32 u_fm_get_part_ns(HANDLE_T h_dir,const CHAR *ps_path,UINT32 *pui4_count)
 {
     FM_DIR_LABEL_T  *pt_dir_lbl  = NULL;
     CHAR            *ps_dir_path = NULL;
     INT32           i4_ret       = FMR_OK;
     UINT32          ui4_minimum  = 0;

     DM_INFO("[FM] %s: ""h_dir %lu, ""ps_path %s\n",__FUNCTION__,h_dir,(ps_path == NULL) ? "NULL" : ps_path);
     ASSERT(NULL != ps_path);
     ASSERT(NULL != pui4_count);

     i4_ret = fm_get_dir_label(h_dir, &pt_dir_lbl);
     if (FMR_OK != i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     i4_ret = fm_get_absolute_path(pt_dir_lbl, ps_path, &ps_dir_path);
     if (FMR_OK != i4_ret)
     {
         if (NULL != ps_dir_path)
         {
             free(ps_dir_path);
         }
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     if ((strlen(ps_dir_path)+1) >= (FM_MAX_PATH_LEN + 2))
     {
         free(ps_dir_path);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_NAME_TOO_LONG);
         return FMR_NAME_TOO_LONG;
     }

     i4_ret = fm_get_part_ns(ps_dir_path, pui4_count, &ui4_minimum);
     if (FMR_OK != i4_ret)
     {
         free(ps_dir_path);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     free(ps_dir_path);
     DM_INFO("[FM] %s: #%d: ui4_count = %lu\n", __FUNCTION__, __LINE__, *pui4_count);
     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_OK);

     return FMR_OK;
 }


 /*-----------------------------------------------------------------------------
  * Name: fm_get_part_info
  *
  * Description:
  *    ps_part_path such as /dev/sda or /dev/sda1
  *    ui4_part_idx such as         1 or        0
  *    not allowed  such as /dev/sda1 + 1
  *
  * Inputs:
  *
  * Outputs:
  *
  * Returns:
  ----------------------------------------------------------------------------*/
 INT32 fm_get_part_info(const CHAR    *ps_part_path,
                           const UINT32    ui4_part_idx,
                           FM_PART_INFO_T  *pt_part_info)
 {
     FM_PART_INFO_DESC_T *pt_part_desc                = NULL;
     CHAR                ps_desc[FM_MAX_FILE_LEN*2+1] = {0};
     BOOL                b_found                      = FALSE;
     CHAR                ps_part_full_path[FM_MAX_FILE_LEN + 1] = {0};
     UINT32              ui4_tmp                = 0;
     UINT32              ui4_revert             = 0;
     UINT32              ui4_len                = 0;

     if ((NULL == ps_part_path) || (NULL == pt_part_info))
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_ARG);
         return FMR_ARG;
     }

     if ((ui4_part_idx > 0)&& (NULL == strstr(ps_part_path, "mmcblk")) &&
        ((ps_part_path[strlen(ps_part_path) - 1] >= '0')&&
        (ps_part_path[strlen(ps_part_path) - 1] <= '9')))
     {
         DM_INFO("[FM] %s: #%d: input not allowed like this: /dev/sda1 + 1\n", __FUNCTION__, __LINE__);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_ARG);
         return FMR_ARG;
     }

     DM_INFO("[FM] %s: #%d: ps_part_path: %s\n", __FUNCTION__, __LINE__, ps_part_path);
     DM_INFO("[FM] %s: #%d: ui4_part_idx: %lu\n", __FUNCTION__, __LINE__, ui4_part_idx);

     strncpy(ps_part_full_path, ps_part_path, strlen(ps_part_path)+1);

     if (0 != ui4_part_idx)
     {
        if (0 == strncmp(ps_part_full_path, "/dev/mmcblk", strlen("/dev/mmcblk")))
        {
            if(NULL == strstr(ps_part_full_path, "p"))
            {
                if (strlen(ps_part_full_path)+strlen("p") >= FM_MAX_FILE_LEN) //for klocwork issue
                {
                    return FMR_NAME_TOO_LONG;
                }
                strncat(ps_part_full_path, "p", strlen("p")+1);
            }
        }
     }

     /* concatenate ps_part_path and ui4_part_idx */
     ui4_tmp = ui4_part_idx;

     while (ui4_tmp != 0)
     {
         ui4_revert *= 10;
         ui4_revert += ui4_tmp % 10;
         ui4_tmp /= 10;
         ui4_len ++;
     }

     while (ui4_len > 0)
     {
         ps_part_full_path[strlen(ps_part_full_path)] = ui4_revert % 10 + '0';
         ui4_revert /= 10;
         ui4_len --;
     }

     ps_part_full_path[strlen(ps_part_full_path)] = '\0';

     DM_INFO("[FM] %s: #%d: ps_part_full_path: %s\n", __FUNCTION__, __LINE__, (ps_part_full_path==NULL)?NULL:ps_part_full_path);

     if (RWLR_OK != u_rwl_read_lock(h_fm_part_info_rwlock, RWL_OPTION_WAIT))
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_CORE);
         return FMR_CORE;
     }

     SLIST_FOR_EACH(pt_part_desc, &t_fm_part_info_list, t_link)
     {
         strncpy(ps_desc, pt_part_desc->t_part_info.ps_part_path, strlen(pt_part_desc->t_part_info.ps_part_path)+1);

         {
              UINT32 ui4PathLen = strlen(pt_part_desc->t_part_info.ps_part_path); //for klockwork issue by Yonglong
              if(ui4PathLen > 0)
              {
                  if(('/' != pt_part_desc->t_part_info.ps_part_path[ui4PathLen - 1])
                      && ('/' != pt_part_desc->t_part_info.ps_part_name[0]))
                  {
                      if (strlen(ps_desc)+1 >= FM_MAX_FILE_LEN + 1)////for klocwork issue
                      {
                          return FMR_NAME_TOO_LONG;
                      }
                      else
                      {
                          strncat(ps_desc, "/", strlen("/")+1);
                      }
                   }
              }
         }

         if (strlen(ps_desc)+strlen(pt_part_desc->t_part_info.ps_part_name)+1 >= FM_MAX_FILE_LEN + 1) //for klocwork issue
         {
             return FMR_NAME_TOO_LONG;
         }

         strncat(ps_desc, pt_part_desc->t_part_info.ps_part_name, strlen(pt_part_desc->t_part_info.ps_part_name)+1);

         DM_INFO("[FM] %s: #%d: stored: ps_desc: %s\n", __FUNCTION__, __LINE__, (ps_desc==NULL)?NULL:ps_desc);

         if ((0 == strcmp(ps_desc, ps_part_full_path))
             ||((0 == strcmp("/dev/mmcblk0p", ps_part_full_path)) && (0 == strcmp("/dev/mmcblk0", ps_desc))))
         {
             DM_INFO("[FM] %s: #%d: found\n", __FUNCTION__, __LINE__);
             b_found = TRUE;
             pt_part_info->b_try_mnt = pt_part_desc->t_part_info.b_try_mnt;
             pt_part_info->b_mnt = pt_part_desc->t_part_info.b_mnt;
             strncpy(pt_part_info->ps_part_name, pt_part_desc->t_part_info.ps_part_name, strlen(pt_part_desc->t_part_info.ps_part_name)+1);
             strncpy(pt_part_info->ps_part_path, pt_part_desc->t_part_info.ps_part_path, strlen(pt_part_desc->t_part_info.ps_part_path)+1);
             strncpy(pt_part_info->ps_mnt_path,  pt_part_desc->t_part_info.ps_mnt_path, strlen(pt_part_desc->t_part_info.ps_mnt_path)+1);
             break;
         }
     }

     u_rwl_release_lock(h_fm_part_info_rwlock);

     if (FALSE == b_found)
     {
         CHAR  *ps_last      = NULL;
         INT32 i4_last_index = 0;

         ps_last = strrchr(ps_part_path, '/');

         if (0 == ps_last)
         {
            strncpy(pt_part_info->ps_part_name, ps_part_path, strlen(ps_part_path)+1);
            memset(pt_part_info->ps_part_path, 0, (sizeof(CHAR) * 32));
         }
         else
         {
             i4_last_index = (ps_part_path - ps_last) / sizeof(CHAR);
             i4_last_index = i4_last_index < 0 ? -i4_last_index : i4_last_index;
             i4_last_index ++;

             strncpy(pt_part_info->ps_part_name, (ps_last + 1), strlen(ps_last)+1);
             if (i4_last_index >= 32 )
             {
                  return FMR_NAME_TOO_LONG;
             }
             strncpy(pt_part_info->ps_part_path, ps_part_path, i4_last_index);
         }

         pt_part_info->b_try_mnt = FALSE;
         pt_part_info->b_mnt = FALSE;
         memset(pt_part_info->ps_mnt_path, 0, (sizeof(CHAR) * 32));
     }

     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_OK);
     return FMR_OK;
 }

 /*-----------------------------------------------------------------------------
  * Name: u_fm_get_partition_info
  *
  * Description:
  *
  * Inputs:
  *          h_partition
  *              handle of partition table
  *
  * Outputs: -
  *
  * Returns: FMR_OK
  *              Success.
  *          FMR_ARG
  *              Invalid argument.
  *          FMR_CORE
  *              OSAI error.
  ----------------------------------------------------------------------------*/
 INT32 u_fm_get_part_info(HANDLE_T h_dir,
                              const CHAR *ps_path,
                              UINT32  ui4_part_idx,
                              FM_PART_INFO_T *pt_part_info)
 {
     INT32           i4_ret       = FMR_OK;
     CHAR            *ps_dir_path = NULL;
     FM_DIR_LABEL_T  *pt_dir_lbl  = NULL;

     DM_INFO("[FM] %s: #%d: h_dir = %lu, ps_path = %s, ui4_part_idx = %lu\n",__FUNCTION__, __LINE__, h_dir, ps_path, ui4_part_idx);

     if (NULL == pt_part_info)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_ARG);
         return FMR_ARG;
     }

     if ((ui4_part_idx > 0) && (NULL == strstr(ps_path, "mmcblk"))&&
        ((ps_path[strlen(ps_path) - 1] >= '0')&& (ps_path[strlen(ps_path) - 1] <= '9')))
     {
         DM_INFO("[FM] %s: #%d: input not allowed like this: /dev/sda1 + 1\n", __FUNCTION__, __LINE__);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_ARG);
         return FMR_ARG;
     }

     memset(pt_part_info, 0, sizeof(FM_PART_INFO_T));
     i4_ret = fm_get_dir_label(h_dir, &pt_dir_lbl);
     if (FMR_OK != i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     i4_ret = fm_get_absolute_path(pt_dir_lbl, ps_path, &ps_dir_path);
     if (FMR_OK != i4_ret)
     {
         if (NULL != ps_dir_path)
         {
             free(ps_dir_path);
         }
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     if ((strlen(ps_dir_path)+1) >= (FM_MAX_PATH_LEN + 2))
     {
         free(ps_dir_path);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_NAME_TOO_LONG);
         return FMR_NAME_TOO_LONG;
     }

     i4_ret = fm_get_part_info(ps_dir_path,ui4_part_idx,pt_part_info);

     free(ps_dir_path);

     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);

     return i4_ret;
 }

 /*-----------------------------------------------------------------------------
  * Name: u_fm_create_dir_fdr
  *
  * Description: Create a new directory with ui4_mode.
  *
  * Inputs:  h_dir
  *              Handle of a directory label.
  *              FM_ROOT_HANDLE can be used to indicate the root directory.
  *              NULL_HANDLE is equivalent to FM_ROOT_HANDLE.
  *          ps_path
  *              Path name of the entry to be created. It can be a single
  *              name or a path. Note that if the first character of
  *              ps_path is '/', h_dir will be ignored and ps_path is
  *              regarded as absolute path. This argument cannot be NULL.
  *          ui4_mode
  *              Access permission of the new directory.
  * Outputs: -
  *
  * Returns: FMR_OK
  *              Success.
  *          FMR_ARG
  *              ps_path is NULL.
  *          FMR_HANDLE
  *              h_dir is an invalid handle.
  *          FMR_INVAL
  *              ps_path is an invalid path.
  *          FMR_NO_ENTRY
  *              ps_path is not found.
  *          FMR_EXIST
  *              ps_path already exists.
  *          FMR_NOT_DIR
  *              A component name in ps_path is not a directory.
  *          FMR_NAME_TOO_LONG
  *              Length of ps_path is too long.
  *          FMR_CORE
  *              OSAI error.
  ----------------------------------------------------------------------------*/
 INT32 u_fm_create_dir_fdr(HANDLE_T h_dir,const CHAR *ps_path,UINT32 ui4_mode)
 {
     FM_DIR_LABEL_T  *pt_dir_lbl  = NULL;
     CHAR            *ps_dir_path = NULL;
     INT32           i4_ret;
     CHAR ps_cmd[64];

     DM_INFO("[FM] %s: ""h_dir %lu, ""ps_path %s, ""ui4_mode %ld\n",  __FUNCTION__,h_dir,
                (ps_path == NULL) ? "NULL" : ps_path,ui4_mode);

     if (NULL == ps_path)
     {
         return FMR_ARG;
     }

     i4_ret = fm_get_dir_label(h_dir, &pt_dir_lbl);
     if (FMR_OK != i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     i4_ret = fm_get_absolute_path(pt_dir_lbl, ps_path, &ps_dir_path);
     if (FMR_OK != i4_ret)
     {
         if (NULL != ps_dir_path)
         {
             free(ps_dir_path);
         }
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     if ((strlen(ps_dir_path)+1) >= (FM_MAX_PATH_LEN + 2))
     {
         free(ps_dir_path);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_NAME_TOO_LONG);
         return FMR_NAME_TOO_LONG;
     }

     snprintf(ps_cmd, 64, "mkdir -p %s", ps_dir_path);
     if(access(ps_dir_path,0) < 0)
     {
         i4_ret = system(ps_cmd);
         if(0==i4_ret)
         {
             snprintf(ps_cmd, 64, "chmod %o %s", (unsigned)ui4_mode, ps_dir_path);
             system(ps_cmd);
         }
     }
     if (-1 == i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
     }
     else
     {
         i4_ret = FMR_OK;
     }

     free(ps_dir_path);

     if ((ui4_mode & 0022) != 0)
     {
         chmod(ps_path,ui4_mode & FM_MODE_PERM_MASK);
     }

     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);

     return i4_ret;
 }

 /*-----------------------------------------------------------------------------
  * Name: u_fm_create_dir
  *
  * Description: Create a new directory with ui4_mode.
  *
  * Inputs:  h_dir
  *              Handle of a directory label.
  *              FM_ROOT_HANDLE can be used to indicate the root directory.
  *              NULL_HANDLE is equivalent to FM_ROOT_HANDLE.
  *          ps_path
  *              Path name of the entry to be created. It can be a single
  *              name or a path. Note that if the first character of
  *              ps_path is '/', h_dir will be ignored and ps_path is
  *              regarded as absolute path. This argument cannot be NULL.
  *          ui4_mode
  *              Access permission of the new directory.
  * Outputs: -
  *
  * Returns: FMR_OK
  *              Success.
  *          FMR_ARG
  *              ps_path is NULL.
  *          FMR_HANDLE
  *              h_dir is an invalid handle.
  *          FMR_INVAL
  *              ps_path is an invalid path.
  *          FMR_NO_ENTRY
  *              ps_path is not found.
  *          FMR_EXIST
  *              ps_path already exists.
  *          FMR_NOT_DIR
  *              A component name in ps_path is not a directory.
  *          FMR_NAME_TOO_LONG
  *              Length of ps_path is too long.
  *          FMR_CORE
  *              OSAI error.
  ----------------------------------------------------------------------------*/
 INT32 u_fm_create_dir(HANDLE_T h_dir,const CHAR *ps_path,UINT32 ui4_mode)
 {
     FM_DIR_LABEL_T  *pt_dir_lbl  = NULL;
     CHAR            *ps_dir_path = NULL;
     INT32           i4_ret;

     DM_INFO("[FM] %s: ""h_dir %lu, ""ps_path %s, ""ui4_mode %lu\n",__FUNCTION__,  h_dir,
            (ps_path == NULL) ? "NULL" : ps_path,ui4_mode);

     if (NULL == ps_path)
     {
         return FMR_ARG;
     }

     i4_ret = fm_get_dir_label(h_dir, &pt_dir_lbl);
     if (FMR_OK != i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     i4_ret = fm_get_absolute_path(pt_dir_lbl, ps_path, &ps_dir_path);
     if (FMR_OK != i4_ret)
     {
         if (NULL != ps_dir_path)
         {
             free(ps_dir_path);
         }
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     if ((strlen(ps_dir_path)+1) >= (FM_MAX_PATH_LEN + 2))
     {
         free(ps_dir_path);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_NAME_TOO_LONG);
         return FMR_NAME_TOO_LONG;
     }

     i4_ret = mkdir(ps_dir_path, ui4_mode);
     if (-1 == i4_ret)
     {
         DM_ERR("[FM] %s: #%d: mkdir fail\n", __FUNCTION__, __LINE__);
     }
     else
     {
         i4_ret = FMR_OK;
     }

     free(ps_dir_path);

     if (0 != (ui4_mode & 0022))
     {
         chmod(ps_path,ui4_mode & FM_MODE_PERM_MASK);
     }

     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);

     return i4_ret;
 }

 /*-----------------------------------------------------------------------------
  * Name: u_fm_delete_dir_fdr
  *
  * Description: Delete an empty directory.
  *
  * Inputs:  h_dir
  *              Handle of a directory label.
  *          ps_path
  *              Path name of the entry to be removed.
  *
  * Outputs: -
  *
  * Returns: FMR_OK
  *              Success.
  *          FMR_HANDLE
  *              h_dir is an invalid handle.
  *          FMR_INVAL
  *              ps_path is an invalid path.
  *          FMR_NO_ENTRY
  *              ps_path is not found.
  *          FMR_NOT_DIR
  *              A component name in ps_path or ps_path is not a directory.
  *          FMR_NAME_TOO_LONG
  *              Length of ps_path is too long.
  *          FMR_DIR_NOT_EMPTY
  *              The directory contains file or directory.
  *          FMR_CORE
  *              OSAI error.
  ----------------------------------------------------------------------------*/
 INT32 u_fm_delete_dir_fdr(HANDLE_T h_dir,const CHAR *ps_path)
 {
     FM_DIR_LABEL_T  *pt_dir_lbl  = NULL;
     CHAR            *ps_dir_path = NULL;
     INT32           i4_ret;
     CHAR ps_cmd[64];

     DM_INFO("[FM] %s: ""h_dir %lu, ""ps_path %s\n",__FUNCTION__,h_dir,(ps_path == NULL) ? "NULL" : ps_path);

     i4_ret = fm_get_dir_label(h_dir, &pt_dir_lbl);
     if (FMR_OK != i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     i4_ret = fm_get_absolute_path(pt_dir_lbl, ps_path, &ps_dir_path);
     if (FMR_OK != i4_ret)
     {
         if (NULL != ps_dir_path)
         {
             free(ps_dir_path);
         }
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     if ((strlen(ps_dir_path)+1) >= (FM_MAX_PATH_LEN + 2))
     {
         free(ps_dir_path);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_NAME_TOO_LONG);
         return FMR_NAME_TOO_LONG;
     }

     snprintf(ps_cmd, 64, "rmdir %s", ps_dir_path);
     i4_ret = system(ps_cmd);
     if (-1 == i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
     }
     else
     {
         i4_ret = FMR_OK;
     }

     free(ps_dir_path);

     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
     return i4_ret;
 }

 /*-----------------------------------------------------------------------------
  * Name: u_fm_delete_dir
  *
  * Description: Delete an empty directory.
  *
  * Inputs:  h_dir
  *              Handle of a directory label.
  *          ps_path
  *              Path name of the entry to be removed.
  *
  * Outputs: -
  *
  * Returns: FMR_OK
  *              Success.
  *          FMR_HANDLE
  *              h_dir is an invalid handle.
  *          FMR_INVAL
  *              ps_path is an invalid path.
  *          FMR_NO_ENTRY
  *              ps_path is not found.
  *          FMR_NOT_DIR
  *              A component name in ps_path or ps_path is not a directory.
  *          FMR_NAME_TOO_LONG
  *              Length of ps_path is too long.
  *          FMR_DIR_NOT_EMPTY
  *              The directory contains file or directory.
  *          FMR_CORE
  *              OSAI error.
  ----------------------------------------------------------------------------*/
 INT32 u_fm_delete_dir(HANDLE_T h_dir,const CHAR *ps_path)
 {
     CHAR            *ps_dir_path = NULL;
     INT32           i4_ret;
     FM_DIR_LABEL_T  *pt_dir_lbl  = NULL;

     DM_INFO("[FM] %s: ""h_dir %lu, ""ps_path %s\n",__FUNCTION__, h_dir,(ps_path == NULL) ? "NULL" : ps_path);

     i4_ret = fm_get_dir_label(h_dir, &pt_dir_lbl);
     if (FMR_OK != i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     i4_ret = fm_get_absolute_path(pt_dir_lbl, ps_path, &ps_dir_path);
     if (FMR_OK != i4_ret)
     {
         if (NULL != ps_dir_path)
         {
             free(ps_dir_path);
         }
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     if ((strlen(ps_dir_path)+1) >= (FM_MAX_PATH_LEN + 2))
     {
         free(ps_dir_path);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_NAME_TOO_LONG);
         return FMR_NAME_TOO_LONG;
     }
     i4_ret = rmdir(ps_dir_path);
     if (-1 == i4_ret)
     {
         DM_ERR("[FM] %s: #%d: rmdir fail!!!!\n", __FUNCTION__, __LINE__);
     }
     else
     {
         i4_ret = FMR_OK;
     }

     free(ps_dir_path);

     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);

     return i4_ret;
 }

 /*-----------------------------------------------------------------------------
  * Name: fm_store_part_info
  *
  * Description:
  *    ps_part_name such as /dev or /dev/
  *    ps_part_path such as sda1
  *    ps_mnt_path  such as /mnt/cdrom   not /mnt/cdrom/
  *
  * Inputs:
  *
  * Outputs:
  *
  * Returns:
  ----------------------------------------------------------------------------*/
 INT32 fm_store_part_info(FM_PART_INFO_T  *pt_part_info)
 {
     FM_PART_INFO_DESC_T *pt_part_desc = NULL;

     if (NULL == pt_part_info)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_ARG);
         return FMR_ARG;
     }

     pt_part_desc = (FM_PART_INFO_DESC_T *)malloc(sizeof(FM_PART_INFO_DESC_T));
     if (NULL == pt_part_desc)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_CORE);
         return FMR_CORE;
     }
     memset(pt_part_desc, 0, sizeof(FM_PART_INFO_DESC_T));
     memcpy(&(pt_part_desc->t_part_info), pt_part_info, sizeof(FM_PART_INFO_T));

     if (RWLR_OK != u_rwl_write_lock(h_fm_part_info_rwlock, RWL_OPTION_WAIT))
     {
         free(pt_part_desc);
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_CORE);
         return FMR_CORE;
     }

     SLIST_INSERT_HEAD(pt_part_desc, &t_fm_part_info_list, t_link);

     u_rwl_release_lock(h_fm_part_info_rwlock);

     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_OK);
     return FMR_OK;
 }


 /*-----------------------------------------------------------------------------
  * Name: u_fm_mount
  *
  * Description: The u_fm_mount function attaches the file system on device
  *              ps_dev to the directory ps_path. File Manager will
  *              automatically recognize the file system type.
  *
  * Inputs:  h_dev_dir
  *              Handle of the directory label relative to ps_dev_path.
  *          ps_dev_path
  *              Path of the block device special file.
  *          h_mp_dir
  *              Handle of the directory label relative to ps_mp_path.
  *          ps_mp_path
  *              Path of the directory to mount. (Mount point)
  *
  * Outputs: -
  *
  * Returns: FMR_OK
  *              Success.
  *          FMR_ARG
  *              ps_dev_path or ps_mp_path is NULL.
  *          FMR_HANDLE
  *              h_dev_dir or h_mp_dir is an invalid handle.
  *          FMR_INVAL
  *              ps_dev_path or ps_mp_path is an invalid path.
  *              Or file system is not supported.
  *          FMR_NO_ENTRY
  *              ps_dev_path or ps_mp_path is not found.
  *          FMR_BUSY
  *              ps_dev_path is already mounted.
  *          FMR_NOT_DIR
  *              A component name in ps_dev_path or in ps_mp_path or ps_mp_path
  *              itself is not a directory.
  *          FMR_NAME_TOO_LONG
  *              Length of ps_dev_path or ps_mp_path is too long.
  *          FMR_CORE
  *              OSAI error.
  ----------------------------------------------------------------------------*/
 INT32 u_fm_mount(HANDLE_T       h_dev_dir,
                      const CHAR     *ps_dev_path,
                      HANDLE_T       h_mp_dir,
                      const CHAR     *ps_mp_path,
                      FM_MNT_INFO_T  *pt_mnt_info
                      )
 {
     FM_DIR_LABEL_T  *pt_dev_lbl           = NULL;
     FM_DIR_LABEL_T  *pt_mp_lbl            = NULL;
     CHAR            *ps_absolute_dev_path = NULL;
     CHAR            *ps_absolute_mp_path  = NULL;
     CHAR            *ps_last              = NULL;
     INT32           i4_last_index;
     INT32           i4_ret;
     INT32           i4_index;
     FM_MNT_COND_T   t_cond;
     FM_PART_INFO_T  *pt_part_info         = NULL;
     BOOL            b_sync                = TRUE;
     AMB_BROADCAST_MSG_T pmsg              = {0};

     CHAR s_type[4][32] = {"vfat", "ntfs", "ext3", "texfat"};

     DM_INFO("[FM] %s: ""h_dev_dir %lu,""ps_dev_path %s,""h_mp_dir %lu,""ps_mp_path %s\n",
                __FUNCTION__,h_dev_dir,
                (NULL == ps_dev_path) ? "NULL" : ps_dev_path,h_mp_dir,
                (NULL == ps_mp_path) ? "NULL" : ps_mp_path);

     i4_ret = fm_get_dir_label(h_dev_dir, &pt_dev_lbl);
     if (FMR_OK != i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     i4_ret = fm_get_absolute_path(pt_dev_lbl, ps_dev_path, &ps_absolute_dev_path);
     if (FMR_OK != i4_ret)
     {
         if (NULL != ps_absolute_dev_path)
         {
             free(ps_absolute_dev_path);
         }
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     i4_ret = fm_get_dir_label(h_mp_dir, &pt_mp_lbl);
     if (FMR_OK != i4_ret)
     {
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     i4_ret = fm_get_absolute_path(pt_mp_lbl, ps_mp_path, &ps_absolute_mp_path);
     if (FMR_OK != i4_ret)
     {
         free(ps_absolute_dev_path);
         if (NULL != ps_absolute_mp_path)
         {
             free(ps_absolute_mp_path);
         }
         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret;
     }

     _fm_record_mnt_path(ps_absolute_mp_path, pt_mnt_info);

     pt_part_info = (FM_PART_INFO_T *)malloc(sizeof(FM_PART_INFO_T));
     if (NULL == pt_part_info)
     {
         free(ps_absolute_dev_path);
         free(ps_absolute_mp_path);
         DM_ERR("[FM] %s: #%d: can not allocate memory for FM_PART_INFO_T\n", __FUNCTION__, __LINE__);
         return FMR_CORE;
     }
     memset(pt_part_info, 0, sizeof(FM_PART_INFO_T));

     u_sema_lock(h_fm_mnt_lock, X_SEMA_OPTION_WAIT);   //  it seems mount system call is not thread safe...

     i4_ret = -1;
     for (i4_index = 0; i4_index < 4; i4_index++)
     {
         INT32 i4_flag = 0;

         DM_INFO("[FM] %s: #%d: mount %s %s %s\n", __FUNCTION__, __LINE__, s_type[i4_index], ps_absolute_dev_path, ps_absolute_mp_path);

         if (0 == strcmp(s_type[i4_index], "vfat"))
         {
             CHAR ps_cmd[128];
             snprintf(ps_cmd, 128, "mount -t %s -o shortname=mixed,iocharset=utf8 %s %s",s_type[i4_index],ps_absolute_dev_path, ps_absolute_mp_path);
             i4_ret = system(ps_cmd);
             if(0 != i4_ret)
             {
                 memset(ps_cmd, 0, sizeof(ps_cmd));
                 snprintf(ps_cmd, 128, "mount -t %s -o ro,shortname=mixed,iocharset=utf8 %s %s",s_type[i4_index],ps_absolute_dev_path, ps_absolute_mp_path);
                 i4_ret = system(ps_cmd);
                 if (0 != i4_ret)
                 {
                     i4_ret = -1;
                     DM_ERR("[FM] %s: #%d: mount %s fail,return %ld\n", __FUNCTION__, __LINE__,s_type[i4_index],i4_ret);
                 }
                 else
                 {
                     DM_INFO("[FM] mount RO %s success.\n",s_type[i4_index]);
                     pt_mnt_info->e_mnt_type = FM_MNT_TYPE_VFAT;
                 }
             }
             else
             {
                DM_INFO("[FM] mount %s success.\n",s_type[i4_index]);
                pt_mnt_info->e_mnt_type = FM_MNT_TYPE_VFAT;
             }
         }
         else if(0 == strcmp(s_type[i4_index], "ntfs"))
         {
             CHAR ps_cmd[64];
             INT32 i4_sys_ret;

             snprintf(ps_cmd, 64, "mount -o nls=utf8 %s %s", ps_absolute_dev_path, ps_absolute_mp_path);
             i4_sys_ret = system(ps_cmd);
             DM_INFO("[FM] i4_sys_ret = %d\n", (int)i4_sys_ret);
             if (0 == i4_sys_ret)
             {
                 t_cond = FM_MNT_OK;
                 pt_part_info->b_mnt = 1;
                 i4_ret = 0;
                 pt_mnt_info->e_mnt_type = FM_MNT_TYPE_NTFS;
             }
             else
             {
                 t_cond = FM_MNT_FAIL;
                 pt_part_info->b_mnt = 0;
                 i4_ret = -1;
             }
         }
         else if (0 == strcmp(s_type[i4_index], "texfat"))
         {
             CHAR ps_cmd[64];
             INT32 i4_sys_ret;

             snprintf(ps_cmd, 64, "mount -t texfat %s %s", ps_absolute_dev_path, ps_absolute_mp_path);
             i4_sys_ret = system(ps_cmd);
             DM_INFO("[FM] i4_sys_ret = %d\n", (int)i4_sys_ret);
             if (0 == i4_sys_ret)
             {
                 t_cond = FM_MNT_OK;
                 pt_part_info->b_mnt = 1;
                 i4_ret = 0;
                 pt_mnt_info->e_mnt_type = FM_MNT_TYPE_TEXFAT;
             }
             else
             {
                 snprintf(ps_cmd, 64, "mount -t texfat -o iocharset=utf8 %s %s", ps_absolute_dev_path, ps_absolute_mp_path);
                 i4_sys_ret = system(ps_cmd);
                 DM_INFO("[FM] i4_sys_ret = %d\n", (int)i4_sys_ret);
                 if (0 == i4_sys_ret)
                 {
                     t_cond = FM_MNT_OK;
                     pt_part_info->b_mnt = 1;
                     i4_ret = 0;
                     pt_mnt_info->e_mnt_type = FM_MNT_TYPE_TEXFAT;
                 }
                 else
                 {
                     t_cond = FM_MNT_FAIL;
                     pt_part_info->b_mnt = 0;
                     i4_ret = -1;
                 }
             }
             /**PATCH END*/
         }
         else
         {
             CHAR ps_cmd[128];
             snprintf(ps_cmd, 128, "mount -t %s %s %s", s_type[i4_index],ps_absolute_dev_path, ps_absolute_mp_path);
             i4_ret = system(ps_cmd);
             if(0 != i4_ret)
             {
                 i4_ret = -1;
                 DM_ERR("[FM] %s: #%d: mount %s fail,return %ld\n", __FUNCTION__, __LINE__,s_type[i4_index],i4_ret);
             }
             else
             {
                 DM_INFO("[FM] mount %s success.\n",s_type[i4_index]);
                 pt_mnt_info->e_mnt_type = FM_MNT_TYPE_EXT3;
             }
         }

         if (-1 == i4_ret)
         {
             t_cond = FM_MNT_FAIL;
             pt_part_info->b_mnt = 0;
             DM_INFO("[FM] %s: #%d: b_mnt %d\n", __FUNCTION__, __LINE__, pt_part_info->b_mnt);
         }
         else
         {
             DM_INFO("[FM] %s: #%d: mounted as %s\n", __FUNCTION__, __LINE__, s_type[i4_index]);
             t_cond = FM_MNT_OK;
             pt_part_info->b_mnt = 1;
             DM_INFO("[FM] %s: #%d: b_mnt %d\n", __FUNCTION__, __LINE__, pt_part_info->b_mnt);
             break;
         }
     }

     u_sema_unlock(h_fm_mnt_lock);

     pt_part_info->b_try_mnt = 1;

     ps_last = strrchr(ps_absolute_dev_path, '/');

     if (0 == ps_last)
     {
         strncpy(pt_part_info->ps_part_name, ps_absolute_dev_path, strlen(ps_absolute_dev_path)+1);
         memset(pt_part_info->ps_part_path, 0, (sizeof(CHAR) * 32));
     }
     else
     {
         i4_last_index = (ps_absolute_dev_path - ps_last) / sizeof(CHAR);
         i4_last_index = i4_last_index < 0 ? -i4_last_index : i4_last_index;
         i4_last_index ++;

         strncpy(pt_part_info->ps_part_name, (ps_last + 1), strlen(ps_last+1)+1);
         if (i4_last_index >= 32)
         {
             free(ps_absolute_dev_path);
             free(ps_absolute_mp_path);
             free(pt_part_info);

             DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_NAME_TOO_LONG);
             return FMR_NAME_TOO_LONG;
         }
         strncpy(pt_part_info->ps_part_path, ps_absolute_dev_path, i4_last_index);
     }

     /* strip the last / in monut point */
     if ('/' == ps_absolute_mp_path[strlen(ps_absolute_mp_path) - 1])
     {
         ps_absolute_mp_path[strlen(ps_absolute_mp_path) - 1] = '\0';
     }

     strncpy(pt_part_info->ps_mnt_path, ps_absolute_mp_path, strlen(ps_absolute_mp_path)+1);

     DM_INFO("[FM] %s: #%d: ps_part_path: %s\n", __FUNCTION__, __LINE__, pt_part_info->ps_part_path);
     DM_INFO("[FM] %s: #%d: ps_part_name: %s\n", __FUNCTION__, __LINE__, pt_part_info->ps_part_name);
     DM_INFO("[FM] %s: #%d: ps_mnt_path : %s\n", __FUNCTION__, __LINE__, pt_part_info->ps_mnt_path);

     i4_ret = fm_store_part_info(pt_part_info);
     if (FMR_OK != i4_ret)
     {
         free(ps_absolute_dev_path);
         free(ps_absolute_mp_path);
         free(pt_part_info);

         DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
         return i4_ret; /* Should return of go on to notify? */
     }

     free(ps_absolute_dev_path);
     free(ps_absolute_mp_path);
     free(pt_part_info);

     DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
     return (FM_MNT_OK == t_cond ? FMR_OK : FMR_ARG);
}


/*-----------------------------------------------------------------------------
 * Name: fm_remove_part_info
 *
 * Description:
 *    ps_mnt_path  such as /mnt/cdrom   not /mnt/cdrom/
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 ----------------------------------------------------------------------------*/
INT32 fm_remove_part_info(const CHAR *ps_mnt_path)
{
    FM_PART_INFO_DESC_T *pt_part_desc = NULL;
    FM_PART_INFO_T      *pt_part_info = NULL;
    BOOL                b_found       = FALSE;

    DM_INFO("[FM] %s: ""ps_mnt_path %s\n",__FUNCTION__,(ps_mnt_path == NULL) ? "NULL" : ps_mnt_path);

    if (NULL == ps_mnt_path)
    {
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_ARG);
        return FMR_ARG;
    }

    if (RWLR_OK != u_rwl_write_lock(h_fm_part_info_rwlock, RWL_OPTION_WAIT))
    {
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_CORE);
        return FMR_CORE;
    }

    SLIST_FOR_EACH(pt_part_desc, &t_fm_part_info_list, t_link)
    {
        DM_INFO("[FM] %s: #%d: stored: %s\n", __FUNCTION__, __LINE__, pt_part_desc->t_part_info.ps_mnt_path);
        DM_INFO("[FM] %s: #%d: input:  %s\n", __FUNCTION__, __LINE__, ps_mnt_path);

        if (0 == strcmp(pt_part_desc->t_part_info.ps_mnt_path, ps_mnt_path))
        {
            DM_INFO("[FM] %s: #%d: found\n", __FUNCTION__, __LINE__);
            b_found = TRUE;
            pt_part_info = &(pt_part_desc->t_part_info);
            SLIST_REMOVE(pt_part_desc, t_link);
            break;
        }
    }

    u_rwl_release_lock(h_fm_part_info_rwlock);

    if (FALSE == b_found)
    {
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_ARG);
        return FMR_ARG;
    }

    free(pt_part_info);

    DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_OK);
    return FMR_OK;
}

/*-----------------------------------------------------------------------------
 * Name: u_fm_umount
 *
 * Description: Detach a mounted file system.
 *
 * Inputs: h_dir
 *              Handle of a directory label.
 *          ps_path
 *              Mount point.
 *
 * Outputs: -
 *
 * Returns: FMR_OK
 *              Success.
 *          FMR_ARG
 *              ps_path is NULL.
 ----------------------------------------------------------------------------*/
INT32 u_fm_umount(HANDLE_T  h_dir,const CHAR *ps_path)
{
    FM_DIR_LABEL_T  *pt_dir_lbl   = NULL;
    CHAR            *ps_dir_path  = NULL;
    INT32           i4_ret;
    BOOL            b_sync        = TRUE;
    AMB_BROADCAST_MSG_T pmsg      = {0};

    DM_INFO("[FM] %s: ""h_dir %lu, ""ps_path %s\n", __FUNCTION__,h_dir,(strlen(ps_path) == 0) ? "NULL" : ps_path);

    i4_ret = fm_get_dir_label(h_dir, &pt_dir_lbl);
    if (FMR_OK != i4_ret)
    {
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
        return i4_ret;
    }

    i4_ret = fm_get_absolute_path(pt_dir_lbl, ps_path, &ps_dir_path);
    if (FMR_OK != i4_ret)
    {
        if (NULL != ps_dir_path)
        {
            free(ps_dir_path);
        }
        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
        return i4_ret;
    }


    CHAR  ps_cmd[64];
    INT32 i4_ret_2;
    snprintf(ps_cmd, 64, "umount %s", ps_dir_path);
    i4_ret = system(ps_cmd);
    memset(ps_cmd, 0, sizeof(ps_cmd));
    snprintf(ps_cmd, 64, "umount -lvd %s", ps_dir_path);
    i4_ret_2 = system(ps_cmd);
    if ((0 != i4_ret) && (0 != i4_ret_2))
    {
        /* umount must success, so try again by command */
        CHAR   ps_cmd[128] = {0};
        CHAR   ps_cmd_2[128] = {0};
        INT32  i4_count = 0;
        INT32  i4_cmd_len = 0;
        INT32  i4_dir_path_len = 0;

        while((i4_ret != 0) && (i4_count < 5)) /* max try 5 times */
        {
            memset(ps_cmd, 0, sizeof(ps_cmd));
            memset(ps_cmd_2, 0, sizeof(ps_cmd_2));
            strncpy((CHAR *)ps_cmd, "umount ", sizeof("umount "));
            strncpy((CHAR *)ps_cmd_2, "umount -lvd ", sizeof("umount -lvd "));
            i4_cmd_len = strlen(ps_cmd);
            i4_dir_path_len = strlen(ps_dir_path);
            if ((SIZE_T)(i4_cmd_len + i4_dir_path_len) < sizeof(ps_cmd))
            {
                strncat(ps_cmd, ps_dir_path, i4_dir_path_len);
                strncat(ps_cmd_2, ps_dir_path, i4_dir_path_len);
                i4_ret = system((CHAR *)ps_cmd);
                i4_ret = system((CHAR *)ps_cmd_2);
                DM_INFO("[FM] %s: #%d: command: %s count: %ld i4_ret: %ld\n", __FUNCTION__, __LINE__, ps_cmd, i4_count, i4_ret);
                i4_count ++;
            }
            else
            {
                free(ps_dir_path);
                DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, FMR_NAME_TOO_LONG);
                return FMR_NAME_TOO_LONG;
            }
        }
    }

    /* strip the last / in monut point */
    if ('/' == ps_dir_path[strlen(ps_dir_path) - 1])
    {
        ps_dir_path[strlen(ps_dir_path) - 1] = '\0';
    }

    i4_ret = fm_remove_part_info(ps_dir_path);
    if (FMR_OK != i4_ret)
    {
        free(ps_dir_path);

        DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
        return i4_ret; /* Should return of go on to notify? */
    }

    free(ps_dir_path);

    DM_INFO("[FM] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret);
    return FMR_OK;
}

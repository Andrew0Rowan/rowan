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
                    include files
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "u_common.h"
#include "u_os.h"
#include "u_dbg.h"
#include "u_handle.h"

#include "acfg.h"

/*-----------------------------------------------------------------------------
                    macro, define
-----------------------------------------------------------------------------*/
static HANDLE_T h_flash_sema_mtx = NULL_HANDLE;

static UINT8*  pui1_flash_cache_adjust = NULL;
static UINT8*  pui1_flash_cache = NULL;

static UINT32  ui4_total_size = 0;


static VOID _flash_lock(VOID)
{
    INT32 i4_ret = APP_CFGR_OK;

    i4_ret = u_sema_lock(h_flash_sema_mtx, X_SEMA_OPTION_WAIT);
    if (0 != i4_ret)
    {
        DBG_ACFG_ERR("Err: Call API u_sema_lock fail\n");
    }
}

static VOID _flash_unlock(VOID)
{
    INT32 i4_ret = APP_CFGR_OK;

    i4_ret = u_sema_unlock(h_flash_sema_mtx);
    if (0 != i4_ret)
    {
        DBG_ACFG_ERR("Err: Call API u_sema_unlock fail\n");
    }
}

static INT32 _acfg_flash_load(UINT16 ui2_total_num)
{
    UINT8   ui1_size_of_type[5] =
    {
        0,
        1,
        2,
        4,
        8
    };

    FILE *fp_acfg = NULL;
    FILE *fp_acfg_backup = NULL;
    INT32 i4_ret = APP_CFGR_OK;
    ACFG_HEADER_T t_acfg_header = {0};

    UINT8 * pui1_tmp_cache = NULL;
    UINT16 ui2_i = 0;
    UINT16 ui2_j = 0;

    UINT32 ui4_rec_off = 0;
    UINT16 ui2_rec_type = 0;
    UINT16 ui2_rec_num = 0;
    UINT32 ui4_rec_size = 0;
    UINT8 * pui1_rec_data = NULL;
    UINT8 * pui1_rec_name = NULL;

    UINT32 ui4_cache_off = 0;
    UINT16 ui2_cache_rec_type = 0;
    UINT16 ui2_cache_rec_num = 0;
    UINT32 ui4_cache_rec_size = 0;

    DBG_ACFG_API("Loading acfg data!\n");

    if(0 != access(ACFG_CONFIG_SAVE_PATH, F_OK))//if file not exist, create it
    {
        UINT8 u1_i;
        DBG_ACFG_API("Create %s\n", ACFG_CONFIG_SAVE_PATH);
        //acfg_flash_store will create file, no need to create first
        //if the set power down before acfg_flash_store, the file size will be 0.
#if 0
        //create file
        fp_acfg = fopen(ACFG_CONFIG_SAVE_PATH, "wb");
        if(NULL == fp_acfg)
        {
            DBG_ACFG_ERR("Err: Fail to create %s, errno:%s\n",
                ACFG_CONFIG_SAVE_PATH,
                strerror(errno));
            return APP_CFGR_CANT_INIT;
        }

        fclose(fp_acfg);
        
        sync();

        //create backup file
        if(0 != access(ACFG_CONFIG_SAVE_PATH_BACKUP, F_OK))
        {
            fp_acfg_backup = fopen(ACFG_CONFIG_SAVE_PATH_BACKUP, "wb");
            if(NULL == fp_acfg_backup)
            {
                DBG_ACFG_ERR("Err: Fail to create %s, errno:%s\n",
                    ACFG_CONFIG_SAVE_PATH_BACKUP,
                    strerror(errno));
                return APP_CFGR_CANT_INIT;
            }
            
            fclose(fp_acfg_backup);
        }
        
        sync();
#endif
        /* write back to flash */
        acfg_flash_store();

        return APP_CFGR_OK;
    }

    fp_acfg = fopen(ACFG_CONFIG_SAVE_PATH, "rb");
    if (NULL == fp_acfg)
    {
        DBG_ACFG_ERR("Err: open %s fail!\r\n", ACFG_CONFIG_SAVE_PATH);
        return APP_CFGR_CANT_INIT;
    }

    //add backup method
    if(EOF == fgetc(fp_acfg))
    {
        printf("if app.cfg is empty, start backup!\n");
        fclose(fp_acfg);

        fp_acfg = fopen(ACFG_CONFIG_SAVE_PATH_BACKUP, "rb");
        if (NULL == fp_acfg)
        {
            DBG_ACFG_ERR("Err: open %s fail!\r\n", ACFG_CONFIG_SAVE_PATH_BACKUP);
            return APP_CFGR_CANT_INIT;
        }
        //if backup file is also empty, write default value back
        if(EOF == fgetc(fp_acfg))
        {
            DBG_ACFG_ERR("if app.cfg.backup is also empty, write back default value!\n");
            fclose(fp_acfg);
            
            acfg_flash_store();
            return APP_CFGR_OK;    
        }
    }

    //seek to the file header
    i4_ret = fseek(fp_acfg, 0, SEEK_SET);
    if (i4_ret != 0)
    {
        DBG_ACFG_ERR("Err: Fail to seek file, err:%s\n", strerror(errno));
        //fclose(fp_acfg);
        //return APP_CFGR_INTERNAL_ERR;
    }

    //read the file header
    i4_ret = fread(&t_acfg_header, 1, ACFG_HEADER_SIZE, fp_acfg);
    if (i4_ret != ACFG_HEADER_SIZE)
    {
        DBG_ACFG_ERR("Err: Fail to read file header(%d), err:%s\n",
            i4_ret,
            strerror(errno));
        i4_ret = APP_CFGR_INTERNAL_ERR;
        goto err_out;
    }

    DBG_ACFG_INFO("Verify file magic number.\n");
    if (ACFG_MAGIC_NUMBER != t_acfg_header.ui4_magic)
    {
        /* magic number error, the flash data is dirty, used default value */
        DBG_ACFG_ERR("Err: magic number is wrong.\r\n");
        fclose(fp_acfg);

        /* write back to flash */
        acfg_flash_store();

        return APP_CFGR_OK;
    }

    pui1_tmp_cache = (UINT8 *)malloc(
        (t_acfg_header.ui4_rec_size + ACFG_HEADER_SIZE) * sizeof(UINT8));

    if (NULL == pui1_tmp_cache)
    {
        DBG_ACFG_ERR("Err: Not enough memory.\r\n");
        i4_ret = APP_CFGR_INTERNAL_ERR;
        goto err_out;
    }

    memset(
        pui1_tmp_cache,
        0,
        (t_acfg_header.ui4_rec_size + ACFG_HEADER_SIZE) * sizeof(UINT8));

    //goto file header
    i4_ret = fseek(fp_acfg, 0, SEEK_SET);
    if (i4_ret != 0)
    {
        DBG_ACFG_ERR("Err: Fail to seek file, err:%s\n", strerror(errno));
        i4_ret = APP_CFGR_INTERNAL_ERR;
        goto err_out;
    }

    //read the whole file
    i4_ret = fread(
        pui1_tmp_cache,
        1,
        (ACFG_HEADER_SIZE + t_acfg_header.ui4_rec_size),
        fp_acfg);
    if (i4_ret != (ACFG_HEADER_SIZE + t_acfg_header.ui4_rec_size))
    {
        DBG_ACFG_ERR("Err: Fail to read file content(%d), err:%s\n",
            i4_ret,
            strerror(errno));
        i4_ret = APP_CFGR_INTERNAL_ERR;
        goto err_out;
    }

    DBG_ACFG_INFO("Copy record data to cache.\n");
    ui4_rec_off = ACFG_HEADER_SIZE;

    /* Load ACFG record from temp cache(flash) to cache */
    for(ui2_i = 0; ui2_i < t_acfg_header.ui4_rec_num ; ui2_i++)
    {
        ui2_rec_type = GET_UINT16_FROM_PTR_LITTLE_END(
            pui1_tmp_cache + ui4_rec_off + ACFG_REC_NAME_SIZE);
        ui2_rec_num = GET_UINT16_FROM_PTR_LITTLE_END(
            pui1_tmp_cache + ui4_rec_off + ACFG_REC_NAME_SIZE + ACFG_REC_TYPE_SIZE);
        ui4_rec_size = ui1_size_of_type[ui2_rec_type] * ui2_rec_num;
        pui1_rec_name = pui1_tmp_cache + ui4_rec_off;
        pui1_rec_data = pui1_tmp_cache + ui4_rec_off + ACFG_REC_INFO_SIZE;

        ui4_cache_off = ACFG_HEADER_SIZE;
        for (ui2_j = 0; ui2_j < ui2_total_num ; ui2_j++)
        {
            ui2_cache_rec_type = GET_UINT16_FROM_PTR_LITTLE_END(
                pui1_flash_cache_adjust + ui4_cache_off + ACFG_REC_NAME_SIZE);
            ui2_cache_rec_num = GET_UINT16_FROM_PTR_LITTLE_END(
                pui1_flash_cache_adjust + ui4_cache_off + ACFG_REC_NAME_SIZE + ACFG_REC_TYPE_SIZE);
            ui4_cache_rec_size = ui1_size_of_type[ui2_cache_rec_type] * ui2_cache_rec_num;

            if (0 == memcmp(
                    pui1_rec_name,
                    pui1_flash_cache_adjust + ui4_cache_off,
                    ACFG_REC_NAME_SIZE))
            {
                /* Type and Num field equal */
                if ((ui2_cache_rec_type == ui2_rec_type)
                     && (ui2_cache_rec_num == ui2_rec_num))
                {
                    /* load data */
                    memcpy(
                        pui1_flash_cache_adjust + ui4_cache_off + ACFG_REC_INFO_SIZE,   /* cache data */
                        pui1_rec_data,     /* flash data */
                        ui4_cache_rec_size);
                }

                break;
            }

            /* update ui4_cache_off */
            ui4_cache_off += (ACFG_REC_INFO_SIZE + ui4_cache_rec_size);
        }

        /* update ui4_flash_off */
        ui4_rec_off += (ACFG_REC_INFO_SIZE + ui4_rec_size);
    }

    DBG_ACFG_INFO("Load acfg data success.\n");
    i4_ret = APP_CFGR_OK;

err_out:

    fclose(fp_acfg);

    if (NULL != pui1_tmp_cache)
    {
        free(pui1_tmp_cache);
        pui1_tmp_cache = NULL;
    }

    return i4_ret;

}


/*-----------------------------------------------------------------------------
 * Name: acfg_flash_store
 *
 * Description:
 *      Store the config data to flash.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
INT32 acfg_flash_store_backup(VOID)
{
    INT32 i4_ret = APP_CFGR_OK;
    FILE * fp_acfg_backup = NULL;

    _flash_lock();

    DBG_ACFG_API("Start save data to flash for backup.\n");

    /* Write ACFG data from cach to flash just for backup*/
    fp_acfg_backup = fopen(ACFG_CONFIG_SAVE_PATH_BACKUP, "wb+");
    if (NULL == fp_acfg_backup)
    {
        DBG_ACFG_ERR("Err: open %s fail!\r\n", ACFG_CONFIG_SAVE_PATH_BACKUP);
        _flash_unlock();
        return APP_CFGR_INTERNAL_ERR;
    }

    i4_ret = fwrite(pui1_flash_cache_adjust, 1, ui4_total_size, fp_acfg_backup);
    if (i4_ret != ui4_total_size)
    {
        DBG_ACFG_ERR("Err: Fail to write file(%d), err:%s\n", i4_ret,strerror(errno));
        i4_ret = APP_CFGR_INTERNAL_ERR;
        goto err_out;
    }

    //flush data to disk
    fflush(fp_acfg_backup);
    fsync(fileno(fp_acfg_backup));

    DBG_ACFG_API("Save data to flash success for backup!\n");

    i4_ret = APP_CFGR_OK;

err_out:

    fclose(fp_acfg_backup);
    _flash_unlock();

    return i4_ret;
}

INT32 acfg_flash_store(VOID)
{
    INT32 i4_ret = APP_CFGR_OK;
    FILE * fp_acfg = NULL;

    _flash_lock();

    DBG_ACFG_API("Start save data to flash.\n");

    /* Write ACFG data from cach to flash */
    fp_acfg = fopen(ACFG_CONFIG_SAVE_PATH, "wb+");
    if (NULL == fp_acfg)
    {
        DBG_ACFG_ERR("Err: open %s fail!\r\n", ACFG_CONFIG_SAVE_PATH);
        _flash_unlock();
        return APP_CFGR_INTERNAL_ERR;
    }

    i4_ret = fwrite(pui1_flash_cache_adjust, 1, ui4_total_size, fp_acfg);
    if (i4_ret != ui4_total_size)
    {
        DBG_ACFG_ERR("Err: Fail to write file(%d), err:%s\n", i4_ret,strerror(errno));
        i4_ret = APP_CFGR_INTERNAL_ERR;
        goto err_out;
    }

    //flush data to disk
    fflush(fp_acfg);
    fsync(fileno(fp_acfg));

    DBG_ACFG_API("Save data to flash success!\n");

    i4_ret = APP_CFGR_OK;

err_out:

    fclose(fp_acfg);
    _flash_unlock();

    //add backup
    acfg_flash_store_backup();

    return i4_ret;
}


/*-----------------------------------------------------------------------------
 * Name: acfg_flash_get
 *
 * Description:
 *      This function get the config data for given index. If it fails to get
 * the config data, it will get the default value.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
INT32 acfg_flash_get(UINT16 ui2_idx, VOID* pt_values, SIZE_T* pz_size)
{
    INT32 i4_ret = APP_CFGR_OK;
    SIZE_T z_size = 0;
    ACFG_DESCR_T* pt_acfg;

    _flash_lock();

    pt_acfg = acfg_get_item(ui2_idx);
    if (pt_acfg == NULL)
    {
        i4_ret = APP_CFGR_INTERNAL_ERR;
        DBG_ACFG_ERR("Err: invalid param\n");
        goto err_out;
    }

    /* calculate the total size */
    switch (pt_acfg->e_type)
    {
        case CFG_8BIT_T:
        case CFG_16BIT_T:
        case CFG_32BIT_T:
        case CFG_64BIT_T:
            z_size = pt_acfg->ui2_num_elem * pt_acfg->e_type;
            break;
        default:
            i4_ret = APP_CFGR_INTERNAL_ERR;
            DBG_ACFG_ERR("Err: invalid type\n");
            goto err_out;
    }

    /* Get from cache is ok because the data in cache is up-to-date */
    memcpy(pt_values,
            pui1_flash_cache_adjust + pt_acfg->ui4_cache_off,
            z_size);

    *pz_size = z_size;

err_out:
    _flash_unlock();

    return APP_CFGR_OK;

}

/*-----------------------------------------------------------------------------
 * Name: acfg_flash_set
 *
 * Description:
 *      This function set the config data to config for given index.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
INT32 acfg_flash_set(UINT16 ui2_idx, VOID* pt_values, UINT16 ui2_elem_num)
{
    INT32 i4_ret = APP_CFGR_OK;
    SIZE_T z_size = 0;
    ACFG_DESCR_T* pt_acfg;

    _flash_lock();

    pt_acfg = acfg_get_item(ui2_idx);

    if ((pt_acfg == NULL) || (ui2_elem_num > pt_acfg->ui2_num_elem))
    {
        i4_ret = APP_CFGR_INTERNAL_ERR;
        DBG_ACFG_ERR("Err: invalid param\n");
        goto err_out;
    }

    /* calculate the total size */
    switch (pt_acfg->e_type)
    {
        case CFG_8BIT_T:
        case CFG_16BIT_T:
        case CFG_32BIT_T:
        case CFG_64BIT_T:
            z_size = ui2_elem_num * pt_acfg->e_type;
            break;
        default:
            i4_ret = APP_CFGR_INTERNAL_ERR;
            DBG_ACFG_ERR("Err: invalid type\n");
            goto err_out;
    }

    /* set the value to cache at first */
    memcpy(pui1_flash_cache_adjust + pt_acfg->ui4_cache_off,
            pt_values,
            z_size);

err_out:
    _flash_unlock();

    return i4_ret;

}

/*---------------------------------------------------------------------
 * Name: acfg_flash_init
 *
 * Description:
 *      Initiate the configuration in flash.
 *
 * Returns:
 *    APP_CFGR_OK        - Successful
 *    APP_CFGR_CANT_INIT - Init fail.
 *
 --------------------------------------------------------------------*/
INT32 acfg_flash_init(VOID)
{
    INT32 i4_ret = APP_CFGR_OK;
    UINT16 ui2_i = 0;
    UINT16 ui2_total_num = 0;
    UINT32 ui4_cache_off = 0;
    UINT32 ui4_rec_size = 0;
    ACFG_HEADER_T t_acfg_header = {0};
    ACFG_DESCR_T* pt_acfg;

    DBG_ACFG_API("acfg_flash_init\n");

    /* allocate mutex for flash*/
    i4_ret = u_sema_create(&h_flash_sema_mtx,
                            X_SEMA_TYPE_MUTEX,
                            X_SEMA_STATE_UNLOCK);
    ACFG_CHK_FAIL(i4_ret,APP_CFGR_CANT_INIT);


    /* caculate the total size and allocate memory for cache */
    ui4_total_size = ACFG_HEADER_SIZE;

    for(ui2_i = 0; ui2_i < IDX_MAX; ui2_i ++)
    {
        pt_acfg = acfg_get_item(ui2_i);
        if(pt_acfg->b_enable == TRUE)
        {
            ui4_total_size += pt_acfg->e_type * pt_acfg->ui2_num_elem + ACFG_REC_INFO_SIZE;
            ui2_total_num ++;
        }
    }

    pui1_flash_cache = (UINT8 *)malloc((ui4_total_size + 0x1f) * sizeof(UINT8));

    if (NULL == pui1_flash_cache)
    {
        DBG_ACFG_ERR("Err: Not enough memory\n");
        return APP_CFGR_INTERNAL_ERR;
    }

    memset(pui1_flash_cache,
            0,
            (ui4_total_size + 0x1f) * sizeof(UINT8));

    //Used right alighment for FM write
    pui1_flash_cache_adjust = (UINT8 *)(((unsigned long)pui1_flash_cache + 0x1f) & (~(0x1f)));

    /* Initialize the cache data refer to ACFG structure user difined */
    ui4_cache_off = ACFG_HEADER_SIZE;

    t_acfg_header.ui4_magic = ACFG_MAGIC_NUMBER;
    t_acfg_header.ui4_rec_num = ui2_total_num;
    t_acfg_header.ui4_rec_size = ui4_total_size - ACFG_HEADER_SIZE;

    memcpy(pui1_flash_cache_adjust,
            &t_acfg_header,
            ACFG_HEADER_SIZE);

    for(ui2_i = 0; ui2_i < IDX_MAX; ui2_i++)
    {
        pt_acfg = acfg_get_item(ui2_i);
        if(pt_acfg->b_enable == TRUE)
        {
            /* fill record name */
            memcpy(
                pui1_flash_cache_adjust + ui4_cache_off,
                pt_acfg->ac_name,
                ACFG_REC_NAME_SIZE);

            /* fill record type */
            memcpy(
                pui1_flash_cache_adjust + ui4_cache_off + ACFG_REC_NAME_SIZE,
                &pt_acfg->e_type,
                ACFG_REC_TYPE_SIZE);

            /* fill record number */
            memcpy(
                pui1_flash_cache_adjust + ui4_cache_off + ACFG_REC_NAME_SIZE + ACFG_REC_TYPE_SIZE,
                &pt_acfg->ui2_num_elem,
                ACFG_REC_NUM_SIZE);

            /* fill default data*/
            ui4_rec_size = pt_acfg->e_type * pt_acfg->ui2_num_elem;

            if (pt_acfg->ui2_def_num <= pt_acfg->ui2_num_elem)
            {
                memcpy(pui1_flash_cache_adjust + ui4_cache_off + ACFG_REC_INFO_SIZE,
                        pt_acfg->pt_def_value,
                        pt_acfg->ui2_def_num * pt_acfg->e_type);
            }
            else
            {
                memcpy(pui1_flash_cache_adjust + ui4_cache_off + ACFG_REC_INFO_SIZE,
                        pt_acfg->pt_def_value,
                        ui4_rec_size);
            }

            /* fill cache data offset */
            pt_acfg->ui4_cache_off = ui4_cache_off + ACFG_REC_INFO_SIZE;

            /* update cache offset */
            ui4_cache_off += (ACFG_REC_INFO_SIZE + ui4_rec_size);
        }
    }

    /* Load ACFG data from Flash to cache */
    i4_ret = _acfg_flash_load(ui2_total_num);
    ACFG_CHK_FAIL(i4_ret,APP_CFGR_INTERNAL_ERR);

    return APP_CFGR_OK;
}

INT32 acfg_flash_uninit(VOID)
{
    if (NULL != pui1_flash_cache)
    {
        free(pui1_flash_cache);
        pui1_flash_cache = NULL;
    }

    u_sema_delete(h_flash_sema_mtx);

    return APP_CFGR_OK;
}



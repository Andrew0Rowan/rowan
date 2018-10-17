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
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <alsa/asoundlib.h>

#include "u_common.h"
#include "u_appman.h"
#include "u_amb.h"
#include "u_os.h"
#include "u_timerd.h"

#include "u_cli.h"
#include "u_dbg.h"
#include "u_app_thread.h"

#include "acfg.h"
#include "u_app_def.h"

#define SET_UID_DELAY_MS (60 * 1000)

#define BOOT_VOLUME_MAX 80
#define BOOT_VOLUME_MIN 20

/*-----------------------------------------------------------------------------
                    external function
-----------------------------------------------------------------------------*/
extern INT32 acfg_grp_system_init(VOID);
/*-----------------------------------------------------------------------------
                    data declarations
-----------------------------------------------------------------------------*/
snd_hctl_t *g_pt_hctl;
static snd_hctl_elem_t *g_pt_helem;

static BOOL  b_g_app_cfg_init = FALSE;
static HANDLE_T h_g_acfg_app = NULL_HANDLE;

static BOOL b_have_store_request = FALSE;
static HANDLE_T h_store_timer = NULL_HANDLE;
static TIMER_TYPE_T g_t_set_uid_timer = {0};
/*------------------------------*/
/*  Config description tables   */
/*------------------------------*/
static ACFG_DESCR_T    at_acfg_descr[IDX_MAX]={0};

/*----------------*/
/*  ACFG Notify   */
/*----------------*/
static HANDLE_T         h_nty_sema_mtx;
//static ACFG_NFY_REC_T   at_acfg_nfy_recs[ACFG_NFY_MAX_NUM]={0};
static UINT16           ui2_acfg_nfy_rec_max_idx = 0;

/*------------------------------*/
/*  Time measurement            */
/*------------------------------*/
static UINT32 ui4_tms_start;
static UINT32 ui4_tms_stop;


static VOID _acfg_tms_reset(VOID)
{
    struct timespec ts_event;
    DBG_ACFG_INFO("reset acfg tms.\n");
    clock_gettime(CLOCK_MONOTONIC, &ts_event);
    ui4_tms_start = ts_event.tv_sec*1000 + ts_event.tv_nsec/1000000;//ms
}

static VOID _acfg_tms_spot(char* ps_log)
{
    struct timespec ts_event;
    clock_gettime(CLOCK_MONOTONIC, &ts_event);
    ui4_tms_stop = ts_event.tv_sec*1000 + ts_event.tv_nsec/1000000;//ms
    DBG_ACFG_INFO("%s in %dms\n", ps_log,
        (ui4_tms_stop>ui4_tms_start)?(ui4_tms_stop-ui4_tms_start):(0xFFFFFFFF-ui4_tms_start+ui4_tms_stop));
}

/*-----------------------------------------------------------------------------
                    private functions implementations
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: _acfg_validate
 *
 * Description:
 *      This function validate data for given index.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static BOOL _acfg_validate(UINT16 ui2_idx, VOID* pt_value, SIZE_T z_size)
{
    ACFG_ASSERT(ui2_idx < IDX_MAX);
    ACFG_ASSERT(pt_value != NULL);

    if (at_acfg_descr[ui2_idx].pf_validate != NULL)
    {
        return at_acfg_descr[ui2_idx].pf_validate(at_acfg_descr[ui2_idx].ui2_id,
                                                  pt_value,
                                                  z_size);
    }
    else
    {
        return TRUE;
    }
}

/*-----------------------------------------------------------------------------
 * Name
 *      _acfg_fire_nfy
 * Description
 *      Fire the update notification. This function should be call when acfg
 * is locked.
 *
 * Input arguments
 *
 * Output arguments
 *
 * Returns
 -----------------------------------------------------------------------------*/
static VOID _acfg_fire_nfy(UINT16 ui2_id)
{
    UINT16  ui2_i = 0;

    UINT16  ui2_group = CFG_GET_GROUP(ui2_id);

    for(ui2_i=0; ui2_i < ui2_acfg_nfy_rec_max_idx; ui2_i++)
    {
        if (ui2_i >= ACFG_NFY_MAX_NUM)
        {
            DBG_ACFG_ERR("Err: index out of range for ACFG update notification.\n");
            return;
        }
        #if 0
        if(at_acfg_nfy_recs[ui2_i].b_used
            && (at_acfg_nfy_recs[ui2_i].ui2_group == ui2_group))
        {
            at_acfg_nfy_recs[ui2_i].pf_acfg_notify(ui2_i,
                                                at_acfg_nfy_recs[ui2_i].pv_tag,
                                                ui2_id);
        }
        #endif
    }
}
/*-----------------------------------------------------------------------------
 * Name: _acfg_nty_lock
 *
 * Description:
 *      The acfg lock function
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static INT32 _acfg_nty_lock(VOID)
{
    return (u_sema_lock(h_nty_sema_mtx, X_SEMA_OPTION_WAIT) == OSR_OK) ?
        APP_CFGR_OK :
        APP_CFGR_INTERNAL_ERR;
}

/*-----------------------------------------------------------------------------
 * Name: _acfg_nty_unlock
 *
 * Description:
 *      The acfg unlokc function
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
static INT32 _acfg_nty_unlock(VOID)
{
    return (u_sema_unlock(h_nty_sema_mtx) == OSR_OK) ?
        APP_CFGR_OK :
        APP_CFGR_INTERNAL_ERR;
}


INT32 _acfg_store_request(VOID)
{
    INT32   i4_ret;

    DBG_ACFG_INFO("acfg store request.\n");

    if (!b_g_app_cfg_init)
    {
        DBG_ACFG_ERR("Err: APP not init.\n");
        return APP_CFGR_OK;
    }

    if (!b_have_store_request)
    {
        b_have_store_request = TRUE;
        i4_ret = acfg_send_msg_to_self(ACFG_PRI_MSG_STORE);
        if (i4_ret != AEER_OK)
        {
            b_have_store_request = FALSE;
            DBG_ACFG_ERR("Err: send msg to self fail.\n");
            return APP_CFGR_CAI_INTL_ERR;
        }
    }

    return APP_CFGR_OK;
}

static INT32 _acfg_start_store_timer(VOID)
{
    INT32 i4_ret = APP_CFGR_OK;
    TIMER_TYPE_T my_timer;

    DBG_ACFG_INFO("start store timer.\n");

    i4_ret = u_timer_stop(h_store_timer);
    ACFG_LOG_ON_FAIL(i4_ret);

    my_timer.h_timer = h_store_timer;
    my_timer.e_flags = X_TIMER_FLAG_ONCE;
    my_timer.ui4_delay = ACFG_STORE_TIMEOUT;

    ACFG_TIMER_MSG_E timer_type = ACFG_TIMER_FLASH_STORE_REPEATER;
    i4_ret = u_timer_start(h_g_acfg_app, &my_timer, &timer_type, sizeof(ACFG_TIMER_MSG_E));
    ACFG_CHK_FAIL(i4_ret,i4_ret);

    return APP_CFGR_OK;
}


INT32 u_acfg_start_set_uid_timer(VOID)
{
    INT32 i4_ret = APP_CFGR_OK;

    DBG_ACFG_INFO("start set uid timer.\n");

    /* create timer for set uid */
    i4_ret = u_timer_create(&g_t_set_uid_timer.h_timer);
    if (OSR_OK != i4_ret)
    {
        DBG_ACFG_ERR("u_timer_create h_set_uid_timer failed! ret:%d\n", i4_ret);
        return APP_CFGR_INTERNAL_ERR;
    }

    g_t_set_uid_timer.e_flags = X_TIMER_FLAG_ONCE;
    g_t_set_uid_timer.ui4_delay = SET_UID_DELAY_MS;

    ACFG_TIMER_MSG_E timer_type = ACFG_TIMER_SET_UID_REPEATER;

    i4_ret = u_timer_start(h_g_acfg_app, &g_t_set_uid_timer, &timer_type, sizeof(ACFG_TIMER_MSG_E));
    ACFG_CHK_FAIL(i4_ret,i4_ret);

    return APP_CFGR_OK;
}


static VOID _acfg_post_init(VOID)
{
    UINT16 ui2_i;

    DBG_ACFG_API("_acfg_post_init()\n");

    /* Call all the update hook if any */
    for (ui2_i = 0; ui2_i < IDX_MAX; ui2_i++)
    {
        if(at_acfg_descr[ui2_i].pf_update != NULL)
        {
            at_acfg_descr[ui2_i].pf_update(at_acfg_descr[ui2_i].ui2_id);
        }
    }

    _acfg_tms_spot("Post init done");
}


static VOID _acfg_timer_proc(VOID)
{
    //only for flash store
    acfg_send_msg_to_self(ACFG_PRI_MSG_FLASH_STORE);
}

static INT32 _acfg_pri_msg_proc(VOID* pv_msg)
{
    UINT32 ui4_msg = *(UINT32*)pv_msg;
    INT32 i4_ret;

    switch(ui4_msg)
    {
        case ACFG_PRI_MSG_STORE:
            b_have_store_request = FALSE;//check it could be delete or not

            /* start the timer to store */
            i4_ret = _acfg_start_store_timer();
            ACFG_CHK_FAIL(i4_ret,i4_ret);
            break;

        case ACFG_PRI_MSG_FLASH_STORE:
            i4_ret = acfg_store();
            ACFG_CHK_FAIL(i4_ret,i4_ret);
            break;

        default:
            DBG_ACFG_ERR("Err: unknow pri msg.\n");
            break;
    }

    return 0;
}

INT32 _acfg_alsa_ctrl_init(VOID)
{
    INT32 i4_ret;

    const char *sz_elem_str = "name='tas5751 Master Volume'";
    snd_ctl_elem_id_t *pt_id;

    snd_ctl_elem_id_alloca(&pt_id);

    i4_ret = snd_hctl_open(&g_pt_hctl, "default", 0);
    if (i4_ret < 0)
    {
        DBG_ACFG_ERR("snd_hctl_open error:%s\n", snd_strerror(i4_ret));
        return i4_ret;
    }

    i4_ret = snd_hctl_load(g_pt_hctl);
    if (i4_ret < 0)
    {
        DBG_ACFG_ERR("snd_hctl_load error:%s\n", snd_strerror(i4_ret));
        goto EXIT;
    }

    i4_ret = snd_ctl_ascii_elem_id_parse(pt_id, sz_elem_str);
    if (i4_ret < 0)
    {
        DBG_ACFG_ERR("snd_ctl_ascii_elem_id_parse error:%s\n", snd_strerror(i4_ret));
        goto EXIT;
    }

    g_pt_helem = snd_hctl_find_elem(g_pt_hctl, pt_id);
    if (!g_pt_helem)
    {
        DBG_ACFG_ERR("snd_hctl_find_elem error:%s\n", snd_strerror(i4_ret));
        i4_ret = -EINVAL;
        goto EXIT;
    }

    return APP_CFGR_OK;

EXIT:
    snd_hctl_close(g_pt_hctl);
    return i4_ret;
}

static VOID _acfg_adjust_volume_on_boot(VOID)
{
    INT32 i4_ret;
    UINT8 ui1_value;

    i4_ret = u_acfg_get_volume(&ui1_value);
    if (i4_ret)
    {
        return;
    }

    if (ui1_value > BOOT_VOLUME_MAX)
    {
        DBG_ACFG_INFO("boot up volume[%d] is too high, change to [%d]!\n", ui1_value, BOOT_VOLUME_MAX);
        u_acfg_set_volume(BOOT_VOLUME_MAX, TRUE);
    }
    else if (ui1_value < BOOT_VOLUME_MIN)
    {
        DBG_ACFG_INFO("boot up volume[%d] is too low, change to [%d]!\n", ui1_value, BOOT_VOLUME_MIN);
        u_acfg_set_volume(BOOT_VOLUME_MIN, TRUE);
    }

    return;
}

/*-----------------------------------------------------------------------------
 * Name: _acfg_app_init_fct
 *
 * Description:
 *      The acfg initial function.
 * Inputs:  ps_name             References the application's name.
 *          h_app               Contains the application handle.
 *
 * Outputs: -
 *
 * Returns: AEER_OK             Routine successful.
 *          Any other value     Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _acfg_app_init_fct (const CHAR* ps_name, HANDLE_T h_app)
{
    INT32 i4_ret;
    UINT16  ui2_i;
    UINT8   *pui1_values = NULL;
    SIZE_T  z_size;

    DBG_ACFG_INFO("Acfg init.\n");

    /* Reserve the handle */
    h_g_acfg_app = h_app;

    ui2_acfg_nfy_rec_max_idx = 0;
    /* reset the notify descriptor */
    //memset(at_acfg_nfy_recs, 0, sizeof(at_acfg_nfy_recs));

    /* reset the descriptor */
    memset(at_acfg_descr, 0, sizeof(at_acfg_descr));


#ifdef CLI_SUPPORT
    /* Register app config cli */
    i4_ret = acfg_attach_cmd_tbl();
    if (i4_ret != CLIR_OK)
    {
        DBG_ACFG_ERR("Err: cli attach cmd tbl failed, ret=%ld\r\n",i4_ret);
        return AEER_FAIL;
    }
#endif/* CLI_SUPPORT */

#if CONFIG_SUPPORT_OPEN_ALSA_VOLUME
    i4_ret = _acfg_alsa_ctrl_init();
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ACFG_ERR("acfg_alsa_ctrl_init failed, ret:%ld!\n",i4_ret);
        return AEER_FAIL;
    }
#endif

	/*bluetooth acfg grp init*/
	i4_ret = acfg_grp_bluetooth_init();
	ACFG_CHK_FAIL(i4_ret, i4_ret);
	DBG_ERROR(("\n ---acfg_grp_bluetooth_init \n"));

    i4_ret = acfg_grp_network_init();
    ACFG_CHK_FAIL(i4_ret, i4_ret);
	DBG_ERROR(("\n ---acfg_grp_network_init \n"));

    /**************************Pre INIT**************************
         *  In this step, declare the config schema and attributes all the config record *
         **********************************************************/
    i4_ret = acfg_grp_system_init();
    ACFG_CHK_FAIL(i4_ret, i4_ret);

    /**************************INIT*****************************
         *  In this step, create or load config from storage. At the same time.            *
         * It will validate the config data.                                                                    *
         **********************************************************/

    /* Initialize the flash config */
    DBG_ACFG_INFO("Initiating config in flash.\n");
    i4_ret = acfg_flash_init();
    if(i4_ret != APP_CFGR_OK)
    {
        return AEER_FAIL;
    }
    _acfg_tms_spot("flash init done");

    /* Validate the configuration data */

    DBG_ACFG_INFO("Validating the config data.\n");

    pui1_values = malloc(ACFG_MAX_REC_SIZE);
    if (NULL == pui1_values)
    {
        DBG_ACFG_ERR("init, memory alloc fail\n");
        return AEER_FAIL;
    }

    for (ui2_i = 0; ui2_i < IDX_MAX; ui2_i++)
    {
        acfg_get_size(ui2_i, &z_size);
        if(z_size > ACFG_MAX_REC_SIZE)
        {
            DBG_ACFG_ERR("Err: Too large field size: %s %d>%d\n",
                       at_acfg_descr[ui2_i].ac_name,
                       z_size,
                       ACFG_MAX_REC_SIZE);
            continue;
        }

        if(FALSE == at_acfg_descr[ui2_i].b_enable)
        {
            continue;
        }

        i4_ret = acfg_get(ui2_i, pui1_values, &z_size);
        if (i4_ret != APP_CFGR_OK)
        {
            DBG_ACFG_ERR("Err: Can't get the value. idx=%d\n",
                at_acfg_descr[ui2_i].ui2_id);
            return AEER_FAIL;
        }

        /* Validate the given field */
        if (!_acfg_validate(ui2_i, pui1_values, z_size))
        {
            DBG_ACFG_API("The item %d is invalid. set to default value\n",
                       at_acfg_descr[ui2_i].ui2_id);

            /* Set to default value. Don't use acfg_set_default(),
                    because it will call the update hook function. However,
                    it may lead to some problems before b_acfg_init
                    becoming TRUE.  */

            /* store in flash */
            i4_ret = acfg_flash_set(ui2_i,
                                  at_acfg_descr[ui2_i].pt_def_value,
                                  at_acfg_descr[ui2_i].ui2_def_num);
            if(i4_ret != APP_CFGR_OK)
            {
                DBG_ACFG_ERR("Err: Can't Set default value to flash. idx=%d\n",
                           at_acfg_descr[ui2_i].ui2_id);
                continue;
            }
        }
    }

    free(pui1_values);
    pui1_values = NULL;

    _acfg_tms_spot("Validate done");


    /* allocate mutex */
    i4_ret = u_sema_create(&h_nty_sema_mtx,
                           X_SEMA_TYPE_MUTEX,
                           X_SEMA_STATE_UNLOCK);
    if(i4_ret != OSR_OK)
    {
        DBG_ACFG_ERR("Err: Can't create mutex\n");
        return APP_CFGR_CANT_INIT;
    }

    /* create timer for Flash store */
    i4_ret = u_timer_create(&h_store_timer);
    ACFG_CHK_FAIL(i4_ret,APP_CFGR_INTERNAL_ERR);

    b_g_app_cfg_init = TRUE;


    /************************Post INIT***************************
         *  In this step, update the config                                                                  *
         **********************************************************/
    _acfg_post_init();

    _acfg_adjust_volume_on_boot();

    _acfg_tms_spot("Init Done");

    return AEER_OK;
}

/*-----------------------------------------------------------------------------
 * Name: _acfg_app_exit_fct
 *
 * Description: This function is called in the context of the application
 *              manager when the application quits. This API should be
 *              responsible for saving data, clean-up, etc.
 *
 * Inputs:  h_app               Contains the application handle.
 *          e_exit_mode         Contains the exit mode.
 *
 * Outputs: -
 *
 * Returns: AEER_OK             Routine successful.
 *          Any other value     Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _acfg_app_exit_fct(HANDLE_T h_app, APP_EXIT_MODE_T e_exit_mode)
{
    INT32 i4_ret = 0;
    DBG_ACFG_API("APP Exit.\n");

#ifdef CLI_SUPPORT
    acfg_detach_cmd_tbl();
#endif/* CLI_SUPPORT */

    snd_hctl_close(g_pt_hctl);

    u_sema_delete(h_nty_sema_mtx);

    if (NULL_HANDLE != h_store_timer)
    {
        u_timer_stop(h_store_timer);
        u_timer_delete(h_store_timer);
    }

    acfg_flash_uninit();

    return AEER_OK;
}

/*-----------------------------------------------------------------------------
 * Name: _acfg_app_process_msg_fct
 *
 * Description:
 *
 * Inputs:  h_app               Contains the application handle.
 *          ui4_type            Contains the type of the data contained in
 *                              pv_msg, or simple a request type. Values are
 *                              defined by the application.
 *          pv_msg              References the message data.
 *          z_msg_len           Contains the length (in bytes) of pv_msg.
 *          b_paused            Sepcifies whether the application is currently
 *                              paused.
 *
 * Outputs: -
 *
 * Returns: AEER_OK             Routine successful.
 *          Any other value     Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _acfg_app_process_msg_fct (HANDLE_T     h_app,
                                                UINT32       ui4_type,
                                                const VOID*  pv_msg,
                                                SIZE_T       z_msg_len,
                                                BOOL         b_paused)
{
    DBG_ACFG_INFO("ACFG get msg, type=%d.\n",ui4_type);

    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        switch(ui4_type)
        {
            /* private message */
            case E_APP_MSG_TYPE_ACFG:
                _acfg_pri_msg_proc(pv_msg);
                break;
            case E_APP_MSG_TYPE_TIMER:
            {
                DBG_ACFG_INFO("timer up!\n");

                ACFG_TIMER_MSG_E timer_type = *(ACFG_TIMER_MSG_E *)pv_msg;

                switch(timer_type)
                {
                    case ACFG_TIMER_FLASH_STORE_REPEATER:
                    {
                        _acfg_timer_proc();
                    }
                    break;

                    case ACFG_TIMER_SET_UID_REPEATER:
                    {
						// no need for non-c4a branch
                    }
                    break;

                    default:
                    break;
                }
            }
            break;
            default:
                break;
        }
    }
    return AEER_OK;
}


/*-----------------------------------------------------------------------------
                    public functions declarations
-----------------------------------------------------------------------------*/
INT32 acfg_set_alsa_volume(UINT32 ui4_volume)
{
    INT32 i4_ret;
    snd_ctl_elem_value_t *pt_value;

    snd_ctl_elem_value_alloca(&pt_value);

    i4_ret = snd_hctl_elem_read(g_pt_helem, pt_value);
    if (i4_ret < 0)
    {
        DBG_ACFG_ERR("snd_hctl_elem_read error:%s\n", snd_strerror(i4_ret));
        return i4_ret;
    }

    snd_ctl_elem_value_set_integer(pt_value, 0, ui4_volume);

    i4_ret = snd_hctl_elem_write(g_pt_helem, pt_value);
    if (i4_ret < 0)
    {
        DBG_ACFG_ERR("snd_hctl_elem_write error:%s\n", snd_strerror(i4_ret));
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 acfg_send_msg_to_self(UINT32 ui4_msg)
{
    INT32 i4_ret = 0;

    i4_ret = u_app_send_msg(h_g_acfg_app,
                            E_APP_MSG_TYPE_ACFG,
                            &ui4_msg,
                            sizeof(UINT32),
                            NULL,
                            NULL);
    if (i4_ret != AEER_OK)
    {
        DBG_ACFG_ERR("Err: c_app_send_msg return fail errno=%d\n", i4_ret);
    }

    return i4_ret;
}

ACFG_DESCR_T* acfg_get_item(UINT16 ui2_idx)
{
    if (ui2_idx >= IDX_MAX)
    {
        DBG_ACFG_ERR("Err: index out of range for ACFG.\n");
        return NULL;
    }

    return &at_acfg_descr[ui2_idx];
}


/*-----------------------------------------------------------------------------
 * Name: acfg_set
 *
 * Description:
 *      This function set the config data to config for given index. If it fails
 * to get, it will set the default value instead.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
INT32 acfg_set(UINT16 ui2_idx, VOID* pt_values, UINT16 ui2_elem_num)
{
    INT32   i4_ret = APP_CFGR_OK;
    INT32   i4_result;
    SIZE_T  z_ttl_size = 0;

    DBG_ACFG_INFO("set acfg item, idx=%d.\n",ui2_idx);

    if (ui2_idx >= IDX_MAX)
    {
        DBG_ACFG_ERR("Err: index out of range for ACFG.\n");
        return APP_CFGR_INV_ARG;
    }

    ACFG_ASSERT(ui2_idx < IDX_MAX);

    if(FALSE == at_acfg_descr[ui2_idx].b_enable)
    {
        DBG_ACFG_INFO("Err: item not enable.\n");
        return APP_CFGR_REC_NOT_FOUND;
    }

    /* calculate the total size */
    switch (at_acfg_descr[ui2_idx].e_type)
    {
        case CFG_8BIT_T:
        case CFG_16BIT_T:
        case CFG_32BIT_T:
        case CFG_64BIT_T:
            z_ttl_size = ui2_elem_num * at_acfg_descr[ui2_idx].e_type;
            break;
        default:
            DBG_ACFG_INFO("Err: invalid type.\n");
            return APP_CFGR_INTERNAL_ERR;
    }

    /* Validate value */
    if (!_acfg_validate(ui2_idx, pt_values, z_ttl_size))
    {
        DBG_ACFG_INFO("Err: validate fail.\n");
        return APP_CFGR_INV_ARG;
    }

    do
    {
        /* store in flash */
        i4_result = acfg_flash_set(ui2_idx, pt_values, ui2_elem_num);
        if(i4_result != APP_CFGR_OK)
        {
            DBG_ACFG_ERR("Err: Can't set to flash\n");
            i4_ret = APP_CFGR_INTERNAL_ERR;
            break;
        }

        /* Call the update hook if any */
        if(at_acfg_descr[ui2_idx].pf_update != NULL)
        {
            at_acfg_descr[ui2_idx].pf_update(at_acfg_descr[ui2_idx].ui2_id);
        }

        /* Fire the update notify */
        _acfg_fire_nfy(at_acfg_descr[ui2_idx].ui2_id);

    }while(0);

    _acfg_store_request();

    return i4_ret;
}

/*-----------------------------------------------------------------------------
 * Name: acfg_get
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
INT32 acfg_get(UINT16 ui2_idx, VOID* pt_values, SIZE_T* pz_size)
{
    INT32 i4_ret = APP_CFGR_OK;

    if (ui2_idx >= IDX_MAX)
    {
        DBG_ACFG_ERR("Err: index out of range for ACFG.\n");
        return APP_CFGR_INV_ARG;
    }

    ACFG_ASSERT(ui2_idx < IDX_MAX);

    if(FALSE == at_acfg_descr[ui2_idx].b_enable)
    {
        return APP_CFGR_REC_NOT_FOUND;
    }

    /* Get Value */
    i4_ret = acfg_flash_get(ui2_idx, pt_values, pz_size);

    if (i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("Err: Can't get item %d\n", at_acfg_descr[ui2_idx].ui2_id);
        /* Get Default Value */
        i4_ret = acfg_get_default(ui2_idx, pt_values, pz_size);
        if (i4_ret != APP_CFGR_OK)
        {
            DBG_ACFG_ERR("Err: Can't get item %d default value.\n",
                at_acfg_descr[ui2_idx].ui2_id);
        }
    }

    return i4_ret;
}

/*-----------------------------------------------------------------------------
 * Name: acfg_get_size
 *
 * Description:
 *      Get the total size for given index.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
INT32 acfg_get_size( UINT16 ui2_idx, SIZE_T* pz_size)
{
    SIZE_T z_size;

    if (ui2_idx >= IDX_MAX)
    {
        DBG_ACFG_ERR("Err: index out of range for ACFG.\n");
        return APP_CFGR_INV_ARG;
    }

    ACFG_ASSERT(ui2_idx < IDX_MAX);

    /* calculate the total size */
    switch (at_acfg_descr[ui2_idx].e_type)
    {
        case CFG_8BIT_T:
        case CFG_16BIT_T:
        case CFG_32BIT_T:
        case CFG_64BIT_T:
            z_size = at_acfg_descr[ui2_idx].ui2_num_elem * at_acfg_descr[ui2_idx].e_type;
            break;
        default:
            return APP_CFGR_INTERNAL_ERR;
    }

    *pz_size = z_size;

    return APP_CFGR_OK;
}

/*-----------------------------------------------------------------------------
 * Name: acfg_set_default
 *
 * Description:
 *      This function set default to config for given index.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
INT32 acfg_set_default(UINT16 ui2_idx)
{
    if (ui2_idx >= IDX_MAX)
    {
        DBG_ACFG_ERR("Err: index out of range for ACFG.\n");
        return APP_CFGR_INV_ARG;
    }

    return acfg_set(ui2_idx,
                    at_acfg_descr[ui2_idx].pt_def_value,
                    at_acfg_descr[ui2_idx].ui2_def_num);
}

/*-----------------------------------------------------------------------------
 * Name: acfg_get_default
 *
 * Description:
 *      This function get default from config for given index.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
INT32 acfg_get_default(UINT16 ui2_idx, VOID* pt_values, SIZE_T* pz_size)
{
    if ((ui2_idx >= IDX_MAX) || (NULL == pt_values) || (NULL == pz_size))
    {
        DBG_ACFG_ERR("Err: index out of range for ACFG.\n");
        return APP_CFGR_INV_ARG;
    }

    ACFG_ASSERT(ui2_idx < IDX_MAX);
    ACFG_ASSERT(pt_values != NULL);
    ACFG_ASSERT(pz_size != NULL);

    *pz_size = at_acfg_descr[ui2_idx].ui2_def_num * at_acfg_descr[ui2_idx].e_type;
    memcpy(pt_values, at_acfg_descr[ui2_idx].pt_def_value, *pz_size);

    return APP_CFGR_OK;
}

/*---------------------------------------------------------------------
 * Name: acfg_store
 *
 * Description:
 *      Store the configuration settings to the persistence storage.
 *
 * Returns:
 *    APP_CFGR_OK           - Successful
 *    APP_CFGR_NOT_INIT     - Not Initiated
 *    APP_CFGR_INTERNAL_ERR - Internal Error
 *
 --------------------------------------------------------------------*/
INT32 acfg_store(VOID)
{
    INT32 i4_ret;
    if (FALSE == b_g_app_cfg_init)
    {
        DBG_ACFG_ERR("Err: APP not init.\n");
        return APP_CFGR_NOT_INIT;
    }

    DBG_ACFG_INFO("store config\n");

    i4_ret = acfg_flash_store();
    if(i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("Err:Can't store config\n");
        return i4_ret;
    }

    return APP_CFGR_OK;
}




/*---------------------------------------------------------------------
 * Name: acfg_set_b_g_app_cfg_init
 *
 * Description:
 *      reset b_g_app_cfg_init.
 *

 --------------------------------------------------------------------*/
 VOID acfg_set_b_g_app_cfg_init(BOOL flag)
{
    b_g_app_cfg_init = flag;
}


/*-----------------------------------------------------------------------------
                    external functions declarations
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: a_acfg_register
 *
 * Description:
 *
 * Inputs:
 *  pt_cmds -   Array of customization commands. The array should be end of
 *              APP_CFG_CUST_CMD_END.
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
VOID  a_acfg_register(AMB_REGISTER_INFO_T*  pt_reg)
{
    _acfg_tms_reset();

    if (b_g_app_cfg_init)
    {
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN+1));
    strncpy(pt_reg->s_name, ACFG_THREAD_NAME, APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _acfg_app_init_fct;
    pt_reg->t_fct_tbl.pf_exit                   = _acfg_app_exit_fct;
    pt_reg->t_fct_tbl.pf_process_msg            = _acfg_app_process_msg_fct;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = ACFG_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = ACFG_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = ACFG_NUM_MSGS;
    pt_reg->t_desc.ui4_app_group_id             = 0;
    pt_reg->t_desc.ui4_app_id                   = 0;
    pt_reg->t_desc.ui2_msg_count                = ACFG_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = ACFG_MAX_MSGS_SIZE;

    return;
}

/*----------------------------------------------------------------------
 * Name: u_acfg_notify_reg
 *
 * Description:
 *      Register the notify update function. If member in the group has been modified, register
 * function will be invoked.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *--------------------------------------------------------------------*/
INT32 u_acfg_notify_reg(UINT16              ui2_group,
                            VOID*               pv_tag,
                            pf_acfg_nfy_func    pf_acfg_notify,
                            UINT16*             pui2_nfy_id)
{
    INT32   i4_ret;
    UINT16  ui2_i;

    if (FALSE == b_g_app_cfg_init)
    {
        DBG_ACFG_ERR("Err: APP not init.\n");
        return APP_CFGR_NOT_INIT;
    }

    if((ui2_group > 0xFF) || (NULL == pf_acfg_notify) || (NULL == pui2_nfy_id))
    {
        DBG_ACFG_ERR("Err: invalid parameter.\n");
        return APP_CFGR_INV_ARG;
    }

    _acfg_nty_lock();

    do
    {
        for(ui2_i = 0; ui2_i < ACFG_NFY_MAX_NUM; ui2_i++)
        {
            //if(FALSE == at_acfg_nfy_recs[ui2_i].b_used)
            {
                break;
            }
        }

        if(ACFG_NFY_MAX_NUM == ui2_i)
        {
            *pui2_nfy_id = ACFG_NFY_IVD_ID;
            i4_ret = APP_CFGR_TOO_MANY_NFY_FUNC;
        }
        else
        {
            //at_acfg_nfy_recs[ui2_i].b_used          = TRUE;
            //at_acfg_nfy_recs[ui2_i].ui2_group       = ui2_group;
            //at_acfg_nfy_recs[ui2_i].pv_tag          = pv_tag;
            //at_acfg_nfy_recs[ui2_i].pf_acfg_notify  = pf_acfg_notify;

            ui2_acfg_nfy_rec_max_idx =
                ui2_i > ui2_acfg_nfy_rec_max_idx ? (ui2_i+1) : ui2_acfg_nfy_rec_max_idx;
            *pui2_nfy_id = ui2_i;

            i4_ret = APP_CFGR_OK;
        }
    }while(0);

    _acfg_nty_unlock();

    return i4_ret;
}

/*-----------------------------------------------------------------------------
 * Name: u_acfg_notify_unreg
 *
 * Description:
 *      Unregister the notify update function.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns:
 *
 *-----------------------------------------------------------------------------*/
INT32 u_acfg_notify_unreg(UINT16  ui2_nfy_id)
{
    if (FALSE == b_g_app_cfg_init)
    {
        DBG_ACFG_ERR("Err: APP not init.\n");
        return APP_CFGR_NOT_INIT;
    }

    if (ui2_nfy_id >= ACFG_NFY_MAX_NUM)
    {
        return APP_CFGR_INV_ARG;
    }

    _acfg_nty_lock();

    //at_acfg_nfy_recs[ui2_nfy_id].b_used = FALSE;

    _acfg_nty_unlock();

    return APP_CFGR_OK;
}




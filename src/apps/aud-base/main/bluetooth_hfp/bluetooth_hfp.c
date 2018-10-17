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
#include <string.h>
#include <stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/* application level */
#include "u_amb.h"
#include "u_am.h"
#include "u_acfg.h"
#include "u_timerd.h"
#include "Interface.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_dbg.h"
#include "u_os.h"
#include "u_appman.h"
#include "u_sm.h"
#include "u_app_thread.h"

/*    btmw   */
#include "c_bt_mw_a2dp_snk.h"
#include "bt_a2dp_alsa_playback.h"
#include "c_bt_mw_gap.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_hfclient_wrapper.h"

/* private */
#include "u_bluetooth_audio.h"
#include "bluetooth_audio_proc.h"
#include "bluetooth_audio_key_proc.h"
#include "bluetooth_audio.h"
#include "bluetooth_hfp.h"
#include "bluetooth_hfp_proc.h"
#include <sys/time.h>
#include "pcm_merge.h"
#include "echo_cancellation.h"
#include "msbc_codec_exp.h"
#include "cvsd_codec_exp.h"
/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
BT_HFP_STATUS_T g_t_bt_hfp_sta = {0};
BT_HFP_OBJ_T g_t_g_bt_hfp = {0};
static UINT16 ui2_bt_hfp_dbg_level = DBG_INIT_LEVEL_APP_BT_HFP;

static pthread_t t_mic_in_record;
static pthread_t t_mic_in_write_bt_driver;
static pthread_t t_bt_driver_in;
static pthread_t t_bt_driver_play;
static pthread_t t_hfp_mic_in;
static pthread_t t_hfp_spk_in;
//static pthread_t t_hfp_media_player;
static BOOL f_is_cvsd_dec_type = FALSE; //FALSE:MSBC,TRUE:CVSD

BOOL dump_bt_driver_in = FALSE;
BOOL dump_bt_driver_in_play = FALSE;
BOOL dump_mic_in_record = FALSE;
BOOL dump_mic_in_write = FALSE;
#define BT_DRIVER_IN_PATH "/data/bt_driver_in.pcm"
#define BT_DRIVER_IN_PLAY_PATH "/data/bt_driver_in_play.pcm"
#define MIC_IN_RECORD_PATH "/data/mic_in_record.pcm"
#define MIC_IN_WRITE_PATH "/data/mic_in_write.pcm"

static pthread_t t_hfp_media_player;
static int flag_exit = FALSE;
static int f_hfp_audio_run = FALSE;
static merge_handle_t * merge_handle = NULL;
extern BT_AUD_BLUETOOTH_STATUS_T g_t_bt_aud_sta;

//extern int hfp_semid;

/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _bluetooth_hfp_app_init (const CHAR* ps_name,
                                            HANDLE_T h_app);
static INT32 _bluetooth_hfp_app_exit (HANDLE_T h_app,
                                            APP_EXIT_MODE_T e_exit_mode);

static INT32 _bluetooth_hfp_app_process_msg (HANDLE_T h_app,
                                                        UINT32 ui4_type,
                                                        const VOID* pv_msg,
                                                        SIZE_T z_msg_len,
                                                        BOOL b_paused);


/*-----------------------------------------------------------------------------
 * extern  methods declarations
 *---------------------------------------------------------------------------*/
extern INT32 _bt_hfp_cli_attach_cmd_tbl(VOID);
extern INT32 _bt_hfp_cli_detach_cmd_tbl(VOID);



VOID a_bluetooth_hfp_register(AMB_REGISTER_INFO_T* pt_reg)
{
    if (g_t_g_bt_hfp.b_app_init_ok == TRUE)
    {
        DBG_ERROR((BTHFP_TAG"Err: bluetooth_hfp already init.\r\n"));
        return;
    }

    /* Application can only use middleware's c_ API */
    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, BLUETOOTH_HFP_THREAD_NAME, APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _bluetooth_hfp_app_init;
    pt_reg->t_fct_tbl.pf_exit                   = _bluetooth_hfp_app_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _bluetooth_hfp_app_process_msg;
    pt_reg->t_desc.ui8_flags                    = AEE_FLAG_WRITE_CONFIG|AEE_FLAG_WRITE_FLM|AEE_FLAG_WRITE_TSL|AEE_FLAG_WRITE_SVL;
    pt_reg->t_desc.t_thread_desc.z_stack_size   = BLUETOOTH_HFP_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = BLUETOOTH_HFP_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = BLUETOOTH_HFP_NUM_MSGS;//64;
    pt_reg->t_desc.ui2_msg_count                = BLUETOOTH_HFP_MSGS_COUNT;//128;
    pt_reg->t_desc.ui2_max_msg_size             = BLUETOOTH_HFP_MAX_MSGS_SIZE;//4096;



    /* init application structue */
    memset(&g_t_g_bt_hfp, 0, sizeof(BT_HFP_OBJ_T));
    g_t_g_bt_hfp.h_app = NULL_HANDLE;
    g_t_g_bt_hfp.b_app_init_ok = FALSE;

 //   g_t_bt_hfp_sta.b_bt_hfp_connect = FALSE;

    return;
}

UINT16 _bt_hfp_get_dbg_level(VOID)
{
    return (ui2_bt_hfp_dbg_level | DBG_LAYER_APP);
}

VOID _bt_hfp_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_bt_hfp_dbg_level = ui2_db_level;
}

static INT32 _bluetooth_hfp_app_init (const CHAR* ps_name,
                                        HANDLE_T h_app)
{
    INT32 i4_ret;

    DBG_API((BTHFP_TAG"_bluetooth_hfp_app_init.\r\n"));

    if (g_t_g_bt_hfp.b_app_init_ok)
    {
        DBG_API((BTHFP_TAG"bluetooth_hfp already init.\r\n"));
        return AEER_OK;
    }
    g_t_g_bt_hfp.h_app = h_app;

#ifdef CLI_SUPPORT
    i4_ret = _bt_hfp_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        DBG_ERROR((BTHFP_TAG"Err: _bt_hfp_cli_attach_cmd_tbl() failed, ret=%ld\r\n",i4_ret ));
        return AEER_FAIL;
    }
#endif/* CLI_SUPPORT */

    _bt_hfp_set_dbg_level(DBG_INIT_LEVEL_APP_BT_HFP);

    g_t_g_bt_hfp.b_app_init_ok = TRUE;

    return AEER_OK;
}

static INT32 _bluetooth_hfp_app_exit (HANDLE_T h_app,
                                        APP_EXIT_MODE_T e_exit_mode)
{
    DBG_API((BTHFP_TAG"_bluetooth_hfp_app_exit.\r\n"));

    if (g_t_g_bt_hfp.b_app_init_ok == FALSE)
    {
        DBG_ERROR((BTHFP_TAG"Err: bluetooth audio doesn't init!\r\n"));
        return AEER_FAIL;
    }

    g_t_g_bt_hfp.b_app_init_ok = FALSE;

    return AEER_OK;
}

static INT32 _bluetooth_hfp_app_process_msg (HANDLE_T h_app,
                                                UINT32 ui4_type,
                                                const VOID* pv_msg,
                                                SIZE_T z_msg_len,
                                                BOOL b_paused)
{
    INT32 i4_ret;

    DBG_INFO((BTHFP_TAG"_bluetooth_hfp_app_process_msg, msgtype=%d.\r\n",ui4_type));
    if (g_t_g_bt_hfp.b_app_init_ok == FALSE)
    {
        DBG_ERROR((BTHFP_TAG"Err: bluetooth hfp doesn't init!\r\n"));
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        //DBG_INFO((BTAUD_TAG"private msg come.\r\n"));
        /* private message */
        switch(ui4_type)
        {
#if 0
		    case E_APP_MSG_TYPE_TIMER:
                _bluetooth_process_timer_msg(pv_msg, z_msg_len);
                break;
#endif
            case E_APP_MSG_TYPE_USER_CMD:
            {
                _bt_hfp_ui_msg_process(pv_msg, z_msg_len);
            }
            break;

			case E_APP_MSG_TYPE_BLUETOOTH_HFP:
			{
                DBG_ERROR((BTHFP_TAG"BLUETOOTH_HFP msg come.\r\n"));

				BT_HFP_MSG_T *pt_hfp_msg = (BT_HFP_MSG_T *)pv_msg;

				switch(pt_hfp_msg->ui4_msg_type)
				{
                    DBG_ERROR((BTHFP_TAG"BLUETOOTH_HFP pt_hfp_msg->ui4_msg_type is : %d \r\n",pt_hfp_msg->ui4_msg_type));

					case BT_HFP_PTE_MSG_HFP:
                    {
                        i4_ret = _bt_hfp_event_msg_proc(pv_msg);
                        BT_AUD_CHK_FAIL_RET(_bt_hfp_event_msg_proc, i4_ret, AEER_FAIL);
                    }
                    break;
#if 0
			        /*PLAY_CMD  message*/
                    case BT_AUD_PTE_MSG_KEY:
                    i4_ret = bt_aud_key_msg_proc(pv_msg);
                    BT_AUD_CHK_FAIL_RET(bt_aud_key_msg_proc, i4_ret, AEER_FAIL);
                    break;
#endif
					default:
						DBG_ERROR((BTHFP_TAG"Err: unkown BTLUETOOTH_HFP private msg, ui4_msg_type=%d.\r\n", pt_hfp_msg->ui4_msg_type));
                        break;

				}
			}
			break;

			/*state manager message*/
			case E_APP_MSG_TYPE_STATE_MNGR:
			{

                DBG_ERROR((BTHFP_TAG"SM msg come.\r\n"));
				_bt_hfp_sm_msg_process((APPMSG_T *)pv_msg);
			}
			break;

            default:
                DBG_ERROR((BTHFP_TAG"Err: unkown private msg, msgtype=%d.\r\n", ui4_type));
                break;
        }

    }

#if 1
    else
    {
        DBG_ERROR((BTHFP_TAG"AMB_BROADCAST msg come.\r\n"));
        #if 0
        switch(ui4_type)
        {
            /*state manager message*/
			case E_APP_MSG_TYPE_STATE_MNGR:
            {
                _bluetooth_sm_msg_process((APPMSG_T *)pv_msg);
            }
            break;

            default:
            break;
        }
		#endif
		//end add
    }
#endif

    DBG_ERROR((BTHFP_TAG"_bluetooth_hfp_app_process_msg done.\r\n"));
    return AEER_OK;
}

INT32 _bt_hfp_send_msg_to_itself(UINT32 ui4_msgtype, BT_HFP_MSG_T* pt_hfp_msg)
{
    INT32 i4_ret = 0;

    //send msg to bluetooth_hfp thread
    if (NULL_HANDLE == g_t_g_bt_hfp.h_app)
    {
        DBG_ERROR((BTHFP_TAG"bluetooth hfp has a NULL_HANDLE.\r\n"));
        return BT_HFP_FAIL;
    }

    DBG_INFO((BTHFP_TAG"send msg to BT_HFP thread, pt_hfp_msg->ui4_msg_id=%d.\r\n", pt_hfp_msg->ui4_msg_id));

    i4_ret = u_app_send_msg(g_t_g_bt_hfp.h_app,
                    ui4_msgtype,
                    pt_hfp_msg,
                    sizeof(BT_HFP_MSG_T),
                    NULL,
                    NULL);
    BT_HFP_CHK_FAIL_RET(u_app_send_msg, i4_ret, BT_HFP_FAIL);

    return BT_HFP_OK;

}

VOID _bt_hfp_set_cvsd_msbc_type(BOOL flag)
{
    f_is_cvsd_dec_type = flag;
}
BOOL _bt_hfp_get_cvsd_msbc_type(VOID)
{
    //TRUE is CVSD, False is mSBC
    return f_is_cvsd_dec_type;
}

#ifdef CONSYS_WIFI_CHIP
int _alsa_set_sw_params_for_bt_mw_read(PCMContainer_t *pcm_params, snd_pcm_uframes_t val)
{
    int ret;
    snd_pcm_sw_params_t *ptr = NULL;

    snd_pcm_sw_params_alloca(&ptr);
    if (NULL == ptr)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_malloc error!\n"));
        goto MALLOC_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_current(pcm_params->handle, ptr);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_current error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_set_start_threshold(pcm_params->handle, ptr, val);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_start_threshold error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }


    ret = snd_pcm_sw_params_set_stop_threshold(pcm_params->handle, ptr, pcm_params->buffer_size * 10);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_stop_threshold error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_set_avail_min(pcm_params->handle, ptr, 1);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_avail_min error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_set_silence_threshold(pcm_params->handle, ptr, 0);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_silence_threshold error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_set_silence_size(pcm_params->handle, ptr, 0);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_silence_size error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params(pcm_params->handle, ptr);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    return 0;

GET_PARAMS_ERR:
    snd_pcm_sw_params_free(ptr);
MALLOC_PARAMS_ERR:
    return -1;
}
int _alsa_set_sw_params_for_bt_mw_write(PCMContainer_t *pcm_params, snd_pcm_uframes_t val)
{
    int ret;
    snd_pcm_sw_params_t *ptr = NULL;

    snd_pcm_sw_params_alloca(&ptr);
    if (NULL == ptr)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_malloc error!\n"));
        goto MALLOC_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_current(pcm_params->handle, ptr);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_current error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_set_start_threshold(pcm_params->handle, ptr, val);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_start_threshold error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }


    ret = snd_pcm_sw_params_set_stop_threshold(pcm_params->handle, ptr, pcm_params->buffer_size);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_stop_threshold error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_set_avail_min(pcm_params->handle, ptr, 1);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_avail_min error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_set_silence_threshold(pcm_params->handle, ptr, 0);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_silence_threshold error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_set_silence_size(pcm_params->handle, ptr, 0);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params_set_silence_size error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params(pcm_params->handle, ptr);
    if (ret < 0)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_sw_params error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

	BT_HFP_FUNC_EXIT();
    return 0;

GET_PARAMS_ERR:
    snd_pcm_sw_params_free(ptr);
MALLOC_PARAMS_ERR:
    return -1;
}

/*-----------------------------------------------------------------------------
 * Name: _mic_in_record_process
 *
 * Description: This thread  record data from TDM.
 ----------------------------------------------------------------------------*/
static VOID* _mic_in_hfp_record_process(VOID *arg)
{
    INT32 i4_ret,data_buf_size = 0,icount = 0;
    snd_pcm_t *handle;
    PCMContainer_t *pcm_params;
    MSGLIST_T * h_msglist = (MSGLIST_T *)arg;
    FILE *pFile;
    VOID * p_head;
    CHAR *send_buf;
    //for enc
    void *pEncHandle_msbc,*pEncHandle_cvsd;
    CHAR *pWorkBuf,*pOutBuf,*pOutBuf_final;
    INT32 uWorkBufSize=0,sOutLen=0;
    short *pInBuf,*pTmp;
    UINT32 iPacket_w=0,index=0;
    const uint8_t btsco_MsbcHeader[4] = {0x08, 0x38, 0xc8, 0xf8};
    //for upsample
    INT32 i,j,up_size_in_byte = 0,in_buf_size_in_byte = 0,s_out_64k = 0,s_in_16k = 0;
    void  *p_16_to_64_up_hdl;
    short *p_in_16_to_64_buf,*p_out_16_to_64_buf,*p_temp_16_to_64_buf;
    //for aec
    void *aecInst;
    short *pmic_in,*pspeaker_in,*aec_outbuf,*aec_outbuf_final;
    INT32 plc_samp = 160,mic_samp = 480;

    //open alsa node
    pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if (NULL == pcm_params)
    {
        DBG_ERROR((BTHFP_TAG"malloc pcm_params failed!\r\n"));
        goto MALLOC_ERR;
    }

    memset(pcm_params, 0x00, sizeof(PCMContainer_t));
    pcm_params->format = SND_PCM_FORMAT_S32_LE;
    pcm_params->handle = handle;
    pcm_params->sample_rate = MIC_IN_SAMPLERATE;
    pcm_params->channels = 8;

    usleep(500 * 1000);

    i4_ret = snd_pcm_open(&pcm_params->handle, MIC_IN_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_open failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    i4_ret = u_alsa_set_hw_params(pcm_params, 120000);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_hw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    DBG_ERROR((BTHFP_TAG"<%s>line:%d.pcm_params->chunk_size =%d,pcm_params->chunk_bytes=%d.\r\n",__FUNCTION__,__LINE__,pcm_params->chunk_size,pcm_params->chunk_bytes));

    //malloc buf
    pTmp = (short *)malloc(mic_samp*sizeof(int)*MIC_IN_CHANNLE); 
    pmic_in = (short *)malloc(plc_samp*sizeof(short));
    pspeaker_in = (short *)malloc(plc_samp*sizeof(short));
    aec_outbuf = (short *)malloc(plc_samp*sizeof(short));
    aec_outbuf_final = (short *)malloc(mic_samp*sizeof(short)); 
    
    //init aec
    i4_ret = WebRtcAec_Create(&aecInst);
    if(i4_ret != 0)
    {
        DBG_ERROR((BTHFP_TAG"<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
    }
    
	i4_ret = WebRtcAec_Init(aecInst,16000,16000);
    if(i4_ret != 0)
    {
        DBG_ERROR((BTHFP_TAG"<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
    }
    
    //init cvsd/msbc encode
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        //init enc
        in_buf_size_in_byte = CVSD_ENC_INPUT_SAMPLE;
        pInBuf    = (short *)malloc(CVSD_ENC_INPUT_SAMPLE*sizeof(short));
        pOutBuf   = (char *)malloc(CVSD_ENC_OUTPUT_BYTE*sizeof(char));
        uWorkBufSize = CVSD_ENC_GetBufferSize();
        pWorkBuf  = (char *)malloc(uWorkBufSize*sizeof(char));
        pEncHandle_cvsd = (void*)CVSD_ENC_Init(pWorkBuf);
        sOutLen = CVSD_ENC_OUTPUT_BYTE;
        //malloc upsample buff, enc need 64k
        s_out_64k = (CVSD_ENC_INPUT_SAMPLE*sizeof(short));
        s_in_16k =  s_out_64k/4;
        up_size_in_byte = CVSD_UpSample_GetMemory();
        p_16_to_64_up_hdl = (void *)malloc(up_size_in_byte);
        CVSD_UpSample_Init(p_16_to_64_up_hdl);
        p_in_16_to_64_buf = (short *)malloc(s_in_16k);
        p_out_16_to_64_buf = (short *)malloc(s_out_64k);
        p_temp_16_to_64_buf = (short *)malloc(s_out_64k);
        //send 8*60 once
        send_buf =(char *)malloc((CVSD_ENC_OUTPUT_BYTE)*8);
        DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    }
    else
    {
        //init enc
        in_buf_size_in_byte = MSBC_ENC_INPUT_SAMPLE;
        pInBuf    = (short *)malloc(in_buf_size_in_byte*sizeof(short));
        pOutBuf   = (char *)malloc(MSBC_ENC_OUTPUT_BYTE*sizeof(char));
        pOutBuf_final   = (char *)malloc((MSBC_ENC_OUTPUT_BYTE+3)*sizeof(char));
        uWorkBufSize = MSBC_ENC_GetBufferSize();
        pWorkBuf  = (char *)malloc(uWorkBufSize*sizeof(char));
        pEncHandle_msbc = (void*)MSBC_ENC_Init(pWorkBuf);
        sOutLen = MSBC_ENC_OUTPUT_BYTE;
        //send 8*60 once
        send_buf =(char *)malloc((MSBC_ENC_OUTPUT_BYTE+3)*8);
        DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    }
    
    //dump data for test if need
    if(dump_mic_in_record) 
    {
        if ((pFile = fopen(MIC_IN_RECORD_PATH, "wb")) == NULL)
        {
            DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
            return NULL;
        }
    }

    //data process
    while (f_hfp_audio_run)
    {
        i4_ret = u_alsa_read_pcm(pcm_params, pcm_params->chunk_size);
        if(-1 == i4_ret)
        {
            DBG_ERROR((BTHFP_TAG"%s,line:%d.read err.\r\n",__FUNCTION__,__LINE__));
            continue;
        }
        //change 8ch to mono ch, and do aec
        memcpy((void *)pTmp,(void *)pcm_params->data_buf,pcm_params->chunk_bytes);
        for(i = 0;i < mic_samp;i++)
        {
            pmic_in[i%plc_samp] = 0;
            pmic_in[i%plc_samp] = pTmp[8*i*2+0*2+1];

            pspeaker_in[i%plc_samp] = 0;
            pspeaker_in[i%plc_samp] = pTmp[8*i*2+7*2+1];

            j = i/plc_samp;
            if((i+1)%plc_samp == 0)
            {
                i4_ret = WebRtcAec_BufferFarend(aecInst,pspeaker_in,plc_samp);
                if(i4_ret != 0)
                {
                    DBG_ERROR((BTHFP_TAG"<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
                }
                
                i4_ret = WebRtcAec_Process(aecInst,pmic_in,NULL,aec_outbuf,NULL,plc_samp,0,0);
                if(i4_ret != 0)
                {
                    DBG_ERROR((BTHFP_TAG"<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
                }

                memcpy(aec_outbuf_final+j*plc_samp,aec_outbuf,plc_samp*sizeof(short));
            }
        }

        //enc
        if(_bt_hfp_get_cvsd_msbc_type())
        {
            for(i=0;i<4;i++)
            {
                memcpy(p_in_16_to_64_buf, aec_outbuf_final+i*MSBC_ENC_INPUT_SAMPLE, MSBC_ENC_INPUT_SAMPLE* sizeof(short));

                CVSD_UpSample_Process_16_To_64(p_16_to_64_up_hdl, p_in_16_to_64_buf, p_out_16_to_64_buf, p_temp_16_to_64_buf, s_in_16k >> 1);
                //DBG_ERROR((BTHFP_TAG"<%s>line:%d.s_out_64k=%d\r\n",__FUNCTION__,__LINE__,s_out_64k));

                //encode PCM to cvsd
                memcpy((void *)pInBuf,(void *)p_out_16_to_64_buf,s_out_64k);
                CVSD_ENC_Process(pEncHandle_cvsd, pInBuf, &in_buf_size_in_byte, pOutBuf, &sOutLen);
                //DBG_ERROR((BTHFP_TAG"<%s>line:%d.in_buf_size_in_byte=%d,pOutBuf[0]=0x%x\r\n",__FUNCTION__,__LINE__,in_buf_size_in_byte,pOutBuf[0]));
                in_buf_size_in_byte = CVSD_ENC_INPUT_SAMPLE;
                sOutLen = CVSD_ENC_OUTPUT_BYTE;
                //for test dump encode data
                if(dump_mic_in_record)
                {
                    fwrite(pOutBuf,sizeof(char),sOutLen, pFile);
                }

                //send encode data to other process.
                memcpy(send_buf+icount*CVSD_ENC_OUTPUT_BYTE,pOutBuf,CVSD_ENC_OUTPUT_BYTE);
                icount++;
                if(8 == icount)
                {
                    icount = 0;
                    u_msglist_send(h_msglist,send_buf,CVSD_ENC_OUTPUT_BYTE*8,MSG_DATA_SEND);
                }
            }
        }
        else
        {
            for(i=0;i<4;i++)
            {
                memcpy(pInBuf, aec_outbuf_final+i*MSBC_ENC_INPUT_SAMPLE, MSBC_ENC_INPUT_SAMPLE* sizeof(short));
                
                //encode PCM to MSBC
                i4_ret = MSBC_ENC_Process(pEncHandle_msbc, pInBuf, &in_buf_size_in_byte, pOutBuf, &sOutLen);
                if(0 != i4_ret)
                {
                    DBG_ERROR((BTHFP_TAG"%s,line:%d.enc err.\r\n",__FUNCTION__,__LINE__));
                    continue;
                }
                //DBG_ERROR((BTHFP_TAG"<%s>line:%d.i4_ret=%d,pOutBuf[0]=0x%x\r\n",__FUNCTION__,__LINE__,i4_ret,pOutBuf[0]));

                //for test dump encode data
                if(dump_mic_in_record)
                {
                    fwrite(pOutBuf,sizeof(char),sOutLen, pFile);
                }

                //add verify byte
                iPacket_w++;
                index = iPacket_w;
                memset(pOutBuf_final,0,MSBC_ENC_OUTPUT_BYTE+3);
                memcpy(pOutBuf_final+2, pOutBuf, 57 * sizeof(char));
                pOutBuf_final[0] = 0x01; //header
                pOutBuf_final[1] = btsco_MsbcHeader[index & 0x3]; //header
                pOutBuf[59] = 0x00; //header
                //DBG_ERROR((BTHFP_TAG"<%s>line:%d.pOutBuf[3]=0x%x\r\n",__FUNCTION__,__LINE__,pOutBuf[3]));

                //send encode data to other process.
                memcpy(send_buf+icount*(MSBC_ENC_OUTPUT_BYTE+3),pOutBuf_final,MSBC_ENC_OUTPUT_BYTE+3);
                icount++;
                if(8 == icount)
                {
                    icount = 0;
                    u_msglist_send(h_msglist,send_buf,(MSBC_ENC_OUTPUT_BYTE+3)*8,MSG_DATA_SEND);
                }
            }
        }
    }

    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    snd_pcm_drop(pcm_params->handle);
    free(pcm_params->data_buf);
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    snd_pcm_close(pcm_params->handle);
    WebRtcAec_Free(aecInst);
    free(send_buf);
    free(aec_outbuf);
    free(pmic_in);
    free(pspeaker_in);
    free(pcm_params);
    free(pInBuf);
    free(pOutBuf);
    free(pWorkBuf);
    free(pTmp);
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        free(p_in_16_to_64_buf);
        free(p_out_16_to_64_buf);
        free(p_temp_16_to_64_buf);
        free(p_16_to_64_up_hdl);
    }
    if(dump_mic_in_record)
    {
        fclose(pFile);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

SET_UP_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    free(pcm_params);
MALLOC_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;
}

/*-----------------------------------------------------------------------------
 * Name: _mic_in_write_bt_driver_process
 *
 * Description: This thread  write ringbuf to bt driver.
 ----------------------------------------------------------------------------*/
static VOID* _mic_in_write_bt_driver_process(VOID *arg)
{
    INT32 i4_ret;
    snd_pcm_t *handle;
    PCMContainer_t *pcm_params;
    MSGLIST_T * h_msglist = (MSGLIST_T *)arg;
    DATANODE_T * msg_mode;

    FILE *pFile;
    VOID * phead;

    if(dump_mic_in_write)
    {
        if ((pFile = fopen(MIC_IN_WRITE_PATH, "wb")) == NULL)
        {
            DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
            return NULL;
        }
    }

    pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if (NULL == pcm_params)
    {
        DBG_ERROR((BTHFP_TAG"malloc failed!\r\n"));
        goto MALLOC_ERR;
    }
    memset(pcm_params, 0, sizeof(PCMContainer_t));
    pcm_params->format = SND_PCM_FORMAT_S16_LE;
    pcm_params->handle = handle;
    pcm_params->sample_rate = MIC_IN_SAMPLERATE;
    pcm_params->channels = 2;

    i4_ret = snd_pcm_open(&pcm_params->handle, BT_HFP_WRITE_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_open failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    i4_ret = u_alsa_set_hw_params_adjust_peroid_time(pcm_params, 127500,2);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_hw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    i4_ret = _alsa_set_sw_params_for_bt_mw_write(pcm_params, pcm_params->buffer_size/2);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_sw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.chunk_size=%d,chunk_bytes=%d,buffer_size=%d\r\n",__FUNCTION__,__LINE__,pcm_params->chunk_size,pcm_params->chunk_bytes,pcm_params->buffer_size));

    while (1)
    {
        msg_mode = u_msglist_receive(h_msglist);
        if(MSG_EXIT == msg_mode->id)
        {
            DBG_ERROR((BTHFP_TAG"<%s>line:%d.Thread exit.\r\n",__FUNCTION__,__LINE__));
            u_datanode_free(msg_mode);
            break;
        }
        phead = msg_mode->data;

        memcpy(pcm_params->data_buf,phead,msg_mode->size);
        if(dump_mic_in_write)
        {
            fwrite(pcm_params->data_buf, sizeof(char), msg_mode->size, pFile);
        }
        i4_ret = u_alsa_write_pcm_for_anysize(pcm_params, pcm_params->buffer_size/17);
        if(-1 == i4_ret)
        {
            DBG_ERROR((BTHFP_TAG"%s,line:%d.write err.\r\n",__FUNCTION__,__LINE__));
        }

        u_datanode_free(msg_mode);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    snd_pcm_drop(pcm_params->handle);
    free(pcm_params->data_buf);
    snd_pcm_close(pcm_params->handle);
    free(pcm_params);
    if(dump_mic_in_write)
    {
        fclose(pFile);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

SET_UP_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    free(pcm_params);
MALLOC_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    return NULL;
}

/*-----------------------------------------------------------------------------
 * Name: _bt_driver_in_process
 *
 * Description: This thread read bt driver data to ringbuf.
 *                    Two way: PCM, cvsd/msbc.
 ----------------------------------------------------------------------------*/
static VOID* _bt_driver_in_process(VOID *arg)
{
    INT32 i4_ret;
    snd_pcm_t *handle;
    PCMContainer_t *hfp_btread_pcm_params;
    MSGLIST_T * h_msglist = (MSGLIST_T *)arg;
    FILE *pFile;

    if(dump_bt_driver_in)
    {
        if ((pFile = fopen(BT_DRIVER_IN_PATH, "ab+")) == NULL)
        {
            DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
            return NULL;
        }
    }

    hfp_btread_pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if (NULL == hfp_btread_pcm_params)
    {
        DBG_ERROR((BTHFP_TAG"malloc pcm_params failed!\r\n"));
        goto MALLOC_ERR;
    }

    memset(hfp_btread_pcm_params, 0x00, sizeof(PCMContainer_t));
    hfp_btread_pcm_params->format = SND_PCM_FORMAT_S16_LE;
    hfp_btread_pcm_params->handle = handle;
    hfp_btread_pcm_params->sample_rate = BT_HFP_SAMPLERATE;
    hfp_btread_pcm_params->channels = BT_HFP_CHANNLE;

    usleep(500 * 1000);

    i4_ret = snd_pcm_open(&hfp_btread_pcm_params->handle, BT_HFP_READ_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_open failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    i4_ret = u_alsa_set_hw_params_adjust_peroid_time(hfp_btread_pcm_params,96000,16);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_hw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    i4_ret = _alsa_set_sw_params_for_bt_mw_read(hfp_btread_pcm_params, 1);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_sw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.pcm_params->chunk_size=%d,pcm_params->chunk_bytes=%d,pcm_params->buffer_size=%d\r\n",__FUNCTION__,__LINE__,hfp_btread_pcm_params->chunk_size,hfp_btread_pcm_params->chunk_bytes,hfp_btread_pcm_params->buffer_size));

    while (f_hfp_audio_run)
    {
        i4_ret = u_alsa_read_pcm(hfp_btread_pcm_params, hfp_btread_pcm_params->chunk_size);
        if(-1 == i4_ret)
        {
            DBG_ERROR((BTHFP_TAG"%s,line:%d.read err.\r\n",__FUNCTION__,__LINE__));
            continue;
        }

        if(dump_bt_driver_in)
        {
            fwrite(hfp_btread_pcm_params->data_buf, sizeof(char), hfp_btread_pcm_params->chunk_bytes, pFile);
        }

        u_msglist_send(h_msglist,hfp_btread_pcm_params->data_buf,hfp_btread_pcm_params->chunk_bytes,MSG_DATA_SEND);
    }

    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    snd_pcm_drop(hfp_btread_pcm_params->handle);
    free(hfp_btread_pcm_params->data_buf);
    snd_pcm_close(hfp_btread_pcm_params->handle);
    free(hfp_btread_pcm_params);
    hfp_btread_pcm_params = NULL;
    if(dump_bt_driver_in)
    {
        fclose(pFile);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

SET_UP_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    free(hfp_btread_pcm_params);
    hfp_btread_pcm_params = NULL;
MALLOC_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    return NULL;
}
/*-----------------------------------------------------------------------------
 * Name: _bt_driver_play_process
 *
 * Description: This thread  write ringbuf to alsa for playback.
 ----------------------------------------------------------------------------*/
static VOID* _bt_driver_play_process(VOID *arg)
{
    INT32 i4_ret,databuf_size = 0,datasize = 0,rynum = 0;
    UINT32 ui4_retry_count = 0;
    snd_pcm_t *handle;
    PCMContainer_t *hfp_main_pcm_params;
    MSGLIST_T * h_msglist = (MSGLIST_T *)arg;
    DATANODE_T * msg_mode;
    CHAR *databuf,*p_head;
    FILE *pFile;

    void *pDecHandle_msbc;
    void *pDecHandle_cvsd;
    CHAR *pInBuf,*pWorkBuf;
    short * pOutBuf;
    INT32 uWorkBufSize=0,sOutLen,dec_input_size = 0;
    INT32 plc_size = 0;
    CHAR * plc_workbuf;
    BOOL f_invalid_data = FALSE;
    
    INT32  s_out_8k = 0,in_buf_size_in_byte =0,hdl_size_in_byte=0;
    static short *p_temp_64_to_8_buf,*p_out_64_to_8_buf;
    static void  *p_64_to_8_down_hdl;

    //init alsa
    hfp_main_pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if (NULL == hfp_main_pcm_params)
    {
        DBG_ERROR((BTHFP_TAG"malloc failed!\r\n"));
        goto MALLOC_ERR;
    }
    memset(hfp_main_pcm_params, 0, sizeof(PCMContainer_t));
    hfp_main_pcm_params->format = SND_PCM_FORMAT_S16_LE;
    hfp_main_pcm_params->handle = handle;
    hfp_main_pcm_params->channels = 1;
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        hfp_main_pcm_params->sample_rate = 8000;
    }
    else
    {
        hfp_main_pcm_params->sample_rate = 16000;
    }

    i4_ret = snd_pcm_open(&hfp_main_pcm_params->handle, "sub1", SND_PCM_STREAM_PLAYBACK, 0);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_open failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        i4_ret = u_alsa_set_hw_params(hfp_main_pcm_params, 80000);
    }
    else
    {
        i4_ret = u_alsa_set_hw_params(hfp_main_pcm_params, 80000);
    }
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_hw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    i4_ret = u_alsa_set_sw_params(hfp_main_pcm_params, hfp_main_pcm_params->buffer_size);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_sw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    if(dump_bt_driver_in_play)
    {
        if ((pFile = fopen(BT_DRIVER_IN_PLAY_PATH, "wb")) == NULL)
        {
            DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
            return NULL;
        }
    }

    databuf = (char *)malloc(HFP_DEC_DATA_BUF*sizeof(char));
    memset(databuf,0,HFP_DEC_DATA_BUF);

    //init cvsd/msbc decode
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        dec_input_size = CVSD_VALID_DEC_INPUT_BYTE;
        pInBuf    = (char *)malloc(CVSD_VALID_DEC_INPUT_BYTE*sizeof(char));
        pOutBuf   = (short *)malloc(CVSD_DEC_OUTPUT_SAMPLE*sizeof(short));
        uWorkBufSize = CVSD_DEC_GetBufferSize();
        pWorkBuf  = (char *)malloc(uWorkBufSize*sizeof(char));
        pDecHandle_cvsd = (void*)CVSD_DEC_Init(pWorkBuf);
        sOutLen = CVSD_DEC_OUTPUT_SAMPLE;
        
        in_buf_size_in_byte = (CVSD_DEC_OUTPUT_SAMPLE*sizeof(short));
        s_out_8k = in_buf_size_in_byte/8;
        hdl_size_in_byte = CVSD_DownSample_GetMemory();
        p_64_to_8_down_hdl = (void *)malloc(hdl_size_in_byte);
        CVSD_DownSample_Init(p_64_to_8_down_hdl);
        p_out_64_to_8_buf = (short *)malloc(s_out_8k);
        p_temp_64_to_8_buf = (short *)malloc(in_buf_size_in_byte);
        //init plc
        plc_size = g711plc_GetMemorySize_v2();
        plc_workbuf = (char *)malloc(plc_size*sizeof(char));
        g711plc_construct_v2((void *)plc_workbuf,8000);
        DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    }
    else
    {
        dec_input_size = MSBC_VALID_DEC_INPUT_BYTE;
        pInBuf    = (char *)malloc(MSBC_VALID_DEC_INPUT_BYTE*sizeof(char));
        pOutBuf   = (short *)malloc(MSBC_DEC_OUTPUT_SAMPLE*sizeof(short));
        uWorkBufSize = MSBC_DEC_GetBufferSize();
        pWorkBuf  = (char *)malloc(uWorkBufSize*sizeof(char));
        pDecHandle_msbc = (void*)MSBC_DEC_Init(pWorkBuf);
        sOutLen = MSBC_DEC_OUTPUT_SAMPLE*sizeof(short);
        DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
        //init plc
        plc_size = g711plc_GetMemorySize_v2();
        plc_workbuf = (char *)malloc(plc_size*sizeof(char));
        g711plc_construct_v2((void *)plc_workbuf,16000);
        DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    }
    //data process flow
    while (f_hfp_audio_run)
    {
        msg_mode = u_msglist_receive(h_msglist);
        if(MSG_EXIT == msg_mode->id)
        {
            DBG_ERROR((BTHFP_TAG"<%s>line:%d.Thread exit.\r\n",__FUNCTION__,__LINE__));
            u_datanode_free(msg_mode);
            break;
        }

        databuf_size = HFP_DEC_DATA_BUF;

        memcpy(databuf,msg_mode->data,msg_mode->size);
        datasize = msg_mode->size;
        p_head = databuf;
		if(!f_hfp_audio_run)
		{
			DBG_ERROR((BTHFP_TAG"<%s>line:%d.if hfp has exit,stop because sometimes the CVSD/MSBC[%d] is changged.\r\n",__FUNCTION__,__LINE__,_bt_hfp_get_cvsd_msbc_type()));
			break;
		}
        if(_bt_hfp_get_cvsd_msbc_type())
        {
            //decode cvsd
            rynum = datasize/CVSD_DEC_INPUT_BYTE;
            while(rynum >0)
            {
                //DBG_ERROR((BTHFP_TAG"<%s>line:%d.%d,rynum=%d,valid data=%d.\r\n",__FUNCTION__,__LINE__,i4_ret,rynum,*(p_head+30)));
                if((*(p_head+30) == 1))
                {
                    //DBG_ERROR((BTHFP_TAG"<%s>line:%d.decode.\r\n",__FUNCTION__,__LINE__));
                    memcpy(pInBuf,p_head,CVSD_VALID_DEC_INPUT_BYTE);
                    CVSD_DEC_Process(pDecHandle_cvsd, pInBuf, &dec_input_size, pOutBuf, &sOutLen);
                    sOutLen = CVSD_DEC_OUTPUT_SAMPLE;
                    dec_input_size = CVSD_VALID_DEC_INPUT_BYTE;

                    //downsample to 8khz
                    CVSD_DownSample_Process_64_To_8(p_64_to_8_down_hdl, pOutBuf, p_out_64_to_8_buf, p_temp_64_to_8_buf, in_buf_size_in_byte >> 1);

                    g711plc_addtohistory_v2(plc_workbuf, (short *)p_out_64_to_8_buf,0);
                }
                else
                {
                    g711plc_dofe_v2(plc_workbuf, (short *)p_out_64_to_8_buf,0);
                    DBG_ERROR((BTHFP_TAG"<%s>line:%d.undecode.do PLC.\r\n",__FUNCTION__,__LINE__));
                }
                
                if(dump_bt_driver_in_play)
                {
                    fwrite(p_out_64_to_8_buf, sizeof(char), s_out_8k, pFile);
                }

                //write data to alsa
                memcpy((void *)(hfp_main_pcm_params->data_buf),(void *)p_out_64_to_8_buf,s_out_8k);
                i4_ret = u_alsa_write_pcm_for_anysize(hfp_main_pcm_params, s_out_8k/2);
                if(-1 == i4_ret)
                {
                    DBG_ERROR((BTHFP_TAG"%s,line:%d.write err.\r\n",__FUNCTION__,__LINE__));
                }

                rynum--;
                //DBG_ERROR((BTHFP_TAG"<%s>line:%d.%d,rynum=%d\r\n",__FUNCTION__,__LINE__,i4_ret,rynum));
                p_head += CVSD_DEC_INPUT_BYTE;
            }
            memset(databuf,0,HFP_DEC_DATA_BUF);
        }
        else
        {
            rynum = datasize/MSBC_DEC_INPUT_BYTE;
            while(rynum >0)
            {
                if((*(p_head+2) == 173)&&(*(p_head+30) == 1)&&(*(p_head+62) == 1))
                {
                    memmove(p_head+30,p_head+32,29*sizeof(char));
                    memcpy(pInBuf,p_head+2,MSBC_VALID_DEC_INPUT_BYTE);
                    i4_ret = MSBC_DEC_Process(pDecHandle_msbc, pInBuf, &dec_input_size, pOutBuf, &sOutLen);
                    sOutLen = MSBC_DEC_OUTPUT_SAMPLE;
                    if(i4_ret != 57)
                    {
                        DBG_ERROR((BTHFP_TAG"<%s>line:%d.invalid data,Error:%d.do plc.\r\n",__FUNCTION__,__LINE__,i4_ret));
                        f_invalid_data = TRUE;
                    }
                    else
                    {
                        g711plc_addtohistory_v2(plc_workbuf, (short *)pOutBuf,0);
                        f_invalid_data = FALSE;
                    }
                }
                else
                {
                    DBG_ERROR((BTHFP_TAG"<%s>line:%d.invalid data,do plc\r\n",__FUNCTION__,__LINE__));
                    f_invalid_data = TRUE;
                }

                if(f_invalid_data)
                {
                    g711plc_dofe_v2(plc_workbuf, (short *)pOutBuf,0);
                    DBG_ERROR((BTHFP_TAG"<%s>line:%d.plc end.\r\n",__FUNCTION__,__LINE__));
                }
                
                if(dump_bt_driver_in_play)
                {
                    fwrite(pOutBuf, sizeof(char), sOutLen*2, pFile);
                }

                memcpy(hfp_main_pcm_params->data_buf,(void *)pOutBuf, sOutLen*2);

                i4_ret = u_alsa_write_pcm_for_anysize(hfp_main_pcm_params, MSBC_DEC_OUTPUT_SAMPLE);
                if(-1 == i4_ret)
                {
                    DBG_ERROR((BTHFP_TAG"%s,line:%d.write err.\r\n",__FUNCTION__,__LINE__));
                }

                rynum--;
                p_head += MSBC_DEC_INPUT_BYTE;
            }
            memset(databuf,0,HFP_DEC_DATA_BUF);
        }
        u_datanode_free(msg_mode);
    }

    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    free(pInBuf);
    free(pOutBuf);
    free(pWorkBuf);
    free(databuf);
    free(plc_workbuf);
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        free(p_out_64_to_8_buf);
        free(p_temp_64_to_8_buf);
        free(p_64_to_8_down_hdl);
    }
    snd_pcm_drop(hfp_main_pcm_params->handle);
    free(hfp_main_pcm_params->data_buf);
    snd_pcm_close(hfp_main_pcm_params->handle);
    free(hfp_main_pcm_params);
    hfp_main_pcm_params = NULL;
    if(dump_bt_driver_in_play)
    {
        fclose(pFile);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

SET_UP_ERR:
    free(hfp_main_pcm_params);
    hfp_main_pcm_params = NULL;
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
MALLOC_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    return NULL;
}
#else
/*-----------------------------------------------------------------------------
 * Name: _mic_in_hfp_record_process
 *
 * Description: This thread  record data from TDM.
-----------------------------------------------------------------------------*/
static VOID* _mic_in_hfp_record_process(VOID *arg)
{
    INT32 i4_ret=0,num=0,i, j;
    snd_pcm_t *handle;
    PCMContainer_t *pcm_params;
    MSGLIST_T * h_msglist = (MSGLIST_T *)arg;
    FILE *pFile;
    short *pInBuf,*pTmp;

    void *p_16_to_8_down_hdl;
    INT32 hdl_size_in_byte = 0, s_out_8k = 0,in_buf_size_in_byte = 0;
    short *p_out_16_to_8_buf,*p_temp_16_to_8_buf;

    //dump mic in data, only for test
    if(dump_mic_in_record)
    {
        if ((pFile = fopen(MIC_IN_RECORD_PATH, "wb+")) == NULL)
        {
            DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
            return NULL;
        }
    }

    //init some buffer for CVSD and mSBC
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        pInBuf = (short *)malloc(CVSD_INPUT_SAMPLE*sizeof(short));
        if (NULL == pInBuf)
        {
            DBG_ERROR((BTHFP_TAG"malloc pInBuf failed!\r\n"));
            goto MALLOC_ERR;
        }
        pTmp = (short *)malloc(CVSD_INPUT_SAMPLE*sizeof(short)*HFP_MIC_IN_CHANNLE);
        if (NULL == pTmp)
        {
            DBG_ERROR((BTHFP_TAG"malloc pTmp failed!\r\n"));
            goto MALLOC_ERR;
        }

        //CVSD malloc downsample buff
        in_buf_size_in_byte = (CVSD_INPUT_SAMPLE*sizeof(short));
        s_out_8k = in_buf_size_in_byte/2;
        hdl_size_in_byte = CVSD_DownSample_GetMemory();
        p_16_to_8_down_hdl = (void *)malloc(hdl_size_in_byte);
        CVSD_DownSample_Init(p_16_to_8_down_hdl);
        p_out_16_to_8_buf = (short *)malloc(s_out_8k);
        p_temp_16_to_8_buf = (short *)malloc(in_buf_size_in_byte);

        DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    }
    else
    {
        pInBuf = (short *)malloc(MSBC_INPUT_SAMPLE*sizeof(short));
        if (NULL == pInBuf)
        {
            DBG_ERROR((BTHFP_TAG"malloc pInBuf failed!\r\n"));
            goto MALLOC_ERR;
        }
        pTmp = (short *)malloc(MSBC_INPUT_SAMPLE*sizeof(short)*HFP_MIC_IN_CHANNLE);
        if (NULL == pTmp)
        {
            DBG_ERROR((BTHFP_TAG"malloc pTmp failed!\r\n"));
            goto MALLOC_ERR;
        }
    }

    //init alsa param
    pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if (NULL == pcm_params)
    {
        DBG_ERROR((BTHFP_TAG"malloc pcm_params failed!\r\n"));
        goto MALLOC_ERR;
    }
    memset(pcm_params, 0x00, sizeof(PCMContainer_t));
    pcm_params->format = SND_PCM_FORMAT_S16_LE;
    pcm_params->handle = handle;
    pcm_params->sample_rate = MIC_IN_SAMPLERATE;
    pcm_params->channels = HFP_MIC_IN_CHANNLE;
    usleep(500 * 1000);
    i4_ret = snd_pcm_open(&pcm_params->handle, MIC_IN_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_open failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        i4_ret = u_alsa_set_hw_params(pcm_params, 120000);
    }
    else
    {
        i4_ret = u_alsa_set_hw_params(pcm_params, 60000);
    }
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_hw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    //start read data and do some processs
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    while (f_hfp_audio_run)
    {
        i4_ret = u_alsa_read_pcm(pcm_params, pcm_params->chunk_size);
        memcpy((void *)pTmp,(void *)pcm_params->data_buf,pcm_params->chunk_bytes);

        if(_bt_hfp_get_cvsd_msbc_type())
        {
            //change 8ch to mono ch, only 6ch is valid, extra 2ch is used for echo cancel
            //if source is 2ch, please change to mono ch by average
            for(i = 0;i < CVSD_INPUT_SAMPLE;i++)
            {
                pInBuf[i] = pTmp[HFP_MIC_IN_CHANNLE * i];
                /*for (j = 0;j < HFP_MIC_IN_CHANNLE;j++)
                {
                    if (j > HFP_MIC_IN_CHANNLE - 3)
                        continue;
                    pInBuf[i] += pTmp[HFP_MIC_IN_CHANNLE * i + j]/6;        //only fetch L channel
                }*/
            }

            if (dump_mic_in_record)//dump data,just for test
            {
                num = fwrite(pInBuf,sizeof(short),CVSD_INPUT_SAMPLE,pFile);
                if(num > 0)
                {
                    DBG_ERROR(("write %d '%s' !!\n",num,__FUNCTION__));
                }
            }

            //SRC from 16Khz to 8Khz
            CVSD_DownSample_Process_16_To_8(p_16_to_8_down_hdl,pInBuf,p_out_16_to_8_buf,p_temp_16_to_8_buf,in_buf_size_in_byte >> 1);
            in_buf_size_in_byte = (CVSD_INPUT_SAMPLE*sizeof(short));

            u_msglist_send(h_msglist,(void *)p_out_16_to_8_buf,CVSD_INPUT_BYTE,MSG_DATA_SEND);
        }
        else
        {
            //change 8ch to mono ch, only 6ch is valid, extra 2ch is used for echo cancel
            //if source is 2ch, please change to mono ch by average
            for(i = 0;i < MSBC_INPUT_SAMPLE;i++)
            {
                pInBuf[i] = pTmp[HFP_MIC_IN_CHANNLE * i];
                /*for (j = 0;j < HFP_MIC_IN_CHANNLE;j++)
                {
                    if (j > HFP_MIC_IN_CHANNLE - 3)
                        continue;
                    pInBuf[i] += pTmp[HFP_MIC_IN_CHANNLE * i + j]/6;        //only fetch L channel
                }*/
            }

            if (dump_mic_in_record)//dump data,just for test
            {
                num = fwrite(pInBuf,sizeof(short),MSBC_INPUT_SAMPLE,pFile);
                if(num > 0)
                {
                    DBG_ERROR(("write %d '%s' !!\n",num,__FUNCTION__));
                }
            }

            u_msglist_send(h_msglist,(void *)pInBuf,MSBC_INPUT_BYTE,MSG_DATA_SEND);
        }
    }

    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    snd_pcm_drain(pcm_params->handle);
    free(pcm_params->data_buf);
    snd_pcm_close(pcm_params->handle);
    free(pcm_params);
    if(dump_mic_in_record)
    {
        fclose(pFile);
    }
    free(pInBuf);
    free(pTmp);
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        free(p_16_to_8_down_hdl);
        free(p_out_16_to_8_buf);
        free(p_temp_16_to_8_buf);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

    SET_UP_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    free(pcm_params);
    MALLOC_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;
}
/*-----------------------------------------------------------------------------
 * Name: _mic_in_hfp_record_process
 *
 * Description: This thread  record data from TDM.
-----------------------------------------------------------------------------*/
static VOID* _mic_in_hfp_record_from_file_process(VOID *arg)
{
    INT32 i4_ret=0,num=0;
    MSGLIST_T * h_msglist = (MSGLIST_T *)arg;
    FILE *pFile;
    short *pInBuf;

    void *p_16_to_8_down_hdl;
    INT32 hdl_size_in_byte = 0, s_out_8k = 0,in_buf_size_in_byte = 0;
    short *p_out_16_to_8_buf,*p_temp_16_to_8_buf;

    //read data from file, only for test
    if ((pFile = fopen("/data/16k_16bit_1ch.wav", "rb+")) == NULL)
    {
        DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
        return NULL;
    }

    //init some buffer for CVSD and mSBC
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        pInBuf = (short *)malloc(CVSD_INPUT_SAMPLE*sizeof(short));
        if (NULL == pInBuf)
        {
            DBG_ERROR((BTHFP_TAG"malloc pInBuf failed!\r\n"));
            goto MALLOC_ERR;
        }

        //CVSD malloc downsample buff
        in_buf_size_in_byte = (CVSD_INPUT_SAMPLE*sizeof(short));
        s_out_8k = in_buf_size_in_byte/2;
        hdl_size_in_byte = CVSD_DownSample_GetMemory();
        p_16_to_8_down_hdl = (void *)malloc(hdl_size_in_byte);
        CVSD_DownSample_Init(p_16_to_8_down_hdl);
        p_out_16_to_8_buf = (short *)malloc(s_out_8k);
        p_temp_16_to_8_buf = (short *)malloc(in_buf_size_in_byte);

        DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    }
    else
    {
        pInBuf = (short *)malloc(MSBC_INPUT_SAMPLE*sizeof(short));
        if (NULL == pInBuf)
        {
            DBG_ERROR((BTHFP_TAG"malloc pInBuf failed!\r\n"));
            goto MALLOC_ERR;
        }
    }

    //start read data and do some processs
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    while (f_hfp_audio_run)
    {
        if(_bt_hfp_get_cvsd_msbc_type())
        {
            num = fread(pInBuf,sizeof(short),CVSD_INPUT_SAMPLE,pFile);
            if(num <= 0)
            {
                DBG_ERROR(("read error '%d' !!\n",num,__LINE__));
                break;
            } 

            //SRC from 16Khz to 8Khz
            CVSD_DownSample_Process_16_To_8(p_16_to_8_down_hdl,pInBuf,p_out_16_to_8_buf,p_temp_16_to_8_buf,in_buf_size_in_byte >> 1);
            in_buf_size_in_byte = (CVSD_INPUT_SAMPLE*sizeof(short));

            u_msglist_send(h_msglist,(void *)p_out_16_to_8_buf,CVSD_INPUT_BYTE,MSG_DATA_SEND);
        }
        else
        {
            num = fread(pInBuf,sizeof(short),MSBC_INPUT_SAMPLE,pFile);
            if(num <= 0)
            {
                DBG_ERROR(("read error '%d' !!\n",num,__LINE__));
                break;
            } 

            u_msglist_send(h_msglist,(void *)pInBuf,MSBC_INPUT_BYTE,MSG_DATA_SEND);
        }
    }

    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    fclose(pFile);
    free(pInBuf);
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        free(p_16_to_8_down_hdl);
        free(p_out_16_to_8_buf);
        free(p_temp_16_to_8_buf);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

    SET_UP_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    MALLOC_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;
}

/*-----------------------------------------------------------------------------
 * Name: _mic_in_write_bt_driver_process
 *
 * Description: This thread  write datalist to bt driver.
 ----------------------------------------------------------------------------*/
static VOID* _mic_in_write_bt_driver_process(VOID *arg)
{
    INT32 i4_ret;
    snd_pcm_t *handle;
    PCMContainer_t *pcm_params;
    MSGLIST_T * h_msglist = (MSGLIST_T *)arg;
    DATANODE_T * msg_mode;
    FILE *pFile;
    void * phead;

    if(dump_mic_in_write)
    {
        if ((pFile = fopen(MIC_IN_WRITE_PATH, "wb+")) == NULL)
        {
            DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
            return NULL;
        }
    }

    pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if (NULL == pcm_params)
    {
        DBG_ERROR((BTHFP_TAG"malloc failed!\r\n"));
        goto MALLOC_ERR;
    }
    memset(pcm_params, 0, sizeof(PCMContainer_t));
    pcm_params->format = SND_PCM_FORMAT_S16_LE;
    pcm_params->handle = handle;
    pcm_params->channels = 1;

    if(_bt_hfp_get_cvsd_msbc_type())
    {
        //CVSD
        pcm_params->sample_rate = 8000;
    }
    else
    {
        //MSBC
        pcm_params->sample_rate = 16000;
    }

    i4_ret = snd_pcm_open(&pcm_params->handle, BT_HFP_WRITE_DEVICE,SND_PCM_STREAM_PLAYBACK, 0);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_open failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    DBG_ERROR((BTHFP_TAG"snd_pcm_open success!\r\n"));

    if(_bt_hfp_get_cvsd_msbc_type())
    {
        //CVSD
        i4_ret = u_alsa_set_hw_params(pcm_params, 120000);
    }
    else
    {
        //MSBC
        i4_ret = u_alsa_set_hw_params(pcm_params, 60000);
    }

    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_hw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    i4_ret = u_alsa_set_sw_params(pcm_params, pcm_params->buffer_size);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_sw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    while (1)
    {
        msg_mode = u_msglist_receive(h_msglist);
        if(MSG_EXIT == msg_mode->id)
        {
            DBG_ERROR((BTHFP_TAG"<%s>line:%d.Thread exit.\r\n",__FUNCTION__,__LINE__));
            u_datanode_free(msg_mode);
            break;
        }
        phead = msg_mode->data;
        memcpy(pcm_params->data_buf,phead,msg_mode->size);
        if(dump_mic_in_write)//dump data,just for test
        {
            fwrite(pcm_params->data_buf, sizeof(char), pcm_params->chunk_bytes, pFile);
        }
        i4_ret = u_alsa_write_pcm(pcm_params, pcm_params->chunk_size);
        u_datanode_free(msg_mode);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    snd_pcm_drain(pcm_params->handle);
    free(pcm_params->data_buf);
    snd_pcm_close(pcm_params->handle);
    free(pcm_params);
    if(dump_mic_in_write)
    {
        fclose(pFile);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

SET_UP_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    free(pcm_params);
MALLOC_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    return NULL;
}

/*-----------------------------------------------------------------------------
 * Name: _bt_driver_in_process
 *
 * Description: This thread read bt driver data to datalist.
 *
 ----------------------------------------------------------------------------*/
static VOID* _bt_driver_in_process(VOID *arg)
{
    INT32 i4_ret;
    snd_pcm_t *handle;
    PCMContainer_t *pcm_params;
    MSGLIST_T * h_msglist = (MSGLIST_T *)arg;
    FILE *pFile;
    //unsigned long data_count = 0;
    //struct timeval sysTime = {0};

    if(dump_bt_driver_in)
    {

        if ((pFile = fopen(BT_DRIVER_IN_PATH, "wb+")) == NULL)
        {
            DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
            return NULL;
        }
    }

    pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if (NULL == pcm_params)
    {
        DBG_ERROR((BTHFP_TAG"malloc pcm_params failed!\r\n"));
        goto MALLOC_ERR;
    }
    memset(pcm_params, 0x00, sizeof(PCMContainer_t));
    pcm_params->format = SND_PCM_FORMAT_S16_LE;
    pcm_params->handle = handle;
    pcm_params->channels = BT_HFP_CHANNLE;
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        pcm_params->sample_rate = 8000;
    }
    else
    {
        pcm_params->sample_rate = 16000;
    }
    usleep(500 * 1000);
    i4_ret = snd_pcm_open(&pcm_params->handle, BT_HFP_READ_DEVICE,SND_PCM_STREAM_CAPTURE, 0);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_open failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        i4_ret = u_alsa_set_hw_params(pcm_params, 120000);
    }
    else
    {
        i4_ret = u_alsa_set_hw_params(pcm_params, 60000);
    }
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_hw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    i4_ret = u_alsa_set_sw_params(pcm_params, 1);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_sw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    while (f_hfp_audio_run)
    {
        //data_count ++;
        //gettimeofday(&sysTime, NULL);
        //DBG_ERROR(("<hfp_read>%s,data_count=%lu,pcm_params->chunk_bytes=%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,data_count,pcm_params->chunk_bytes,sysTime.tv_sec,sysTime.tv_usec/1000));

        i4_ret = u_alsa_read_pcm(pcm_params, pcm_params->chunk_size);
        if(dump_bt_driver_in)//dump data,just for test
        {
            DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
            fwrite(pcm_params->data_buf, sizeof(char), pcm_params->chunk_bytes, pFile);
        }

        u_msglist_send(h_msglist,pcm_params->data_buf,pcm_params->chunk_bytes,MSG_DATA_SEND);
        //gettimeofday(&sysTime, NULL);
        //DBG_ERROR(("<hfp_read>%s,Time end:(%lu.%03lu)s \n",__FUNCTION__,sysTime.tv_sec,sysTime.tv_usec/1000));
        //DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    }

    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    snd_pcm_drain(pcm_params->handle);
    free(pcm_params->data_buf);
    snd_pcm_close(pcm_params->handle);
    free(pcm_params);
    if(dump_bt_driver_in)
    {
        fclose(pFile);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

SET_UP_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    free(pcm_params);
MALLOC_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    return NULL;
}
/*-----------------------------------------------------------------------------
 * Name: _bt_driver_play_process
 *
 * Description: This thread  write datalist to alsa for playback.
 ----------------------------------------------------------------------------*/
static VOID* _bt_driver_play_process(VOID *arg)
{
    INT32 i4_ret;
    snd_pcm_t *handle;
    PCMContainer_t *hfp_main_pcm_params;
    MSGLIST_T * h_msglist = (MSGLIST_T *)arg;
    DATANODE_T * msg_mode;
    FILE *pFile;
    INT32 databuf_size = 0;
    //unsigned long data_count = 0;
    //struct timeval sysTime = {0};

    if(dump_bt_driver_in_play)
    {
        if ((pFile = fopen(BT_DRIVER_IN_PLAY_PATH, "wb+")) == NULL)
        {
            DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
            return NULL;
        }
    }

    hfp_main_pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if (NULL == hfp_main_pcm_params)
    {
        DBG_ERROR((BTHFP_TAG"malloc failed!\r\n"));
        goto MALLOC_ERR;
    }
    memset(hfp_main_pcm_params, 0, sizeof(PCMContainer_t));
    hfp_main_pcm_params->format = SND_PCM_FORMAT_S16_LE;
    hfp_main_pcm_params->handle = handle;
    hfp_main_pcm_params->channels = 1;
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        hfp_main_pcm_params->sample_rate = 8000;
    }
    else
    {
        hfp_main_pcm_params->sample_rate = 16000;
    }

    i4_ret = snd_pcm_open(&hfp_main_pcm_params->handle, "sub1",SND_PCM_STREAM_PLAYBACK, 0);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"snd_pcm_open failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    if(_bt_hfp_get_cvsd_msbc_type())
    {
        i4_ret = u_alsa_set_hw_params(hfp_main_pcm_params, 120000);
    }
    else
    {
        i4_ret = u_alsa_set_hw_params(hfp_main_pcm_params, 60000);
    }
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_hw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    i4_ret = u_alsa_set_sw_params(hfp_main_pcm_params, hfp_main_pcm_params->buffer_size);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_alsa_set_sw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    while (f_hfp_audio_run)
    {
        msg_mode = u_msglist_receive(h_msglist);
        if(MSG_EXIT == msg_mode->id)
        {
            DBG_ERROR((BTHFP_TAG"<%s>line:%d.Thread exit.\r\n",__FUNCTION__,__LINE__));
            u_datanode_free(msg_mode);
            break;
        }

        memcpy(hfp_main_pcm_params->data_buf,msg_mode->data,msg_mode->size);
        if(dump_bt_driver_in_play)//dump data,just for test
        {
            fwrite(hfp_main_pcm_params->data_buf, sizeof(char), hfp_main_pcm_params->chunk_bytes, pFile);
            DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
        }
        // write data to alsa
        //data_count ++;
        //gettimeofday(&sysTime, NULL);
        //DBG_ERROR(("<chong_play>%s,data_count=%lu,pcm_params->chunk_bytes=%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,data_count,pcm_params->chunk_bytes,sysTime.tv_sec,sysTime.tv_usec/1000));
        i4_ret = u_alsa_write_pcm(hfp_main_pcm_params, hfp_main_pcm_params->chunk_size);
        //gettimeofday(&sysTime, NULL);
        //DBG_ERROR(("<chong_play>%s,Time end:(%lu.%03lu)s \n",__FUNCTION__,sysTime.tv_sec,sysTime.tv_usec/1000));
        u_datanode_free(msg_mode);
    }

    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    snd_pcm_drain(hfp_main_pcm_params->handle);
    free(hfp_main_pcm_params->data_buf);
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    snd_pcm_close(hfp_main_pcm_params->handle);
    if(NULL != hfp_main_pcm_params)
    {
        free(hfp_main_pcm_params);
        hfp_main_pcm_params = NULL;
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    if(dump_bt_driver_in_play)
    {
        fclose(pFile);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

SET_UP_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    free(hfp_main_pcm_params);
    hfp_main_pcm_params = NULL;
MALLOC_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    return NULL;
}

#endif

/*-----------------------------------------------------------------------------
 * Name: _mic_in_thread
 *
 * Description: This API read audio data from TDM.
 *                    This thread will create two thread.
 *                    One will read TDM data to ringbuf;The other will write ringbuf to bt driver.
 ----------------------------------------------------------------------------*/
INT32 _mic_hfp_in_thread(VOID)
{
    INT32 i4_ret;
    MSGLIST_T *h_msglist;

    i4_ret = u_msglist_create(&h_msglist);
    if (i4_ret != 0)
    {
        DBG_ERROR((BTHFP_TAG"msglist malloc failed!.\r\n"));
        goto RINGBUF_ERR;
    }

    i4_ret = pthread_create(&t_mic_in_record, NULL, _mic_in_hfp_record_process, (VOID *)h_msglist);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"mic_in record thread create failed!\r\n"));
        goto TDM_IN_THREAD_ERR;
    }

    i4_ret = pthread_create(&t_mic_in_write_bt_driver, NULL, _mic_in_write_bt_driver_process, (VOID *)h_msglist);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"mic_in write bt driver thread create failed!\r\n"));
        goto PLAY_THREAD_ERR;
    }

    pthread_join(t_mic_in_record, NULL);
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    usleep(10 * 1000);
    u_msglist_send(h_msglist,NULL,0,MSG_EXIT);
    pthread_join(t_mic_in_write_bt_driver, NULL);

    if(NULL != h_msglist)
    {
        u_msglist_destroy(&h_msglist);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

PLAY_THREAD_ERR:
    pthread_cancel(t_mic_in_record);
TDM_IN_THREAD_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    if(NULL != h_msglist)
    {
        u_msglist_destroy(&h_msglist);
    }

RINGBUF_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    return NULL;
}

/*-----------------------------------------------------------------------------
 * Name: _bt_driver_in_thread
 *
 * Description: This API read audio data from bt hfp driver.
 *                    The data may be PCM or cvsd/msbc.
 *                    This thread will create two thread.
 *                    One will read bt driver data to ringbuf;The other will write ringbuf to alsa for playback.
 ----------------------------------------------------------------------------*/
INT32 _bt_driver_in_thread(VOID)
{
    INT32 i4_ret;
    MSGLIST_T *h_msglist;

    i4_ret = u_msglist_create(&h_msglist);
    if (i4_ret != 0)
    {
        DBG_ERROR((BTHFP_TAG"msglist malloc failed!.\r\n"));
        goto RINGBUF_ERR;
    }

    i4_ret = pthread_create(&t_bt_driver_in, NULL, _bt_driver_in_process, (VOID *)h_msglist);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"bt hfp in thread create failed!\r\n"));
        goto TDM_IN_THREAD_ERR;
    }

    i4_ret = pthread_create(&t_bt_driver_play, NULL, _bt_driver_play_process, (VOID *)h_msglist);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"bt hfp play thread create failed!\r\n"));
        goto PLAY_THREAD_ERR;
    }

    pthread_join(t_bt_driver_in, NULL);
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    usleep(10 * 1000);
    u_msglist_send(h_msglist,NULL,0,MSG_EXIT);
    pthread_join(t_bt_driver_play, NULL);

    if(NULL != h_msglist)
    {
        u_msglist_destroy(&h_msglist);
    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

PLAY_THREAD_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    pthread_cancel(t_bt_driver_in);
TDM_IN_THREAD_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    if(NULL != h_msglist)
    {
        u_msglist_destroy(&h_msglist);
    }
RINGBUF_ERR:
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    return NULL;
}

INT32 _bt_hfp_mic_spk_thread(VOID)
{
    INT32 i4_ret;

    DBG_ERROR((BTHFP_TAG"_bt_hfp_mic_spk_thread \r\n"));
    #ifdef WIFI_CHIP_6630
    system("amixer -c 0 cset name='O02 I05 Switch' 1");
    system("amixer -c 0 cset name='O02 I06 Switch' 1");
    system("amixer -c 0 cset name='O11 I02 Switch' 1");
    system("amixer -c 0 cset name='DAIBT Mux' 1");
    system("amixer -c 0 cset name='MRG BT O02 Switch' 1");
    system("amixer -c 0 cset name='PCM0 O02 Switch' 0");
    #endif
    #ifdef WIFI_CHIP_7668
    //RX
    system("amixer -c 0 cset name='O05 I00 Switch' 1");
    system("amixer -c 0 cset name='O06 I01 Switch' 1");
    //TX
    system("amixer -c 0 cset name='O00 I07 Switch' 1");
    system("amixer -c 0 cset name='O01 I08 Switch' 1");
    #endif
    i4_ret = pthread_create(&t_hfp_spk_in, NULL, _bt_driver_in_thread, NULL);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"_bt_driver_in_thread thread  create failed!\r\n"));
        goto HFP_MIC_IN_THREAD_ERR;
    }

    i4_ret = pthread_create(&t_hfp_mic_in, NULL, _mic_hfp_in_thread, NULL);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"_mic_in_thread thread create failed!\r\n"));
        goto HFP_DRIVER_IN_THREAD_ERR;
    }

    pthread_join(t_hfp_spk_in, NULL);
	pthread_join(t_hfp_mic_in, NULL);
    flag_exit = FALSE;
    DBG_ERROR((BTHFP_TAG"%s,%d. \r\n",__FUNCTION__,__LINE__));

    return NULL;

HFP_MIC_IN_THREAD_ERR:
    DBG_ERROR((BTHFP_TAG"%s,%d. \r\n",__FUNCTION__,__LINE__));
    pthread_cancel(t_hfp_spk_in);
HFP_DRIVER_IN_THREAD_ERR:
    DBG_ERROR((BTHFP_TAG"%s,%d. \r\n",__FUNCTION__,__LINE__));
    pthread_cancel(t_hfp_mic_in);
}
INT32 _bt_hfp_mic_spk_thread_create(VOID)
{
    INT32 i4_ret;
    pthread_t ntid;
    pthread_attr_t t_attr;

    BT_HFP_FUNC_ENTRY();
  
    while(flag_exit)
    {
        usleep(300 * 1000);
    }
    
    if(f_hfp_audio_run)
    {
        DBG_ERROR((BTHFP_TAG"thread exists!!! no need to create again. \n"));
        return NULL;
    }

    f_hfp_audio_run = TRUE;
    g_t_bt_hfp_sta.b_bt_hfp_mic_spk_thread_creat = TRUE;
    DBG_ERROR((BTHFP_TAG"_bt_hfp_mic_spk_thread_create, hfp_audio_connect=%d\n",f_hfp_audio_run));

    i4_ret = pthread_attr_init(&t_attr);
    if(0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"pthread_attr_init error!\n"));
        return NULL;
    }

    i4_ret = pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
    if(0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"pthread_attr_setdetachstate error!\n"));
        goto ATTR_SET_ERR;
    }

    i4_ret = pthread_create(&ntid, &t_attr, _bt_hfp_mic_spk_thread, NULL);
    if(0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"pthread_create error!\n"));
        goto ATTR_SET_ERR;
    }

    pthread_attr_destroy(&t_attr);

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;

ATTR_SET_ERR:
    pthread_attr_destroy(&t_attr);

    BT_HFP_FUNC_EXIT();
    return BT_HFP_FAIL;
}
INT32 _bt_hfp_mic_spk_thread_exit(VOID)
{
    INT32 i4_ret;

	BT_HFP_FUNC_ENTRY();

    DBG_ERROR((BTHFP_TAG"_bt_hfp_mic_spk_thread_exit \r\n"));

    if(TRUE == g_t_bt_hfp_sta.b_bt_hfp_mic_spk_thread_creat)
    {
        g_t_bt_hfp_sta.b_bt_hfp_mic_spk_thread_creat = FALSE;
    }
    else
    {
        DBG_ERROR((BTHFP_TAG"Err: no calls in progress , no thread exit!!! \n"));
        return NULL;
    }

    f_hfp_audio_run = FALSE;
    DBG_ERROR((BTHFP_TAG"_bt_hfp_mic_spk_thread_exit, hfp_audio_connect=%d\n",f_hfp_audio_run));

    flag_exit = TRUE;
    BT_HFP_FUNC_EXIT();
    return NULL;
}

VOID _bt_hfp_media_player_thread(VOID)
{
    INT32 i4_ret;

    DBG_ERROR((BTHFP_TAG"_bt_hfp_media_player_thread \r\n"));
    i4_ret = _bt_hfp_player_init();
    BT_HFP_CHK_FAIL_RET(_bt_hfp_player_init, i4_ret, BT_HFP_FAIL);

	i4_ret = setDataSource_url(g_t_g_bt_hfp.h_player, BT_HFP_RING_PATH,1);
	BT_HFP_CHK_FAIL(setDataSource_url, i4_ret);

	i4_ret = prepareAsync(g_t_g_bt_hfp.h_player);
	BT_HFP_CHK_FAIL(prepareAsync, i4_ret);

//	i4_ret = start(g_t_bt_hfp_sta.h_player);
//	BT_HFP_CHK_FAIL(start, i4_ret);

    BT_HFP_FUNC_EXIT();
}

INT32 _bt_hfp_media_player_thread_creat(VOID)
{
    INT32 i4_ret;

    DBG_ERROR((BTHFP_TAG"_bt_hfp_media_player_thread_creat \r\n"));
    i4_ret = pthread_create(&t_hfp_media_player, NULL, _bt_hfp_media_player_thread, NULL);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"bt hfp media player thread create failed!\r\n"));
        return BT_AUD_FAIL;
    }

    pthread_join(t_hfp_media_player, NULL);

	BT_HFP_FUNC_EXIT();
	return BT_AUD_OK;
}

INT32 _bt_hfp_media_player_thread_exit(VOID)
{
    INT32 i4_ret;

    DBG_ERROR((BTHFP_TAG"_bt_hfp_media_player_thread_exit \r\n"));
    _bt_hfp_player_deinit();

    i4_ret = pthread_cancel(t_hfp_media_player);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"t_hfp_mic_in thread cancle failed!\r\n"));
        return BT_HFP_FAIL;
    }

    return BT_HFP_OK;
}

VOID open_hfp_dump(INT8 cmd)
{
	BT_HFP_FUNC_ENTRY();
	
	switch(cmd)
    {
        case 0:
            dump_bt_driver_in = TRUE;
            dump_bt_driver_in_play = TRUE;
            dump_mic_in_record = TRUE;
            dump_mic_in_write = TRUE;
        break;
        case 1:
            dump_bt_driver_in = TRUE;
            dump_bt_driver_in_play = FALSE;
            dump_mic_in_record = FALSE;
            dump_mic_in_write = FALSE;
        break;
        case 2:
            dump_bt_driver_in = FALSE;
            dump_bt_driver_in_play = TRUE;
            dump_mic_in_record = FALSE;
            dump_mic_in_write = FALSE;
        break;
        case 3:
            dump_bt_driver_in = FALSE;
            dump_bt_driver_in_play = FALSE;
            dump_mic_in_record = TRUE;
            dump_mic_in_write = FALSE;
        break;
        case 4:
            dump_bt_driver_in = FALSE;
            dump_bt_driver_in_play = FALSE;
            dump_mic_in_record = FALSE;
            dump_mic_in_write = TRUE;
        case 5:
            system("rm -rf "BT_DRIVER_IN_PATH);
            system("rm -rf "BT_DRIVER_IN_PLAY_PATH);
            system("rm -rf "MIC_IN_RECORD_PATH);
            system("rm -rf "MIC_IN_WRITE_PATH);
        break;
        case 6:
            dump_bt_driver_in = TRUE;
            dump_bt_driver_in_play = TRUE;
            dump_mic_in_record = FALSE;
            dump_mic_in_write = FALSE;
        break;
        default:
        break;
    }
    DBG_ERROR((BTHFP_TAG"%s open data dump!\r\n",__FUNCTION__));
}
VOID _set_test_enable(VOID)
{
    f_hfp_audio_run = TRUE;
}

//test AEC from WebRtc, read data from file
VOID test_webrtc_file_aec(VOID)
{
    INT32 i4_ret=0,i,num;
    FILE *pFile,*pFile1,*pFile2;
    short *outbuf,*pTmp;
    short *pmic_in,*pspeaker_in;
    void *aecmInst;
    INT32 size = 80;

    if ((pFile = fopen("/data/speaker.pcm", "rb+")) == NULL)
    {
        DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
        return NULL;
    }
    if ((pFile1 = fopen("/data/micin.pcm", "rb+")) == NULL)
    {
        DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
        return NULL;
    }
    if ((pFile2 = fopen("/data/out.pcm", "wb+")) == NULL)
    {
        DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
        return NULL;
    }
    
    pmic_in = (short *)malloc(size*sizeof(short));
    pspeaker_in = (short *)malloc(size*sizeof(short));
    outbuf = (short *)malloc(size*sizeof(short));

    i4_ret = WebRtcAec_Create(&aecmInst);
    if(i4_ret != 0)
    {
        DBG_ERROR((BTHFP_TAG"<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
        return NULL;
    }
	i4_ret = WebRtcAec_Init(aecmInst,8000,8000);
    if(i4_ret != 0)
    {
        DBG_ERROR((BTHFP_TAG"<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
        return NULL;
    }

    //start read data and do some processs
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    while (1)
    {
        num = fread(pspeaker_in,sizeof(short),size,pFile);
        if(num <= 0)
        {
            DBG_ERROR(("read error '%d' !!\n",num,__LINE__));
            break;
        }     

        num = fread(pmic_in,sizeof(short),size,pFile1);
        if(num <= 0)
        {
            DBG_ERROR(("read error '%d' !!\n",num,__LINE__));
            break;
        }
        
        i4_ret = WebRtcAec_BufferFarend(aecmInst,pspeaker_in,size);
        if(i4_ret != 0)
        {
            DBG_ERROR((BTHFP_TAG"<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
        }
        
        i4_ret = WebRtcAec_Process(aecmInst,pmic_in,NULL,outbuf,NULL,size,40,0);
        if(i4_ret != 0)
        {
            DBG_ERROR((BTHFP_TAG"<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
        }
        
        num = fwrite(outbuf,sizeof(short),size,pFile2);
        if(num <= 0)
        {
            DBG_ERROR(("write error '%d' !!\n",num,__LINE__));
        }

    }
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    WebRtcAec_Free(aecmInst);
    free(outbuf);
    free(pmic_in);
    free(pspeaker_in);
    fclose(pFile);
    fclose(pFile1);
    fclose(pFile2);
    DBG_ERROR((BTHFP_TAG"<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;
}

//test internal DAC and AWB loopback,please push 9.wav to /data/
VOID * internal_dac_play(VOID * arg)
{
    INT32 i4_ret;
    snd_pcm_t *handle;
    PCMContainer_t *pcm_params;
    FILE *pFile;

    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
  
    pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if (NULL == pcm_params)
    {
        DBG_ERROR(("malloc failed!\r\n"));
        goto MALLOC_ERR;
    }
    memset(pcm_params, 0, sizeof(PCMContainer_t));
    pcm_params->format = SND_PCM_FORMAT_S16_LE;
    pcm_params->handle = handle;
    pcm_params->channels = 2;
    pcm_params->sample_rate = 48000;
    
    i4_ret = snd_pcm_open(&pcm_params->handle, "hw:0,6",SND_PCM_STREAM_PLAYBACK, 0);
    if (0 != i4_ret)
    {
        DBG_ERROR(("snd_pcm_open failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    i4_ret = u_alsa_set_hw_params(pcm_params, 80000);
    
    if (0 != i4_ret)
    {
        DBG_ERROR(("u_alsa_set_hw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }

    i4_ret = u_alsa_set_sw_params(pcm_params, pcm_params->buffer_size);
    if (0 != i4_ret)
    {
        DBG_ERROR(("u_alsa_set_sw_params failed! line:%d.\r\n",__LINE__));
        goto SET_UP_ERR;
    }
    
    if ((pFile = fopen("/data/9.wav", "rb")) == NULL)
    {
        DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
        return NULL;
    }

    while (1)
    {
        i4_ret = fread(pcm_params->data_buf,sizeof(char),pcm_params->chunk_bytes,pFile);
        if(i4_ret < 0)
        {
            DBG_ERROR(("read file error. line:%d.\r\n",__LINE__));
            break;
        }
        i4_ret = u_alsa_write_pcm(pcm_params, pcm_params->chunk_size);
    }

    snd_pcm_drain(pcm_params->handle);
    free(pcm_params->data_buf);
    snd_pcm_close(pcm_params->handle);
    if(NULL != pcm_params)
    {
        free(pcm_params);
        pcm_params = NULL;
    }
    fclose(pFile);
    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    return NULL;

SET_UP_ERR:
    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    free(pcm_params);
    pcm_params = NULL;
MALLOC_ERR:
    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    return NULL;
}

//test merge API, adjust the TDM IN data and AWB data for AEC.Please kill Assiscenter first
//test AEC from WebRtc
VOID * aec_test(VOID * arg)
{
    short *buffer,*mic_in,*speaker,*aec;
    int buffer_size,buffer_bytes;
    int read_size;
    long long tstamp;
    int flag;
    int i4_ret;
    int i;
    int aec_size;
    FILE *pFile,*pFile1,*pFile2;
    void *aecInst;
    
    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    struct merge_config config = *(struct merge_config *)arg;

    if ((pFile = fopen("/data/micin.pcm", "wb")) == NULL)
    {
        DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
        return NULL;
    }
    if ((pFile1 = fopen("/data/speaker.pcm", "wb")) == NULL)
    {
        DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
        return NULL;
    }
    if ((pFile2 = fopen("/data/aec.pcm", "wb")) == NULL)
    {
        DBG_ERROR(("Cannot create output file '%s' !!\n",__FUNCTION__));
        return NULL;
    }
    DBG_ERROR(("%s config.main.chnum=%d.\n",__FUNCTION__,config.main.chnum));
    
    buffer_size = config.period_time*config.out.rate/1000;
    aec_size = 160;
    buffer_bytes = buffer_size*config.out.bitdepth*config.out.chnum/8;
    buffer = (short *)malloc(buffer_bytes);
    mic_in = (short *)malloc(aec_size*sizeof(short));
    speaker = (short *)malloc(aec_size*sizeof(short));
    aec = (short *)malloc(aec_size*sizeof(short));

    //init aec
    i4_ret = WebRtcAec_Create(&aecInst);
    if(i4_ret != 0)
    {
        DBG_ERROR(("<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
    }
	i4_ret = WebRtcAec_Init(aecInst,16000,16000);
    if(i4_ret != 0)
    {
        DBG_ERROR(("<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
    }

    while(1)
    {
        read_size = buffer_size;
        i4_ret = merge_read(merge_handle,(void *)buffer,&read_size,&tstamp,&flag); 
        if(i4_ret < 0)
        {
            DBG_ERROR(("merge fail.%d",__LINE__));  
        }
        for(i=0;i<buffer_size;i++)
        {
            mic_in[i%aec_size] = buffer[i*config.out.chnum+0];
            speaker[i%aec_size] = buffer[i*config.out.chnum+config.out.chnum-1];

            if((i+1)%aec_size == 0)
            {
                DBG_ERROR(("<%s>line:%d.packet:%d\r\n",__FUNCTION__,__LINE__,i/aec_size));
                
                fwrite(mic_in,sizeof(short),aec_size,pFile);
                fwrite(speaker,sizeof(short),aec_size,pFile1);

                i4_ret = WebRtcAec_BufferFarend(aecInst,speaker,aec_size);
                if(i4_ret != 0)
                {
                    DBG_ERROR(("<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
                }
                
                i4_ret = WebRtcAec_Process(aecInst,mic_in,NULL,aec,NULL,aec_size,0,0);
                if(i4_ret != 0)
                {
                    DBG_ERROR(("<%s>line:%d.error\r\n",__FUNCTION__,__LINE__));
                }
                fwrite(aec,sizeof(short),aec_size,pFile2);
            }
        }
    }

    free(buffer);
    free(mic_in);
    free(speaker);
    free(aec);
    fclose(pFile);
    fclose(pFile1);
    fclose(pFile2);
    WebRtcAec_Free(aecInst);
    return NULL;
}
INT32 init_merge(struct merge_config *config)
{
    const char *device[]={"hw:0,1","hw:0,7"};
    int ret;

    config->main.type = PCM_HW_ALSA_LIB;
    config->main.name = (char *)device[0];
    config->main.chnum = 8;
    config->main.bitdepth = 32;
    config->main.rate = 16000;
    config->main.period_count = 8;

    config->sub.type = PCM_HW_ALSA_LIB;
    config->sub.name = (char *)device[1];
    config->sub.chnum = 2;
    config->sub.bitdepth = 16;
    config->sub.rate = 48000;
    config->sub.period_count = 8;

    config->out.chnum = 9;
    config->out.bitdepth = 16;
    config->out.rate = 16000;
    config->out.period_count = 8;
    config->out.use_main_chnum = 7;
    config->out.use_sub_chnum = 2;
    config->out.chnum_idx[0] = 0;
    config->out.chnum_idx[1] = 1;
    config->out.chnum_idx[2] = 2;
    config->out.chnum_idx[3] = 3;
    config->out.chnum_idx[4] = 4;
    config->out.chnum_idx[5] = 5;
    config->out.chnum_idx[6] = 6;
    config->out.chnum_idx[7] = 0;
    config->out.chnum_idx[8] = 1;

    config->period_time = 20;
    config->offset_time = 0;
    config->cb = NULL;
    config->cb_priv = NULL;
    config->config_flags = 0;

    ret=merge_create(&merge_handle,config);
    if(ret!=0||merge_handle == NULL)
        return ret;
    ret=merge_start(merge_handle);
    if(ret!=0)
    {   
        merge_destroy(merge_handle);
        return ret;
    }
    else
    {
        return 0;
    }
    
}

//1.test internal DAC and AWB loopback
//2.test merge API, adjust the TDM IN data and AWB data for AEC
//3.test AEC from WebRtc
VOID test_internal_dac_aec(VOID)
{   
    pthread_t ntid;
    pthread_attr_t t_attr;
    pthread_t ntid1;
    pthread_attr_t t_attr1;
    INT32 i4_ret = 0;
    struct merge_config *config;
    
    //DL1 playback by internal DAC
    system("amixer -c 0 cset name='O03 I05 Switch' 1");
    system("amixer -c 0 cset name='O04 I06 Switch' 1");
    system("amixer -c 0 cset name='INT ADDA O03_O04 Switch' 1");
    system("amixer -c 0 cset name='HPOUT Mux' AUDIO_AMP");
    system("amixer -c 0 cset name='HP Ext Amp Switch' 1");
    //DL1 AWB loopback
    system("amixer -c 0 cset name='O05 I05 Switch' 1");
    system("amixer -c 0 cset name='O06 I06 Switch' 1");

    config = (struct merge_config *)malloc(sizeof(struct merge_config));
    memset((void *)config,0,sizeof(struct merge_config));
    i4_ret = init_merge(config);
    if(0 != i4_ret)
    {
        DBG_ERROR(("init_merge error!\n"));
        return NULL;
    }
    DBG_ERROR(("init_merge sucess!config.main.chnum=%d.\n",config->main.chnum));
    
    i4_ret = pthread_create(&ntid, NULL, internal_dac_play, NULL);
    if(0 != i4_ret)
    {
        DBG_ERROR(("pthread_create error!\n"));
        goto ATTR_SET_ERR;
    }
    
    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    i4_ret = pthread_attr_init(&t_attr1);
    if(0 != i4_ret)
    {
        DBG_ERROR(("pthread_attr_init error!\n"));
        return NULL;
    }

    i4_ret = pthread_attr_setdetachstate(&t_attr1, PTHREAD_CREATE_DETACHED);
    if(0 != i4_ret)
    {
        DBG_ERROR(("pthread_attr_setdetachstate error!\n"));
        goto ATTR_SET_ERR;
    }
    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    i4_ret = pthread_create(&ntid1, &t_attr1, aec_test, (void *)config);
    if(0 != i4_ret)
    {
        DBG_ERROR(("pthread_create error!\n"));
        goto ATTR_SET_ERR;
    }
    
    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));

    pthread_join(ntid, NULL);
    DBG_ERROR(("<%s>line:%d.\r\n",__FUNCTION__,__LINE__));
    pthread_attr_destroy(&t_attr1);
    return NULL;

ATTR_SET_ERR:
    
    return NULL;
}

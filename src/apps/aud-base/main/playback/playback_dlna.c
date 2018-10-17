/* public */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/* application level */
#include "u_amb.h"
#include "u_common.h"
#include "u_app_thread.h"
#include "u_cli.h"
#include "u_os.h"
#include "u_timerd.h"
#include "u_assert.h"
#include "u_sm.h"
#include "u_dm.h"
#include "u_playback_dlna.h"
#include "u_assistant_stub.h"
#include "u_datalist.h"
#include "Interface.h"
/* private */
#include "playback_dlna.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/
#define DLNA_ASSERT(cond, action, ...)       \
    do {                                    \
        if (cond)                           \
        {                                   \
            DLNA_ERR(__VA_ARGS__);           \
            action;                         \
        }                                   \
    }                                       \
    while (0)

#define DLNA_BUFFER_LEN   2
#define DLNA_REV_FIFO_NAME "/tmp/dlna_rev_fifo"  //recieve cmd data from DMR
#define DLNA_SEND_FIFO_NAME "/tmp/dlna_send_fifo" //send SM request to DMR


/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * static variable declarations
 *---------------------------------------------------------------------------*/
static UINT16 g_ui2_playback_dlna_dbg_level = DBG_INIT_LEVEL_APP_PLAYBACK_DLNA;
static PLAYBACK_DLNA_OBJ_T g_t_dlna = {0};
static pthread_t t_dlna_write_thread = 0;

/*-----------------------------------------------------------------------------
* private methods declarations
*---------------------------------------------------------------------------*/

UINT16 playback_dlna_get_dbg_level(VOID)
{
    return (g_ui2_playback_dlna_dbg_level | DBG_LAYER_APP);
}

VOID playback_dlna_set_dbg_level(UINT16 ui2_db_level)
{
    g_ui2_playback_dlna_dbg_level = ui2_db_level;
}


INT32 u_playback_dlna_send_msg(const PB_DLNA_MSG_T* pt_event)
{
    INT32 i4_ret;

    if (FALSE == g_t_dlna.b_app_init_ok)
    {
        return PB_DLNA_FAIL;
    }

    i4_ret = u_app_send_msg(g_t_dlna.h_app,
                            E_APP_MSG_TYPE_DLNA,
                            pt_event,
                            sizeof(PB_DLNA_MSG_T),
                            NULL,
                            NULL);
    DLNA_ASSERT(AEER_OK != i4_ret, , "_app_send_msg sent E_APP_MSG_TYPE_DLNA failed, i4_ret[%ld]\n",i4_ret);
    return (i4_ret == AEER_OK ? PB_DLNA_OK : PB_DLNA_FAIL);
}

static VOID _playback_dlna_send_self_msg(UINT32 ui4_msg_id)
{
    PB_DLNA_MSG_T pb_dlna_msg;
    memset(&pb_dlna_msg, 0x00, sizeof(PB_DLNA_MSG_T));
    pb_dlna_msg.ui4_msg_id = ui4_msg_id;
    u_playback_dlna_send_msg(&pb_dlna_msg);
}



static int _playback_dlna_open_rev_fifo(VOID)
{
    int server_fifo_fd;

    if(-1 == access(DLNA_REV_FIFO_NAME, F_OK))
    {
        if (-1 == mkfifo(DLNA_REV_FIFO_NAME, 0777))
        {
            DLNA_ERR("create dlna rev fifo error!\n");
            return PB_DLNA_FAIL;
        }
    }

    server_fifo_fd = open(DLNA_REV_FIFO_NAME, O_RDONLY);
    if (-1 == server_fifo_fd)
    {
        DLNA_ERR("open dlna rev fifo error!\n");
        return PB_DLNA_FAIL;
    }

    return server_fifo_fd;
}

static int _playback_dlna_open_send_fifo(VOID)
{
    int server_fifo_fd;

    if(-1 == access(DLNA_SEND_FIFO_NAME, F_OK))
    {
        if (-1 == mkfifo(DLNA_SEND_FIFO_NAME, 0777))
        {
            DLNA_ERR("create dlna send fifo error!\n");
            return PB_DLNA_FAIL;
        }
    }

    server_fifo_fd = open(DLNA_SEND_FIFO_NAME, O_WRONLY);
    if (-1 == server_fifo_fd)
    {
        DLNA_ERR("open dlna send fifo error!\n");
        return PB_DLNA_FAIL;
    }

    return server_fifo_fd;
}



void *_playback_dlna_write_data_thread(void* arg)
{
	int ret;
    int i4_fifo_fd;
	char dlnaWrite;
	i4_fifo_fd = _playback_dlna_open_send_fifo();
	if (-1 == i4_fifo_fd)
	{
		DLNA_ERR("dlna send fifo open failed!\n");
		return NULL;
	}
	DLNA_ERR("dlna send cmd %s to render\n",
		(g_t_dlna.t_play_msg.ui4_msg_id == DLNA_STOP_REQ)?"DLNA_STOP_REQ":\
		(g_t_dlna.t_play_msg.ui4_msg_id == DLNA_PLAY_REQ)?"DLNA_PLAY_REQ":\
		(g_t_dlna.t_play_msg.ui4_msg_id == DLNA_PAUSE_REQ)?"DLNA_PAUSE_REQ":\
		(g_t_dlna.t_play_msg.ui4_msg_id == DLNA_STOP_DONE)?"DLNA_STOP_DONE":\
		(g_t_dlna.t_play_msg.ui4_msg_id == DLNA_PLAY_DONE)?"DLNA_PLAY_DONE":\
		(g_t_dlna.t_play_msg.ui4_msg_id == DLNA_PAUSE_DONE)?"DLNA_PAUSE_DONE":"Other");
	switch(g_t_dlna.t_play_msg.ui4_msg_id)
	{
		case DLNA_STOP_REQ:
			dlnaWrite = '0';
		break;
		case DLNA_PLAY_REQ:
			dlnaWrite = '1';
		break;
		case DLNA_PAUSE_REQ:
			dlnaWrite = '2';
		break;
		case DLNA_STOP_DONE:
			dlnaWrite = '3';
		break;
		case DLNA_PLAY_DONE:
			dlnaWrite = '4';
		break;
		case DLNA_PAUSE_DONE:
			dlnaWrite = '5';
		break;		
		default:
			break;
	}
	ret = write(i4_fifo_fd,&dlnaWrite,1);
	close(i4_fifo_fd);
	if(ret != 1)
		DLNA_ERR("dlna send cmd fail to DMR\n");
    return NULL;
}



void *_playback_dlna_recv_data_thread(void* arg)
{
    int ret;
    int i4_fifo_fd;
	char dlnaCmd[DLNA_BUFFER_LEN];

    pthread_detach(pthread_self());

    while (1)
    {
        i4_fifo_fd = _playback_dlna_open_rev_fifo();
        if (-1 == i4_fifo_fd)
        {
            DLNA_ERR("dlna rev fifo open failed!\n");
            return NULL;
        }

        while (read(i4_fifo_fd, &dlnaCmd, sizeof(dlnaCmd)) > 0)
        {
            DLNA_INFO("dlnaCmd=%s\n",dlnaCmd);			
			switch(dlnaCmd[0])
			{
				case '0':
					DLNA_ERR("DLNA STOP REQ\n");
					_playback_dlna_send_self_msg(DLNA_STOP_REQ);
				break;
				case '1':
					DLNA_ERR("DLNA PLAY REQ\n");
					_playback_dlna_send_self_msg(DLNA_PLAY_REQ);
				break;
				case '2':
					DLNA_ERR("DLNA PAUSE REQ\n");
					_playback_dlna_send_self_msg(DLNA_PAUSE_REQ);
				break;
				case '3':
					_playback_dlna_send_self_msg(DLNA_STOP_DONE);
				break;
				case '4':
					_playback_dlna_send_self_msg(DLNA_PLAY_DONE);
				break;
				case '5':
					_playback_dlna_send_self_msg(DLNA_PAUSE_DONE);
				break;				
				default:
					DLNA_ERR("recieve invalid cmd from DMR\n");
				break;
			}
        }
        close(i4_fifo_fd);
    }
    DLNA_ERR("_playback_tts_recv_data_thread exit \n");
    return NULL;
}



static INT32 _playback_dlna_init(
        const CHAR*                 ps_name,
        HANDLE_T                    h_app
        )
{
    INT32 i4_ret;
    pthread_t t_data_thread;

    memset(&g_t_dlna, 0, sizeof(PLAYBACK_DLNA_OBJ_T));
    g_t_dlna.h_app = h_app;

    if (g_t_dlna.b_app_init_ok)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = playback_dlna_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        DLNA_ERR("Err: playback_dlna_cli_attach_cmd_tbl() failed, ret=%ld\r\n", i4_ret);
        return AEER_FAIL;
    }
#endif/* CLI_SUPPORT */

    i4_ret = pthread_mutex_init(&g_t_dlna.t_data_mutex, NULL);
    DLNA_ASSERT(i4_ret, return AEER_FAIL, "pthread_mutex_init failed, ret=%ld\n", i4_ret);

    i4_ret = pthread_create(&t_data_thread, NULL, _playback_dlna_recv_data_thread, NULL);
    DLNA_ASSERT(i4_ret, return AEER_FAIL, "data pthread create failed, ret=%ld\n", i4_ret);

    g_t_dlna.b_app_init_ok = TRUE;

    return AEER_OK;
}

static INT32 _playback_dlna_exit (
        HANDLE_T                    h_app,
        APP_EXIT_MODE_T             e_exit_mode
        )
{
    if (FALSE == g_t_dlna.b_app_init_ok)
    {
        return AEER_FAIL;
    }

    pthread_mutex_destroy(&g_t_dlna.t_data_mutex);

    g_t_dlna.b_app_init_ok = FALSE;

    return AEER_OK;
}

static INT32 _playback_dlna_inform_playback_status_to_sm(SM_MSG_BDY_E PB_BODY)
{
    INT32 i4_ret = PB_DLNA_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_DLNA,
                                SM_MAKE_MSG(SM_INFORM_GRP,PB_BODY,0),
                                &t_sm_param,
                                sizeof(t_sm_param));
    if (AEER_OK != i4_ret)
    {
        DLNA_ERR("_playback_dlna_inform_playback_status_to_sm failed, i4_ret[%ld]\n",i4_ret);
    }
    DLNA_INFO("playback send %d status to sm!\n", PB_BODY);

    return (i4_ret == AEER_OK ? PB_DLNA_OK : PB_DLNA_FAIL);
}


static INT32 _playback_dlna_request_playback_to_sm(SM_MSG_BDY_E PB_BODY)
{
    INT32 i4_ret = PB_DLNA_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_DLNA,
                                SM_MAKE_MSG(SM_REQUEST_GRP,PB_BODY,0),
                                &t_sm_param,
                                sizeof(t_sm_param));
    if (AEER_OK != i4_ret)
    {
        DLNA_ERR("_playback_dlna_request_playback_to_sm failed, i4_ret[%ld]\n",i4_ret);
    }
    DLNA_INFO("playback send %d status to sm!\n", PB_BODY);

    return (i4_ret == AEER_OK ? PB_DLNA_OK : PB_DLNA_FAIL);
}


INT32 playback_dlna_process_self_msg(APPMSG_T* pv_msg)
{
    PB_DLNA_MSG_T *pt_dlna_msg = (PB_DLNA_MSG_T *)pv_msg;
    DLNA_ERR("dlna msg id [%d]\n", pt_dlna_msg->ui4_msg_id);

    switch (pt_dlna_msg->ui4_msg_id)
    {
	   case DLNA_STOP_REQ:
			 _playback_dlna_request_playback_to_sm(SM_BODY_STOP);
			 break;
       case DLNA_PLAY_REQ:
            _playback_dlna_request_playback_to_sm(SM_BODY_PLAY);
            break;
       case DLNA_PAUSE_REQ:
            _playback_dlna_request_playback_to_sm(SM_BODY_PAUSE);
            break;
       case DLNA_STOP_DONE:
            _playback_dlna_inform_playback_status_to_sm(SM_BODY_STOP);
            break;
       case DLNA_PLAY_DONE:
            _playback_dlna_inform_playback_status_to_sm(SM_BODY_PLAY);
            break;
		case DLNA_PAUSE_DONE:
			_playback_dlna_inform_playback_status_to_sm(SM_BODY_PLAY);
			break;
       default:
            break;
    }
    return AEER_OK;
}


static VOID _playback_dlna_process_sm_msg(SM_MSG_BDY_E P_BODY)
{
	INT32 i4_ret;
    if (t_dlna_write_thread)
    {
        pthread_join(t_dlna_write_thread, NULL);
        t_dlna_write_thread = 0;
        DLNA_INFO("dlna write data thread join!\n");
    }
	g_t_dlna.t_play_msg.ui4_msg_id = P_BODY;
    i4_ret = pthread_create(&t_dlna_write_thread, NULL, _playback_dlna_write_data_thread, NULL);
    if (i4_ret)
    {
        DLNA_ERR("write data thread create failed, ret:[%d]!\n", i4_ret);
    }
	return;
}



INT32 playback_dlna_process_sm_msg(APPMSG_T* pv_msg)
{
    DLNA_INFO("sm msg grp:[%d] bdy:[%d] id:[%d]\n", SM_MSG_GRP(pv_msg->ui4_msg_type), SM_MSG_BDY(pv_msg->ui4_msg_type), SM_MSG_ID(pv_msg->ui4_msg_type));
    switch(SM_MSG_GRP(pv_msg->ui4_msg_type))
    {
        case SM_REQUEST_GRP:
        case SM_PERMIT_GRP:
            _playback_dlna_process_sm_msg(SM_MSG_BDY(pv_msg->ui4_msg_type));
            break;
        case SM_FORBID_GRP:
            DLNA_ERR("forbid msg body[%d]\n", SM_MSG_BDY(pv_msg->ui4_msg_type));
            break;
        default:
            break;
    }
    return AEER_OK;
}


static INT32 _playback_dlna_process_msg (
        HANDLE_T                    h_app,
        UINT32                      ui4_type,
        const VOID*                 pv_msg,
        SIZE_T                      z_msg_len,
        BOOL                        b_paused
        )
{
    APPMSG_T *pt_app_msg = pv_msg;

    if (FALSE == g_t_dlna.b_app_init_ok) {
        return AEER_FAIL;
    }

    if (NULL == pv_msg)
    {
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        /* private message */
        switch(ui4_type)
        {
            case E_APP_MSG_TYPE_DLNA:
				DLNA_ERR("playback_dlna_process_self_msg\n");
                playback_dlna_process_self_msg(pt_app_msg);
                break;
            case E_APP_MSG_TYPE_STATE_MNGR:
				DLNA_ERR("playback_dlna_process_sm_msg\n");
                playback_dlna_process_sm_msg(pt_app_msg);
                break;
            default:
                break;
        }
    }
    return AEER_OK;
}


VOID a_playback_dlna_register(AMB_REGISTER_INFO_T* pt_reg)
{
    if (TRUE == g_t_dlna.b_app_init_ok)
    {
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, PB_DLNA_THREAD_NAME, APP_NAME_MAX_LEN);

    pt_reg->t_fct_tbl.pf_init                   = _playback_dlna_init;
    pt_reg->t_fct_tbl.pf_exit                   = _playback_dlna_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _playback_dlna_process_msg;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = PB_DLNA_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = PB_DLNA_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = PB_DLNA_NUM_MSGS;
    pt_reg->t_desc.ui2_msg_count                = PB_DLNA_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = PB_DLNA_MAX_MSGS_SIZE;
}


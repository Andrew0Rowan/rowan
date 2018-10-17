#ifdef CLI_SUPPORT
#include "u_cli.h"
#include "u_playback_dlna.h"

#include "playback_dlna.h"
#include "playback_dlna_cli.h"
/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _playback_dlna_msg(INT32 i4_argc, const CHAR** pps_argv);
static INT32 _playback_dlna_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv);
static INT32 _playback_dlna_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv);

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/

/* command table */
static CLI_EXEC_T at_pb_dlna_cmd_tbl[] =
{
    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _playback_dlna_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        CLI_SET_DBG_LVL_STR,
        NULL,
        _playback_dlna_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        "dlna_msg",
        "msg",
        _playback_dlna_msg,
        NULL,
        "[dlna]dlna_play_start",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

/* SVL Builder root command table */
static CLI_EXEC_T at_playback_dlna_root_cmd_tbl[] =
{
    {   "playback_dlna",
        "dlna",
        NULL,
        at_pb_dlna_cmd_tbl,
        "playback_dlna commands",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

INT32 playback_dlna_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(at_playback_dlna_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}

static INT32 _playback_dlna_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = u_cli_show_dbg_level(playback_dlna_get_dbg_level());

    return i4_ret;
}

static INT32 _playback_dlna_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    UINT16 ui2_dbg_level;
    INT32  i4_ret;

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);

    if (i4_ret == CLIR_OK){
        playback_dlna_set_dbg_level(ui2_dbg_level);
    }

    return i4_ret;
}

static INT32 _playback_dlna_msg(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret=0;

    PB_DLNA_MSG_T t_msg;

    t_msg.ui4_msg_id = atoi(pps_argv[1]);
    t_msg.ui4_data1  = 0;
    t_msg.ui4_data2  = 0;
    t_msg.ui4_data3  = 0;

    u_playback_dlna_send_msg(&t_msg);

    return i4_ret;
}

#endif

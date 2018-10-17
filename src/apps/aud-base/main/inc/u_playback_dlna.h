#ifndef _U_PLAYBACK_DLNA_H_
#define _U_PLAYBACK_DLNA_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"

#define PB_DLNA_OK                            ((INT32)0)
#define PB_DLNA_FAIL                          ((INT32)-1) /* abnormal return must < 0 */
#define PB_DLNA_INV_ARG                       ((INT32)-2)

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
/*the struct of playback mic in application's message*/
typedef struct _PB_DLNA_MSG_T
{
    UINT32          ui4_msg_id;
    UINT32          ui4_data1;
    UINT32          ui4_data2;
    UINT32          ui4_data3;
} PB_DLNA_MSG_T;

typedef enum
{
    DLNA_STOP_REQ = 0,
    DLNA_PLAY_REQ,
    DLNA_PAUSE_REQ,
    DLNA_STOP_DONE,
    DLNA_PLAY_DONE,
    DLNA_PAUSE_DONE,
    DLNA_RESUME,
} DLNA_CONTROL_MSG_T;

extern INT32 u_playback_dlna_send_msg(const PB_DLNA_MSG_T* pt_event);
#endif
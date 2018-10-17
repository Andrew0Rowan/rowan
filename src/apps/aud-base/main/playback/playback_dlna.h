#ifndef _PLAYBACK_DLNA_H_
#define _PLAYBACK_DLNA_H_

#include "u_handle.h"
#include "u_dbg.h"


#define DBG_INIT_LEVEL_APP_PLAYBACK_DLNA (DBG_LEVEL_ALL|DBG_LAYER_APP)

#undef  DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE playback_dlna_get_dbg_level()

#define DLNA_TAG            "<DLNA>"
#define DLNA_INFO(fmt, args...) do{DBG_INFO(("%s[%s:%d]:"fmt, DLNA_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define DLNA_ERR(fmt, args...)  do{DBG_ERROR(("%s[%s:%d]:"fmt, DLNA_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define DLNA_MSG(fmt, args...)  do{DBG_ERROR(("%s[%s:%d]:"fmt, DLNA_TAG, __FUNCTION__, __LINE__, ##args));}while(0)


typedef enum
{
    DLNA_PLAY_NORMAL = 0,
    DLNA_PLAY_ERROR
} PLAYBACK_DLNA_PLAY_RESULT_E;

typedef struct _PLAYBACK_DLNA_PLAY_MSG_T
{
    UINT32          ui4_msg_id;
    PLAYBACK_DLNA_PLAY_RESULT_E      e_play_result;
} PLAYBACK_DLNA_PLAY_MSG_T;

/* application structure */
typedef struct _PLAYBACK_DLNA_OBJ_T
{
    HANDLE_T                    h_app;
    BOOL                        b_app_init_ok;
    pthread_mutex_t             t_data_mutex;
    PLAYBACK_DLNA_PLAY_MSG_T    t_play_msg;
} PLAYBACK_DLNA_OBJ_T;

#endif

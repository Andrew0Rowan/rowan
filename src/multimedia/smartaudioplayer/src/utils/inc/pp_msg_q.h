#ifndef _PP_MSG_Q_H_
#define _PP_MSG_Q_H_

#include "common.h"
#include "os.h"


#undef NDEBUG

#ifndef NDEBUG
    #define CHECK_ASSERT(ret) do{int y=(int)ret;if(y!=0){assert(0);printf("CHECK_ASSERT in fun %s at line %d ret:%d\n",__FILE__,__LINE__,y);}}while(0)
    #define ASSERT(x)		do{int y=(int)(x);assert(y);}while(0)
    #define VERIFY(x)		ASSERT(x)

#else	// NDEBUG
    #define CHECK_ASSERT(ret)  ((void)ret)
    #define ASSERT(x)		((void)x)
    #define VERIFY(x)		((void)(x))

#endif	// NDEBUG


#define ABORT(cat,code) DBG_ABORT(code)
#define NULL_HANDLE (0)
#define SIZE_T size_t

#define TMS_LEVEL_NONE 0x0

#define DBG_PREFIX
#define DBG_INFO_EX DBG_INFO
#define DBG_ERROR_EX DBG_ERROR

#define UNUSED(x) 

#ifdef  __cplusplus
extern "C" {
#endif

/* MsgQ API's */
extern INT32 x_msg_q_create (HANDLE_T*    ph_msg_q,
                             const CHAR*  ps_name,
                             SIZE_T       z_msg_size,
                             UINT16       ui2_msg_count);

extern INT32 x_msg_q_attach (HANDLE_T*    ph_msg_q,
                             const CHAR*  ps_name);

extern INT32 x_msg_q_delete (HANDLE_T  h_msg_q);

extern INT32 x_msg_q_send (HANDLE_T     h_msg_q,
                           const VOID*  pv_msg,
                           SIZE_T       z_size,
                           UINT8        ui1_priority);

extern INT32 x_msg_q_receive (UINT16*        pui2_index,
                              VOID*          pv_msg,
                              SIZE_T*        pz_size,
                              HANDLE_T*      ph_msg_q_mon_list,
                              UINT16         ui2_msg_q_mon_count,
                              MSGQ_OPTION_T  e_options);

extern INT32 x_msg_q_receive_timeout (UINT16*    pui2_index,
                                      VOID*      pv_msg,
                                      SIZE_T*    pz_size,
                                      HANDLE_T*  ph_msg_q_mon_list,
                                      UINT16     ui2_msg_q_mon_count,
                                      UINT32     ui4_time);

extern INT32 x_msg_q_num_msgs (HANDLE_T  h_msg_q,
                               UINT16*   pui2_num_msgs);

extern INT32 x_msg_q_get_max_msg_size (HANDLE_T  h_msg_q,
                                       SIZE_T*   pz_maxsize);
extern INT32 x_msg_q_flush(HANDLE_T   h_msg_hdl);
#ifdef  __cplusplus
}
#endif

#endif

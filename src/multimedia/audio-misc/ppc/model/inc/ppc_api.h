#ifndef PPC_API_H
#define PPC_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_format.h"
#include "ppc_def.h"
#include "id.h"

/* 
 * ppc_api return error type
 */
typedef enum{
	PPC_REINIT              = 1,
	PPC_SUCCESS             = 0,
	PPC_ALLOCATE_MEMORY_FAIL   = -100,
	PPC_LOAD_INFO_CONFIG_FAIL  = -101,
	PPC_LOAD_DEFAULT_INFO_FAIL = -102,
	PPC_NO_PAGE_FOUND          = -103,
	PPC_CREATE_ERROR           = -104,
	PPC_NOT_INIT               = -105,
	PPC_INIT_PPC_HDL_FAIL      = -106,
	PPC_PROCESS_FAIL           = -107,
	PPC_SET_PARA_FAIL          = -108,
	PPC_RESET_FAIL             = -109,
	PPC_REINIT_FAIL            = -110,
	PPC_BAD_DUMP_ENABLED       = -120,
	PPC_GET_PARA_FAIL          = -130,
	PPC_INIT_SWIP_MAP_FAIL     = -140,
	PPC_PROJ_CHECK_FAIL      = -150,
	PPC_VERSION_CHECK_FAIL      = -160,
} PPC_RESULT;

/*** 
 * The following function returns info_size for vs_info and hdl_size
 * for vs_hdl.
 ***/
PPC_RESULT ppc_size(int* info_size, int* hdl_size);

/*** 
 * The following function triggers ppc to load config file 
 * to get element_type and page info. 
 * If there is no config file, use initial info defined in 
 * element_type.h and page.h.
 ***/
PPC_RESULT ppc_load(void* ppc_info, void* ppc_hdl, char* config_path);
/*** 
 * The following function triggers ppc to load config file 
 * to get element_type and page info. 
 * If the file pointer ele_fp or page_fp is equal to NULL,
 * use initial info defined in element_type.h and page.h.
 ***/
PPC_RESULT ppc_load_fp(void *ppc_info, void *ppc_hdl, FILE* ele_file, FILE* page_file);

/*** 
 * The following function triggers ppc to create 
 * post-processing chain according element type and page info. 
 * < input >
 * ppc_info: ppc_info from ppc_load
 * ppc_hdl : memory for ppc_hdl
 * sys_info: system information
 * < output >
 * sys_info.frame_size: current page frame size
 ***/
PPC_RESULT ppc_create(void* ppc_info, void *ppc_hdl, SYS_INFO *sys_info);

/*** 
 * The following function set parameter to element and 
 * page info. 
 ***/
PPC_RESULT ppc_set_para(void *ppc_hdl, unsigned int element_id, unsigned int para_id, int num, int* value);

/*** 
 * The following function get parameter to element and 
 * page info. 
 ***/
PPC_RESULT ppc_get_para(void *ppc_hdl, unsigned int element_id, unsigned int para_id, int num, int* value);

/*** 
 * The following function run ppc process for fixed system frame size
 * < input >
 * ppc_hdl : ppc handler
 * in_buf  : input data
 * < output >
 * out_buf : output data
 ***/
PPC_RESULT ppc_process(void *ppc_hdl, AUDIO_BUF *in_buf, AUDIO_BUF *out_buf);

/*** 
 * The following function run ppc process for variant system frame size
 * < input >
 * ppc_hdl : ppc handler
 * in_buf  : input data
 * frames  : frame size of input data
 * < output >
 * out_buf : output data
 * latency : the output latency of current input data 
 ***/
PPC_RESULT ppc_process_vf(void *ppc_hdl, AUDIO_BUF *in_buf, AUDIO_BUF *out_buf, unsigned int frames, int *latency);

/*** 
 * The following function destroy ppc handler
 ***/
PPC_RESULT ppc_destroy_hdl(void *ppc_hdl);

/*** 
 * The following function destroy ppc info
 ***/
PPC_RESULT ppc_destroy_info(void *ppc_info);

/*** 
 * The following function reset all swip
 ***/
PPC_RESULT ppc_reset(void *ppc_hdl);

/*** 
 * The following function re-init pp chain, including re-query channel 
 ***/
PPC_RESULT ppc_reinit(void *ppc_hdl, SYS_INFO *sys_info);

/*** 
 * The following function dump element input/ output buffer
 ***/
PPC_RESULT ppc_dump_element(void *ppc_hdl, unsigned int element_id, int enabled, char* dump_folder);

/*** 
 * The following function bypass element
 ***/
PPC_RESULT ppc_bypass_element(void *ppc_hdl, unsigned int element_id, int enabled);

/*** 
 * The following function get element bypass status
 ***/
PPC_RESULT ppc_get_bypass_status(void *ppc_hdl, unsigned int element_id, int* enabled);

/*** 
 * The following function dump current page info
 ***/
PPC_RESULT ppc_dump_status(void *ppc_hdl, char* config_path, int passwd);

/*** 
 * The following function get total delay of the signal flow
 ***/
PPC_RESULT ppc_get_delay(void *ppc_hdl, int *delay);

/*** 
 * The following function check the tool version matched ppc framework version
 ***/
PPC_RESULT ppc_check_version(void *ppc_hdl, unsigned int tool_version);

/*** 
 * The following function check the tool version matched ppc swip version
 ***/
PPC_RESULT ppc_check_swip_version(void *ppc_hdl, unsigned int type_id,unsigned int tool_version);


#ifdef __cplusplus
}
#endif

#endif

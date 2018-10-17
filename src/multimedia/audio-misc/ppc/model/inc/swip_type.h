#ifndef SWIP_TYPE_H
#define SWIP_TYPE_H

#include "audio_format.h"

typedef enum pp_result{
	SUCCESS = 0,
	FAIL    = -1,
	PARAMETER_EXCEED_MAX_VALUE = -2,
	INVALID_OPERATION = -3,
	NEED_RESET = 1
	/* can define SWIP error code, 
	 * upper 16 bit use type id
	 */
}PP_RESULT;

typedef struct swip_handler{
// used for system to save pp_hdl, swip don't care
	void* hdl;
	
/*******************************************
 * get_mem_size: get swip memory size
 * output:  
 *   hdl_size  : total memory size
 *******************************************/
	PP_RESULT (*get_mem_size)(int*hdl_size, SYS_INFO *sys_info);
	
/*******************************************
 * init: swip init
 * input : 
 *   pp_hdl: memory allocated by system with 
 *           hdl_size.
 *   sys_info: current system information
 *   swip_para: current swip parameter
 * output:  
 *   pp_hdl: initialized pp_hdl
 *******************************************/
	PP_RESULT (*init)(void *pp_hdl, SYS_INFO *sys_info);

/*******************************************
 * set_channel: set input channel, get output 
 *              channel
 * input :
 *   pp_hdl : current handler
 *   in_channel: input channel
 * output:  
 *   out_channel: output channel
 *******************************************/
	PP_RESULT (*set_channel)(void *pp_hdl, unsigned int in_channel, unsigned int* out_channel);

/*******************************************
 * set_para: set parameter to swip
 * input : 
 *   pp_hdl
 *   para : parameter id
 *   value: parameter value
 *******************************************/
	PP_RESULT (*set_para)(void *pp_hdl, unsigned int para, int num, int* value);

/*******************************************
 * get_para: get parameter from swip
 * input : 
 *   pp_hdl
 *   para : parameter id
 * output:
 *   value: parameter value
 *******************************************/
	PP_RESULT (*get_para)(void *pp_hdl, unsigned int para, int num, int* value);

/*******************************************
 * dump para: dump all swip parameter 
 *******************************************/
	PP_RESULT (*dump_para)(void *pp_hdl);

/*******************************************
 * process: process data
 * input : 
 *   pp_hdl
 *   input_buffer : input data
 *   num_sample   : input sample number
 * output:
 *   output_buffer: output data
 *******************************************/
	PP_RESULT (*process)(void *pp_hdl, AUDIO_BUF input_buffer, AUDIO_BUF output_buffer, int num_frame);

/*******************************************
 * reset: clean history buffer and reset SWIP
 * input : 
 *   pp_hdl
 *******************************************/
	PP_RESULT (*reset)(void *pp_hdl);

/*******************************************
 * get_delay: get delay information from SWIP
 * input : 
 *   pp_hdl
 *   delay: SWIP delay
 *******************************************/
	PP_RESULT (*get_delay)(void *pp_hdl, int *delay);

/*******************************************
 * get_version: get SWIP version
 * output : 
 *   swip_version: current swip version
 *   min_tool_version: min tool version supported by this swip
 *******************************************/
	PP_RESULT (*get_version)(int *swip_version, int *min_tool_version);

}SWIP_HANDLER;


#endif


 
 


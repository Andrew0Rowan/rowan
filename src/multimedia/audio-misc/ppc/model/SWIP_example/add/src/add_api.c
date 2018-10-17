#include "add_api.h"
#include "add_type.h"
#include "id.h"
#include "ppc_common.h"

#define OUT_CH AUDIO_CHANNEL_MONO
#define COEF_Q 23
#define HEADROOM 5
#define RANGE_MAX 67108863
#define RANGE_MIN (-RANGE_MAX-1)

PP_RESULT add_get_mem_size(int *hdl_size, SYS_INFO *sys_info){
	print_log("add_get_mem_size!\n");
	*hdl_size = sizeof(ADD_HANDLER);
	return SUCCESS;
}
	

PP_RESULT add_init(void *pp_hdl, SYS_INFO *sys_info){
	ADD_HANDLER *hdl = pp_hdl;
	print_log("add_init!\n");
	
	return SUCCESS;
}

PP_RESULT add_set_channel(void *pp_hdl, unsigned int in_channel, unsigned int* out_channel){
	ADD_HANDLER *hdl = pp_hdl;
	print_log("add_set_channel!\n");
	*out_channel = OUT_CH;
	hdl->in_ch = in_channel;
	hdl->out_ch = OUT_CH;
	return SUCCESS;
}


PP_RESULT add_set_para(void *pp_hdl, unsigned int para, int num, int* value){
	ADD_HANDLER* hdl = pp_hdl;

	if(num == 0){
		return SUCCESS;
	}

	print_must("add_set_para!para id : 0x%x, num: %d value: %d\n", para, num, *value);
	
	switch(para){
	case PARA_ADD_WEIGHT:
		memcpy(hdl->weight, value, sizeof(int) * TOTAL_CH_NUM);
		break;

	default:
		print_must("no fit parameter 0x%x\n", para);
		return FAIL;
	}


	return SUCCESS;
}


PP_RESULT add_get_para(void *pp_hdl, unsigned int para, int num, int* value){
	ADD_HANDLER* hdl = pp_hdl;

	print_log("add_get_para!\n");
	
	switch(para){
	case PARA_ADD_WEIGHT:
		memcpy(value, hdl->weight, sizeof(int) * TOTAL_CH_NUM);

	default:
		print_must("no fit parameter 0x%x\n", para);
		return FAIL;
	}


	return SUCCESS;
}

PP_RESULT add_dump_para(void *pp_hdl){
	ADD_HANDLER* hdl = pp_hdl;
	int i=0;
	print_log("add_dump_para\n");
	print_must("in_ch: 0x%x, out_ch\n", hdl->in_ch, hdl->out_ch);
	for(i=0; i<TOTAL_CH_NUM; i++){
		print_must("weight[%d] : 0x%x\n", i, hdl->weight[i]);
	}

	return SUCCESS;
}


PP_RESULT add_process(void *pp_hdl, AUDIO_BUF input_buffer, AUDIO_BUF output_buffer, int num_frame){
	ADD_HANDLER* hdl = pp_hdl;
	int ch = 0;
	int sample = 0;
	int weight = 0;
	__int64 acc = 0;
	int out_ch_index = map_ch_index(OUT_CH);
	int shift = COEF_Q + HEADROOM;
	
	print_log("add_process!\n");

	for(sample = 0; sample < num_frame; sample++){
		acc = 0;
		for(ch = 0; ch < TOTAL_CH_NUM; ch++){
			if((hdl->in_ch & ch_map[ch])!= 0){
				weight = hdl->weight[ch];
				acc += ((__int64) input_buffer.buf[ch][sample] * (__int64)weight) >> shift;				
			}
		}

		// clip
		if(acc > RANGE_MAX){
			acc = RANGE_MAX;
		}else if(acc < RANGE_MIN){
			acc = RANGE_MIN;
		}
		acc <<= HEADROOM;

		output_buffer.buf[out_ch_index][sample] = (int) acc; 
	}

	return SUCCESS;
}


PP_RESULT add_reset(void *pp_hdl){
	print_log("add_reset!\n");
	return SUCCESS;
}



PP_RESULT add_get_handler(SWIP_HANDLER* swip_hdl){
	print_log("add_get_handler!\n");
	swip_hdl->get_mem_size = add_get_mem_size;
	swip_hdl->init = add_init;
	swip_hdl->set_channel = add_set_channel;
	swip_hdl->process = add_process;
	swip_hdl->get_para = add_get_para;
	swip_hdl->set_para = add_set_para;
	swip_hdl->reset = add_reset;
	swip_hdl->dump_para = add_dump_para;
	return SUCCESS;
}

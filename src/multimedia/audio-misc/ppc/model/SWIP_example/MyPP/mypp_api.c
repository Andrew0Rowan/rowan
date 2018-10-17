#include "mypp_api.h"
#include "shift_api.h"
#include "mypp_type.h"
#include "id.h"
#include "ppc_common.h"

PP_RESULT mypp_get_mem_size(int *hdl_size, SYS_INFO *sys_info){
	print_must("mypp_get_mem_size!\n");
	*hdl_size = sizeof(MYPP_HANDLER);
	*hdl_size += sizeof(MTK_MEM_HANDLE);
	*hdl_size += 2 * sizeof(int) * sys_info->ppc_frame_size * TOTAL_CH_NUM;
	*hdl_size += shift_get_obj_size();
	return SUCCESS;
}

PP_RESULT mypp_init(void *pp_hdl, SYS_INFO *sys_info){
	MYPP_HANDLER *hdl = pp_hdl;
	char* buf = (char*)pp_hdl + sizeof(MYPP_HANDLER);
	int hdl_size = 0;
	MTK_MEM_HANDLE* mem_hdl;
	int frame = sys_info->ppc_frame_size;
	PP_RESULT result = SUCCESS;

	print_log("mypp_init!\n");

	// memory handle initialize
	if (mypp_get_mem_size(&hdl_size, sys_info) != SUCCESS) {
		print_must("[mypp_init] get mem size fail\n");
		result = FAIL;
		goto exit;
	}

	if (mem_init(pp_hdl, (char *)buf, hdl_size - sizeof(MYPP_HANDLER))!= MEM_SUCCESS)
	{
		print_must("[mbdrc_init] mem init fail\n");
		result = FAIL;
		goto exit;
	}
	mem_hdl = hdl->mem_hdl;

	if (mem_calloc(mem_hdl, (long*)&(hdl->in_buf), sizeof(int)*frame*TOTAL_CH_NUM) != MEM_SUCCESS)
	{
		print_must("fail to allocate memory!");
		return FAIL;
		goto exit;
	}

	if (mem_calloc(mem_hdl, (long*)&(hdl->out_buf), sizeof(int)*frame*TOTAL_CH_NUM) != MEM_SUCCESS)
	{
		print_must("fail to allocate memory!");
		return FAIL;
		goto exit;
	}

	if (mem_calloc(mem_hdl, (long*)&(hdl->obj), shift_get_obj_size()) != MEM_SUCCESS) {
		print_must("fail to allocate memory!");
		return FAIL;
		goto exit;
	}
	memset((void *)hdl->obj, 0, shift_get_obj_size());

	shift_init(hdl->obj);

exit:
	return result;
}


PP_RESULT mypp_set_channel(void *pp_hdl, unsigned int in_channel, unsigned int* out_channel){
	MYPP_HANDLER *hdl = pp_hdl;
	print_log("mypp_set_channel!\n");
	*out_channel = in_channel;
	hdl->in_ch = in_channel;
	hdl->out_ch = in_channel;
	shift_set_channel_num(hdl->obj, in_channel);
	return SUCCESS;
}


PP_RESULT mypp_set_para(void *pp_hdl, unsigned int para, int num, int* value){
	MYPP_HANDLER* hdl = pp_hdl;

	if(num == 0){
		return SUCCESS;
	}

	print_log("mypp_set_para!para id : 0x%x, num: %d value: %d\n", para, num, *value);

	switch(para){
	case PARA_MYPP_SET_SHIFT_BIT:
		shift_set_bit(hdl->obj, *value);
		break;
	case PARA_MYPP_SET_ENABLE:
		shift_set_enable(hdl->obj, *value);
		break;
	default:
		print_must("no fit parameter 0x%x\n", para);
		return FAIL;
	}


	return SUCCESS;
}


PP_RESULT mypp_get_para(void *pp_hdl, unsigned int para, int num, int* value){
	MYPP_HANDLER* hdl = pp_hdl;

	print_log("mypp_get_para!\n");

	switch(para){
	case PARA_MYPP_SET_SHIFT_BIT:
		*value = shift_get_bit(hdl->obj);
		break;
	case PARA_MYPP_SET_ENABLE:
		*value = shift_get_enable(hdl->obj);
		break;
	default:
		print_must("no fit parameter 0x%x\n", para);
		return FAIL;
	}


	return SUCCESS;
}

PP_RESULT mypp_dump_para(void *pp_hdl){
	MYPP_HANDLER* hdl = pp_hdl;
	int ret_val = 0;
	print_must("mypp_dump_para\n");
	print_must("in_ch: 0x%x, out_ch: 0x%x\n", hdl->in_ch, hdl->out_ch);

	ret_val = shift_get_bit(hdl->obj);
	print_must("shift bit: %d\n", ret_val);

	ret_val = shift_get_enable(hdl->obj);
	print_must("shift enabled: %d\n", ret_val);

	return SUCCESS;
}

PP_RESULT mypp_process(void *pp_hdl, AUDIO_BUF input_buffer, AUDIO_BUF output_buffer, int num_frame){
	MYPP_HANDLER* hdl = pp_hdl;
	int i = 0, ch = 0;
	int* input = hdl->in_buf;
	int* output = hdl->out_buf;

	for(ch=0; ch<TOTAL_CH_NUM; ch++){
		if(hdl->in_ch & ch_map[ch]){
			memcpy((void*)input, (void*)input_buffer.buf[ch], sizeof(int)*num_frame);
			input += num_frame;
		}
	}

	shift_process(hdl->obj, (void *)hdl->in_buf, (void *)hdl->out_buf, num_frame);

	for(ch=0; ch<TOTAL_CH_NUM; ch++){
		if(hdl->out_ch & ch_map[ch]){
			memcpy((void*)output_buffer.buf[ch], (void*)output, sizeof(int)*num_frame);
			output += num_frame;
		}
	}


	return SUCCESS;
}

PP_RESULT mypp_reset(void *pp_hdl){
	print_log("mypp_reset!\n");
	return SUCCESS;
}



PP_RESULT mypp_get_handler(SWIP_HANDLER* swip_hdl){
	print_log("mypp_get_handler!\n");
	swip_hdl->get_mem_size = mypp_get_mem_size;
	swip_hdl->init = mypp_init;
	swip_hdl->set_channel = mypp_set_channel;
	swip_hdl->process = mypp_process;
	swip_hdl->get_para = mypp_get_para;
	swip_hdl->set_para = mypp_set_para;
	swip_hdl->reset = mypp_reset;
	swip_hdl->dump_para = mypp_dump_para;
	return SUCCESS;
}

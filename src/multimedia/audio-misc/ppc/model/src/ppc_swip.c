#include "audio_format.h"
#include "ppc_swip.h"
#include "element_pool.h"
#include "page_list.h"

// swip header
#include "input_api.h"
#include "output_api.h"
#include "add_api.h"
#include "chmix_api.h"
#include "gain_control_api.h"
#include "filter_api.h"
#include "MasterVolume_api.h"
#include "peq_api.h"
#include "delay_api.h"
#include "mbdrc_api.h"

/* SWIP NUM*/
#define MTK_SWIP_NUM 10

int get_swip_num(void){
	int swip_num = MTK_SWIP_NUM;
	return swip_num;
}

int get_proj_code(void){
	int proj_code = 0;

#ifdef PROJ_CODE
	proj_code = PROJ_CODE;
#endif

	return proj_code;
}

unsigned int module_num;

const unsigned int module_id[] = {
	INPUT_TYPE_ID,
	OUTPUT_TYPE_ID,
	ADD_TYPE_ID,
	CHMIX_TYPE_ID,
	GAIN_CONTROL_TYPE_ID,
	FILTER_TYPE_ID,
	MASTERVOL_TYPE_ID,
	PEQ_TYPE_ID,
	DELAY_TYPE_ID,
	MB_DRC_TYPE_ID,
};

PP_RESULT(*get_module_handler[])(SWIP_HANDLER *swip_hdl) = {
	input_get_handler,
	output_get_handler,
	add_get_handler,
	chmix_get_handler,
	gain_control_get_handler,
	filter_get_handler,
	master_volume_get_handler,
	peq_get_handler,
	delay_get_handler,
	mbdrc_get_handler,
};

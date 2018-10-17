#ifndef ADD_TYPE_H
#define ADD_TYPE_H
#include "mem.h"

typedef struct add_handler{
	// must be here: for memory handle
	MTK_MEM_HANDLE* mem_hdl;

	// must be here!
	unsigned int in_ch;
	unsigned int out_ch;
	
	// parameter 
	int weight[TOTAL_CH_NUM]; // Q23

}ADD_HANDLER;

#endif
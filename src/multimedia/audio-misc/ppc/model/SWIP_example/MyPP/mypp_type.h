#ifndef MYPP_TYPE_H
#define MYPP_TYPE_H
#include "mem.h"

typedef struct mypp_handler{
	// must be here: for memory handle
	MTK_MEM_HANDLE* mem_hdl;

	// must be here!
	unsigned int in_ch;
	unsigned int out_ch;

	// parameter
	int* in_buf;
	int* out_buf;
	SHIFT_OBJ* obj;

}MYPP_HANDLER;

#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shift_api.h"

#define COEF_Q 23
#define HEADROOM 5
#define RANGE_MAX 67108863
#define RANGE_MIN -67108863

SHIFT_RESULT shift_init(SHIFT_OBJ *obj_ptr) {
	SHIFT_RESULT result = SHIFT_NO_ERROR;

	printf("shift_init\n");

	if(obj_ptr) {
		obj_ptr->shift_bit = 1;
		obj_ptr->channel_num = 2;
		obj_ptr->enable = 0;
	} else
		result = SHIFT_ERR_ALLOC_FAILED;

	return result;

}

int shift_get_obj_size()
{
	printf("shift_get_obj_size\n");
	return sizeof(SHIFT_OBJ);
}

SHIFT_RESULT shift_process(SHIFT_OBJ *obj_ptr, void *input, void *output, int blockSize) {
	SHIFT_RESULT result = SHIFT_NO_ERROR;
	int i = 0, ch = 0;
	int *in_buf = (int*)input;
	int *out_buf = (int*)output;

	if (!in_buf || !out_buf) {
		printf("%s is NULL!\n", !in_buf?"input buf":"output buf");
		result = SHIFT_ERR_ALLOC_FAILED;
	} else {
		if (obj_ptr->enable) {
			for (ch = 0; ch < obj_ptr->channel_num; ch++) {
				for (i = 0; i < blockSize; i++) {
					unsigned int data = (unsigned int)(*in_buf++);
					unsigned int flag = data & 0x80000000;
					if (flag) {
						data = ~data;
						data = data >> (obj_ptr->shift_bit);
						*out_buf++ = ~data;
					} else
						*out_buf++ = data  >> (obj_ptr->shift_bit);
				}
			}
		} else
			memcpy((void*)out_buf, (void*)in_buf, sizeof(int)*blockSize*obj_ptr->channel_num);
	}

	return result;
}

SHIFT_RESULT shift_set_enable(SHIFT_OBJ *obj_ptr, int enable) {
	SHIFT_RESULT result = SHIFT_NO_ERROR;

	printf("shift_set_enable\n");

	if (enable != 0 && enable != 1) {
		printf("error! enable = %d!\n", enable);
		result = SHIFT_ERR_INVALID_PARAMETER;
	} else
		obj_ptr->enable = enable;

	return result;
}

int shift_get_enable(SHIFT_OBJ *obj_ptr) {
	printf("shift_get_enable\n");
	return obj_ptr->enable;
}

SHIFT_RESULT shift_set_bit(SHIFT_OBJ *obj_ptr, int bit) {
	SHIFT_RESULT result = SHIFT_NO_ERROR;

	printf("shift_set_bit\n");

	if (bit < 0 || bit >= 16) {
		printf("error! shift bit (%d) is beyond limit!\n", bit);
		result = SHIFT_ERR_INVALID_PARAMETER;
	} else
		obj_ptr->shift_bit = bit;

	return result;
}

int shift_get_bit(SHIFT_OBJ *obj_ptr) {
	printf("shift_get_bit\n");
	return obj_ptr->shift_bit;
}

SHIFT_RESULT shift_set_channel_num(SHIFT_OBJ *obj_ptr, int channel) {
	SHIFT_RESULT result = SHIFT_NO_ERROR;

	printf("shift_set_channel_num\n");

	if (channel <= 0 && channel >= 16) {
		printf("error! channel number (%d) is beyond limit!\n", channel);
		result = SHIFT_ERR_INVALID_PARAMETER;
	} else
		obj_ptr->channel_num = channel;

	return result;
}

int shift_get_channel_num(SHIFT_OBJ *obj_ptr) {
	printf("shift_get_channel_num\n");
	return obj_ptr->channel_num;
}



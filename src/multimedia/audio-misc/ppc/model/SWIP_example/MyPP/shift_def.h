#ifndef __SHIFT_DEF_H__
#define __SHIFT_DEF_H__

typedef enum
{
	SHIFT_NO_ERROR = 0,			//Status: no error
	SHIFT_ERR_UNINITIALIZED = -1,	//All error codes must be <0
	SHIFT_ERR_INVALID_PARAMETER = -2,
	SHIFT_ERR_NOT_SUPPORTED = -3,
	SHIFT_ERR_ALLOC_FAILED = -4,
} SHIFT_RESULT;

/*Data type definition here:*/
typedef struct shift_obj{
	int shift_bit;
	int enable;
	int channel_num;
	int format;
}SHIFT_OBJ;

////////////////////////////////////////////////////////////////////////////

#endif //__SRS_TVOLHDMC_DEF_H__


#ifndef __SHIFT_API_H__
#define __SHIFT_API_H__

#include "shift_def.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

//API declaration here:

SHIFT_RESULT		shift_init(SHIFT_OBJ *obj_ptr);
int			shift_get_obj_size();
SHIFT_RESULT		shift_process(SHIFT_OBJ *obj_ptr, void *input, void *output, int blockSize);
SHIFT_RESULT		shift_set_enable(SHIFT_OBJ *obj_ptr, int enable);
int			shift_get_enable(SHIFT_OBJ *obj_ptr);
SHIFT_RESULT		shift_set_bit(SHIFT_OBJ *obj_ptr, int bit);
int			shift_get_bit(SHIFT_OBJ *obj_ptr);
SHIFT_RESULT		shift_set_channel_num(SHIFT_OBJ *obj_ptr, int channel);
int			shift_get_channel_num(SHIFT_OBJ *obj_ptr);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__SHIFT_API_H__*/


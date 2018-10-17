#ifndef INPUT_API_H
#define INPUT_API_H

#include "swip_type.h"

// define SWIP ID 
#define INPUT_TYPE_ID 0x0000

// declare SWIP get handler function
PP_RESULT input_get_handler(SWIP_HANDLER* swip_hdl);

#endif
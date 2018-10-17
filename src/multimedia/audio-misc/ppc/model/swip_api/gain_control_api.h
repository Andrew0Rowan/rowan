#ifndef GAIN_CONTROL_API_H
#define GAIN_CONTROL_API_H

#include "swip_type.h"

// define SWIP ID 
#define GAIN_CONTROL_TYPE_ID 0x0005


// declare SWIP get handler function
PP_RESULT gain_control_get_handler(SWIP_HANDLER* swip_hdl);

#endif
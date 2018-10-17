#ifndef BYPASS_API_H
#define BYPASS_API_H

#include "swip_type.h"

// special SWIP for bypass

// define SWIP ID 
#define BYPASS_TYPE_ID 0xFFFE

// declare SWIP get handler function
PP_RESULT bypass_get_handler(SWIP_HANDLER* swip_hdl);

#endif
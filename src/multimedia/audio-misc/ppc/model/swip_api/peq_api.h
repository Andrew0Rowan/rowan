#ifndef PEQ_API_H
#define PEQ_API_H

#include "swip_type.h"

// define SWIP ID 
#define PEQ_TYPE_ID 0x0007


// declare SWIP get handler function
PP_RESULT peq_get_handler(SWIP_HANDLER* swip_hdl);

#endif
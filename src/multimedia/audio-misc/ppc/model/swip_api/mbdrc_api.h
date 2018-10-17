#ifndef MB_DRC_API_H
#define MB_DRC_API_H

#include "swip_type.h"

// define SWIP ID
#define MB_DRC_TYPE_ID 0x000f


// declare SWIP get handler function
PP_RESULT mbdrc_get_handler(SWIP_HANDLER* swip_hdl);

#endif
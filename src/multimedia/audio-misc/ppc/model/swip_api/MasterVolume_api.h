#ifndef MASTER_VOLUME_API_H
#define MASTER_VOLUME_API_H

#include "swip_type.h"

// define SWIP ID 
#define MASTERVOL_TYPE_ID 0x0001


// declare SWIP get handler function
PP_RESULT master_volume_get_handler(SWIP_HANDLER* swip_hdl);

#endif
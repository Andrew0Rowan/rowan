//
//  DRAMC_COMMON.H
//

#ifndef _DRAMC_COMMON_H_
#define _DRAMC_COMMON_H_

#include "platform/dramc_register.h"
#include "platform/dramc_pi_api.h"
#include "platform/emi.h"
#include "platform/mtk_timer.h"
#include <printf.h>

#include <platform.h>
#if __FLASH_TOOL_DA__
#include "../gpt_timer/gpt_timer.h"
#include <debug.h>
#include <lib/string.h>
#include "assert.h"
#endif
/***********************************************************************/
/*                  Public Types                                       */
/***********************************************************************/

/*------------------------------------------------------------*/
/*                  macros, defines, typedefs, enums          */
/*------------------------------------------------------------*/
/************************** Common Macro *********************/
//#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")

// K2?? : The following needs to be porting.
// choose a proper mcDELAY
//====
#if __FLASH_TOOL_DA__
    #define mcDELAY_US(x)       gpt4_busy_wait_us(x)
    #define mcDELAY_MS(x)       gpt4_busy_wait_us(x*1000)
#else
    #define mcDELAY_US(x)       gpt_busy_wait_us(x)
    #define mcDELAY_MS(x)       gpt_busy_wait_us(x*1000)
#endif

typedef enum {
    CHIP_VER_E1 = 0x0,
    CHIP_VER_E2 = 0x1,
} chip_ver_t;


/**********************************************/
/* Priority of debug log                      */
/*--------------------------------------------*/
/* mcSHOW_DBG_MSG: High                       */
/* mcSHOW_DBG_MSG2: Medium High               */
/* mcSHOW_DBG_MSG3: Medium Low                */
/* mcSHOW_DBG_MSG4: Low                       */
/**********************************************/
extern U32 u4DRAMdebugLOgEnable;
extern U32 u4DRAMdebugLOgEnable2;
#define platform_chip_ver()  CHIP_VER_E2

#if __FLASH_TOOL_DA__
  #define print LOGI
  #define CHIP_VER_E1 1
  #define platform_chip_ver()  CHIP_VER_E1
#endif

#define mcSHOW_DBG_MSG(_x_)   if(u4DRAMdebugLOgEnable) {printf _x_;}
#define mcSHOW_DBG_MSG2(_x_)  if(u4DRAMdebugLOgEnable2) {printf _x_;}
#define mcSHOW_DBG_MSG3(_x_)
#define mcSHOW_DBG_MSG4(_x_)
#define mcSHOW_USER_MSG(_x_)
#define mcSHOW_ERR_MSG(_x_)   printf _x_

extern int dump_log;
#endif   // _DRAMC_COMMON_H_

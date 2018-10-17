#pragma once

#include <stdbool.h>
#include <hardware/bluetooth.h>

#include "osi/include/config.h"

#if defined(MTK_STACK_CONFIG_LOG) && (MTK_STACK_CONFIG_LOG == TRUE)
typedef struct {
  bool (*get_pack_hexlists)(void);
  const uint8_t *(*get_whole_hexlists)(void);
} stack_config_pack_hexlist_t;

const stack_config_pack_hexlist_t *stack_config_fwlog_hexs_get_interface();
#endif

#if defined(MTK_STACK_CONFIG) && (MTK_STACK_CONFIG == TRUE)
extern bool parse_override_cfg(config_t * config);
#endif

/******************************************************************************
 *
 *  Copyright (C) 2014 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#pragma once

#include <stdbool.h>

#include "osi/include/config.h"
#include "module.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char STACK_CONFIG_MODULE[] = "stack_config_module";

typedef struct {
  const char *(*get_btsnoop_log_path)(void);
  bool (*get_btsnoop_turned_on)(void);
  bool (*get_btsnoop_should_save_last)(void);
#if defined(MTK_LINUX_SNOOP_CONFIG) && (MTK_LINUX_SNOOP_CONFIG == TRUE)
  int (*get_btsnoop_log_mode)(void);
  int (*get_btsnoop_max_log_file_size)(void);
  int (*get_btsnoop_trimmed_packet_size)(void);
#endif
  bool (*get_trace_config_enabled)(void);
  bool (*get_pts_secure_only_mode)(void);
  bool (*get_pts_conn_updates_disabled)(void);
  bool (*get_pts_crosskey_sdp_disable)(void);
  const char* (*get_pts_smp_options)(void);
  int (*get_pts_smp_failure_case)(void);
#if defined(MTK_LINUX_GAP_PTS_TEST) && (MTK_LINUX_GAP_PTS_TEST == TRUE)
  bool (*get_pts_passive_encrypt)(void);
  bool (*get_pts_passive_security_request)(void);
  bool (*get_pts_disable_get_remote_services)(void);
  bool (*get_pts_extend_idle_timeout)(void);
  bool (*get_pts_non_resolvable_private_address)(void);
  bool (*get_pts_disable_ble_connection_update)(void);
  bool (*get_pts_disable_discover)(void);
#endif
#if defined(MTK_LINUX_STACK_LOG2FILE) && (MTK_LINUX_STACK_LOG2FILE == TRUE)
  const char *(*get_btstack_log_path)(void);
  bool (*get_btstack_log2file_turned_on)(void);
  bool (*get_btstack_should_save_last)(void);
#endif
  config_t *(*get_all)(void);
} stack_config_t;

const stack_config_t *stack_config_get_interface();

#ifdef __cplusplus
}
#endif

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

#define LOG_TAG "bt_stack_config"

#include "stack_config.h"

#include <assert.h>

#include "bt_target.h"
#include "osi/include/future.h"
#include "osi/include/log.h"
#if defined(MTK_STACK_CONFIG) && (MTK_STACK_CONFIG == TRUE)
#include "mdroid_stack_config.h"
#endif

const char *BTSNOOP_LOG_PATH_KEY = "BtSnoopFileName";
const char *BTSNOOP_TURNED_ON_KEY = "BtSnoopLogOutput";
const char *BTSNOOP_SHOULD_SAVE_LAST_KEY = "BtSnoopSaveLog";
#if defined(MTK_LINUX_SNOOP_CONFIG) && (MTK_LINUX_SNOOP_CONFIG == TRUE)
const char *BTSNOOP_LOG_MODE_KEY = "BtSnoopLogMode";
const char *BTSNOOP_MAX_LOG_FILE_SIZE_KEY = "BtSnoopMaxLogFileSize";
const char *BTSNOOP_TRIMMED_PACKET_SIZE_KEY = "BtSnoopTrimmedPacketSize";
#endif
const char *TRACE_CONFIG_ENABLED_KEY = "TraceConf";
const char *PTS_SECURE_ONLY_MODE = "PTS_SecurePairOnly";
const char *PTS_LE_CONN_UPDATED_DISABLED = "PTS_DisableConnUpdates";
const char *PTS_DISABLE_SDP_LE_PAIR = "PTS_DisableSDPOnLEPair";
const char *PTS_SMP_PAIRING_OPTIONS_KEY = "PTS_SmpOptions";
const char *PTS_SMP_FAILURE_CASE_KEY = "PTS_SmpFailureCase";
#if defined(MTK_LINUX_GAP_PTS_TEST) && (MTK_LINUX_GAP_PTS_TEST == TRUE)
const char *PTS_PASSIVE_ENCRYPT = "PTS_PassiveLEEncrypt";
const char *PTS_PTSSIVE_SECURITY_REQUEST = "PTS_PassiveSecurityRequest";
const char *PTS_DISABLE_GET_REMOTE_SERVICES = "PTS_DisableGetRemoteServices";
const char *PTS_EXTEND_IDLE_TIMEOUT = "PTS_ExtendIdleTimeout";
const char *PTS_NON_RPA = "PTS_NonResolvablePrivateAddress";
const char *PTS_DISABLE_CONNECTION_UPDATE = "PTS_DisableLEConnectionUpdate";
const char *PTS_DISABLE_DISCOVER = "PTS_DisableDiscover";
#endif

#if defined(MTK_LINUX_STACK_LOG2FILE) && (MTK_LINUX_STACK_LOG2FILE == TRUE)
const char *BTSTACK_LOG_PATH_KEY = "BtStackFileName";
const char *BTSTACK_LOG2FILE_TURNED_ON_KEY = "BtStackLog2File";
const char *BTSTACK_SHOULD_SAVE_LAST_KEY = "BtStackSaveLog";
#endif

static config_t *config;

// Module lifecycle functions

static future_t *init() {
// TODO(armansito): Find a better way than searching by a hardcoded path.
#if defined(OS_GENERIC)
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
  const char *path = BT_CONF_PATH"bt_stack.conf";
#else
  const char *path = "bt_stack.conf";
#endif
#else  // !defined(OS_GENERIC)
  const char *path = "/etc/bluetooth/bt_stack.conf";
#endif  // defined(OS_GENERIC)
  assert(path != NULL);

  LOG_INFO(LOG_TAG, "%s attempt to load stack conf from %s", __func__, path);

  config = config_new(path);
  if (!config) {
    LOG_INFO(LOG_TAG, "%s file >%s< not found", __func__, path);
    return future_new_immediate(FUTURE_FAIL);
  }

#if defined(MTK_STACK_CONFIG) && (MTK_STACK_CONFIG == TRUE)
  config_dump(config); /* dump config before override */

  /* override stack config */
  if (parse_override_cfg(config)) {
    LOG_INFO(LOG_TAG, "%s M_BTCONF Override file is deployed", __func__);

    config_dump(config); /* dump config after override */

  } else
      LOG_INFO(LOG_TAG, "%s M_BTCONF Override file is not deployed", __func__);
#endif /* MTK_STACK_CONFIG */

  return future_new_immediate(FUTURE_SUCCESS);
}

static future_t *clean_up() {
  config_free(config);
  config = NULL;
  return future_new_immediate(FUTURE_SUCCESS);
}

EXPORT_SYMBOL const module_t stack_config_module = {
  .name = STACK_CONFIG_MODULE,
  .init = init,
  .start_up = NULL,
  .shut_down = NULL,
  .clean_up = clean_up,
  .dependencies = {
    NULL
  }
};

// Interface functions

static const char *get_btsnoop_log_path(void) {
  return config_get_string(config, CONFIG_DEFAULT_SECTION, BTSNOOP_LOG_PATH_KEY,
      "/data/misc/bluetooth/logs/btsnoop_hci.log");
}

static bool get_btsnoop_turned_on(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, BTSNOOP_TURNED_ON_KEY, false);
}

static bool get_btsnoop_should_save_last(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, BTSNOOP_SHOULD_SAVE_LAST_KEY, false);
}

#if defined(MTK_LINUX_SNOOP_CONFIG) && (MTK_LINUX_SNOOP_CONFIG == TRUE)
static int get_btsnoop_log_mode(void) {
  return config_get_int(config, CONFIG_DEFAULT_SECTION, BTSNOOP_LOG_MODE_KEY, 0);
}

static int get_btsnoop_max_log_file_size(void) {
  return config_get_int(config, CONFIG_DEFAULT_SECTION, BTSNOOP_MAX_LOG_FILE_SIZE_KEY, 0);
}

static int get_btsnoop_trimmed_packet_size(void) {
  return config_get_int(config, CONFIG_DEFAULT_SECTION, BTSNOOP_TRIMMED_PACKET_SIZE_KEY, 0);
}
#endif

static bool get_trace_config_enabled(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, TRACE_CONFIG_ENABLED_KEY, false);
}

static bool get_pts_secure_only_mode(void) {
    return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_SECURE_ONLY_MODE, false);
}

static bool get_pts_conn_updates_disabled(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_LE_CONN_UPDATED_DISABLED, false);
}

static bool get_pts_crosskey_sdp_disable(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_DISABLE_SDP_LE_PAIR, false);
}

static const char *get_pts_smp_options(void) {
  return config_get_string(config, CONFIG_DEFAULT_SECTION, PTS_SMP_PAIRING_OPTIONS_KEY, NULL);
}

static int get_pts_smp_failure_case(void) {
  return config_get_int(config, CONFIG_DEFAULT_SECTION, PTS_SMP_FAILURE_CASE_KEY, 0);
}

#if defined(MTK_LINUX_GAP_PTS_TEST) && (MTK_LINUX_GAP_PTS_TEST == TRUE)
static bool get_pts_passive_encrypt(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_PASSIVE_ENCRYPT, false);
}
static bool get_pts_passive_security_request(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_PTSSIVE_SECURITY_REQUEST, false);
}
static bool get_pts_disable_get_remote_services(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_DISABLE_GET_REMOTE_SERVICES, false);
}
static bool get_pts_extend_idle_timeout(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_EXTEND_IDLE_TIMEOUT, false);
}
static bool get_pts_non_resolvable_private_address(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_NON_RPA, false);
}
static bool get_pts_disable_ble_connection_update(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_DISABLE_CONNECTION_UPDATE, false);
}
static bool get_pts_disable_discover(void) {
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, PTS_DISABLE_DISCOVER, false);
}
#endif

#if defined(MTK_LINUX_STACK_LOG2FILE) && (MTK_LINUX_STACK_LOG2FILE == TRUE)
static const char *get_btstack_log_path(void)
{
  return config_get_string(config, CONFIG_DEFAULT_SECTION, BTSTACK_LOG_PATH_KEY, "/data/misc/bluetooth/logs/bt_stack.log");
}

static bool get_btstack_log2file_turned_on(void)
{
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, BTSTACK_LOG2FILE_TURNED_ON_KEY, false);
}

static bool get_btstack_should_save_last(void)
{
  return config_get_bool(config, CONFIG_DEFAULT_SECTION, BTSTACK_SHOULD_SAVE_LAST_KEY, false);
}
#endif

static config_t *get_all(void) {
  return config;
}

const stack_config_t interface = {
  get_btsnoop_log_path,
  get_btsnoop_turned_on,
  get_btsnoop_should_save_last,
#if defined(MTK_LINUX_SNOOP_CONFIG) && (MTK_LINUX_SNOOP_CONFIG == TRUE)
  get_btsnoop_log_mode,
  get_btsnoop_max_log_file_size,
  get_btsnoop_trimmed_packet_size,
#endif
  get_trace_config_enabled,
  get_pts_secure_only_mode,
  get_pts_conn_updates_disabled,
  get_pts_crosskey_sdp_disable,
  get_pts_smp_options,
  get_pts_smp_failure_case,
#if defined(MTK_LINUX_GAP_PTS_TEST) && (MTK_LINUX_GAP_PTS_TEST == TRUE)
  get_pts_passive_encrypt,
  get_pts_passive_security_request,
  get_pts_disable_get_remote_services,
  get_pts_extend_idle_timeout,
  get_pts_non_resolvable_private_address,
  get_pts_disable_ble_connection_update,
  get_pts_disable_discover,
#endif
#if defined(MTK_LINUX_STACK_LOG2FILE) && (MTK_LINUX_STACK_LOG2FILE == TRUE)
  get_btstack_log_path,
  get_btstack_log2file_turned_on,
  get_btstack_should_save_last,
#endif
  get_all
};

#if defined(MTK_LINUX_EXPORT_API) && (MTK_LINUX_EXPORT_API == TRUE)
EXPORT_SYMBOL const stack_config_t *stack_config_get_interface() {
#else
const stack_config_t *stack_config_get_interface() {
#endif
  return &interface;
}

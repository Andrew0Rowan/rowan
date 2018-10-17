#define LOG_TAG "mdroid_stack_config"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "osi/include/config.h"
#include "osi/include/log.h"
#include "osi/include/list.h"
#include "stack_config.h"
#include "mdroid_buildcfg.h"
#include "mdroid_stack_config.h"
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
#include "osi/include/compat.h"
#endif

#if defined(MTK_STACK_CONFIG_LOG) && (MTK_STACK_CONFIG_LOG == TRUE)
const char *BTLOG_FWLOG_HCI_CMD1 = "C1";
const char *BTLOG_FWLOG_HCI_CMD2 = "C2";
#endif /* MTK_STACK_CONFIG_LOG */

#if defined(MTK_STACK_CONFIG) && (MTK_STACK_CONFIG == TRUE)
#define CONFIG_MTK_CONF_SECTION "MtkBtConf"
static const char *STACK_CONF_OVERRIDE_KEY = "MtkStackConfigOverride";
static const char *EXTFILE_OVERRIDE_TMPKEY = "OverrideConf"; /* easy to parse /sdcard/btsc, it's not a key in config */

#define CONFIG_MTK_FWLOG_SECTION "MtkBtFWLog"

static const char *BTSNOOP_TURNED_ON_KEY = "BtSnoopLogOutput";

#if defined(MTK_STACK_CONFIG_DEFAULT_OVERRIDE) && (MTK_STACK_CONFIG_DEFAULT_OVERRIDE == TRUE)
const char *BTDefaultConfOverrideFile = "bt_stack.conf.sqc";
#endif

/**
 * Override default configure file /etc/bluetooth/bt_stack.conf
 *
 * Current Design:
 *  1. Upper Layer or User control the config file path written in sdcard/btrc
 *  2. Here override the stack configure according to the preset configure
 *
 * TODO:
 * 1. Move config relevant definition to local. e.g. MTK_STACK_CONFIG_FPATH_LEN
 * 2. Use system property to control the log mode
 */
bool parse_override_cfg(config_t * config) {
  FILE * target_file = NULL;
  char str_ovconf_fpath[MTK_STACK_CONFIG_FPATH_LEN] = { '\0' };
  const char *p_redir_ovconf_fpath = NULL;
  char * p_fpath = NULL;
  char prefix_fpath1[MTK_STACK_CONFIG_FPATH_LEN] = "/etc/bluetooth/";
  char str_redir_ov_fpath[MTK_STACK_CONFIG_FPATH_LEN] = { '\0' };
  bool b_override = false;

  if (config == NULL) {
    LOG_ERROR(LOG_TAG, "%s Override fail. The default config content is NULL.", __func__);
    return b_override;
  }

  /* MtkStackConfigOverride = /scard/btsc in bt_stack.conf */
  strlcpy(str_redir_ov_fpath, config_get_string(config, CONFIG_MTK_CONF_SECTION, STACK_CONF_OVERRIDE_KEY, ""), sizeof(str_redir_ov_fpath));

  LOG_INFO(LOG_TAG,"%s M_BTCONF redir file is \"%s\"", __func__, str_redir_ov_fpath);

  p_redir_ovconf_fpath = str_redir_ov_fpath;

  target_file = fopen(p_redir_ovconf_fpath, "rt");
  if (!target_file) {
    LOG_INFO(LOG_TAG,"%s M_BTCONF open redir-file %s fails!", __func__, p_redir_ovconf_fpath);

#if defined(MTK_STACK_CONFIG_DEFAULT_OVERRIDE) && (MTK_STACK_CONFIG_DEFAULT_OVERRIDE == TRUE)
    LOG_INFO(LOG_TAG,"%s M_BTCONF set the override default config: %s!", __func__, BTDefaultConfOverrideFile);
    strlcpy(str_ovconf_fpath, BTDefaultConfOverrideFile, sizeof(str_ovconf_fpath));
#else
    /* MTK_STACK_CONFIG_DEFAULT_OVERRIDE is not defined or MTK_STACK_CONFIG_DEFAULT_OVERRIDE == 0 */
    return false; /* Don't override config - keep it as default config of bluedroid */
#endif

  } else {
      fclose(target_file);

      config_t *redir_config = config_new(p_redir_ovconf_fpath);
      if (redir_config) {
          /* copy ov filepath from /scard/btsc */
          strlcpy(str_ovconf_fpath, config_get_string(redir_config, CONFIG_DEFAULT_SECTION, EXTFILE_OVERRIDE_TMPKEY, ""), sizeof(str_ovconf_fpath));

          config_free(redir_config);
      }
  }

  LOG_INFO(LOG_TAG,"%s M_BTCONF OverrideConf= %s", __func__, str_ovconf_fpath);

  if (str_ovconf_fpath[0] != '\0') {
    FILE *test_file = NULL;

    if (!strcmp(str_ovconf_fpath, "bt_stack.conf.sqc") ||
        !strcmp(str_ovconf_fpath, "bt_stack.conf.debug")) {

      if ((strlen(str_ovconf_fpath) + strlen(prefix_fpath1)) <= (MTK_STACK_CONFIG_FPATH_LEN - 1))
        strcat(prefix_fpath1, str_ovconf_fpath);
      else {
        LOG_ERROR(LOG_TAG,"%s M_BTCONF file/path \"prefix+overrideconf_fpath\" exceeds the size of array: %d", __func__, MTK_STACK_CONFIG_FPATH_LEN);
        return false;
      }

      test_file = fopen(prefix_fpath1, "rt");
      if (!test_file) {
        LOG_INFO(LOG_TAG,"%s M_BTCONF open %s fails!", __func__, prefix_fpath1);
        return false;
      } else {

        fclose(test_file);
        p_fpath = prefix_fpath1;
      }
    } else {

      test_file = fopen(str_ovconf_fpath, "rt");
      if (!test_file) {
        LOG_INFO(LOG_TAG,"%s M_BTCONF open %s fails!", __func__, str_ovconf_fpath);
        return false;
      } else {

        fclose(test_file);
        p_fpath = str_ovconf_fpath;
      }
    }
  }

  if (p_fpath) {
    LOG_INFO(LOG_TAG,"%s M_BTCONF config_override file/path \"%s\"", __func__, p_fpath);
    b_override = config_override(config, p_fpath);
  } else
      LOG_INFO(LOG_TAG,"%s M_BTCONF config_override file/path is NULL", __func__);

  return b_override;
}

#endif

#if defined(MTK_STACK_CONFIG_LOG) && (MTK_STACK_CONFIG_LOG == TRUE)
uint8_t fwcfg_tx[MTK_STACK_CONFIG_NUM_OF_HEXLIST][MTK_STACK_CONFIG_NUM_OF_HEXITEMS];

static void init_fwlog_cmd_ary(void) {
  int fi;

  /* Initialize fwcfg_tx[fi][0], device/src/Controller.c will send hci commands depends on the value is not ZERO,
   * So the max. number of hci commands can be sent is 5 NUM_OF_EXTCMD
   */
  for (fi = 0 ; fi < MTK_STACK_CONFIG_NUM_OF_HEXLIST; fi++) {
    fwcfg_tx[fi][0] = 0;
  }
}

static bool parse_fwlog_pairs(config_t *pick_fwlog_conf) {
  bool b_check_hci_cmd_ready = false;

  if (!pick_fwlog_conf) {
    LOG_INFO(LOG_TAG, "%s M_BTCONF invalid conf for fwlog key/vals", __func__);
    return false;

  } else {
    #define NUM_OF_OCTS_BF_PARAMETER 4
    #define IDX_OF_PARAMLEN_IN_OCTS 3
    /* C1 = 01 BE FC 01 05 */
    /* C2 = 01 5F FC 2A 50 01 09 00 00 00
              01 00 00 00
              00 00 00 00
              01 00 00 00
              01 00 00 00
              01 00 00 00
              01 00 00 00
              01 00 01 00
              03 38 00 00
              01 00 00 00

       param len of C1 = 0x01
       param len of C2 = 0x2A
       NUM_OF_OCTS_BF_PARAMETER 4 including len oct
     */

    unsigned int c1val[MTK_STACK_CONFIG_NUM_OF_HEXITEMS] = {'\0'};
    unsigned int c2val[MTK_STACK_CONFIG_NUM_OF_HEXITEMS] = {'\0'};
    const char* C1;
    const char *C2[MTK_STACK_CONFIG_NUM_OF_HEXROWITEMS] = { "\0" };
    int hexcidx = 0;
    char strkey[32] = { '\0' };

    C1 = config_get_string(pick_fwlog_conf, CONFIG_MTK_FWLOG_SECTION, BTLOG_FWLOG_HCI_CMD1, "");

    C2[0] = config_get_string(pick_fwlog_conf, CONFIG_MTK_FWLOG_SECTION, BTLOG_FWLOG_HCI_CMD2, "");
    for (hexcidx = 1; hexcidx < MTK_STACK_CONFIG_NUM_OF_HEXROWITEMS; hexcidx++) {
      sprintf(strkey, "%s%02d", BTLOG_FWLOG_HCI_CMD2, hexcidx);
      LOG_INFO(LOG_TAG, "strkey=%s", strkey);
      C2[hexcidx] = config_get_string(pick_fwlog_conf, CONFIG_MTK_FWLOG_SECTION, strkey, "");
    }

    if (*C1) {
      int j, plen;
      int ret = sscanf(C1, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                       c1val, c1val+1, c1val+2, c1val+3, c1val+4, c1val+5, c1val+6, c1val+7,
                       c1val+8, c1val+9, c1val+10, c1val+11, c1val+12, c1val+13, c1val+14, c1val+15);
      LOG_INFO(LOG_TAG, "M_BTCONF scan %d hex", ret);
      for (j = 0; j < ret; j++) {
          LOG_INFO(LOG_TAG, " 0x%x", c1val[j]);
          fwcfg_tx[0][j] = (uint8_t)c1val[j];
      }

      plen = fwcfg_tx[0][IDX_OF_PARAMLEN_IN_OCTS];
      if (ret == (plen + NUM_OF_OCTS_BF_PARAMETER)) {
        LOG_INFO(LOG_TAG, "M_BTCONF scanned param num matches the value (0x%02x) in command", plen);
        b_check_hci_cmd_ready = true;
      } else {
        LOG_INFO(LOG_TAG, "M_BTCONF scanned param num (0x%02x) doesn't match the value (0x%02x) in command", ret - NUM_OF_OCTS_BF_PARAMETER, plen);
        b_check_hci_cmd_ready = false;
      }
    } else {
      LOG_INFO(LOG_TAG, "M_BTCONF no \"C1\" section");
      b_check_hci_cmd_ready = false;
    }

    if (*C2[0] && b_check_hci_cmd_ready) {
      int j, plen, ret, cidx = 0, didx = 0;

      while (cidx < MTK_STACK_CONFIG_NUM_OF_HEXROWITEMS && C2[cidx]) {
        ret = sscanf(C2[cidx], "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                     c2val, c2val+1, c2val+2, c2val+3, c2val+4, c2val+5, c2val+6, c2val+7,
                     c2val+8, c2val+9, c2val+10, c2val+11, c2val+12, c2val+13, c2val+14, c2val+15);
        LOG_INFO(LOG_TAG, "M_BTCONF scan %d hex", ret);
        if (ret > 0) {
          for (j = 0; j < ret; j++) {
            LOG_INFO(LOG_TAG, " 0x%x", c2val[j]);
            fwcfg_tx[1][didx] = (uint8_t)c2val[j];
            didx++;
          }
        }
        cidx++;
      }

      plen = fwcfg_tx[1][IDX_OF_PARAMLEN_IN_OCTS];
      LOG_INFO(LOG_TAG, "M_BTCONF plen = 0x%x", fwcfg_tx[1][IDX_OF_PARAMLEN_IN_OCTS]);
      if (didx == (plen + NUM_OF_OCTS_BF_PARAMETER)) {
        LOG_INFO(LOG_TAG, "M_BTCONF scanned param num matches the value (0x%02x) in command", plen);
        b_check_hci_cmd_ready = true;
      } else {
        LOG_INFO(LOG_TAG, "M_BTCONF scanned param num (0x%02x) doesn't match the value (0x%02x) in command", didx - NUM_OF_OCTS_BF_PARAMETER, plen);
        b_check_hci_cmd_ready = false;
      }
    } else {
      if (!*C2[0]) {
        LOG_INFO(LOG_TAG, "M_BTCONF no \"C2\" section");
        b_check_hci_cmd_ready = false;
      }
    }
    return b_check_hci_cmd_ready;
  }

  return false;
}

static bool get_fwlog_hci_pack_hexlists(void) {

  int ret = false;
  init_fwlog_cmd_ary();
  config_t * config = stack_config_get_interface()->get_all();
  if (config_get_bool(config, CONFIG_DEFAULT_SECTION, BTSNOOP_TURNED_ON_KEY, false)) {
    LOG_INFO(LOG_TAG, "M_BTCONF attempt to parse fwlog hci cmds");
    if (false == parse_fwlog_pairs(config)) {
      /* empty hci commands */
      LOG_INFO(LOG_TAG, "M_BTCONF empty hci cmds");
      init_fwlog_cmd_ary();
    } else
      ret = true;

  } else {
    LOG_INFO(LOG_TAG, "%s M_BTCONF %s=false => Don't parse fwlog hci cmds", __func__, BTSNOOP_TURNED_ON_KEY);
  }

  return ret;
}

static const uint8_t *get_fwlog_hci_whole_hexlist(void) {
  return (uint8_t *)fwcfg_tx;
}

const stack_config_pack_hexlist_t fwlog_hci_hexlist_interface = {
  get_fwlog_hci_pack_hexlists,
  get_fwlog_hci_whole_hexlist,
};

const stack_config_pack_hexlist_t *stack_config_fwlog_hexs_get_interface() {
  return &fwlog_hci_hexlist_interface;
}

#endif /* MTK_STACK_CONFIG_LOG == TRUE */


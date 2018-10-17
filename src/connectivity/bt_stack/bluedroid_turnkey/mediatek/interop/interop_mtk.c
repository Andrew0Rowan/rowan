/******************************************************************************
 *
 *  Copyright (C) 2010 MediaTek Inc.
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

/******************************************************************************
 *
 *  This file contains functions for the MTK defined interop function
 *
 ******************************************************************************/
#define LOG_TAG "bt_device_interop_mtk"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "interop_mtk.h"
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
#include "bdaddr.h"
#include "bt_types.h"
#include "osi/include/log.h"
#include "btif_config.h"
#include "btm_api.h"

bt_status_t btmtk_get_ble_device_name(const bt_bdaddr_t *remote_bd_addr, BD_NAME bd_name)
{
    bdstr_t bdstr;
    int length = BD_NAME_LEN;
    bdaddr_to_string(remote_bd_addr, bdstr, sizeof(bdstr));
    int ret = btif_config_get_str(bdstr, "Name", (char*)bd_name, &length);
    return ret ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

bool interop_mtk_match_name(const interop_feature_t feature, const bt_bdaddr_t* addr)
{
    char *dev_name_str;
    BD_NAME remote_name = {0};
    BD_ADDR bd_addr;

    bdcpy(bd_addr, addr->address);
    dev_name_str = BTM_SecReadDevName(bd_addr);

    //need to load ble device name from config
    if (dev_name_str == NULL || dev_name_str[0] == '\0')
    {
        dev_name_str = (char *)remote_name;
        btmtk_get_ble_device_name(addr, remote_name);
    }

    if (dev_name_str != NULL && dev_name_str[0] != '\0')
    {
        char bdstr[20] = {0};
        LOG_DEBUG(LOG_TAG, "%s match device %s(%s) for interop workaround.", __func__,
            dev_name_str, bdaddr_to_string(addr, bdstr, sizeof(bdstr)));

        return interop_match_name(feature, (const char *)dev_name_str);
    }
    return false;
}

bool interop_mtk_match_addr_name(const interop_feature_t feature, const bt_bdaddr_t* addr)
{
    if(interop_match_addr(feature, addr))
        return true;

    return interop_mtk_match_name(feature, addr);
}

#endif

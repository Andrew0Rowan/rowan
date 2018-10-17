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
#pragma once

#include "mdroid_buildcfg.h"
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
#include "device/include/interop.h"

/*******************************************************************************
**
** Function         interop_mtk_match_name
**
** Description      Looks up the interop_name_database for the
**                  specified BD address.
**
** Returns          TRUE if matched, else FALSE
**
*******************************************************************************/
extern bool interop_mtk_match_name(const interop_feature_t feature, const bt_bdaddr_t* addr);

/*******************************************************************************
**
** Function         interop_mtk_match_addr_name
**
** Description      Looks up the interop_addr_database and interop_name_database for the
**                  specified BD address.
**
** Returns          TRUE if matched, else FALSE
**
*******************************************************************************/
extern bool interop_mtk_match_addr_name(const interop_feature_t feature, const bt_bdaddr_t* addr);

#endif

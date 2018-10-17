/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define __ALLOCATE_CFG_AUDIO_DEFAULT_H

#include "CFG_file_public.h"
#include "libnvram.h"
#include "inc/CFG_file_lid.h"
#include "inc/CFG_module_file.h"
#include "inc/CFG_module_default.h"
#include "inc/CFG_file_info.h"
#include "CFG_file_info_custom.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_FILENAMELEN 128;
#define RESERVE_PATH	"Reserved"
#define RESERVE_VER	"000"

int BT_ConvertFunc(int, int, char*, char*);

const TCFG_FILE g_akCFG_File[]= {
    {
        "/data/nvram/APCFG/APRDCL/FILE_VER",		VER(AP_CFG_FILE_VER_INFO_LID), 		4,
        CFG_FILE_VER_FILE_REC_TOTAL,			DEFAULT_ZERO,						0,  DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDEB/BT_Addr",		VER(AP_CFG_RDEB_FILE_BT_ADDR_LID),	CFG_FILE_BT_ADDR_REC_SIZE,
        CFG_FILE_BT_ADDR_REC_TOTAL,			SIGNLE_DEFUALT_REC,		(char *)&stBtDefault,  DataConvert , BT_ConvertFunc
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_Para",	VER(AP_CFG_RDCL_CAMERA_PARA_LID),   CFG_FILE_CAMERA_PARA_REC_SIZE,
        CFG_FILE_CAMERA_PARA_REC_TOTAL,			DEFAULT_ZERO,						0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_3A",	   	VER(AP_CFG_RDCL_CAMERA_3A_LID),     CFG_FILE_CAMERA_3A_REC_SIZE,
        CFG_FILE_CAMERA_3A_REC_TOTAL,			    DEFAULT_ZERO,						0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_DEFECT",	VER(AP_CFG_RDCL_CAMERA_DEFECT_LID), CFG_FILE_CAMERA_DEFECT_REC_SIZE,
        CFG_FILE_CAMERA_DEFECT_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SENSOR",	VER(AP_CFG_RDCL_CAMERA_SENSOR_LID), CFG_FILE_CAMERA_SENSOR_REC_SIZE,
        CFG_FILE_CAMERA_SENSOR_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_LENS",	VER(AP_CFG_RDCL_CAMERA_LENS_LID),   CFG_FILE_CAMERA_LENS_REC_SIZE,
        CFG_FILE_CAMERA_LENS_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/BWCS",			VER(AP_CFG_RDCL_BWCS_LID), 	        CFG_FILE_BWCS_CONFIG_SIZE,
        CFG_FILE_BWCS_CONFIG_TOTAL,				SIGNLE_DEFUALT_REC,					(char *)&stBWCSConfigDefault, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/Voice_Recognize_Param",   VER(AP_CFG_RDCL_FILE_VOICE_RECOGNIZE_PARAM_LID), CFG_FILE_VOICE_RECOGNIZE_PAR_SIZE,
        CFG_FILE_VOICE_RECOGNIZE_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Voice_Recognize_Par_default, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/Audio_AudEnh_Control_Opt",   VER(AP_CFG_RDCL_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_LID), CFG_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_SIZE,
        CFG_FILE_AUDIO_AUDENH_CONTROL_OPTION_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&AUDENH_Control_Option_Par_default, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/Audio_VOIP_Param",   VER(AP_CFG_RDCL_FILE_AUDIO_VOIP_PAR_LID), CFG_FILE_AUDIO_VOIP_PAR_SIZE,
        CFG_FILE_AUDIO_VOIP_PAR_TOTAL, SIGNLE_DEFUALT_REC  ,    (char *)&Audio_VOIP_Par_default, DataReset , NULL
    },
 
    {
        "/data/nvram/APCFG/APRDCL/CAMERA_VERSION",	VER(AP_CFG_RDCL_CAMERA_VERSION_LID),   CFG_FILE_CAMERA_VERSION_REC_SIZE,
        CFG_FILE_CAMERA_VERSION_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_FEATURE",	VER(AP_CFG_RDCL_CAMERA_FEATURE_LID),   CFG_FILE_CAMERA_FEATURE_REC_SIZE,
        CFG_FILE_CAMERA_FEATURE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_GEOMETRY",	VER(AP_CFG_RDCL_CAMERA_GEOMETRY_LID),   CFG_FILE_CAMERA_GEOMETRY_REC_SIZE,
        CFG_FILE_CAMERA_GEOMETRY_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING2",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING3",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING4",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING5",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING6",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING7",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING8",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING9",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING10",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING11",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_SHADING12",	VER(AP_CFG_RDCL_CAMERA_SHADING_LID),CFG_FILE_CAMERA_SHADING_REC_SIZE,
        CFG_FILE_CAMERA_SHADING_REC_TOTAL,			DEFAULT_ZERO,					    0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE2",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE3",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE4",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE5",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE6",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE7",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE8",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE9",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE10",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE11",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },

    {
        "/data/nvram/APCFG/APRDCL/CAMERA_PLINE12",	VER(AP_CFG_RDCL_CAMERA_PLINE_LID),   CFG_FILE_CAMERA_PLINE_REC_SIZE,
        CFG_FILE_CAMERA_PLINE_REC_TOTAL,			        DEFAULT_ZERO,				0, DataReset , NULL
    },
};
int iCustomBeginLID=AP_CFG_CUSTOM_BEGIN_LID;
extern int iCustomBeginLID;
int iFileVerInfoLID=AP_CFG_FILE_VER_INFO_LID;
extern int iFileVerInfoLID;
int iFileBTAddrLID=AP_CFG_RDEB_FILE_BT_ADDR_LID;
extern int iFileBTAddrLID;

typedef struct {
    char	cFileName[128];
    unsigned int iLID;
} FileName;
typedef struct {
    char cCheckFileName[128];
    int  iCheckFileIndex;

} CheckFileName;

typedef struct {
    char cRecordCallerName[128];
    int  iRecordCallerIndex;
} RecordCallerName;


typedef struct {
    int lid;
    unsigned char meta_mode_write;
    unsigned char security_enable;
} TCFG_FILE_PROTECT;



FileName aBackupToBinRegion[]= {
    {"FILE_VER",AP_CFG_FILE_VER_INFO_LID},
    {"BT_Addr",AP_CFG_RDEB_FILE_BT_ADDR_LID},
    {"WIFI",AP_CFG_RDEB_FILE_WIFI_LID},
    {"WIFI_CUSTOM",AP_CFG_RDEB_WIFI_CUSTOM_LID},
    {"GPS",AP_CFG_CUSTOM_FILE_GPS_LID},
#ifndef MTK_PRODUCT_INFO_SUPPORT
    {"PRODUCT_INFO",AP_CFG_REEB_PRODUCT_INFO_LID},
#endif
    {"XOCAP", AP_CFG_CUSTOM_FILE_XOCAP_LID},
	{"ETHERNET", AP_CFG_RDED_ETHERNET_CUSTOM_LID},
};
FileName aPerformance[]= {
    {"CAMERA_Para",AP_CFG_RDCL_CAMERA_PARA_LID},
    {"CAMERA_3A",AP_CFG_RDCL_CAMERA_3A_LID},
    {"CAMERA_SHADING",AP_CFG_RDCL_CAMERA_SHADING_LID},
    {"CAMERA_DEFECT",AP_CFG_RDCL_CAMERA_DEFECT_LID},
    {"CAMERA_SENSOR",AP_CFG_RDCL_CAMERA_SENSOR_LID},
    {"CAMERA_LENS",AP_CFG_RDCL_CAMERA_LENS_LID},
    {"CAMERA_SHADING2",AP_CFG_RDCL_CAMERA_SHADING2_LID},
    {"CAMERA_SHADING3",AP_CFG_RDCL_CAMERA_SHADING3_LID},
    {"CAMERA_SHADING4",AP_CFG_RDCL_CAMERA_SHADING4_LID},
    {"CAMERA_SHADING5",AP_CFG_RDCL_CAMERA_SHADING5_LID},
    {"CAMERA_SHADING6",AP_CFG_RDCL_CAMERA_SHADING6_LID},
    {"CAMERA_SHADING7",AP_CFG_RDCL_CAMERA_SHADING7_LID},
    {"CAMERA_SHADING8",AP_CFG_RDCL_CAMERA_SHADING8_LID},
    {"CAMERA_SHADING9",AP_CFG_RDCL_CAMERA_SHADING9_LID},
    {"CAMERA_SHADING10",AP_CFG_RDCL_CAMERA_SHADING10_LID},
    {"CAMERA_SHADING11",AP_CFG_RDCL_CAMERA_SHADING11_LID},
    {"CAMERA_SHADING12",AP_CFG_RDCL_CAMERA_SHADING12_LID},
    {"CAMERA_PLINE",AP_CFG_RDCL_CAMERA_PLINE_LID},
    {"CAMERA_PLINE2",AP_CFG_RDCL_CAMERA_PLINE2_LID},
    {"CAMERA_PLINE3",AP_CFG_RDCL_CAMERA_PLINE3_LID},
    {"CAMERA_PLINE4",AP_CFG_RDCL_CAMERA_PLINE4_LID},
    {"CAMERA_PLINE5",AP_CFG_RDCL_CAMERA_PLINE5_LID},
    {"CAMERA_PLINE6",AP_CFG_RDCL_CAMERA_PLINE6_LID},
    {"CAMERA_PLINE7",AP_CFG_RDCL_CAMERA_PLINE7_LID},
    {"CAMERA_PLINE8",AP_CFG_RDCL_CAMERA_PLINE8_LID},
    {"CAMERA_PLINE9",AP_CFG_RDCL_CAMERA_PLINE9_LID},
    {"CAMERA_PLINE10",AP_CFG_RDCL_CAMERA_PLINE10_LID},
    {"CAMERA_PLINE11",AP_CFG_RDCL_CAMERA_PLINE11_LID},
    {"CAMERA_PLINE12",AP_CFG_RDCL_CAMERA_PLINE12_LID},
};
const TCFG_FILE_PROTECT g_akCFG_File_Protect[]= {
    // {AP_CFG_RDEB_FILE_BT_ADDR_LID,1,1},
};
CheckFileName aCheckExistList[]= {
///	{"/data/nvram/APCFG/APRDCL/FILE_VER",		0},


};

RecordCallerName aRecordCallerList[]= {
    {"Barcode",		1},
    {"IMEI",		2},
};



extern const TCFG_FILE g_akCFG_File[];
extern FileName aBackupToBinRegion[];
extern FileName aPerformance[];
extern CheckFileName aCheckExistList[];
extern const TCFG_FILE_PROTECT g_akCFG_File_Protect[];



const unsigned int g_i4CFG_File_Count = sizeof(g_akCFG_File)/sizeof(TCFG_FILE);
const unsigned int g_Backup_File_Count = sizeof(aBackupToBinRegion)/(sizeof(FileName));
const unsigned int g_Performance_File_Count = sizeof(aPerformance)/(sizeof(FileName));
const unsigned int g_Check_File_Count = sizeof(aCheckExistList)/(sizeof(CheckFileName));
const unsigned int g_i4CFG_File_Protect_Count = sizeof(g_akCFG_File_Protect)/(sizeof(TCFG_FILE_PROTECT));
const unsigned int g_RecordCaller_Count = sizeof(aRecordCallerList)/(sizeof(RecordCallerName));

extern const unsigned int g_i4CFG_File_Count;
extern const unsigned int g_Backup_File_Count;
extern const unsigned int g_Performance_File_Count;
extern const unsigned int g_Check_File_Count ;
extern const unsigned int g_i4CFG_File_Protect_Count;
extern const unsigned int g_RecordCaller_Count;
//misc log feature
int nvram_misc_log_feature = 0;
int nvram_misc_log_block_offset = 2;


//check exist feature
int nvram_check_exist_feature = 0;
int nvram_check_exist_block_offset = 1;


//read back check feature
int nvram_read_back_feature = 0;

//Add for new nvram partition feature
pfCallbackForDaemon callback_for_nvram_daemon = NULL;
extern pfCallbackForDaemon callback_for_nvram_daemon;

#ifdef MTK_PRODUCT_INFO_SUPPORT
extern bool nvram_new_partition_support()
{
    return true;
}
const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[] = {
    { AP_CFG_REEB_PRODUCT_INFO_LID, 0, 1024 * 1024 },
};
const unsigned int g_new_nvram_lid_count = sizeof(g_new_nvram_lid)/sizeof(TABLE_FOR_SPECIAL_LID);
const char *nvram_new_partition_name = "/dev/pro_info";
extern const char *nvram_new_partition_name;
extern const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[];
extern const unsigned int g_new_nvram_lid_count;
#else
extern bool nvram_new_partition_support()
{
    return false;
}
const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[] = {0 , 0, 0};
const unsigned int g_new_nvram_lid_count = 0;
const char *nvram_new_partition_name = NULL;
extern const char *nvram_new_partition_name;
extern const TABLE_FOR_SPECIAL_LID g_new_nvram_lid[];
extern const unsigned int g_new_nvram_lid_count;
#endif
//end new nvram partition feature

int BT_ConvertFunc(int CurrentVerID, int NewVerID, char *pSrcMem, char *pDstMem)
{
    int rec_size = CFG_FILE_BT_ADDR_REC_SIZE;
    int rec_num = CFG_FILE_BT_ADDR_REC_TOTAL;

    NVRAM_LOG("BT_ConvertFunc: CurrentVerID[%d], NewVerID[%d]\n", CurrentVerID, NewVerID);

    if(NULL == pSrcMem || NULL == pDstMem) {
        NVRAM_LOG("NULL == pSrcMem || NULL == pDstMem\n");
        return false;
    } else {
        if(0 == CurrentVerID && 1 == NewVerID) {
            memcpy(pDstMem, pSrcMem, 30); // Keep the customization data
            memset(pDstMem + 30, 0, rec_size*rec_num - 30);
            return true;
        }
        return false;
    }
}

#ifdef MTK_EMMC_SUPPORT
extern bool nvram_emmc_support()
{
    return true;
}
#else
extern bool nvram_emmc_support()
{
    return false;
}
#endif

extern bool nvram_md5_support()
{
    return false;
}

extern bool nvram_evdo_support()
{
    return false;
}

#ifdef NVRAM_MULTI_STORAGE_SUPPORT
extern int nvram_multi_storage_support(NVRAM_PLATFORM_T* pPlatform )
{
    pPlatform->log_block = g_CpPlatform_Block_Custom.log_block;
    pPlatform->resv_block = g_CpPlatform_Block_Custom.resv_block;
    pPlatform->DM_block = g_CpPlatform_Block_Custom.DM_block;
    pPlatform->layout_version = g_CpPlatform_Block_Custom.layout_version;
    pPlatform->header_offset = g_CpPlatform_Block_Custom.header_offset;
    return true;
}
#else
extern int nvram_multi_storage_support(NVRAM_PLATFORM_T* pPlatform )
{
    return false;
}
#endif

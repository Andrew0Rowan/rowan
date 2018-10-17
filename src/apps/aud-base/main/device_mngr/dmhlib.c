/* Copyright Statement:                                                        
 *                                                                             
 * This software/firmware and related documentation ("MediaTek Software") are  
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without 
 * the prior written permission of MediaTek inc. and/or its licensors, any     
 * reproduction, modification, use or disclosure of MediaTek Software, and     
 * information contained herein, in whole or in part, shall be strictly        
 * prohibited.                                                                 
 *                                                                             
 * MediaTek Inc. (C) 2014. All rights reserved.                                
 *                                                                             
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")     
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER  
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL          
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED    
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR          
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH 
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,            
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.   
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK       
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE  
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR     
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S 
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE       
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE  
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE  
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.    
 *                                                                             
 * The following software/firmware and/or related documentation ("MediaTek     
 * Software") have been modified by MediaTek Inc. All revisions are subject to 
 * any receiver's applicable license agreements with MediaTek Inc.             
 */



#include <unistd.h>
#include <pthread.h>
#include <strings.h>

#include "dm.h"
#include "dmhlib.h"
#include "u_os.h"
#include "u_common.h"
#include "u_rwlock.h"
#include "u_ipcd.h"

BOOL   _fgDmh_mount_usbfs = TRUE;
INT32  g_root_usb_num     = 0;

extern DMH_DEV_T  *t_dmh_dev;

#define PATH_SIZE       256
#define LINE_SIZE       256
#define DMMINORBITS     8
#define DMMINORMASK     ((1U << DMMINORBITS) - 1)
#define MIN_STACK_SIZE  200
#define STACK_INC_SIZE  10

#define DMMAJOR(dev)    ((unsigned int) ((dev) >> DMMINORBITS))
#define DMMINOR(dev)    ((unsigned int) ((dev) & DMMINORMASK))
#define TS_ASSERT(x)    { if (!(x)) { printf("TS_ASSERT: file: %s, line: %d.\n", __FILE__, __LINE__); while (1); } }

typedef struct _tagTreeStack
{
    char *pPath;
} TreeStack_T;

typedef struct _tagStackHandle_T
{
    TreeStack_T **ppEntry;
    INT32 i4Size;
    INT32 i4Top;
} StackHandle_T;

int dm_mknod(const char *pathname, int type, mode_t mode, dev_t dev)
{
    int ret;
    char bCmd[256];
    char bType;

    if(S_IFCHR == type)
    {
        bType = 'c';
    }
    else if(S_IFBLK == type)
    {
        bType = 'b';
    }
    else
    {
        return mknod(pathname, type | mode, dev);
    }

    memset(bCmd, 0, sizeof(bCmd));
    snprintf(bCmd, sizeof(bCmd), "mknod %s %c %u %u", pathname, bType, DMMAJOR(dev), DMMINOR(dev));
    ret = system(bCmd);
    if(ret)
    {
        goto ERROR;
    }

    memset(bCmd, 0, sizeof(bCmd));
    UNUSED(mode);  // mode is decimal, not octal.
    snprintf(bCmd, sizeof(bCmd), "chmod 0660 %s", pathname);
    ret = system(bCmd);
    if(ret)
    {
        goto ERROR;
    }

    return ret;

ERROR:

    return -1;
}

BOOL dmh_check_file(const CHAR *ps_dev_path)
{
    struct stat statbuf;

    if (0 == stat(ps_dev_path, &statbuf))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

INT32 dmh_parse_serial_file(const CHAR *p_dev_path,CHAR *p_serialstr)
{
    CHAR filename[PATH_SIZE];
    FILE *f;
    CHAR line[LINE_SIZE];
    INT32 length = 0;

    if ((NULL == p_dev_path) || (NULL == p_serialstr))
    {
        return DMH_INVALID_PARM;
    }

    memset(filename,0,sizeof(filename));
    strncpy(filename, p_dev_path, sizeof(filename));
    length = strlen(p_dev_path);
    if(strncmp(&p_dev_path[length-1],"/",1)==0)
    {
        strncat(filename, "serial", sizeof(filename));
    }
    else
    {
        strncat(filename, "/serial", sizeof(filename));
    }

    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return DMH_INVALID_PARM;
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';
    }

    strncpy(p_serialstr, line, strlen(line));

    fclose(f);

    return DMH_OK;
}

INT32 dmh_parse_uevent_file(const CHAR *p_dev_path,BLK_UEVENT_ATTR_T *pt_uevent_attr)
{
    CHAR filename[PATH_SIZE];
    FILE *file;
    CHAR line[LINE_SIZE];
    INT32 length = 0;

    if ((NULL == p_dev_path) || (NULL == pt_uevent_attr))
    {
        return DMH_INVALID_PARM;
    }

    memset(filename,0,sizeof(filename));
    strncpy(filename, p_dev_path, sizeof(filename));
    length = strlen(p_dev_path);
    if(0 == strncmp(&p_dev_path[length-1],"/",1))
    {
        strncat(filename, "uevent", sizeof(filename));
    }
    else
    {
        strncat(filename, "/uevent", sizeof(filename));
    }

    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    DM_INFO("[DMH]file:%s\n", filename);

    file = fopen(filename, "r");
    if (NULL == file)
    {
        DM_ERR("[DMH]%10s(%d): file %s, err message: %s\n", __FUNCTION__,__LINE__,filename, strerror(errno));
        return DMH_INVALID_PARM;
    }

    while (fgets(line, sizeof(line), file))
    {
        CHAR *pos;
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';
        DM_INFO("[DMH]line:%s\n", line);

        if (0 == strncmp(line, "MAJOR=", 6))
        {
            pt_uevent_attr->ui4_major = strtoull(&line[6], NULL, 10);
        }
        else if (0 == strncmp(line, "MINOR=", 6))
        {
            pt_uevent_attr->ui4_minor = strtoull(&line[6], NULL, 10);
        }
        else if (0 == strncmp(line, "DEVTYPE=", 8))
        {
            if (pt_uevent_attr->ps_dev_type)
            {
                free(pt_uevent_attr->ps_dev_type);
            }
            pt_uevent_attr->ps_dev_type = strdup(&line[8]);
        }
        else if (0 == strncmp(line, "PHYSDEVPATH=", 12))
        {
            if (pt_uevent_attr->ps_phys_dev_path)
            {
                free(pt_uevent_attr->ps_phys_dev_path);
            }
            pt_uevent_attr->ps_phys_dev_path = strdup(&line[12]);
        }
        else if (0 == strncmp(line, "PHYSDEVDRIVER=", 14))
        {
            if (pt_uevent_attr->ps_phys_dev_driver)
            {
                free(pt_uevent_attr->ps_phys_dev_driver);
            }
            pt_uevent_attr->ps_phys_dev_driver = strdup(&line[14]);
        }
        else if (0 == strncmp(line, "DRIVER=", 7))
        {
            if (pt_uevent_attr->ps_driver)
            {
                free(pt_uevent_attr->ps_driver);
            }
            pt_uevent_attr->ps_driver = strdup(&line[7]);
        }
        else if (0 == strncmp(line, "PRODUCT=", 8))
        {
            if (pt_uevent_attr->ps_product)
            {
                free(pt_uevent_attr->ps_product);
            }
            pt_uevent_attr->ps_product = strdup(&line[8]);
        }
    }

    fclose(file);

    return DMH_OK;
}

INT32 dmh_free_uevent_file(BLK_UEVENT_ATTR_T *pt_uevent_attr)
{

    if(NULL == pt_uevent_attr)
    {
        return DMH_INVALID_PARM;
    }

    if(NULL != pt_uevent_attr->ps_dev_type)
    {
        free(pt_uevent_attr ->ps_dev_type);
        pt_uevent_attr ->ps_dev_type = NULL;
    }

    if(NULL != pt_uevent_attr->ps_phys_dev_driver)
    {
        free(pt_uevent_attr->ps_phys_dev_driver);
        pt_uevent_attr->ps_phys_dev_driver = NULL;
    }

    if(NULL != pt_uevent_attr->ps_phys_dev_path)
    {
        free(pt_uevent_attr->ps_phys_dev_path);
        pt_uevent_attr->ps_phys_dev_path = NULL;
    }

    if(NULL != pt_uevent_attr->ps_driver)
    {
        free(pt_uevent_attr->ps_driver);
        pt_uevent_attr->ps_driver = NULL;
    }
    
    if(NULL != pt_uevent_attr->ps_product)
    {
        free(pt_uevent_attr->ps_product);
        pt_uevent_attr->ps_product = NULL;
    }

    return DMH_OK;
}

CHAR *dmh_get_attr_txt(const CHAR *ps_dev_path,const CHAR *ps_attr_name )
{
    CHAR filename[PATH_SIZE];
    INT32 length = 0;
    CHAR line[LINE_SIZE];
    CHAR * tmp_char = NULL;
    FILE *f;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    length=strlen(ps_dev_path);

    if(0 == strncmp(&ps_dev_path[length-1],"/",1))
    {
        strncat(filename, ps_attr_name, sizeof(filename) - strlen(filename));
    }
    else
    {
        strncat(filename, "/", sizeof(filename));
        strncat(filename, ps_attr_name, sizeof(filename) - strlen(filename));
    }

    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return NULL;
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';

        if(tmp_char)
        {
            free(tmp_char);
        }

        tmp_char = strdup(&line[0]);
    }

    fclose(f);

    return tmp_char;
}

INT32 dmh_get_attr_num(const CHAR *ps_dev_path,const CHAR *ps_attr_name )
{
    CHAR filename[PATH_SIZE];
    INT32 length=0;
    CHAR line[LINE_SIZE];
    INT32 ui4_value = 0;
    FILE *f;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    length=strlen(ps_dev_path);
    if(0 == strncmp(&ps_dev_path[length-1],"/",1))
    {
        strncat(filename, ps_attr_name, sizeof(filename) - strlen(filename));
    }
    else
    {
        strncat(filename, "/", sizeof(filename));
        strncat(filename, ps_attr_name, sizeof(filename) - strlen(filename));
    }

    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return DMH_ERR;
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';

        ui4_value = strtoull(line, NULL, 10);
    }

    fclose(f);

    return ui4_value;
}

INT32 dmh_get_attr_vendor_id(const CHAR *ps_dev_path)
{
    CHAR filename[PATH_SIZE];
    INT32 length=0;
    CHAR line[LINE_SIZE];
    INT32 ui4_value = 0;
    FILE *f;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    length=strlen(ps_dev_path);
    if(0 == strncmp(&ps_dev_path[length-1],"/",1))
    {
        strncat(filename, "idVendor", sizeof(filename));
    }
    else
    {
        strncat(filename, "/idVendor", sizeof(filename));
    }
    DM_INFO("dmh_get_attr_vendor_id filename=%s .\n", filename);
    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return DMH_ERR;
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';
        ui4_value = strtoull(line, NULL, 16);
    }

    fclose(f);

    return ui4_value;
}

INT32 dmh_get_attr_product_id(const CHAR *ps_dev_path)
{
    CHAR filename[PATH_SIZE];
    INT32 length=0;
    CHAR line[LINE_SIZE];
    INT32 ui4_value =0;
    FILE *f;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    length=strlen(ps_dev_path);
    if(0 == strncmp(&ps_dev_path[length-1],"/",1))
    {
        strncat(filename, "idProduct", sizeof(filename));
    }
    else
    {
        strncat(filename, "/idProduct", sizeof(filename));
    }
    DM_INFO("dmh_get_attr_product_id filename=%s .\n", filename);
    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return DMH_ERR;
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';
        ui4_value = strtoull(line, NULL, 16);
    }

    fclose(f);

    return ui4_value;
}

INT32 dmh_get_path_length(const char *p_dev_path)
{
    INT32 i4_path_length = 0;

    if(NULL == p_dev_path)
    {
        return DMH_INVALID_PARM;
    }

    while(*p_dev_path != '\0')
    {
        if(*p_dev_path == '/' )
        {
            i4_path_length++;
        }
        p_dev_path ++;
    }

    return i4_path_length;
}

INT32 dmh_free_unfinish_dev(struct _DMH_DEV_T *pt_usb_dev)
{
    INT32 i4_ret = 0;

    if(NULL == pt_usb_dev)
    {
        return DMH_INVALID_PARM;
    }

    i4_ret = dmh_free_uevent_file(&(pt_usb_dev->t_uevent_attr));

    if(NULL != pt_usb_dev->ps_dev_node)
    {
        free(pt_usb_dev->ps_dev_node);
        pt_usb_dev->ps_dev_node = NULL;
    }

    if(NULL != pt_usb_dev->ps_product)
    {
        free(pt_usb_dev->ps_product);
        pt_usb_dev->ps_product = NULL;
    }

    if(NULL != pt_usb_dev->ps_manufacturer)
    {
        free(pt_usb_dev->ps_manufacturer);
        pt_usb_dev->ps_manufacturer = NULL;
    }

    if(NULL != pt_usb_dev->ps_version)
    {
        free(pt_usb_dev->ps_version);
        pt_usb_dev->ps_version = NULL;
    }

    if(NULL != pt_usb_dev->ps_model)
    {
        free(pt_usb_dev->ps_model);
        pt_usb_dev->ps_model = NULL;
    }

    if(NULL != pt_usb_dev->ps_vendor)
    {
        free(pt_usb_dev->ps_vendor);
        pt_usb_dev->ps_vendor = NULL;
    }

    if(NULL != pt_usb_dev->ps_rev)
    {
        free(pt_usb_dev->ps_rev);
        pt_usb_dev->ps_rev = NULL;
    }

    if(NULL != pt_usb_dev->ps_capability)
    {
        free(pt_usb_dev->ps_capability);
        pt_usb_dev->ps_capability = NULL;
    }

    if(NULL != pt_usb_dev->ps_block_name)
    {
        free(pt_usb_dev->ps_block_name);
        pt_usb_dev->ps_block_name = NULL;
    }

    if(NULL != pt_usb_dev->ps_full_dev_path)
    {
        free(pt_usb_dev->ps_full_dev_path);
        pt_usb_dev->ps_full_dev_path = NULL;
    }

    if(NULL != pt_usb_dev->ps_state)
    {
        free(pt_usb_dev->ps_state);
        pt_usb_dev->ps_state = NULL;
    }

    if(NULL != pt_usb_dev->ps_stat)
    {
        free(pt_usb_dev->ps_stat);
        pt_usb_dev->ps_stat = NULL;
    }

    DM_INFO("%10s(%d):\n",__FUNCTION__,__LINE__);

    free(pt_usb_dev);
    pt_usb_dev = NULL;

    return DMH_OK;

}

INT32 dmh_fill_hub_dev(const CHAR *ps_dev_path,struct _DMH_DEV_T *pt_dmh_dev)
{
    INT32 i4_ret=0;
    BLK_UEVENT_ATTR_T  t_uevent_attr;

    if ((NULL == ps_dev_path) || (NULL == pt_dmh_dev))
    {
        return DMH_INVALID_PARM;
    }

    memset(&t_uevent_attr,0, sizeof(t_uevent_attr));
    i4_ret = dmh_parse_uevent_file(ps_dev_path,&t_uevent_attr);
    if(0 != i4_ret)
    {
        dmh_free_uevent_file(&t_uevent_attr);
        return DMH_ERR;
    }

    pt_dmh_dev->t_uevent_attr.ps_dev_type        = t_uevent_attr.ps_dev_type;
    pt_dmh_dev->t_uevent_attr.ps_phys_dev_driver = t_uevent_attr.ps_phys_dev_driver;
    pt_dmh_dev->t_uevent_attr.ps_phys_dev_path   = strdup(&ps_dev_path[4]);
    pt_dmh_dev->t_uevent_attr.ui4_major          = t_uevent_attr.ui4_major;
    pt_dmh_dev->t_uevent_attr.ui4_minor          = t_uevent_attr.ui4_minor;

    pt_dmh_dev->ps_manufacturer   = dmh_get_attr_txt(ps_dev_path,"manufacturer");
    pt_dmh_dev->ps_product        = dmh_get_attr_txt(ps_dev_path,"product");
    pt_dmh_dev->ps_version        = dmh_get_attr_txt(ps_dev_path,"version");
    pt_dmh_dev->ui4_dev_num       = dmh_get_attr_num(ps_dev_path,"devnum");
    pt_dmh_dev->ui4_max_child_num = dmh_get_attr_num(ps_dev_path,"maxchild");
    pt_dmh_dev->ui4_product_id    = dmh_get_attr_num(ps_dev_path,"idProduct");
    pt_dmh_dev->ui4_vendor_id     = dmh_get_attr_vendor_id(ps_dev_path);
    pt_dmh_dev->ui4_speed_num     = dmh_get_attr_num(ps_dev_path,"speed");
    DM_INFO("%s, Path: %s @Line %d\n",__FUNCTION__, ps_dev_path, __LINE__);
    pt_dmh_dev->ui1_slot_num      = strtoull(&ps_dev_path[strlen(ps_dev_path)-1], NULL, 10);

    return DMH_OK;

}

UINT64 dmh_get_dev_hw_sector_size(const CHAR *ps_dev_path)
{
    CHAR filename[PATH_SIZE];
    UINT64 length=0;
    CHAR line[LINE_SIZE];
    FILE *f;
    UINT8 ui1_loop = 0;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    strncat(filename, "/queue/hw_sector_size", sizeof(filename));
    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        while(1)
        {
            usleep(10000);
            f = fopen(filename, "r");
            ui1_loop++;
            if((10 < ui1_loop) && (NULL == f))
            {
                ui1_loop = 0;
                DM_ERR("Can't read driver type %10s(%d):\n",__FUNCTION__,__LINE__);
                return DMH_ERR;
            }

            if(NULL == f)
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';
        length = strtoull(&line[0], NULL, 10);
    }

    fclose(f);

    return length;
}

UINT64 dmh_get_dev_size(const CHAR *ps_dev_path)
{
    CHAR filename[PATH_SIZE];
    UINT64 length=0;
    CHAR line[LINE_SIZE];
    FILE *f;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    strncat(filename, "/size", sizeof(filename));
    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    DM_INFO("[%s:%d]filename:%s\n", __FUNCTION__, __LINE__, filename);

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return DMH_ERR;
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;
        DM_INFO("[%s:%d]line:%s\n", __FUNCTION__, __LINE__, line);
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';
        length = strtoull(&line[0], NULL, 10);
    }

    fclose(f);

    DM_INFO("[%s:%d]dev size:%llu\n", __FUNCTION__, __LINE__, length);
    return (length);
}

CHAR *dmh_get_model_name(const CHAR * ps_dev_path)
{
    CHAR filename[PATH_SIZE];
    INT32 length;
    CHAR line[LINE_SIZE];
    CHAR * tmp_char = NULL;
    FILE *f;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    length=strlen(ps_dev_path);

    if(0 == strncmp(&ps_dev_path[length-1],"/",1))
    {
        strncat(filename, "model", sizeof(filename));
    }
    else
    {
        strncat(filename, "/model", sizeof(filename));
    }

    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return NULL;
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';

        if (tmp_char)
        {
            free(tmp_char);
        }
        tmp_char = strdup(&line[0]);
    }

    fclose(f);

    return tmp_char;
}

CHAR *dmh_get_vendor_name(const CHAR *ps_dev_path)
{
    CHAR filename[PATH_SIZE];
    INT32 length;
    CHAR line[LINE_SIZE];
    CHAR * tmp_char = NULL;
    FILE *f;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    length=strlen(ps_dev_path);

    if(0 == strncmp(&ps_dev_path[length-1],"/",1))
    {
        strncat(filename, "vendor", sizeof(filename));
    }
    else
    {
        strncat(filename, "/vendor", sizeof(filename));
    }
    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return NULL;
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;

        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';

        if (tmp_char)
        {
            free(tmp_char);
        }
        tmp_char = strdup(&line[0]);
    }

    fclose(f);

    return tmp_char;
}

CHAR *dmh_get_state_name(const CHAR *ps_dev_path)
{
    CHAR filename[PATH_SIZE];
    INT32 length;
    CHAR line[LINE_SIZE];
    CHAR * tmp_char = NULL;
    FILE *f;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    length=strlen(ps_dev_path);

    if(0 == strncmp(&ps_dev_path[length-1],"/",1))
    {
        strncat(filename, "state", sizeof(filename));
    }
    else
    {
        strncat(filename, "/state", sizeof(filename));
    }
    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return NULL;
    }
    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;

        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';

        if (tmp_char)
        {
            free(tmp_char);
        }
        tmp_char = strdup(&line[0]);
    }

    fclose(f);

    return tmp_char;
}

CHAR *dmh_get_rev_name(const CHAR *ps_dev_path)
{
    CHAR filename[PATH_SIZE];
    INT32 length;
    CHAR line[LINE_SIZE];
    CHAR * tmp_char = NULL;
    FILE *f;

    memset(filename,0,sizeof(filename));
    strncpy(filename, ps_dev_path, sizeof(filename));
    length=strlen(ps_dev_path);

    if(0 == strncmp(&ps_dev_path[length-1],"/",1))
    {
        strncat(filename, "rev", sizeof(filename));
    }
    else
    {
        strncat(filename, "/rev", sizeof(filename));
    }
    if (PATH_SIZE <= strlen(filename))
    {
        DM_ERR("[DMH]Fail: filename Size is long: %d, @Line %d\n", (int)sizeof(filename), __LINE__);
    }

    f = fopen(filename, "r");
    if (NULL == f)
    {
        return NULL;
    }

    while (fgets(line, sizeof(line), f))
    {
        CHAR *pos;
        pos = strchr(line, '\n');
        if (NULL == pos)
        {
            continue;
        }
        pos[0] = '\0';
        if (tmp_char)
        {
            free(tmp_char);
        }
        tmp_char = strdup(&line[0]);
    }

    fclose(f);

    return tmp_char;
}

INT32 dmh_add_root_hub_dev(const char *ps_dev_path)
{
    INT32 i4_ret = 0;
    struct _DMH_DEV_T * pt_dmh_dev;

    pt_dmh_dev = malloc(sizeof(struct _DMH_DEV_T));
    if(NULL == pt_dmh_dev)
    {
        return DMH_NO_MEM;
    }
    memset(pt_dmh_dev,0,sizeof(DMH_DEV_T));

    if (RWLR_OK != u_rwl_write_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
    {
        free(pt_dmh_dev);
        return DMH_OS;
    }

    if(NULL != strstr(ps_dev_path,"usb1"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB1;
    }
    else if(NULL != strstr(ps_dev_path,"usb2"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB2;
    }
    else if(NULL != strstr(ps_dev_path,"usb3"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB3;
    }

    pt_dmh_dev->t_device_type    = DMH_DEV_SKT_TYPE_ROOTHUB;
    pt_dmh_dev->ps_full_dev_path = strdup(ps_dev_path);
    pt_dmh_dev->ui1_dev_level    = dmh_get_path_length(ps_dev_path) - DEFAULT_PATH_LEVEL;
    pt_dmh_dev->ui4_dev_flag     = DMH_FLAG_SOCKET_DEV;
    i4_ret = dmh_fill_hub_dev(ps_dev_path, pt_dmh_dev);
    if (DMR_OK != i4_ret)
    {
        dmh_free_unfinish_dev(pt_dmh_dev);
        return DMH_ERR;
    }

    SLIST_INSERT_HEAD(pt_dmh_dev,&(t_dmh_dev->t_dev_list),t_link);

    u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

    return DMH_OK;
}

INT32 dmh_add_real_hub_dev(const char *ps_dev_path,struct _DMH_DEV_T **pt_return_dev)
{
    INT32 i4_ret = 0;

    struct _DMH_DEV_T * pt_dmh_dev;
    pt_dmh_dev = malloc(sizeof(struct _DMH_DEV_T));
    if(NULL == pt_dmh_dev)
    {
        return DMH_NO_MEM;
    }
    memset(pt_dmh_dev,0,sizeof(DMH_DEV_T));

    if (RWLR_OK != u_rwl_write_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
    {
        free(pt_dmh_dev);
        return DMH_OS;
    }

    if(NULL != strstr(ps_dev_path,"usb1"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB1;
    }
    else if(NULL != strstr(ps_dev_path,"usb2"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB2;
    }
    else if(NULL != strstr(ps_dev_path,"usb3"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB3;
    }

    pt_dmh_dev->t_device_type    = DMH_DEV_SKT_TYPE_REALHUB;
    pt_dmh_dev->ps_full_dev_path = strdup(ps_dev_path);
    pt_dmh_dev ->ui1_dev_level   = dmh_get_path_length(ps_dev_path) - DEFAULT_PATH_LEVEL;
    pt_dmh_dev->ui4_dev_flag     = DMH_FLAG_SOCKET_DEV;
    i4_ret = dmh_fill_hub_dev(ps_dev_path, pt_dmh_dev);
    if (DMR_OK != i4_ret)
    {
        dmh_free_unfinish_dev(pt_dmh_dev);
        return DMH_ERR;
    }

    SLIST_INSERT_HEAD(pt_dmh_dev,&(t_dmh_dev->t_dev_list),t_link);

    *pt_return_dev = pt_dmh_dev;

    u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

    return DMH_OK;
}

INT32 dmh_add_fake_hub_dev(const char *ps_dev_path,struct _DMH_DEV_T **pt_return_dev)
{
    INT32 i4_ret = 0;
    struct _DMH_DEV_T * pt_dmh_dev;

    pt_dmh_dev = malloc(sizeof(struct _DMH_DEV_T));
    if(NULL == pt_dmh_dev)
    {
        return DMH_NO_MEM;
    }
    memset(pt_dmh_dev,0,sizeof(DMH_DEV_T));

    if (RWLR_OK != u_rwl_write_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
    {
        free(pt_dmh_dev);
        return DMH_OS;
    }

    if(NULL != strstr(ps_dev_path,"usb1"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB1;
    }
    else if(NULL != strstr(ps_dev_path,"usb2"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB2;
    }
    else if(NULL != strstr(ps_dev_path,"usb3"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB3;
    }

    pt_dmh_dev->ps_full_dev_path = strdup(ps_dev_path);
    pt_dmh_dev->t_device_type    = DMH_DEV_SKT_TYPE_FAKEHUB;
    pt_dmh_dev ->ui1_dev_level   = dmh_get_path_length(ps_dev_path) - DEFAULT_PATH_LEVEL;
    pt_dmh_dev->ui4_dev_flag     = DMH_FLAG_SOCKET_DEV;
    i4_ret = dmh_fill_hub_dev(ps_dev_path, pt_dmh_dev);
    if (DMR_OK != i4_ret)
    {
        dmh_free_unfinish_dev(pt_dmh_dev);

        u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

        return DMH_ERR;
    }

    SLIST_INSERT_HEAD(pt_dmh_dev,&(t_dmh_dev->t_dev_list),t_link);

    *pt_return_dev = pt_dmh_dev;

    u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

    return DMH_OK;
}

INT32 dmh_add_usb_storage_dev(const char *ps_dev_path)
{
    INT32 i4_ret;
    CHAR  temp_dev_path[128];
    CHAR  temp_node_name[20];
    struct _DMH_DEV_T *       pt_usb_storage_dev;
    struct _BLK_UEVENT_ATTR_T t_blk_uevent_attr;

    pt_usb_storage_dev = malloc(sizeof(DMH_DEV_T));
    if(NULL == pt_usb_storage_dev)
    {
        return DMH_NO_MEM;
    }
    memset(pt_usb_storage_dev,0,sizeof(DMH_DEV_T));
    memset(&t_blk_uevent_attr,0,sizeof(BLK_UEVENT_ATTR_T));

    if (RWLR_OK != u_rwl_write_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
    {
        free(pt_usb_storage_dev);
        return DMH_OS;
    }

    if(NULL != strstr(ps_dev_path,"usb1"))
    {
        pt_usb_storage_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB1;
    }
    else if(NULL != strstr(ps_dev_path,"usb2"))
    {
        pt_usb_storage_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB2;
    }
    else if(NULL != strstr(ps_dev_path,"usb3"))
    {
        pt_usb_storage_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB3;
    }

    pt_usb_storage_dev->ps_full_dev_path = strdup(ps_dev_path);
    pt_usb_storage_dev->t_device_type    = DMH_DEV_MED_TYPE_USBSTORAGE;
    pt_usb_storage_dev->ui1_dev_level    = dmh_get_path_length(ps_dev_path) - DEFAULT_PATH_LEVEL -5;
    pt_usb_storage_dev->ui4_dev_flag     = DMH_FLAG_BLOCK_DEV;
    i4_ret = dmh_parse_uevent_file(ps_dev_path,&t_blk_uevent_attr);
    if(DMH_OK != i4_ret)
    {
        if (NULL != pt_usb_storage_dev->ps_full_dev_path)
        {
            free(pt_usb_storage_dev->ps_full_dev_path);
            pt_usb_storage_dev->ps_full_dev_path = NULL;
        }
        free(pt_usb_storage_dev);
        pt_usb_storage_dev = NULL;
        dmh_free_uevent_file(&t_blk_uevent_attr);

        u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

        return DMH_INVALID_PARM;
    }

    pt_usb_storage_dev->t_uevent_attr.ps_dev_type        = t_blk_uevent_attr.ps_dev_type;
    pt_usb_storage_dev->t_uevent_attr.ps_phys_dev_driver = t_blk_uevent_attr.ps_phys_dev_driver;
    pt_usb_storage_dev->t_uevent_attr.ps_phys_dev_path   = t_blk_uevent_attr.ps_phys_dev_path;
    pt_usb_storage_dev->t_uevent_attr.ui4_major          = t_blk_uevent_attr.ui4_major;
    pt_usb_storage_dev->t_uevent_attr.ui4_minor          = t_blk_uevent_attr.ui4_minor;

    strncpy(temp_node_name,"/dev/",sizeof(temp_node_name));
    strncat(temp_node_name, rindex(ps_dev_path,'s'), sizeof(temp_node_name) - strlen(temp_node_name));
    pt_usb_storage_dev->ps_dev_node = strdup(temp_node_name);

    memset(temp_dev_path,0,128);
    size_t path_len = strlen(ps_dev_path)-strlen("block")-strlen(rindex(ps_dev_path,'s'))-1;

    if (path_len > 127)
    {
        strncpy(temp_dev_path,ps_dev_path,127);
    }
    else
    {
        strncpy(temp_dev_path,ps_dev_path,path_len);
    }

    DM_INFO("%s, Path: %s @Line %d\n", __FUNCTION__, temp_dev_path, __LINE__);
    pt_usb_storage_dev->ui1_slot_num       = strtoull(&(rindex(temp_dev_path,':')[1]), NULL, 10);
    pt_usb_storage_dev->ps_model           = dmh_get_model_name(temp_dev_path);
    pt_usb_storage_dev->ps_vendor          = dmh_get_vendor_name(temp_dev_path);
    pt_usb_storage_dev->ui8_size           = dmh_get_dev_size(ps_dev_path);
    pt_usb_storage_dev->ps_state           = dmh_get_state_name(ps_dev_path);
    pt_usb_storage_dev->ui4_hw_sector_size = dmh_get_dev_hw_sector_size(ps_dev_path);
    pt_usb_storage_dev->ps_rev             = dmh_get_rev_name(ps_dev_path);
    pt_usb_storage_dev->ui1_removable      = dmh_get_attr_num(ps_dev_path,"removable");

    SLIST_INSERT_HEAD(pt_usb_storage_dev,&(t_dmh_dev->t_dev_list),t_link);

    u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

    return DMH_OK;
}


INT32 dmh_add_usb_partition_dev(const char *ps_dev_path,const char *ps_dev_nod)
{
    INT32 i4_ret;
    CHAR  temp_dev_path[128];
    struct _DMH_DEV_T *       pt_dmh_dev;
    struct _BLK_UEVENT_ATTR_T t_blk_uevent_attr;

    pt_dmh_dev = malloc(sizeof(DMH_DEV_T));
    if(NULL == pt_dmh_dev)
    {
        return DMH_NO_MEM;
    }
    memset(pt_dmh_dev,0,sizeof(DMH_DEV_T));
    memset(&t_blk_uevent_attr,0,sizeof(BLK_UEVENT_ATTR_T));

    if (RWLR_OK != u_rwl_write_lock(t_dmh_dev->h_dev_rwlock, RWL_OPTION_WAIT))
    {
        free(pt_dmh_dev);
        return DMH_OS;
    }

    i4_ret = dmh_parse_uevent_file(ps_dev_path,&t_blk_uevent_attr);
    if(DMH_OK != i4_ret)
    {
        DM_ERR("[DMH]dmh_parse_uevent_file error, @Line %d\n", __LINE__);
        free(pt_dmh_dev);
        dmh_free_uevent_file(&t_blk_uevent_attr);

        u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

        return DMH_INVALID_PARM;
    }
    pt_dmh_dev->t_device_type                    = DMH_DEV_MED_TYPE_USBPARTI;
    pt_dmh_dev->t_uevent_attr.ps_phys_dev_driver = t_blk_uevent_attr.ps_phys_dev_driver;
    pt_dmh_dev->t_uevent_attr.ps_phys_dev_path   = t_blk_uevent_attr.ps_phys_dev_path;
    pt_dmh_dev->t_uevent_attr.ui4_major          = t_blk_uevent_attr.ui4_major;
    pt_dmh_dev->t_uevent_attr.ui4_minor          = t_blk_uevent_attr.ui4_minor;
    pt_dmh_dev->ui8_size                         = dmh_get_dev_size( ps_dev_path);
    pt_dmh_dev->ps_dev_node                      = strdup(ps_dev_nod);
    DM_INFO("%s, Path: %s @Line %d\n",__FUNCTION__, ps_dev_path, __LINE__);
    pt_dmh_dev->ui1_slot_num                     = strtoull(&ps_dev_path[strlen(ps_dev_path)-1], NULL, 10);
    pt_dmh_dev->ui4_dev_flag                     = DMH_FLAG_BLOCK_DEV;

    CHAR dmh_dev_temp_path[45]={0};
    strncpy(dmh_dev_temp_path, ps_dev_path, 42);
    pt_dmh_dev->ui4_product = dmh_get_attr_product_id(dmh_dev_temp_path);
    pt_dmh_dev->ui4_vendor  = dmh_get_attr_vendor_id(dmh_dev_temp_path);

    DM_INFO("[DMH] ui4_product_id=%d, ui4_vendor_id=%d.\n", (int)pt_dmh_dev->ui4_product, (int)pt_dmh_dev->ui4_vendor);
    if(NULL != strstr(ps_dev_path,"usb1"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB1;
    }
    else if(NULL != strstr(ps_dev_path,"usb2"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB2;
    }
    else if(NULL != strstr(ps_dev_path,"usb3"))
    {
        pt_dmh_dev->ui4_root_dev_id = DMH_ROOT_DEV_USB3;
    }

    pt_dmh_dev->ui1_dev_level    = dmh_get_path_length(ps_dev_path) - DEFAULT_PATH_LEVEL -5;
    pt_dmh_dev->ps_full_dev_path = strdup(ps_dev_path);

    memset(temp_dev_path,0,128);
    strncpy(temp_dev_path,ps_dev_path,strlen(ps_dev_path)-strlen(rindex(ps_dev_path,'s'))-1);
    pt_dmh_dev->ui4_hw_sector_size = dmh_get_dev_hw_sector_size(temp_dev_path);

    SLIST_INSERT_HEAD(pt_dmh_dev,&(t_dmh_dev->t_dev_list),t_link);

    u_rwl_release_lock(t_dmh_dev->h_dev_rwlock);

    return DMH_OK;

}

static INT32 dmh_hand_path(const char *ps_dev_path)
{
    INT32 i4_ret = 0;
    INT32 g_ui4_BusNum =0;
    INT32 g_ui4_DevNum =0;
    struct _BLK_UEVENT_ATTR_T * pt_blk_uevent_attr = NULL;

    CHAR *ps_tmp_name = malloc(256);
    if (NULL != ps_tmp_name)
    {
        memset(ps_tmp_name,0,256);
    }
    else
    {
        return DMH_NO_MEM;
    }

    DM_INFO("ps_dev_path = %s!\n",ps_dev_path);
    strncpy(ps_tmp_name,ps_dev_path, (strlen(ps_dev_path)+1));
    strncat(ps_tmp_name,"/uevent",256);
    if(!dmh_check_file(ps_tmp_name))
    {
        free(ps_tmp_name);
        return DMH_INVALID_PARM;
    }
    free(ps_tmp_name);

    pt_blk_uevent_attr = malloc(sizeof(struct _BLK_UEVENT_ATTR_T));
    if(NULL ==pt_blk_uevent_attr)
    {
        return DMH_NO_MEM;
    }
    memset(pt_blk_uevent_attr,0,sizeof(struct _BLK_UEVENT_ATTR_T));
    i4_ret = dmh_parse_uevent_file(ps_dev_path,pt_blk_uevent_attr);
    if(DMH_OK != i4_ret)
    {
        DM_ERR("dmh_parse_uevent_file error \n");
        dmh_free_uevent_file(pt_blk_uevent_attr);
        free(pt_blk_uevent_attr);

        return DMH_INVALID_PARM;
    }

    DM_INFO("ui4_major: 0x%x @Line %d!\n",(uint)pt_blk_uevent_attr->ui4_major, __LINE__);
    DM_INFO("ui4_minor: 0x%x @Line %d!\n",(uint)pt_blk_uevent_attr->ui4_minor, __LINE__);

    if (NULL != pt_blk_uevent_attr->ps_dev_type)
    {
        DM_INFO("ps_dev_type: %s @Line %d!\n",pt_blk_uevent_attr->ps_dev_type, __LINE__);
    }

    if (NULL != pt_blk_uevent_attr->ps_phys_dev_path)
    {
        DM_INFO("ps_phys_dev_path: %s @Line %d!\n",pt_blk_uevent_attr->ps_phys_dev_path, __LINE__);
    }

    if (NULL != pt_blk_uevent_attr->ps_phys_dev_driver)
    {
        DM_INFO("ps_phys_dev_driver: %s @Line %d!\n",pt_blk_uevent_attr->ps_phys_dev_driver, __LINE__);
    }

    if (NULL != pt_blk_uevent_attr->ps_driver)
    {
        DM_INFO("ps_driver: %s @Line %d!\n",pt_blk_uevent_attr->ps_driver, __LINE__);
    }

    if (NULL != pt_blk_uevent_attr->ps_product)
    {
        DM_INFO("ps_product: %s @Line %d!\n",pt_blk_uevent_attr->ps_product, __LINE__);
    }

    /* USB stub */
    if(NULL != strstr(ps_dev_path,"usb"))
    {
        if((NULL == pt_blk_uevent_attr->ps_dev_type))
        {
            dmh_free_uevent_file(pt_blk_uevent_attr);
            free(pt_blk_uevent_attr);
            return DMH_INVALID_DEV_TYPE;
        }

        if((0 != dmh_get_attr_num(ps_dev_path,"maxchild")) &&
           (1 == dmh_get_attr_num(ps_dev_path,"devnum")))
        {
            DM_INFO("[DMH] [%s:%d] dev is root hub\n", __FUNCTION__, __LINE__);
            dmh_free_uevent_file(pt_blk_uevent_attr);
            free(pt_blk_uevent_attr);

            i4_ret = dmh_add_root_hub_dev(ps_dev_path);
            if(DMH_OK != i4_ret)
            {
                return DMH_ERR;
            }
            else
            {
                return DMH_OK;
            }
        }
        else if(0 == strncmp(pt_blk_uevent_attr->ps_dev_type,"usb_device",strlen("usb_device")))
        {
#if 1
             if (_fgDmh_mount_usbfs)
             {
                    INT32 i4_sys_ret;
                    i4_sys_ret = system("mount -t usbfs none /proc/bus/usb");
                    DM_INFO("[DMH] mount usbfs i4_sys_ret = %d\n", (int)i4_sys_ret);

                    if (-1 == i4_sys_ret)
                    {
                        i4_sys_ret = system("mount -t usbdevfs /proc/bus/usb");
                        DM_INFO("[DMH] mount usbdevfs i4_sys_ret = %d\n", (int)i4_sys_ret);
                    }
                    _fgDmh_mount_usbfs = FALSE;
             }
#endif
            INT32 u4LoopCnt = 0;
            while(NULL == pt_blk_uevent_attr->ps_driver && u4LoopCnt < 20)
            {
                memset(pt_blk_uevent_attr,0,sizeof(struct _BLK_UEVENT_ATTR_T));
                i4_ret = dmh_parse_uevent_file(ps_dev_path,pt_blk_uevent_attr);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("dmh_parse_uevent_file error \n");
                    dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    return DMH_INVALID_PARM;
                }
                u_thread_delay(20);
                u4LoopCnt++;
            }

            if (20 == u4LoopCnt)
            {
                DM_ERR("Can't read driver!\n");
                dmh_free_uevent_file(pt_blk_uevent_attr);
                free(pt_blk_uevent_attr);
                return DMH_INVALID_PARM;
            }
            DM_INFO("%s(%d):pt_blk_uevent_attr->ui4_major = %d,pt_blk_uevent_attr->ui4_minor=%d@\n",__FUNCTION__,__LINE__,(int)pt_blk_uevent_attr->ui4_major,(int)pt_blk_uevent_attr->ui4_minor);
            g_ui4_BusNum = dmh_get_attr_num(ps_dev_path,"busnum");
            g_ui4_DevNum = dmh_get_attr_num(ps_dev_path,"devnum");
            DM_INFO("[FLOW]fucntion :%s ,@Line %d, g_ui4_MTPBusNum == %ld,g_ui4_MTPDevNum ==%ld !\n",__FUNCTION__,__LINE__,g_ui4_BusNum,g_ui4_DevNum);

            char temp_node_name[20];
            dev_t t_tmp;
            mode_t mode = 0660;
            sprintf(temp_node_name, "/dev/usbdev%d.%d", (int)g_ui4_BusNum, (int)g_ui4_DevNum);
            t_tmp = (pt_blk_uevent_attr->ui4_major<<8)|(pt_blk_uevent_attr->ui4_minor);

            i4_ret = dm_mknod(temp_node_name,S_IFCHR,mode,t_tmp);
            DM_INFO("[Flow] func:%10s,in line(%d) mknode call end here !!!:\n",__FUNCTION__,__LINE__);
            if(NULL == pt_blk_uevent_attr->ps_driver)
            {
                dmh_free_uevent_file(pt_blk_uevent_attr);
                free(pt_blk_uevent_attr);
                return DMH_ERR;
            }

            DM_INFO("usb driver found!\n");
            if(0 == strncmp(pt_blk_uevent_attr->ps_driver,"usb",strlen("usb")))
            {
                /* detect mtp device*/
                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                if(dmh_get_attr_num(ps_dev_path,"maxchild") !=0)
                {
                    dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    struct _DMH_DEV_T * pt_return_dev;
                    i4_ret = dmh_add_real_hub_dev(ps_dev_path,&pt_return_dev);
                    if(DMH_OK != i4_ret)
                    {
                        return DMH_ERR;
                    }
                    else
                    {
                        return DMH_OK;
                    }
                }
                else
                {
                    dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    return DMH_OK;
                }
             }
             else
             {
                dmh_free_uevent_file(pt_blk_uevent_attr);
                free(pt_blk_uevent_attr);
                return DMH_ERR;
             }

        }

        else if(0 == strncmp(pt_blk_uevent_attr->ps_dev_type,"usb_interface",strlen("usb_interface")))
        {
             INT32 u4LoopCnt = 0;
             while(NULL == pt_blk_uevent_attr->ps_driver && u4LoopCnt < 20)
             {
                memset(pt_blk_uevent_attr,0,sizeof(struct _BLK_UEVENT_ATTR_T));
                i4_ret = dmh_parse_uevent_file(ps_dev_path,pt_blk_uevent_attr);
                if(DMH_OK != i4_ret)
                {
                    DM_ERR("dmh_parse_uevent_file error \n");
                    dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    return DMH_INVALID_PARM;
                }
                u_thread_delay(20);
                u4LoopCnt++;
            }
            if ((20 == u4LoopCnt)||(NULL == pt_blk_uevent_attr->ps_driver) ||
                (0 == strncmp(pt_blk_uevent_attr->ps_driver,"usb-storage",strlen("usb-storage"))))
            {
                struct _DMH_DEV_T * pt_tmp_dev;
                CHAR* dmh_dev_path = malloc(256);
                if (!dmh_dev_path)
                {
                    dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    return DMH_ERR;
                }
                memset(dmh_dev_path,0,256);
                int i4_temp = strlen(ps_dev_path)-strlen(rindex(ps_dev_path,'/'));  //for clockwork issue
                if(i4_temp < 256)
                {
                    strncat(dmh_dev_path,ps_dev_path,i4_temp);
                }

                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                UINT16 ui2_product_id;
                UINT16 ui2_vendor_id;
                DM_INFO("the current path is %s \n",(CHAR*)dmh_dev_path);
                ui2_product_id = dmh_get_attr_product_id((CHAR*)dmh_dev_path);
                ui2_vendor_id  = dmh_get_attr_vendor_id((CHAR*)dmh_dev_path);
                DM_INFO("ui2_product_id = %d,ui2_vendor_id = %d!\n", (int)ui2_product_id, (int)ui2_vendor_id);
                DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);

                if ((NULL != pt_blk_uevent_attr->ps_driver) &&
                    (0 == strncmp(pt_blk_uevent_attr->ps_driver,"usb-storage",strlen("usb-storage"))))
                {
                       DM_INFO("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                       free(dmh_dev_path);
                }
                else
                {
                    DM_INFO("the current path is %s \n",(CHAR*)dmh_dev_path);
                    i4_ret = dmh_add_fake_hub_dev(dmh_dev_path,&pt_tmp_dev);
                    if(DMH_OK != i4_ret)
                    {
                        DM_ERR("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                        i4_ret = dmh_free_uevent_file(pt_blk_uevent_attr);
                        free(pt_blk_uevent_attr);
                        free(dmh_dev_path);
                        return DMH_ERR;
                    }
                   if (  ((ui2_vendor_id == 0x0CF3) && (ui2_product_id == 0x7011))
                       ||((ui2_vendor_id == 0x0CF3) && (ui2_product_id == 0x7010))
                       ||((ui2_vendor_id == 0x0CF3) && (ui2_product_id == 0x9271))
                       ||((ui2_vendor_id == 0x045E) && (ui2_product_id == 0x02A7))
                       ||((ui2_vendor_id == 0x0471) && (ui2_product_id == 0x209E))
                       ||((ui2_vendor_id == 0x04CA) && (ui2_product_id == 0x4605))
                       ||((ui2_vendor_id == 0x0411) && (ui2_product_id == 0x017F))
                       ||((ui2_vendor_id == 0x148F) && (ui2_product_id == 0x3370))
                       ||((ui2_vendor_id == 0x1286) && (ui2_product_id == 0x2041))
                       ||((ui2_vendor_id == 0x1286) && (ui2_product_id == 0x2042)))
                    {
                        if(strstr(ps_dev_path,"usb1"))
                        {
                            g_root_usb_num=1;
                        }
                        else if (strstr(ps_dev_path,"usb2"))
                        {
                            g_root_usb_num=2;
                        }
                        else if(strstr(ps_dev_path,"usb3"))
                        {
                            g_root_usb_num=3;
                        }
                        else
                        {
                            DM_ERR("[DMH]: not invalid path:dmh_dev_path=%s\n",ps_dev_path);
                        }
                            pt_tmp_dev->t_device_type = DMH_DEV_SKT_TYPE_ATHMAG;
                    }
                    else
                    {
                        DM_ERR("[DMH]%10s(%d):\n",__FUNCTION__,__LINE__);
                        pt_tmp_dev->t_device_type = DMH_DEV_UKW_TYPE_UNKNOWN;
                    }
                    pt_tmp_dev->ui1_dev_level = 2;
                    i4_ret = dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    free(dmh_dev_path);
                    return DMH_OK;
                }

            }

            if (0 == strncmp(pt_blk_uevent_attr->ps_driver,"btusb",strlen("btusb")))
            {
                DM_INFO("dm btusb driver  @Line %d!\n",__LINE__);
                struct _DMH_DEV_T * pt_tmp_dev;
                CHAR* dmh_dev_path = malloc(256);
                if (!dmh_dev_path)
                {
                    dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    return DMH_ERR;
                }
                memset(dmh_dev_path,0,256);

                int i4_temp = strlen(ps_dev_path)-strlen(rindex(ps_dev_path,'/'));  //for clockwork issue
                if(i4_temp < 256)
                {
                    strncat(dmh_dev_path,ps_dev_path,i4_temp);
                }
                UINT16 ui2_product_id;
                UINT16 ui2_vendor_id;
                DM_INFO("the current path is %s \n",(CHAR*)dmh_dev_path);
                ui2_product_id = dmh_get_attr_product_id((CHAR*)dmh_dev_path);
                ui2_vendor_id  = dmh_get_attr_vendor_id((CHAR*)dmh_dev_path);
                DM_INFO("ui2_product_id = %d,ui2_vendor_id = %d!\n", (int)ui2_product_id, (int)ui2_vendor_id);

                i4_ret = dmh_add_fake_hub_dev(dmh_dev_path,&pt_tmp_dev);
                if(DMH_OK != i4_ret)
                {
                    i4_ret = dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    free(dmh_dev_path);
                    return DMH_ERR;
                }

                pt_tmp_dev->t_device_type = DMH_DEV_UKW_TYPE_UNKNOWN;

                DM_INFO("dm, it is%sMarvell type! @Line %d",(pt_tmp_dev->t_device_type == DMH_DEV_SKT_TYPE_MARVELL)?" ":" NO ",__LINE__);
                pt_tmp_dev->ui1_dev_level = 2;
                i4_ret = dmh_free_uevent_file(pt_blk_uevent_attr);
                free(pt_blk_uevent_attr);
                free(dmh_dev_path);

                return DMH_OK;
            }

            if (0 == strncmp(pt_blk_uevent_attr->ps_driver,"rtsta",strlen("rtsta"))||
                0 == strncmp(pt_blk_uevent_attr->ps_driver,"rt2870",strlen("rt2870")))
            {
                 struct _DMH_DEV_T * pt_tmp_dev;
                 CHAR* dmh_dev_path = malloc(256);
                 if (!dmh_dev_path)
                 {
                     dmh_free_uevent_file(pt_blk_uevent_attr);
                     free(pt_blk_uevent_attr);
                     return DMH_ERR;
                 }
                 memset(dmh_dev_path,0,256);
                 strncat(dmh_dev_path,ps_dev_path, strlen(ps_dev_path)-strlen(rindex(ps_dev_path,'/')));
                 i4_ret = dmh_add_fake_hub_dev(dmh_dev_path,&pt_tmp_dev);
                 if(DMH_OK != i4_ret)
                 {
                     dmh_free_uevent_file(pt_blk_uevent_attr);
                     free(pt_blk_uevent_attr);
                     free(dmh_dev_path);
                     return DMH_OK;
                 }

                  DM_INFO("[DMH]: pt_tmp_dev->ui4_product_id: 0x%x\n",(unsigned int)(pt_tmp_dev->ui4_product_id));
                  DM_INFO("[DMH]: pt_tmp_dev->ui4_vendor_id: 0x%x\n",(unsigned int)(pt_tmp_dev->ui4_vendor_id));

                 if (((0x43e == pt_tmp_dev->ui4_vendor_id)&&(0x7A15== pt_tmp_dev->ui4_product_id)) ||
                     ((0x148f == pt_tmp_dev->ui4_vendor_id)&&(7601 == pt_tmp_dev->ui4_product_id)) ||
                     ((0xe8d == pt_tmp_dev->ui4_vendor_id)&&(7650 == pt_tmp_dev->ui4_product_id)) ||
                     ((0xe8d == pt_tmp_dev->ui4_vendor_id)&&(7652 == pt_tmp_dev->ui4_product_id)) ||
                     ((0xe8d == pt_tmp_dev->ui4_vendor_id)&&(7603 == pt_tmp_dev->ui4_product_id)) )
                 {
                     pt_tmp_dev->t_device_type = DMH_DEV_SKT_TYPE_RT3370;
                 }
                 else if(((0xe8d == pt_tmp_dev->ui4_vendor_id)&&(7600 == pt_tmp_dev->ui4_product_id)) ||
                         ((0xe8d == pt_tmp_dev->ui4_vendor_id)&&(7612 == pt_tmp_dev->ui4_product_id)) ||
                         ((0xe8d == pt_tmp_dev->ui4_vendor_id)&&(7632 == pt_tmp_dev->ui4_product_id)) ||
                         ((0xe8d == pt_tmp_dev->ui4_vendor_id)&&(7662 == pt_tmp_dev->ui4_product_id)))
                 {
                     pt_tmp_dev->t_device_type = DMH_DEV_SKT_TYPE_RTSTA;
                 }
                 else
                 {
                     pt_tmp_dev->t_device_type = DMH_DEV_UKW_TYPE_UNKNOWN;
                 }
                 dmh_free_uevent_file(pt_blk_uevent_attr);
                 free(pt_blk_uevent_attr);
                 free(dmh_dev_path);
                 return DMH_OK;
             }
             else if (0 == strncmp(pt_blk_uevent_attr->ps_driver,"usb-storage",strlen("usb-storage")))
             {
                struct _DMH_DEV_T * pt_tmp_dev;
                CHAR* dmh_dev_path = malloc(256);
                if(NULL != dmh_dev_path)
                {
                    memset(dmh_dev_path,0,256);
                    int i4_temp = strlen(ps_dev_path)-strlen(rindex(ps_dev_path,'/'));  //for clockwork issue
                    if(i4_temp < 256)
                    {
                         strncat(dmh_dev_path,ps_dev_path,i4_temp);
                    }

                    DM_INFO(" [DMH]strncmp t_blk_uevent_attr.ps_driver = usb_storage \n");
                    i4_ret = dmh_add_fake_hub_dev(dmh_dev_path,&pt_tmp_dev);
                    if(DMH_OK != i4_ret)
                    {
                        dmh_free_uevent_file(pt_blk_uevent_attr);
                        free(pt_blk_uevent_attr);
                        free(dmh_dev_path);
                        return DMH_ERR;
                    }

                    DM_INFO("usb_storage Added @ %s(%d) && ui_level=%d!\n",__FUNCTION__,__LINE__,(int)pt_tmp_dev->ui1_dev_level);
                    dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    free(dmh_dev_path);
                    return DMH_OK;
                }
                else
                {
                    dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    return DMH_ERR;
                }
             }

        }
        else if(strncmp(pt_blk_uevent_attr->ps_dev_type,"disk",strlen("disk")) ==0)
        {
            DM_INFO("[DMH]scan nand flash path  ps_dev_path %s \n",ps_dev_path);
            i4_ret = dmh_add_usb_storage_dev(ps_dev_path);
            i4_ret = dmh_free_uevent_file(pt_blk_uevent_attr);
            if(DMH_OK != i4_ret)
            {
                return DMH_ERR;
            }
            free(pt_blk_uevent_attr);
            return DMH_OK;
        }
        else if(strncmp(pt_blk_uevent_attr->ps_dev_type,"partition",strlen("partition")) ==0)
        {
            CHAR s_dev_name[255];
            strncpy(s_dev_name,"/dev/",sizeof(s_dev_name));
            strncat(s_dev_name, rindex(ps_dev_path, 's'), sizeof(s_dev_name) - strlen(s_dev_name));

            i4_ret = dmh_add_usb_partition_dev(ps_dev_path,s_dev_name);
            i4_ret = dmh_free_uevent_file(pt_blk_uevent_attr);
            if(DMH_OK != i4_ret)
            {
                return DMH_ERR;
            }
            free(pt_blk_uevent_attr);
            return DMH_OK;
        }
        else if((NULL != pt_blk_uevent_attr->ps_driver) &&
                (0 == strncmp(pt_blk_uevent_attr->ps_driver,"btmtk_usb",strlen("btmtk_usb"))))
        {
            struct _DMH_DEV_T * pt_tmp_dev;
            CHAR* dmh_dev_path = malloc(256);
            if(NULL != dmh_dev_path)
            {
                memset(dmh_dev_path,0,256);
                int i4_temp = strlen(ps_dev_path)-strlen(rindex(ps_dev_path,'/'));  //for clockwork issue
                if(i4_temp < 256)
                {
                    strncat(dmh_dev_path,ps_dev_path,i4_temp);
                }

                DM_INFO(" [DMH]strncmp t_blk_uevent_attr.ps_driver = btmtk_usb \n");
                i4_ret = dmh_add_fake_hub_dev(dmh_dev_path,&pt_tmp_dev);
                if (DMH_OK != i4_ret)
                {
                    dmh_free_uevent_file(pt_blk_uevent_attr);
                    free(pt_blk_uevent_attr);
                    pt_blk_uevent_attr = NULL;
                    free(dmh_dev_path);
                    dmh_dev_path = NULL;
                    return DMH_ERR;
                }
                pt_tmp_dev->t_device_type = DMH_DEV_SKT_TYPE_BLUETOOTH;
                DM_INFO("btmtk_usb Added @ %s(%d) && ui_level=%d!\n",__FUNCTION__,__LINE__,(int)pt_tmp_dev->ui1_dev_level);
                dmh_free_uevent_file(pt_blk_uevent_attr);
                free(pt_blk_uevent_attr);
                pt_blk_uevent_attr = NULL;
                free(dmh_dev_path);
                dmh_dev_path = NULL;
                return DMH_OK;
            }
            else
            {
                dmh_free_uevent_file(pt_blk_uevent_attr);
                free(pt_blk_uevent_attr);
                return DMH_ERR;
            }
        }
        else if( NULL != pt_blk_uevent_attr->ps_dev_type)
        {
            i4_ret = dmh_free_uevent_file(pt_blk_uevent_attr);
            if(i4_ret != DMH_OK)
            {
                return DMH_ERR;
            }
            free(pt_blk_uevent_attr);
            return DMH_OK;
        }
    }
    
    dmh_free_uevent_file(pt_blk_uevent_attr);
    free(pt_blk_uevent_attr);
    return DMH_OK;
}

static int dmh_is_directory(char *name)
{
    struct stat buff;

    if (lstat(name,&buff) < 0)
    {
        return 0;
    }

    return S_ISDIR(buff.st_mode);
}
static BOOL IsStackFull(StackHandle_T *phStack)
{
    TS_ASSERT(NULL != phStack)

    if (phStack)
    {
        if ((phStack->i4Top + 1) > phStack->i4Size)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    DM_ERR("StackHandle error, line: %d.\n", __LINE__);

    return FALSE;
}

static BOOL IsStackEmpty(StackHandle_T *phStack)
{
    TS_ASSERT(NULL != phStack);

    if (phStack)
    {
        if (-1 == phStack->i4Top)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    DM_ERR("StackHandle error, line: %d.\n", __LINE__);

    return FALSE;
}

static StackHandle_T *InitStack(void)
{
    TreeStack_T **ppEntry;
    StackHandle_T *phStack;

    phStack = (StackHandle_T*)malloc(sizeof(StackHandle_T));
    if (!phStack)
    {
        DM_ERR("malloc for stack handle failed, %d.\n", __LINE__);
        return NULL;
    }

    ppEntry = (TreeStack_T**)malloc(sizeof(TreeStack_T*) * MIN_STACK_SIZE);
    if (!ppEntry)
    {
        free(phStack);
        phStack = NULL;
        return NULL;
    }

    phStack->ppEntry = ppEntry;
    phStack->i4Size = MIN_STACK_SIZE;
    phStack->i4Top = -1;

    return phStack;
}

static void UninitStack(StackHandle_T *phStack)
{
    TS_ASSERT(NULL != phStack);

    if (phStack)
    {
        if (phStack->ppEntry)
        {
            free(phStack->ppEntry);
            phStack->ppEntry = NULL;
        }

        free(phStack);
        phStack = NULL;
    }
}

static StackHandle_T *StackPush(StackHandle_T *phStack, TreeStack_T *pNode)
{
    TS_ASSERT((NULL != phStack) && (NULL != pNode));

    if (!phStack || !pNode)
    {
        DM_ERR("invalid arg at line: %d.\n", __LINE__);
        return NULL;
    }

    if (IsStackFull(phStack))
    {
        DM_INFO("stack full, realloc memory for stack!!\n");
        /*[TODO] add code to realloc mem for stack, and remove the assert code*/
        TS_ASSERT(0);
    }

    phStack->i4Top++;

    *(phStack->ppEntry + phStack->i4Top) = pNode;

    return phStack;
}

static TreeStack_T *StackPop(StackHandle_T *phStack)
{
    TreeStack_T *pNode;

    TS_ASSERT(NULL != phStack);

    if (!phStack)
    {
        DM_ERR("invalid arg at line: %d.\n", __LINE__);
        return NULL;
    }

    if (IsStackEmpty(phStack))
    {
        return NULL;
    }

    pNode = *(phStack->ppEntry + phStack->i4Top);
    *(phStack->ppEntry + phStack->i4Top) = NULL;
    phStack->i4Top--;

    return pNode;
}

INT32 dmh_scan_path(char *ps_path)
{
    DIR *dir = NULL;
    struct dirent *dir_entry = NULL;
    StackHandle_T* pHandle = NULL;
    TreeStack_T *pNode = NULL;
    INT32 i4RetCode = DMH_OK;
    size_t len;

    pHandle = InitStack();
    if (NULL == pHandle)
    {
        i4RetCode = DMH_NO_MEM;
        goto clean_exit;
    }

    /*allocate space for root*/
    pNode = (TreeStack_T*)malloc(sizeof(TreeStack_T));
    if (NULL == pNode)
    {
        i4RetCode = DMH_NO_MEM;
        goto clean_exit;
    }

    len = strlen(ps_path);
    pNode->pPath = (char*)malloc(len + 1);
    if (NULL == pNode->pPath)
    {
        free(pNode);
        pNode = NULL;

        i4RetCode = DMH_NO_MEM;
        goto clean_exit;
    }
    memset(pNode->pPath, 0, (len + 1));
    strncpy(pNode->pPath, ps_path, len);
    StackPush(pHandle, pNode);

    while (!IsStackEmpty(pHandle))
    {
        pNode = StackPop(pHandle);
        if (pNode)
        {
            DM_INFO("scan path: %s\n", pNode->pPath);

            dir = opendir(pNode->pPath);
            if (NULL == dir)
            {
                if (ENOTDIR == errno)
                {
                    DM_ERR("%s is not a dir\n", pNode->pPath);
                }
                else
                {
                    DM_ERR("open %s fail, errno=%d\n", pNode->pPath, errno);
                }
            }
            else
            {
                size_t cur_dir_path_len;
                cur_dir_path_len = strlen(pNode->pPath);
                dmh_hand_path(pNode->pPath);

                /*get all children node*/
                while ((dir_entry = readdir(dir)) != NULL)
                {
                    if (!strcmp(dir_entry->d_name, ".") ||
                        !strcmp(dir_entry->d_name, "..")||
                        !strcmp(dir_entry->d_name, "power")||
                        !strcmp(dir_entry->d_name, "usb_device")||
                        !strcmp(dir_entry->d_name, "usb_endpoint")||
                        !strcmp(dir_entry->d_name, "scsi_disk")||
                        !strcmp(dir_entry->d_name, "scsi_device")||
                        (strstr(dir_entry->d_name,"loop")!=NULL) ||
                        (strstr(dir_entry->d_name,"ram")!=NULL))
                    {
                        // skip
                    }
                    else
                    {
                        TreeStack_T *pNewNode;
                        size_t new_path_len;

                        pNewNode = (TreeStack_T*)malloc(sizeof(TreeStack_T));
                        if (NULL == pNewNode)
                        {
                            DM_ERR("failed to malloc: %d\n", __LINE__);
                            continue;
                        }

                        new_path_len = strlen(dir_entry->d_name) + cur_dir_path_len + 1; // add 1 for '/'
                        pNewNode->pPath = (char *)malloc(new_path_len + 1); // add 1 for '\0'
                        if (NULL == pNewNode->pPath)
                        {
                            free(pNewNode);
                            pNewNode = NULL;

                            DM_ERR("failed to malloc: %d\n", __LINE__);
                            continue;
                        }
                        memset(pNewNode->pPath, 0, (new_path_len + 1));
                        snprintf(pNewNode->pPath, (new_path_len + 1), "%s/%s", pNode->pPath, dir_entry->d_name);
                        DM_INFO("[DMH] pNewNode->pPath=%s\n",pNewNode->pPath);

                        if (!dmh_is_directory(pNewNode->pPath))
                        {
                            free(pNewNode->pPath);
                            pNewNode->pPath = NULL;
                            free(pNewNode);
                            pNewNode = NULL;
                            continue;
                        }

                        StackPush(pHandle, pNewNode);
                    }
                }

                closedir(dir);
                dir = NULL;
            }

            if (pNode->pPath)
            {
                free(pNode->pPath);
                pNode->pPath = NULL;
            }

            free(pNode);
            pNode = NULL;
        }
    }

clean_exit:
    UninitStack(pHandle);

    return i4RetCode;
}


INT32 dmh_free_dev(struct _DMH_DEV_T *pt_usb_dev)
{
    INT32 i4_ret = 0;

    if(NULL == pt_usb_dev)
    {
        return DMH_INVALID_PARM;
    }

    i4_ret = dmh_free_uevent_file(&(pt_usb_dev->t_uevent_attr));
    if(DMH_OK != i4_ret)
    {
        return DMH_ERR;
    }

    if(NULL != pt_usb_dev->ps_dev_node)
    {
        free(pt_usb_dev->ps_dev_node);
        pt_usb_dev->ps_dev_node = NULL;
    }

    if(NULL != pt_usb_dev->ps_product)
    {
        free(pt_usb_dev->ps_product);
        pt_usb_dev->ps_product = NULL;
    }

    if(NULL != pt_usb_dev->ps_manufacturer)
    {
        free(pt_usb_dev->ps_manufacturer);
        pt_usb_dev->ps_manufacturer = NULL;
    }

    if(NULL != pt_usb_dev->ps_version)
    {
        free(pt_usb_dev->ps_version);
        pt_usb_dev->ps_version = NULL;
    }

    if(NULL != pt_usb_dev->ps_model)
    {
        free(pt_usb_dev->ps_model);
        pt_usb_dev->ps_model = NULL;
    }

    if(NULL != pt_usb_dev->ps_vendor)
    {
        free(pt_usb_dev->ps_vendor);
        pt_usb_dev->ps_vendor = NULL;
    }

    if(NULL != pt_usb_dev->ps_rev)
    {
        free(pt_usb_dev->ps_rev);
        pt_usb_dev->ps_rev = NULL;
    }

    if(NULL != pt_usb_dev->ps_capability)
    {
        free(pt_usb_dev->ps_capability);
        pt_usb_dev->ps_capability = NULL;
    }

    if(NULL != pt_usb_dev->ps_block_name)
    {
        free(pt_usb_dev->ps_block_name);
        pt_usb_dev->ps_block_name = NULL;
    }

    if(NULL != pt_usb_dev->ps_full_dev_path)
    {
        free(pt_usb_dev->ps_full_dev_path);
        pt_usb_dev->ps_full_dev_path = NULL;
    }

    if(NULL != pt_usb_dev->ps_state)
    {
        free(pt_usb_dev->ps_state);
        pt_usb_dev->ps_state = NULL;
    }

    if(NULL != pt_usb_dev->ps_stat)
    {
        free(pt_usb_dev->ps_stat);
        pt_usb_dev->ps_stat = NULL;
    }
    DM_INFO("%10s(%d):\n",__FUNCTION__,__LINE__);

    free(pt_usb_dev);
    pt_usb_dev = NULL;

    return DMH_OK;

}


INT32 dmh_nfy_func(struct _DMH_DEV_T *pt_current_dev,UINT8 ui1_level,DEV_STATUS_T e_status)
{
    struct _DMH_DEV_T *pt_parent_dev;
    DMH_ROOT_DEV_T     t_root_dev = DMH_ROOT_DEV_UNSUPPORT;
    DM_INFO("[DMH]%10s(%d):ui1_level=(%d)\n",__FUNCTION__,__LINE__,(int)ui1_level);
    BOOL bfound = FALSE;

    if(RWLR_OK != u_rwl_read_lock(t_dmh_dev->h_nfy_rwlock, RWL_OPTION_WAIT))
    {
        return DMH_OS;
    }

    SLIST_FOR_EACH(pt_parent_dev, &(t_dmh_dev->t_dev_list), t_link)
    {
        DM_INFO("[DMH]%10s(%d):pt_parent_dev->ui1_dev_level=%d pt_parent_dev->ps_full_dev_path=%s,pt_parent_dev->t_device_type = %d\n",
                __FUNCTION__,__LINE__, pt_parent_dev->ui1_dev_level,
                pt_parent_dev->ps_full_dev_path,(int)pt_parent_dev->t_device_type);

        if(0x555 == pt_parent_dev->t_uevent_attr.ui4_major)
        {
            DM_ERR("[DMH]%10s(%d):don't map the visual device!\n",__FUNCTION__,__LINE__);
            continue;
        }

        if(((pt_parent_dev->ui1_dev_level == (ui1_level -1)) &&
             (NULL != strstr(pt_current_dev->ps_full_dev_path,pt_parent_dev->ps_full_dev_path)))
             ||((DMH_DEV_MED_TYPE_MMC_STORAGE == pt_parent_dev->t_device_type) &&
             (pt_parent_dev->ui1_dev_level == ui1_level) &&
             (NULL != strstr(pt_current_dev->ps_full_dev_path,pt_parent_dev->ps_full_dev_path))))
        {
            struct _DMH_NFY_DESC_T * pt_nfy_desc = NULL;

            if ((DMH_DEV_UKW_TYPE_UNKNOWN == pt_parent_dev->t_device_type) &&
                (DMH_DEV_MED_TYPE_USBSTORAGE == pt_current_dev->t_device_type))
            {
                DM_INFO("SPECIAL DEVICE, NO NOTIFY!\n");
                continue;
            }

            if ((DMH_DEV_MED_TYPE_USBSTORAGE == pt_current_dev->t_device_type) &&
                (TRUE == pt_current_dev->b_send) && (DEV_STATUS_MED_ATTACH == e_status))
            {
                 DM_INFO("The USB Storage has been send before ...so quit!\n");
                 break;
            }

            SLIST_FOR_EACH(pt_nfy_desc, &(t_dmh_dev->t_dev_nfy_list), t_link)
            {
                if(2 == ui1_level|| 3 == ui1_level)
                {
                    if(NULL != strstr(pt_current_dev->ps_full_dev_path,"MtkUsbHcd.0"))
                    {
                        t_root_dev = DMH_ROOT_DEV_USB1;
                    }
                    if(NULL != strstr(pt_current_dev->ps_full_dev_path,"MtkUsbHcd.1"))
                    {
                        t_root_dev = DMH_ROOT_DEV_USB2;
                    }
                    if(NULL != strstr(pt_current_dev->ps_full_dev_path,"MtkUsbHcd.2"))
                    {
                        t_root_dev = DMH_ROOT_DEV_USB3;
                    }
                    if(NULL != strstr(pt_current_dev->ps_full_dev_path,"mmcblk"))
                    {
                        t_root_dev = DMH_ROOT_DEV_MMC;
                    }
                }

#if 0
                if((DMH_DEV_MED_TYPE_USBSTORAGE == pt_current_dev->t_device_type) &&
                   (0 == pt_current_dev->ui8_size))
                {
                    if(DEV_STATUS_MED_ATTACH == e_status)
                    {
                        DM_INFO("%10s linux %d t_device_type %d:\n",__FUNCTION__,__LINE__,pt_current_dev->t_device_type);
                        break;
                    }
                }
#endif

                if((DMH_DEV_MED_TYPE_USBPARTI == pt_current_dev->t_device_type) &&
                   (pt_current_dev->ui8_size <=10))
                {
                    //if(e_status == DEV_STATUS_MED_ATTACH)
                    {
                        DM_INFO("%10s linux %d t_device_type %d:\n",__FUNCTION__,__LINE__,pt_current_dev->t_device_type);
                        break;
                    }
                }

                if ((DMH_DEV_MED_TYPE_USBSTORAGE == pt_current_dev->t_device_type) &&
                    (DEV_STATUS_MED_DETACH == e_status))
                {
                    if (pt_current_dev->pf_nptify == pt_nfy_desc)
                    {
                        DM_INFO("USB detach pf_nptify = %x!\n", (unsigned int)(pt_current_dev->pf_nptify));
                        bfound = TRUE;
                    }
                }
                else
                {

                    DM_INFO("[%s:%d]pt_nfy_desc major:%lu minor:%lu ui4_root_dev_id:%u\n", __FUNCTION__, __LINE__, pt_nfy_desc->ui4_major
                                                                                    , pt_nfy_desc->ui4_minor, pt_nfy_desc->ui4_root_dev_id);
                    DM_INFO("[%s:%d]pt_parent_dev major:%lu minor:%lu t_root_dev:%u\n", __FUNCTION__, __LINE__, pt_parent_dev->t_uevent_attr.ui4_major
                                                                                    , pt_parent_dev->t_uevent_attr.ui4_minor, t_root_dev);
                    if (((2 == ui1_level|| 3== ui1_level)&&(pt_nfy_desc->ui4_root_dev_id == t_root_dev))
                         ||((pt_nfy_desc->ui4_major== pt_parent_dev->t_uevent_attr.ui4_major)&&
                            (pt_nfy_desc->ui4_minor== pt_parent_dev->t_uevent_attr.ui4_minor)))
                    {
                        bfound = TRUE;
                    }
                }

                if (bfound)
                {
                    struct _DMH_CB_DESC_T*  pt_cb_desc;
                    pt_cb_desc = malloc(sizeof(struct _DMH_CB_DESC_T));
                    if(NULL == pt_cb_desc)
                    {
                        return DMH_NO_MEM;
                    }
                    memset(pt_cb_desc,0,sizeof(struct _DMH_CB_DESC_T));

                    /*need to call the callback function to notify DM */
                    pt_cb_desc->ui4_skt_no   = pt_current_dev->ui1_slot_num;
                    pt_cb_desc->t_dev_type   = pt_current_dev->t_device_type;
                    pt_cb_desc->e_status     = e_status;
                    pt_cb_desc->ui4_major    = pt_current_dev->t_uevent_attr.ui4_major;
                    pt_cb_desc->ui4_minor    = pt_current_dev->t_uevent_attr.ui4_minor;
                    pt_cb_desc->ui4_dev_flag = pt_current_dev->ui4_dev_flag;
                    pt_cb_desc->ui4_product  = pt_current_dev->ui4_product;
                    pt_cb_desc->ui4_vendor   = pt_current_dev->ui4_vendor;

                    if( (pt_current_dev->t_device_type == DMH_DEV_MED_TYPE_USBSTORAGE)      ||
                        (pt_current_dev->t_device_type == DMH_DEV_MED_TYPE_ATA_HDD)         ||
                        (pt_current_dev->t_device_type == DMH_DEV_MED_TYPE_USBPARTI)        ||
                        (pt_current_dev->t_device_type == DMH_DEV_SKT_TYPE_OPTI_DRV)        ||
                        (pt_current_dev->t_device_type == DMH_DEV_MED_TYPE_ATA_HDD_PARTI)   ||
                        (pt_current_dev->t_device_type == DMH_DEV_MED_TYPE_ATA_BD)          ||
                        (pt_current_dev->t_device_type == DMH_DEV_MED_TYPE_MMC_STORAGE)     ||
                        (pt_current_dev->t_device_type == DMH_DEV_MED_TYPE_MMC_PARTI))
                    {
                        DM_INFO("pt_cb_desc->ps_dev_name:%s, &pt_current_dev->ps_dev_node[5]:%s\n", pt_cb_desc->ps_dev_name, (&pt_current_dev->ps_dev_node[5]));
                        strncpy(pt_cb_desc->ps_dev_name,(&pt_current_dev->ps_dev_node[5]),sizeof(pt_cb_desc->ps_dev_name));
                    }

                    DM_INFO("[DMH]pf_nfy %10s(%d): pt_cb_desc->t_dev_type = %d, pt_cb_desc->e_status = %d\n",__FUNCTION__,__LINE__,pt_cb_desc->t_dev_type, pt_cb_desc->e_status);

                    pt_nfy_desc->pf_nfy(pt_nfy_desc->pv_tag,pt_cb_desc->e_status,pt_cb_desc);


                    if ((DMH_DEV_MED_TYPE_USBSTORAGE == pt_current_dev->t_device_type) &&
                        (FALSE == pt_current_dev->b_send) &&
                        (DEV_STATUS_MED_ATTACH == e_status))
                    {
                          DM_INFO("OOpps...so change the status, no send again!\n");
                          pt_current_dev->b_send = TRUE;
                          pt_current_dev->pf_nptify = pt_nfy_desc;
                          DM_INFO("(%s)%d USB attach pf_nptify = %u!\n",__FUNCTION__,__LINE__,(unsigned int)pt_current_dev->pf_nptify);
                          //break;
                    }
                    else if ((DMH_DEV_MED_TYPE_USBSTORAGE == pt_current_dev->t_device_type) &&
                             (pt_current_dev->b_send) && (DEV_STATUS_MED_DETACH== e_status))
                    {
                          DM_INFO("Change the status!\n");
                          pt_current_dev->b_send = FALSE;
                          pt_current_dev->pf_nptify = pt_nfy_desc;
                          DM_INFO("(%s)%d USB attach pf_nptify = %x!\n",__FUNCTION__,__LINE__,(unsigned int)pt_current_dev->pf_nptify);
                    }

                    if((DEV_STATUS_HUB_DETACH == e_status)||(DEV_STATUS_MED_DETACH == e_status))
                    {
                        SLIST_FOR_EACH(pt_nfy_desc, &(t_dmh_dev->t_dev_nfy_list), t_link)
                        {
                            if ((pt_nfy_desc->ui4_major== pt_current_dev->t_uevent_attr.ui4_major)&&
                                (pt_nfy_desc->ui4_minor== pt_current_dev->t_uevent_attr.ui4_minor))
                            {
                                if((DMH_DEV_MED_TYPE_ATA_BD == pt_current_dev->t_device_type) ||
                                   (DMH_DEV_MED_TYPE_ISO == pt_current_dev->t_device_type))
                                {
                                    break;
                                }

                                SLIST_REMOVE(pt_nfy_desc,t_link);

                                free(pt_nfy_desc);

                                break;
                            }
                        }
                    }
                    /*free */
                    free(pt_cb_desc);
                    break;
                }
            }

            break;
        }
    }

    u_rwl_release_lock(t_dmh_dev->h_nfy_rwlock);

    DM_INFO("[%s:%d] pt_current_dev major:%lu minor:%lu call over!\n", __FUNCTION__, __LINE__, pt_current_dev->t_uevent_attr.ui4_major, pt_current_dev->t_uevent_attr.ui4_minor);
    return DMH_OK;

}




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
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
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

/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Thu Jun 22 14:11:32 2017'. */
/* Do NOT modify this source file. */



/* Start of source pre-amble file 'preamble_file.h'. */

#if defined(BT_RPC_DBG_SERVER)

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "config/bt_dbg_config.h"
#include "mtk_bt_service_dbg.h"

int gap_var[8] = {1,2,3,4,5,6,7,8};

int gap_var_test(int array_index, int offset, char *name, char *data, int length)
{
    if (offset >= 1)
    {
        return 0;
    }

    sprintf(name, "gap_var");
    sprintf(data, "%d", gap_var[array_index]);
    return offset + 1;
}

typedef struct
{
    int int_demo;
    char str_demo[256];
}str_test;

str_test gap_str[16] =
{
    {1, "aaa"},
    {2, "bbb"},
    {3, "ccc"},
    {4, "ddd"},
    {5, "eee"},
    {6, "fff"},
    {7, "ggg"},
    {8, "hhh"},
    {9, "iii"},
    {10, "jjj"},
    {11, "kkk"},
    {12, "lll"},
    {13, "mmm"},
    {14, "nnn"},
    {15, "ooo"},
    {16, "ppp"},
};

int gap_str_test(int array_index, int offset, char *name, char *data, int length)
{
    switch(offset)
    {
        case 0:
            sprintf(name, "int_demo");
            sprintf(data, "%d", gap_str[array_index].int_demo);
            break;
        case 1:
            sprintf(name, "str_demo");
            sprintf(data, "%s", gap_str[array_index].str_demo);
            break;
        default:
            return 0;
    }

    return offset + 1;
}

static int bt_dbg_read(int cmd, int array_index)
{
    int m_index = 0, s_index = 0, ret_index = 0, fd = 0;
    char memb[BUFF_LEN] = {0};
    char data[BUFF_LEN] = {0};

    for (m_index = 0; NULL != m_cmd[m_index].c_mod; m_index++)
    {
        if (m_cmd[m_index].d_mod == (cmd >> MOD_LOCATION & 0xFF))
            break;
    }

    if (NULL == m_cmd[m_index].c_mod)
    {
        printf("Input module error!\n");
        return -1;
    }

    for (s_index = 0; 0 != m_cmd[m_index].s_cmd[s_index].cmd; s_index++)
    {
        if (m_cmd[m_index].s_cmd[s_index].cmd == cmd)
            break;
    }

    if (0 == m_cmd[m_index].s_cmd[s_index].cmd)
    {
        printf("Input cmd error!\n");
        return -1;
    }

    fd = open(BT_DBG_R_FILE, O_RDWR | O_CREAT, 0777);
    if (fd < 0)
    {
        printf("%s open bt-dbg-r file failed %d\n", __func__, fd);
        return -1;
    }

    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);

    memb[0] = MEMB_C;
    data[0] = DATA_C;

    memset(&memb[1], 0, BUFF_LEN - 1);
    memset(&data[1], 0, BUFF_LEN - 1);
    ret_index = m_cmd[m_index].s_cmd[s_index].func_r(array_index, 0, &memb[1],  &data[1], BUFF_LEN - 1);
    while(0 != ret_index)
    {
        data[strlen(data)] = '\n';
        data[strlen(data)+1] = '\0';
        strcat(memb, data);
        write(fd, memb, strlen(memb));

        memset(&memb[1], 0, BUFF_LEN - 1);
        memset(&data[1], 0, BUFF_LEN - 1);
        ret_index = m_cmd[m_index].s_cmd[s_index].func_r(array_index, ret_index, &memb[1], &data[1], BUFF_LEN - 1);
    }

    close(fd);
    return 0;
}

static int bt_dbg_write(int cmd, int array_index)
{
    int m_index = 0, s_index = 0;

    for (m_index = 0; NULL != m_cmd[m_index].c_mod; m_index++)
    {
        if (m_cmd[m_index].d_mod == (cmd >> MOD_LOCATION & 0xFF))
            break;
    }

    if (NULL == m_cmd[m_index].c_mod)
    {
        printf("Input module error!\n");
        return -1;
    }

    for (s_index = 0; 0 != m_cmd[m_index].s_cmd[s_index].cmd; s_index++)
    {
        if (m_cmd[m_index].s_cmd[s_index].cmd == cmd)
            break;
    }

    if (0 == m_cmd[m_index].s_cmd[s_index].cmd)
    {
        printf("Input cmd error!\n");
        return -1;
    }

    return 0;
}

int x_mtkapi_bt_dbg_op(int cmd, int op, int index)
{
    if (DBG_OP_READ == op)
    {
        bt_dbg_read(cmd, index);
    }

    if (DBG_OP_WRITE == op)
    {
        bt_dbg_write(cmd, index);
    }

    return 0;
}
#endif

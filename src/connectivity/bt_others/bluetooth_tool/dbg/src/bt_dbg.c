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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#if defined(BT_RPC_DBG_CLIENT)
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "bt_dbg_config.h"
#include "client_common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define for_each_opt(opt, long, short) while ((opt=getopt_long(argc, argv, short ? short:"+", long, NULL)) != -1)

/**************************************************************************
 *                 G L O B A L   V A R I A B L E S                        *
***************************************************************************/

/**************************************************************************
 *                         F U N C T I O N S                              *
***************************************************************************/

typedef struct
{
    char c;
    char *s;
}DBG_FIELD;

typedef enum
{
    HEAD_I = 0,
    TYPE_I,
    ARRY_I,
    NAME_I,
    TAIL_I,
    MEMB_I,
    DATA_I,

    END_I
}FIELD_I;

static DBG_FIELD field[END_I] = {
    {HEAD_C, HEAD},
    {TYPE_C, TYPE},
    {ARRY_C, ARRY},
    {NAME_C, NAME},
    {TAIL_C, TAIL},
    {MEMB_C, MEMB},
    {DATA_C, DATA}
};

static struct option main_options[] = {
    { "help", 0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static struct option sub_options[] = {
    { "help",   0, 0, 'h' },
    { "read",   0, 0, 'r' },
    { "write",  0, 0, 'w' },
    { "all",    0, 0, 'a' },
    { "cmd",    1, 0, 'c' },
    { "index",  1, 0, 'i' },
    { "value",  1, 0, 'v' },
    { 0,        0, 0, 0 }
};

static int parse_description(char *p_dscr, FIELD_I s, char *p_output)
{
    int length = 0;
    FIELD_I e = 0;
    char *p_s = NULL, *p_e = NULL;

    p_output[0] = '\0';

    p_s = strstr(p_dscr, field[s].s);
    if (NULL == p_s)
    {
        if (ARRY_I != s)
        {
            printf("%s:%d conld not find %d %s\n", __func__, __LINE__, s, field[s].s);
        }
        return -1;
    }
    p_s++;

    e = s+1;

    p_e = strstr(p_s, field[e].s);
    if (NULL == p_e)
    {
        if (ARRY_I == e)
        {
            p_e = strstr(p_s, field[e+1].s);
            if (NULL == p_e)
            {
                printf("%s:%d conld not find %d %s\n", __func__, __LINE__, e, field[e].s);
                return -1;
            }
        }
    }

    length = p_e - p_s;
    memcpy(p_output, p_s, length);
    p_output[length] = '\0';

    return 0;
}

static char *parse_buff(char *buff, char *p_name, char *p_data)
{
    int length = 0;
    FIELD_I e = 0;
    char *p_s = NULL, *p_e = NULL;

    p_name[0] = '\0';
    p_data[0] = '\0';

    p_s = strstr(buff, field[MEMB_I].s);
    if (NULL == p_s)
    {
        printf("%s:%d conld not find member\n", __func__, __LINE__);
        return NULL;
    }
    p_s++;

    p_e = strstr(p_s, field[DATA_I].s);
    if (NULL == p_e)
    {
        printf("%s:%d conld not find member tail\n", __func__, __LINE__);
        return NULL;
    }

    length = p_e - p_s;
    memcpy(p_name, p_s, length);
    p_name[length] = '\0';

    p_s = p_e + 1;
    p_e = strstr(p_s, field[MEMB_I].s);
    if (NULL == p_e)
    {
        p_e = strstr(p_s, "\n");
        if (NULL == p_e)
        {
            printf("%s:%d conld not find value tail\n", __func__, __LINE__);
            return NULL;
        }
        else
        {
            p_e++;
        }
    }

    length = p_e - p_s;
    memcpy(p_data, p_s, length);
    p_data[length] = '\0';

    return strstr(p_e, field[MEMB_I].s);
}

static void print_output(char *dscr, int index)
{
    int fd = 0, len = 0, location = 0;
    char *buff = NULL;
    char *p_buff = NULL;
    char name[1024] = {0};
    char value[1024] = {0};

    fd = open(BT_DBG_R_FILE, O_RDONLY, 0777);
    if (fd < 0)
    {
        printf("%s open bt-dbg-r file failed %d\n", __func__, fd);
        return;
    }

    len = lseek(fd, 0, SEEK_END);
    buff = malloc(len + 1);
    if (NULL == buff)
    {
        printf("%s malloc data failed\n", __func__);
        close(fd);
        return;
    }

    memset(buff, 0, len + 1);
    lseek(fd, 0, SEEK_SET);
    read(fd, buff, len);
    p_buff = buff;

    if (NULL != strstr(dscr, TYPE"VAR"))
    {
        p_buff = parse_buff(p_buff, name, value);

        if (NULL == strstr(dscr, ARRY))
        {
            printf("The value of %s is %s\n", name, value);
        }
        else
        {
            printf("The value of %s[%d] is %s\n", name, index, value);
        }
    }
    else if (NULL != strstr(dscr, TYPE"STR"))
    {
        parse_description(dscr, NAME_I, name);
        printf("The members of %s", name);

        if (NULL != strstr(dscr, ARRY))
        {
            printf("[%d]", index);
        }
        printf("\n");

        while(NULL != p_buff)
        {
            p_buff = parse_buff(p_buff, name, value);
            if (0 != strcmp(value, "NO DATA\n"))
            {
                location = 64 - (3 + strlen(name) + strlen(value));

                printf(" %s: ", name);
                while(location > 0)
                {
                    printf(" ");
                    location--;
                }
                printf("%s", value);
            }
        }

        printf("\n");
    }
    else
    {
        printf("Get help info error %d\n", __LINE__);
    }

    free(buff);
    close(fd);

    system("rm -rf "BT_DBG_R_FILE);
    return;
}
static void print_sub_help(int mod)
{
    int i;
    char output[1024] = {0};
    char *ptr_print = NULL, *ptr_end = NULL;
    printf("Usage:\n");
    printf("\tbt-dbg %s --read --all --cmd <integer> --index <integer>\n", m_cmd[mod].c_mod);
//    printf("\tbt-dbg %s --write --cmd <cmd> --index <index> --value <value>\n", m_cmd[mod].c_mod);
    printf("\t --all\tshow all cmd\n");
    printf("\t --cmd\n");
    for (i = 0; i < m_cmd[mod].length - 1; i++)
    {
        printf("\t\t %d:\t", i);
        parse_description(m_cmd[mod].s_cmd[i].dscr, TYPE_I, output);
        if (0 == strcmp(output, "VAR"))
        {
            printf("VARIABLE");
        }
        if (0 == strcmp(output, "STR"))
        {
            printf("STRUCTRUE");
        }

        if (0 == parse_description(m_cmd[mod].s_cmd[i].dscr, ARRY_I, output))
        {
            printf("(%s)", output);
        }

        parse_description(m_cmd[mod].s_cmd[i].dscr, NAME_I, output);
        printf("\t%s\n", output);
    }
    printf("\t --index\tOptional. The index of array for read/write array only.\n");
//    printf("\t --value\tThe value for write only.\n");
    printf("Example\n");
    printf("\tbt-dbg %s --read --all\n", m_cmd[mod].c_mod);
    printf("\tbt-dbg %s --read --cmd 0\n", m_cmd[mod].c_mod);
//    printf("\t bt-dbg %s --write --cmd 0 --value 1\n", m_cmd[mod].c_mod);
    return;
}

static void print_help(void)
{
    int i;

    printf("bluetooth debug test - ver %s\n", VERSION);
    printf("Usage:\n"
        "\tbt-dbg <module> <operation> ...... \n");
    printf("modules:\n");
    for (i = 0; m_cmd[i].c_mod; i++) {
        printf("\t%-8s\t%s\n", m_cmd[i].c_mod, m_cmd[i].dscr);
    }
    printf("\n"
        "For more information on the usage of each cmd use:\n"
        "\tbt-dbg <module> --help\n\n\n");
}

static int param_check(int m_index, int s_index, int index, int flag, int value_flag)
{
    int i = 0, array_index = 0;
    char s_array_index[16] = {0};

    if (s_index >= (m_cmd[m_index].length - 1))
    {
        printf("cmd value %d is invalid, should < %d\n", s_index, m_cmd[m_index].length - 1);
        return -1;
    }

    if (0 != parse_description(m_cmd[m_index].s_cmd[s_index].dscr, ARRY_I, s_array_index))
    {
        if (0 != index)
        {
            printf("Parameter index is not necessary\n", index);
            return -1;
        }
    }
    else
    {
        array_index = atoi(s_array_index);
        if (index >= array_index)
        {
            printf("index value %d is invalid, should < %d\n", index, array_index);
            return -1;
        }
    }

    if ((DBG_OP_WRITE == flag) && (1 != value_flag))
    {
        printf("Value field is necessary for write operation\n");
        return -1;
    }

    return 0;
}

static int rpc_op(int cmd, int flag, int index)
{
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, cmd);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, flag);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, index);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_dbg_op");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

int main(int argc, char *argv[])
{
    int opt, ret = 0;
    int m_index = 0, s_index = 0, wr_flag = 0, all_flag = 0, index = 0, value_flag = 0;

    while ((opt=getopt_long(argc, argv, "+h", main_options, NULL)) != -1) {
        switch (opt) {
        case 'h':
        default:
            print_help();
            exit(0);
        }
    }

    argc -= optind;
    argv += optind;
    optind = 0;

    if (argc < 1) {
        print_help();
        exit(0);
    }

    for (m_index = 0; m_cmd[m_index].c_mod; m_index++) {
        if (0 == strcmp(m_cmd[m_index].c_mod, argv[0]))
        {
            break;
        }
    }

    if (NULL == m_cmd[m_index].c_mod)
    {
        printf("The %s module is invalid.\n", argv[0]);
        exit(0);
    }

    for_each_opt(opt, sub_options, "+r:w:a:c:i:v:h") {
        switch (opt) {
        case 'r':
            wr_flag = DBG_OP_READ;
            break;

        case 'w':
            wr_flag = DBG_OP_WRITE;
            break;

        case 'a':
            all_flag = 1;
            break;

        case 'c':
            s_index = atoi(optarg);
            break;

        case 'i':
            index = atoi(optarg);
            break;

        case 'v':
            value_flag = 1;
            break;

        case 'h':
        default:
            print_sub_help(m_index);
            exit(0);
        }
    }

    if (argc < 2) {
        print_sub_help(m_index);
        exit(0);
    }

    if (0 != param_check(m_index, s_index, index, wr_flag, value_flag))
    {
        exit(0);
    }

    chmod("/tmp/mtk_bt_service",
          S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);

    bt_rpc_init(NULL);
    c_rpc_init_mtk_bt_service_client();
    bt_rpcu_tl_log_start();

    sleep(3);

    if (1 == all_flag)
    {
        int s_i = 0, array_index = 0, a_i = 0;
        CHAR s_array_index[16] = {0};
        s_index = m_cmd[m_index].length;
        for (s_i = 0; s_i < s_index; s_i++)
        {
            if (0 == m_cmd[m_index].s_cmd[s_i].cmd)
            {
                continue;
            }

            if (0 == parse_description(m_cmd[m_index].s_cmd[s_i].dscr, ARRY_I, s_array_index))
            {
                array_index = atoi(s_array_index);
                for (a_i = 0; a_i < array_index; a_i++)
                {
                    ret = rpc_op(m_cmd[m_index].s_cmd[s_i].cmd, wr_flag, a_i);
                    if (ret)
                    {
                        printf("rpc_op error %d\n", ret);
                        return 0;
                    }

                    print_output(m_cmd[m_index].s_cmd[s_i].dscr, a_i);
                }
            }
            else
            {
                ret = rpc_op(m_cmd[m_index].s_cmd[s_i].cmd, wr_flag, 0);
                if (ret)
                {
                    printf("rpc_op error %d\n", ret);
                    return 0;
                }

                print_output(m_cmd[m_index].s_cmd[s_i].dscr, 0);
            }
        }
    }
    else
    {
        ret = rpc_op(m_cmd[m_index].s_cmd[s_index].cmd, wr_flag, index);
        if (ret)
        {
            printf("rpc_op error %d\n", ret);
            return 0;
        }

        print_output(m_cmd[m_index].s_cmd[s_index].dscr, index);
    }
    return 0;
}
#endif


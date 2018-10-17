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

// System header files
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>

#include "btmw_test_cli.h"

static void usage(void)
{
    printf(
"Usage: btcli [OPTION]...\n"
"\n"
"-h, --help              help\n"
"-c, --cli=#             choose cli mode. 0=disable, 1=enable\n"
"    --cli_debug=#       choose cli debug bitmap. #=2 byte hex number\n"
        );
}


int main(int argc, char **argv)
{
    int option_index;
    static const char short_options[] = "hc:";
    static const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {"cli", 1, 0, 'c'},
        {"cli_debug", 1, 0, OPT_CLI_DEBUG},
        {0, 0, 0, 0}
    };
    int c;
    int cli_mode = 1;
    int cli_debug = 0;
    char *cli_debug_bitmap;
    int opt_argc;
    char *opt_argv[2];

    btmw_test_log_init();

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'h':
            usage();
            return 0;
        case 'c':
            cli_mode = strtol(optarg, NULL, 0);
            break;
        case OPT_CLI_DEBUG:
            cli_debug = 1;
            cli_debug_bitmap = optarg;
            break;
        default:
            fprintf(stderr, "Try --help' for more information.\n");
            return 1;
        }
    }

    if (cli_debug)
    {
        opt_argc = 2;
        opt_argv[0] = cli_debug_bitmap;
        opt_argv[1] = "-1";

        btmw_test_set_btcli_handler(opt_argc, &opt_argv[0]);
    }

    if (btmw_test_init())
    {
        return -1;
    }

    if (cli_mode)
    {
        btmw_test_run();
        btmw_test_deinit();
    }
    else
    {
        pause();
    }

    return 0;
}

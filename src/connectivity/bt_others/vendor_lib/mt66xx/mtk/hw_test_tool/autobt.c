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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "bt_hw_test.h"

#define TX_RX_STOP_CMD 0

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define VERSION "5.0"

#define AUTOBT_FILE_PATH "/data/misc/autobt/"
#if TX_RX_STOP_CMD
#define TX_STOP_FILE AUTOBT_FILE_PATH"tx_stop"
#define NSRX_STOP_FILE AUTOBT_FILE_PATH"nsrx_stop"
#define BLETX_STOP_FILE AUTOBT_FILE_PATH"bletx_stop"
#define BLERX_STOP_FILE AUTOBT_FILE_PATH"blerx_stop"
#endif

#define for_each_opt(opt, long, short) while ((opt=getopt_long(argc, argv, short ? short:"+", long, NULL)) != -1)

/**************************************************************************
 *                 G L O B A L   V A R I A B L E S                        *
***************************************************************************/
//static int bt_init = false;
static bool g_inquiry_complete = false;

/**************************************************************************
 *                         F U N C T I O N S                              *
***************************************************************************/

static int check_hex_str(char *str)
{
    int i = 0;
    int len = strlen(str);

    for (; i < len; i++) {
        if ((('a' <= str[i]) && (str[i] <= 'f')) || (('A' <= str[i]) && (str[i] <= 'F')) ||
            (('0' <= str[i]) && (str[i] <= '9')))
            continue;
        else
            return -1;
    }
    return 0;
}

void print_info(char *info)
{
    printf("%s", info);
    if (0 == strcmp(info, "---Inquiry completed---\n"))
        g_inquiry_complete = true;
}

#if 0
static struct option poweron_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static const char *poweron_help =
    "Usage:\n"
    "\tpoweron [no option]\n";

static void cmd_power_on(int argc, char **argv)
{
    int opt;

    for_each_opt(opt, poweron_options, "+h") {
        switch (opt) {
        case 'h':
        default:
            printf("%s", poweron_help);
            return;
        }
    }

    if (bt_init == true) {
        printf("BT device is already on\n");
    }
    else {
        if (HW_TEST_BT_init()) {
            printf("BT device power on success\n");
            bt_init = true;
        }
        else {
            printf("BT device power on failed\n");
        }
    }
    return;
}

static struct option poweroff_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static const char *poweroff_help =
    "Usage:\n"
    "\tpoweroff [no option]\n";

static void cmd_power_off(int argc, char **argv)
{
    int opt;

    for_each_opt(opt, poweroff_options, "+h") {
        switch (opt) {
        case 'h':
        default:
            printf("%s", poweroff_help);
            return;
        }
    }

    if (bt_init == false) {
        printf("BT device is already off\n");
    }
    else {
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        bt_init = false;
    }
    return;
}

static struct option reset_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static const char *reset_help =
    "Usage:\n"
    "\treset [no option]\n";

static void cmd_reset(int argc, char **argv)
{
    int opt;

    for_each_opt(opt, reset_options, "+h") {
        switch (opt) {
        case 'h':
        default:
            printf("%s", reset_help);
            return;
        }
    }

    if (bt_init == false) {
        printf("BT device is off, run \"autobt poweron\" first\n");
    }
    else {
        if (HW_TEST_BT_reset()) {
            printf("BT device reset success\n");
        }
        else {
            printf("BT device reset failed\n");
        }
    }
    return;
}
#endif

static struct option tx_options[] = {
    { "help",	0, 0, 'h' },
    { "pattern", 1, 0, 'p' },
    { "hopping", 1, 0, 'o' },
    { "channel", 1, 0, 'c' },
    { "type", 1, 0, 't' },
    { "length", 1, 0, 'l' },
    { 0, 0, 0, 0 }
};

static const char *tx_help =
    "Usage:\n"
    "\ttx [--pattern] 0x01:\tTx 0000 pattern\n"
    "\t               0x02:\tTx 1111 pattern\n"
    "\t               0x03:\tTx 1010 pattern\n"
    "\t               0x04:\tTx pseudo random bit sequence\n"
    "\t               0x09:\tTx 11110000 pattern\n"
    "\t               0x0A:\tTx single tone\n"
    "\n"
    "\t   [--hopping] 0x00:\tSingle frequency\n"
    "\t               0x01:\t79 channels frequency hopping\n"
    "\n"
    "\t   [--channel] Integer (0~78 channel for single frequency)\n"
    "\n"
    "\t   [--type]    0x00:\tNULL\n"
    "\t               0x01:\tPOLL\n"
    "\t               0x02:\tFHS\n"
    "\t               0x03:\tDM1\n"
    "\t               0x04:\tDH1\n"
    "\t               0x05:\tHV1\n"
    "\t               0x06:\tHV2\n"
    "\t               0x07:\tHV3\n"
    "\t               0x08:\tDV\n"
    "\t               0x09:\tAUX\n"
    "\t               0x0A:\tDM3\n"
    "\t               0x0B:\tDH3\n"
    "\t               0x0E:\tDM5\n"
    "\t               0x0F:\tDH5\n"
    "\t               0x17:\tEV3\n"
    "\t               0x1C:\tEV4\n"
    "\t               0x1D:\tEV5\n"
    "\t               0x24:\t2-DH1\n"
    "\t               0x28:\t3-DH1\n"
    "\t               0x2A:\t2-DH3\n"
    "\t               0x2B:\t3-DH3\n"
    "\t               0x2E:\t2-DH5\n"
    "\t               0x2F:\t3-DH5\n"
    "\t               0x36:\t2-EV3\n"
    "\t               0x37:\t3-EV3\n"
    "\t               0x3C:\t2-EV5\n"
    "\t               0x3D:\t3-EV5\n"
    "\n"
    "\t   [--length]  integer\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt tx --pattern 0x01 --hopping 0x00 --channel 7 --type 0x04 --length 27\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start Tx only test, Controller will send out the specified Tx packet continuously\n"
    "\tTo end test, please type in \"exit\"\n"
    "\n";

static void cmd_tx_test(int argc, char **argv)
{
    int opt, opt_num = 0;
    unsigned char pattern, hopping, type;
    int channel;
    unsigned int length;
    /* To receive terminal input */
    char buf[5] = {0};
    char tmp;
    int i;
    for_each_opt(opt, tx_options, "+p:o:c:t:l:h") {
        opt_num ++;
        switch (opt) {
        case 'p':
            pattern = (unsigned char)strtoul(optarg, NULL, 16);
            break;

        case 'o':
            hopping = (unsigned char)strtoul(optarg, NULL, 16);
            break;

        case 'c':
            channel = atoi(optarg);
            if (channel < 0 || channel > 78) {
                printf("Invalid command option parameter!\n");
                printf("%s", tx_help);
                return;
            }
            break;

        case 't':
            type = (unsigned char)strtoul(optarg, NULL, 16);
            break;

        case 'l':
            length = (unsigned int)strtoul(optarg, NULL, 10);
            break;

        case 'h':
        default:
            printf("%s", tx_help);
            return;
        }
    }

    if (opt_num < 5) {
        printf("Incomplete command options!\n");
        printf("%s", tx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        //printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    //printf("Tx pattern: 0x%02x\n", pattern);
    //printf("Hopping: 0x%02x\n", hopping);
    //printf("Tx channel: %d\n", channel);
    //printf("Packet type: 0x%02x\n", type);
    //printf("Packet length: %d\n", length);

    /* Test start */
    if (HW_TEST_BT_TxOnlyTest_start(
          pattern,
          hopping,
          channel,
          type,
          length) == true) {
        //printf("Tx test start...\n");
    }
    else{
        printf("Try to start Tx test, failed\n");
        HW_TEST_BT_deinit();
        //printf("BT device power off\n");
        return;
    }

#if TX_RX_STOP_CMD
if (0 == access(TX_STOP_FILE, F_OK)) {
    system("rm -r "TX_STOP_FILE);
}
#endif

    /* Loop in waiting for user type "exit" */
    i = 0;
    do {
#if TX_RX_STOP_CMD
        sleep(2);
        if (0 == access(TX_STOP_FILE, F_OK)) {
            system("rm -r "TX_STOP_FILE);
            break;
        }
#else
        if (i >= 5)
            i = 0; /*rollback*/

        tmp = getchar();
        buf[i] = tmp;

        if (tmp != '\r' && tmp != '\n') {
            i ++;
        }
        else {
            buf[i] = '\0';
            if (0 != strcmp(buf, "exit"))
                i = 0; /*discard this string*/
            else
                break;
        }
#endif
    } while(1);

    /* Test end */
    HW_TEST_BT_TxOnlyTest_end();
    //printf("Tx test complete\n");
    /* BT power off */
    HW_TEST_BT_deinit();
    //printf("BT device power off\n");

    return;
}

static struct option nsrx_options[] = {
    { "help",	0, 0, 'h' },
    { "pattern", 1, 0, 'p' },
    { "channel", 1, 0, 'c' },
    { "type", 1, 0, 't' },
    { "addr", 1, 0, 'a' },
    { "second", 1, 0, 's'},
    { 0, 0, 0, 0 }
};

static const char *nsrx_help =
    "Usage:\n"
    "\tnsrx [--pattern] 0x01:\tRx 0000 pattern\n"
    "\t                 0x02:\tRx 1111 pattern\n"
    "\t                 0x03:\tRx 1010 pattern\n"
    "\t                 0x04:\tRx pseudo random bit sequence\n"
    "\t                 0x09:\tRx 11110000 pattern\n"
    "\n"
    "\t     [--channel] Integer (0~78 channel for single frequency)\n"
    "\n"
    "\t     [--type]    -- BR packet --\n"
    "\t                 0x03:\tDM1\n"
    "\t                 0x04:\tDH1\n"
    "\t                 0x0A:\tDM3\n"
    "\t                 0x0B:\tDH3\n"
    "\t                 0x0E:\tDM5\n"
    "\t                 0x0F:\tDH5\n"
    "\t                 -- EDR packet --\n"
    "\t                 0x24:\t2-DH1\n"
    "\t                 0x28:\t3-DH1\n"
    "\t                 0x2A:\t2-DH3\n"
    "\t                 0x2B:\t3-DH3\n"
    "\t                 0x2E:\t2-DH5\n"
    "\t                 0x2F:\t3-DH5\n"
    "\n"
    "\t     [--addr]    Hex XXXXXXXX (UAP+LAP 4 bytes)\n"
    "\t                 if set 0, use default value 0x00A5F0C3\n"
    "\n"
    "\t     [--second] Integer (total Rx time by second, 0 is always Rx util input exit)\n"
    "\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt nsrx --pattern 0x02 --channel 5 --type 0x2A --addr 88C0FFEE --second 0\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start Non-Signal-Rx test on a specified channel\n"
    "\tTo end test, please type in \"exit\", then the PER & BER during test are returned\n"
    "\n";

char rx_str[256] = {0};
#define NSRX_CNT_FILE AUTOBT_FILE_PATH"nsrx_cnt"
static void cmd_non_signal_rx(int argc, char **argv)
{
    int opt, opt_num = 0;
    unsigned char pattern = 0, type = 0;
    int channel = 0;
    int second = 0;
    unsigned int addr = 0; /* UAP+LAP 4 bytes */
    unsigned int pkt_count = 0;
    unsigned int byte_count = 0;
    float PER, BER;
    /* To receive terminal input */
    char buf[5] = {0};
    char tmp;
    int i;

    for_each_opt(opt, nsrx_options, "+p:c:t:a:s:h") {
        opt_num ++;
        switch (opt) {
        case 'p':
            pattern = (unsigned char)strtoul(optarg, NULL, 16);
            break;

        case 'c':
            channel = atoi(optarg);
            if (channel < 0 || channel > 78) {
                printf("Invalid command option parameter!\n");
                printf("%s", nsrx_help);
                return;
            }
            break;

        case 't':
            type = (unsigned char)strtoul(optarg, NULL, 16);
            break;

        case 'a':
            if (0 != strcmp(optarg, "0")) {
                if (strlen(optarg) != 8) {
                    printf("Invalid command option parameter!\n");
                    printf("%s", nsrx_help);
                    return;
                }
                else {
                    if (check_hex_str(optarg)) {
                        printf("Invalid command option parameter!\n");
                        printf("%s", nsrx_help);
                        return;
                    }
                    else {
                        addr = (unsigned int)strtoul(optarg, NULL, 16);
                    }
                }
            } else {
                addr = 0x00A5F0C3;
            }
            break;

        case 's':
            second = atoi(optarg);
            if (second < 0) {
                printf("Invalid command option parameter!\n");
                printf("%s", nsrx_help);
                return;
            }
            break;

        case 'h':
        default:
            printf("%s", nsrx_help);
            return;
        }
    }

    if (opt_num < 4) {
        printf("Incomplete command options!\n");
        printf("%s", nsrx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        //printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    //printf("Rx pattern: 0x%02x\n", pattern);
    //printf("Rx channel: %d\n", channel);
    //printf("Packet type: 0x%02x\n", type);
    //printf("Tester address: %08x\n", addr);
    //printf("Rx time: %d second\n", second);

    /* Test start */
    if (HW_TEST_BT_NonSignalRx_start(
          pattern,
          channel,
          type,
          addr) == true) {
        //printf("Non-Signal-Rx test start...\n");
    }
    else {
        printf("Try to start Non-Signal-Rx test, failed\n");
        HW_TEST_BT_deinit();
        //printf("BT device power off\n");
        return;
    }

#if TX_RX_STOP_CMD
if (0 == access(NSRX_STOP_FILE, F_OK)) {
    system("rm -r "NSRX_STOP_FILE);
}
#endif

    if (0 == second) {
        /* Loop in waiting for user type "exit" */
        i = 0;
        do {
#if TX_RX_STOP_CMD
            sleep(2);
            if (0 == access(NSRX_STOP_FILE, F_OK)) {
                system("rm -r "NSRX_STOP_FILE);
                break;
            }
#else
            if (i >= 5)
                i = 0; /*rollback*/

            tmp = getchar();
            buf[i] = tmp;

            if (tmp != '\r' && tmp != '\n') {
                i ++;
            }
            else {
                buf[i] = '\0';
                if (0 != strcmp(buf, "exit"))
                    i = 0; /*discard this string*/
                else
                    break;
            }
#endif
        } while(1);

    } else {
        sleep(second);
    }

    /* Test end */
    if (HW_TEST_BT_NonSignalRx_end(
          &pkt_count,
          &PER,
          &byte_count,
          &BER) == true) {
        char *p_str = rx_str;
        int fd = 0;
        memset(rx_str, 0, sizeof(rx_str));

        //printf("Non-Signal-Rx test complete\n");
        p_str += sprintf(p_str, "Total received packet: %d\n", pkt_count);
        p_str += sprintf(p_str, "Packet Error Rate: %f%%\n", PER);
        p_str += sprintf(p_str, "Total received payload byte: %d\n", byte_count);
        p_str += sprintf(p_str, "Bit Error Rate: %f%%\n", BER);
        printf(rx_str);

        if (0 != access(AUTOBT_FILE_PATH, F_OK)) {
            system("mkdir "AUTOBT_FILE_PATH);
        }
        if (0 == access(NSRX_CNT_FILE, F_OK)) {
            system("rm -r "NSRX_CNT_FILE);
        }

        fd = open(NSRX_CNT_FILE, O_WRONLY | O_CREAT);
        if (fd < 0)
        {
            printf("open save file failed\n");
        }
        else
        {
            write(fd, rx_str, strlen(rx_str));
            close(fd);
        }
    }
    else {
        printf("Try to end Non-Signal-Rx test, failed\n");
    }

    /* BT power off */
    HW_TEST_BT_deinit();
    //printf("BT device power off\n");
    return;
}

static struct option testmode_options[] = {
    { "help",	0, 0, 'h' },
    { "power", 1, 0, 'p' },
    { 0, 0, 0, 0 }
};

static const char *testmode_help =
    "Usage:\n"
    "\ttestmode [--power] integer (range: 0~7)\n"
    "\t                   if set out of range or not set, use default value defined in nvram\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt testmode --power 6\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to enable BT device under test mode\n"
    "\tTo exit test mode, please type in \"exit\"\n"
    "\n";

static void cmd_test_mode(int argc, char **argv)
{
    int opt;
    int power = -1;
    /* To receive terminal input */
    char buf[5] = {0};
    char tmp;
    int i;

    for_each_opt(opt, testmode_options, "+p:h") {
        switch (opt) {
        case 'p':
            power = atoi(optarg);
#if 0
            if (power < 0 || power > 7) {
                printf("Invalid command option parameter!\n");
                printf("%s", testmode_help);
                return;
            }
#endif
            break;

        case 'h':
        default:
            printf("%s", testmode_help);
            return;
        }
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    /* Test start */
    if (HW_TEST_BT_TestMode_enter(power) == true) {
        printf("Test mode entered, you can start to test...\n");
    }
    else {
        printf("Enable BT device under test mode failed\n");
        HW_TEST_BT_deinit();
        printf("BT device power off\n");
        return;
    }

    /* Loop in waiting for user type "exit" */
    i = 0;
    do {
        if (i >= 5)
            i = 0; /*rollback*/

        tmp = getchar();
        buf[i] = tmp;

        if (tmp != '\r' && tmp != '\n') {
            i ++;
        }
        else {
            buf[i] = '\0';
            if (0 != strcmp(buf, "exit"))
                i = 0; /*discard this string*/
            else
                break;
        }
    } while(1);

    /* Test end */
    HW_TEST_BT_TestMode_exit();
    printf("Test mode exit\n");
    /* BT power off */
    HW_TEST_BT_deinit();
    printf("BT device power off\n");

    return;
}

static struct option inquiry_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

static const char *inquiry_help =
    "Usage:\n"
    "\tinquiry [no command option]\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt inquiry\n"
    "\n";

static void cmd_inquiry(int argc, char **argv)
{
    int opt;

    for_each_opt(opt, inquiry_options, "+h") {
        switch (opt) {
        case 'h':
        default:
            printf("%s", inquiry_help);
            return;
        }
    }

    /* BT power on & initialize */
    if (HW_TEST_BT_init()) {
        //printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    if (HW_TEST_BT_Inquiry(print_info) == true) {
        //printf("Inquiry remote devices...\n");
    }
    else {
        printf("Start inquiry procedure failed\n");
        HW_TEST_BT_deinit();
        //printf("BT device power off\n");
        return;
    }

    /* Loop in waiting for inquiry complete */
    g_inquiry_complete = false;
    do {
        sleep(7); /* Since the inquiry length is 6.4s set in driver */
    } while (!g_inquiry_complete);


    /* BT power off */
    HW_TEST_BT_deinit();
    //printf("BT device power off\n");

    return;
}

static struct option bletx_options[] = {
    { "help",	0, 0, 'h' },
    { "pattern", 1, 0, 'p' },
    { "channel", 1, 0, 'c' },
    { 0, 0, 0, 0 }
};

static const char *bletx_help =
    "Usage:\n"
    "\tbletx [--pattern] 0x00:\tTx pseudo random bit sequence 9\n"
    "\t                  0x01:\tTx 11110000 pattern\n"
    "\t                  0x02:\tTx 10101010 pattern\n"
    "\n"
    "\t      [--channel] Integer (0~39 channel for frequency range 2402MHz~2480MHz)\n"
    "\t                  channel = (frequency-2402)/2\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt bletx --pattern 0x00 --channel 20\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start LE Tx test, LE Controller will send out the specified Tx packet continuously\n"
    "\tTo end test, please type in \"exit\"\n"
    "\n";

static void cmd_ble_tx(int argc, char **argv)
{
    int opt, opt_num = 0;
    unsigned char pattern;
    int channel;
    /* To receive terminal input */
    char buf[5] = {0};
    char tmp;
    int i;

    for_each_opt(opt, bletx_options, "+p:c:h") {
        opt_num ++;
        switch (opt) {
        case 'p':
            pattern = (unsigned char)strtoul(optarg, NULL, 16);
            break;

        case 'c':
            channel = atoi(optarg);
            if (channel < 0 || channel > 39) {
                printf("Invalid command option parameter!\n");
                printf("%s", bletx_help);
                return;
            }
            break;

        case 'h':
        default:
            printf("%s", bletx_help);
            return;
        }
    }

    if (opt_num < 2) {
        printf("Incomplete command options!\n");
        printf("%s", bletx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        //printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    //printf("Tx pattern: 0x%02x\n", pattern);
    //printf("Tx channel: %d\n", channel);

    /* Test start */
    if (HW_TEST_BT_LE_Tx_start(pattern, channel) == true) {
        //printf("LE Tx test start...\n");
    }
    else{
        printf("Try to start LE Tx test, failed\n");
        HW_TEST_BT_deinit();
        //printf("BT device power off\n");
        return;
    }

#if TX_RX_STOP_CMD
if (0 == access(BLETX_STOP_FILE, F_OK)) {
    system("rm -r "BLETX_STOP_FILE);
}
#endif

    /* Loop in waiting for user type "exit" */
    i = 0;
    do {
#if TX_RX_STOP_CMD
        sleep(2);
        if (0 == access(BLETX_STOP_FILE, F_OK)) {
            system("rm -r "BLETX_STOP_FILE);
            break;
        }
#else
        if (i >= 5)
            i = 0; /*rollback*/

        tmp = getchar();
        buf[i] = tmp;

        if (tmp != '\r' && tmp != '\n') {
            i ++;
        }
        else {
            buf[i] = '\0';
            if (0 != strcmp(buf, "exit"))
                i = 0; /*discard this string*/
            else
                break;
        }
#endif
    } while(1);

    /* Test end */
    if (HW_TEST_BT_LE_Tx_end() == true) {
        //printf("LE Tx test complete\n");
    }
    else {
        printf("Try to end LE Tx test, failed\n");
    }

    /* BT power off */
    HW_TEST_BT_deinit();
    //printf("BT device power off\n");

    return;
}

#if 0 /* unused warning */
static struct option blerx_options[] = {
    { "help",	0, 0, 'h' },
    { "channel", 1, 0, 'c' },
    { 0, 0, 0, 0 }
};
#endif

static const char *blerx_help =
    "Usage:\n"
    "\tblerx [--channel] Integer (0~39 channel for frequency range 2402MHz~2480MHz)\n"
    "\t                  channel = (frequency-2402)/2\n"
    "\n"
    "\t     [--second] Integer (total Rx time by second, 0 is always Rx util input exit)\n"
    "\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt blerx --channel 20 --second 0\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start LE Rx test on a specified channel\n"
    "\tTo end test, please type in \"exit\", then the total received packet count during test are returned\n"
    "\n";

static void cmd_ble_rx(int argc, char **argv)
{
    int opt, opt_num = 0;
    int channel = 0;
    int second = 0;
    unsigned short pkt_count = 0;
    /* To receive terminal input */
    char buf[5] = {0};
    char tmp;
    int i;

    for_each_opt(opt, nsrx_options, "+c:s:h") {
        opt_num ++;
        switch (opt) {
        case 'c':
            channel = atoi(optarg);
            if (channel < 0 || channel > 39) {
                printf("Invalid command option parameter!\n");
                printf("%s", blerx_help);
                return;
            }
            break;

        case 's':
            second = atoi(optarg);
            if (second < 0) {
                printf("Invalid command option parameter!\n");
                printf("%s", blerx_help);
                return;
            }
            break;

        case 'h':
        default:
            printf("%s", blerx_help);
            return;
        }
    }

    if (opt_num < 1) {
        printf("Incomplete command options!\n");
        printf("%s", blerx_help);
        return;
    }

    /* BT power on & Initialize */
    if (HW_TEST_BT_init()) {
        //printf("BT device power on success\n");
    }
    else {
        printf("BT device power on failed\n");
        return;
    }

    //printf("Rx pattern: PRBS(pseudo random bit sequence)\n");
    //printf("Rx channel: %d\n", channel);
    //printf("Rx time: %d second\n", second);

    /* Test start */
    if (HW_TEST_BT_LE_Rx_start(channel) == true) {
        //printf("LE Rx test start...\n");
    }
    else {
        printf("Try to start LE Rx test, failed\n");
        HW_TEST_BT_deinit();
        //printf("BT device power off\n");
        return;
    }

#if TX_RX_STOP_CMD
if (0 == access(BLERX_STOP_FILE, F_OK)) {
    system("rm -r "BLERX_STOP_FILE);
}
#endif

    if (0 == second) {
        /* Loop in waiting for user type "exit" */
        i = 0;
        do {
#if TX_RX_STOP_CMD
            sleep(2);
            if (0 == access(BLERX_STOP_FILE, F_OK)) {
                system("rm -r "BLERX_STOP_FILE);
                break;
            }
#else
            if (i >= 5)
                i = 0; /*rollback*/

            tmp = getchar();
            buf[i] = tmp;

            if (tmp != '\r' && tmp != '\n') {
                i ++;
            }
            else {
                buf[i] = '\0';
                if (0 != strcmp(buf, "exit"))
                    i = 0; /*discard this string*/
                else
                    break;
            }
#endif
        } while(1);

    } else {
        sleep(second);
    }

    /* Test end */
    if (HW_TEST_BT_LE_Rx_end(&pkt_count) == true) {
        //printf("LE Rx test complete\n");
        printf("Total received packet: %d\n", pkt_count);
    }
    else {
        printf("Try to end LE Rx test, failed\n");
    }

    /* BT power off */
    HW_TEST_BT_deinit();
    //printf("BT device power off\n");
    return;
}

static struct option relayer_options[] = {
    { "help", 0, 0, 'h' },
    { "port", 1, 0, 'p' },
    { "speed", 1, 0, 's' },
    { 0, 0, 0, 0 }
};

static const char *relayer_help =
    "Usage:\n"
    "\trelayer [--port]  Interface to communicate with PC\n"
    "\t                  0:\tUART1\n"
    "\t                  1:\tUART2\n"
    "\t                  2:\tUART3\n"
    "\t                  3:\tUART4\n"
    "\t                  4:\tUSB\n"
    "\n"
    "\t        [--speed] Speed of UART serial port, if not set, use default value 115200\n"
    "\t                  Not required for USB\n"
    "\t                  9600\n"
    "\t                  19200\n"
    "\t                  38400\n"
    "\t                  57600\n"
    "\t                  115200\n"
    "\t                  230400\n"
    "\t                  460800\n"
    "\t                  500000\n"
    "\t                  576000\n"
    "\t                  921600\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt relayer --port 0 --speed 115200\n"
    "\t\tautobt relayer --port 4\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to start BT relayer function for Controller Certification\n"
    "\tBT relayer mode is a bridge for data transmission between PC simulation tool and Bluetooth Controller."
    " If it is configured to be running via UART serial port (port 0~3), H/W rework may be needed;"
    " if it is configured to be running via USB interface (port 4), be noticed to set \"sys.usb.config\""
    " property to \"acm_third\" first to open a VCOM.\n"
    "\tTo end test, please type in \"exit\"\n"
    "\n";

extern BOOL RELAYER_start(int serial_port, int serial_speed);
extern void RELAYER_exit(void);

static void cmd_relayer(int argc, char **argv)
{
    int opt;
    int port = -1, speed = 115200;
    /* To receive terminal input */
    char buf[5] = {0};
    char tmp;
    int i;

    for_each_opt(opt, relayer_options, "+p:s:h") {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;

        case 's':
            speed = atoi(optarg);
            break;

        case 'h':
        default:
            printf("%s", relayer_help);
            return;
        }
    }

    if (port < 0 || port > 4) {
        printf("Invalid command option parameter!\n");
        printf("%s", relayer_help);
        return;
    }

    printf("Serial port number: %d\n", port);
    printf("Serial port speed: %d\n", speed);

    /* Test start */
    if (RELAYER_start(port, speed) == true) {
        printf("BT relayer mode start, you can run test script from PC simulation tool now...\n");
    }
    else {
        printf("Start BT relayer mode failed\n");
        return;
    }

    /* Loop in waiting for user type "exit" */
    i = 0;
    do {
        if (i >= 5)
            i = 0; /*rollback*/

        tmp = getchar();
        buf[i] = tmp;

        if (tmp != '\r' && tmp != '\n') {
            i++;
        }
        else {
            buf[i] = '\0';
            if (0 != strcmp(buf, "exit"))
                i = 0; /*discard this string*/
            else
                break;
        }
    } while (1);

    /* Test end */
    RELAYER_exit();
    printf("BT relayer mode exit\n");

    return;
}

#if TX_RX_STOP_CMD
static void cmd_stop(int argc, char **argv)
{
    if (0 != access(AUTOBT_FILE_PATH, F_OK)) {
        system("mkdir "AUTOBT_FILE_PATH);
    }

    if (0 == memcmp("tx", argv[1], strlen("tx"))) {
        close(open(TX_STOP_FILE, O_CREAT, 0777));
        return;
    }

    if (0 == memcmp("nsrx", argv[1], strlen("nsrx"))) {
        close(open(NSRX_STOP_FILE, O_CREAT, 0777));
        return;
    }

    if (0 == memcmp("bletx", argv[1], strlen("bletx"))) {
        close(open(BLETX_STOP_FILE, O_CREAT, 0777));
        return;
    }

    if (0 == memcmp("blerx", argv[1], strlen("blerx"))) {
        close(open(BLERX_STOP_FILE, O_CREAT, 0777));
        return;
    }

    printf("Usage:\n"
    "\t     tx:\t stop tx test\n"
    "\t     nsrx:\t stop nsrx test\n"
    "\t     bletx:\t stop bletx test\n"
    "\t     blerx:\t stop blerx test\n");
    return;
}
#endif

static void cmd_get_nsrx(int argc, char **argv)
{
    int fd = 0;

    fd = open(NSRX_CNT_FILE, O_RDONLY);
    if (fd < 0)
    {
        printf("open save file failed\n");
        return;
    }
    memset(rx_str, 0, sizeof(rx_str));
    read(fd, rx_str, sizeof(rx_str));
    close(fd);

    printf(rx_str);
}

static struct option txpoweroffset_options[] = {
    { "help",   0, 0, 'h' },
    { "read",   0, 0, 'r' },
    { "write",  0, 0, 'w' },
    { "group0", 1, 0, '0' },
    { "group1", 1, 0, '1' },
    { "group2", 1, 0, '2' },
    { "group3", 1, 0, '3' },
    { "group4", 1, 0, '4' },
    { "group5", 1, 0, '5' },
    { 0, 0, 0, 0 }
};

static const char *txpoweroffset_help =
    "Usage:\n"
    "\ttxpoweroffset --read\n"
    "\ttxpoweroffset --write [--group0] integer [--group1] integer "
    "[--group2] integer [--group3] integer "
    "[--group4] integer [--group5] integer\n"
    "\t        integer: range 0~15"
    "\t                   if set out of range or not set, use default value defined in nvram\n"
    "\n\n"
    "\texample:\n"
    "\t\tautobt txpoweroffset --write --group0 6 --group3 11\n"
    "\n\n"
    "Notice:\n"
    "\tThis command is to set BT tx power offset before bt test\n"
    "\n";
#define TX_POWER_GROUP_NUM 6
static void cmd_tx_power_offset(int argc, char **argv)
{
    int opt, fd = 0, i = 0, wr_flag = 0;;
    char group_offset[TX_POWER_GROUP_NUM] = {0};
    char group_flag[TX_POWER_GROUP_NUM] = {0};

    memset(group_offset, 0, TX_POWER_GROUP_NUM *sizeof(char));
    memset(group_flag, 0, TX_POWER_GROUP_NUM *sizeof(char));

    for_each_opt(opt, txpoweroffset_options, "+r:w:0:1:2:3:4:5:h") {
        switch (opt) {
        case 'r':
            wr_flag = 1;
            break;
        case 'w':
            wr_flag = 2;
            break;
        case '0':
            group_offset[0] = atoi(optarg);
            group_flag[0] = 1;
            break;
        case '1':
            group_offset[1] = atoi(optarg);
            group_flag[1] = 1;
            break;
        case '2':
            group_offset[2] = atoi(optarg);
            group_flag[2] = 1;
            break;
        case '3':
            group_offset[3] = atoi(optarg);
            group_flag[3] = 1;
            break;
        case '4':
            group_offset[4] = atoi(optarg);
            group_flag[4] = 1;
            break;
        case '5':
            group_offset[5] = atoi(optarg);
            group_flag[5] = 1;
            break;

        case 'h':
        default:
            printf("%s", txpoweroffset_help);
            return;
        }
    }

    if (2 == wr_flag)
    {
        for (i = 0; i < TX_POWER_GROUP_NUM; i++) {
            if (1 == group_flag[i]) {
                if ((group_offset[i] > 15) || (group_offset[i] < 0)) {
                    printf("Error: Group %d is out of range", i);
                    return;
                }
            }
        }
    }

    if ((1 != wr_flag) && (2 != wr_flag))
    {
        printf("%s", txpoweroffset_help);
        return;
    }
    HW_TEST_TX_POWER_OFFSET_op(wr_flag, group_offset, group_flag);

    if (1 == wr_flag)
    {
        printf("TX Power Offset:\n");
        for (i = 0; i < TX_POWER_GROUP_NUM; i++) {
            printf(" Group %d is %d\n", i, group_offset[i]);
        }
    }
    return;
}

static struct {
    char *cmd;
    void (*func)(int argc, char **argv);
    char *doc;
} command[] = {
    { "tx",             cmd_tx_test,            "Tx only test"                      },
    { "nsrx",           cmd_non_signal_rx,      "Non-Signal-Rx test"                },
    { "testmode",       cmd_test_mode,          "Enable BT device under test mode"  },
    { "inquiry",        cmd_inquiry,            "Inquiry remote devices"            },
    { "bletx",          cmd_ble_tx,             "LE Tx test"                        },
    { "blerx",          cmd_ble_rx,             "LE Rx test"                        },
    { "relayer",        cmd_relayer,            "BT relayer mode"                   },
#if TX_RX_STOP_CMD
    { "stop",           cmd_stop,               "BT stop test"                      },
#endif
    { "getrx",          cmd_get_nsrx,           "Get Non-Signal-Rx counter"         },
    { "txpoweroffset",  cmd_tx_power_offset,    "Set TX power offset"               },
    { NULL, NULL, 0 }
};

static void usage(void)
{
    int i;

    printf("autobt test tool - ver %s\n", VERSION);
    printf("Usage:\n"
        "\tautobt <command> [command options] [command parameters]\n");
    printf("Commands:\n");
    for (i = 0; command[i].cmd; i++) {
        printf("\t%-8s\t%s\n", command[i].cmd, command[i].doc);
    }
    printf("\n"
        "For more information on the usage of each command use:\n"
        "\tautobt <command> --help\n\n\n");
}

static struct option main_options[] = {
    { "help",	0, 0, 'h' },
    { 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
    int opt, i;

    while ((opt=getopt_long(argc, argv, "+h", main_options, NULL)) != -1) {
        switch (opt) {
        case 'h':
        default:
            usage();
            exit(0);
        }
    }

    argc -= optind;
    argv += optind;
    optind = 0;

    if (argc < 1) {
        usage();
        exit(0);
    }

    for (i = 0; command[i].cmd; i++) {
        if (strcmp(command[i].cmd, argv[0]))
           continue;
        command[i].func(argc, argv);
        break;
    }

    return 0;
}

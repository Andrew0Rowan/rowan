/**
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <dirent.h>

#include "common.h"

//---------------------------------------------------------------------------
#define VERSION     "6.0.17070301"
#define LOG_VERSION 0x100
#define BT_DEV      "/sys/kernel/debug/mtkbt/bt_dev"
#define FWLOG_DEV   "/dev/stpbtfwlog"


//---------------------------------------------------------------------------
static const uint64_t BTSNOOP_EPOCH_DELTA = 0x00dcddb30f2f8000ULL;
static uint64_t timestamp = 0;
static uint8_t buffer[2048] = {0};
static uint8_t cont = 1;    /** loop continue running */
static int file_size_remain_to_switch = 0;

//---------------------------------------------------------------------------
uint64_t btsnoop_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // Timestamp is in microseconds.
    timestamp = tv.tv_sec * 1000000ULL;
    timestamp += tv.tv_usec;
    timestamp += BTSNOOP_EPOCH_DELTA;
    return timestamp;
}

//---------------------------------------------------------------------------
void fillheader(unsigned char *header, int headerlen,
        unsigned short int dump_file_seq_num)
{
    int copy_hedare_len = 0;
    unsigned int logversion = htobe32(LOG_VERSION);
    memset(header, 0, headerlen);
    memcpy(header, &logversion, sizeof(logversion));
    copy_hedare_len += 4;   /** 4 byte for logversion */
    copy_hedare_len += 4;   /** 4 byte for chip id, not implement yet */
    dump_file_seq_num = htobe16(dump_file_seq_num);
    memcpy(header + copy_hedare_len, &dump_file_seq_num, sizeof(dump_file_seq_num));
    copy_hedare_len += 2;   /** 2 byte for sequence number */
    copy_hedare_len += 6;   /** first hci log length(2), zero(4) */

    btsnoop_timestamp();
    timestamp = htobe64(timestamp);
    memcpy(header + copy_hedare_len, &timestamp, sizeof(timestamp));
}

//---------------------------------------------------------------------------
static void picus_sig_handler(int signum)
{
    DBGPRINT(MT_DEBUG_SHOW, "%s: %d\n", __func__, signum);
    system("echo 01 be fc 01 00 > "FWLOG_DEV);      // disable picus log
    cont = 0;                                       // stop loop
}

//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    FILE *fscript = 0;
    FILE *fw_dump_fscript = 0;
    int nRead = 0;
    int fd = 0;
    int ret = 0;
    int opt;
    int dump_name_index = 0;
    int writetofilelength = 0;
    char dump_file_name[64] = {0};
    int fw_dump_writetofilelength = 0;
    char fw_dump_file_name[64] = {0};
    int retry_file_open = 0;
    int file_number = 6;
    int file_size = FW_LOG_SWITCH_SIZE;     /** default file size is 20 MB */
    char *log_path = DEFAULT_PATH;
    char command[200] = {0};
    unsigned char padding[8] = {0};
    unsigned char header[24] = {0};
    int fw_log_len = 0;
    unsigned short int dump_file_seq_num = 0;
    char timestamp_buffer[24];
    fd_set rset;                            /** For select */
    struct timeval tv;
    time_t local_timestamp;
    uint8_t logmore = 0;                    /** Default log level */
    struct sigaction sigact;
    struct flock fl;

    DBGPRINT(MT_DEBUG_SHOW, "%s Version: %s", argv[0], VERSION);
    sigact.sa_handler = picus_sig_handler;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigaction(SIGKILL, &sigact, NULL);
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_pid = getpid();
    fl.l_start = 0;
    fl.l_len = 0;

    while ((opt = getopt(argc, argv, "d:c:p:n:s:f")) != -1) {
        struct stat sb;
        switch (opt) {
        /* debug */
        case 'd':
            if (strcmp(optarg, "kill") == 0) {
                system("echo 01 be fc 01 00 > "FWLOG_DEV);     // disable picus log firstly
                system("killall picus");
                DBGPRINT(MT_DEBUG_SHOW, "Kill all picus process.\n");
                goto done;
            } else if (strcmp(optarg, "trigger") == 0) {
                system("echo 01 be fc 01 00 > "FWLOG_DEV);     // disable picus log firstly
                DBGPRINT(MT_DEBUG_SHOW, "Manual Trigger FW Assert.\n");
                system("echo 01 6f fc 05 01 02 01 00 08 > "FWLOG_DEV);
                return 0;
            }
            break;
        /* send command */
        case 'c':
            snprintf(command, sizeof(command), "echo %s > "FWLOG_DEV, optarg);
            DBGPRINT(MT_DEBUG_SHOW, "Send command = %s from users.\n",command);
            system(command);
            return 0;
        /* change path */
        case 'p':
            if (stat(optarg, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                log_path = optarg;
                DBGPRINT(MT_DEBUG_SHOW, "Log path is %s\n", log_path);
            } else {
                DBGPRINT(MT_DEBUG_SHOW, "Directory is invalid");
                goto done;
            }
            break;
        /* change file number*/
        case 'n':
            file_number = atoi(optarg);
            DBGPRINT(MT_DEBUG_SHOW, "Change the number of file to %d.\n", file_number);
            break;
        /* change file size*/
        case 's':
            file_size = atoi(optarg);
            DBGPRINT(MT_DEBUG_SHOW, "Change the size of file to %d.\n", file_size);
            break;
        /* full log */
        case 'f':
            logmore = 1;
            break;
        /* command Usage */
        case '?':
        default:
            DBGPRINT(MT_DEBUG_SHOW, "Usage: picus [option] [path | command]");
            DBGPRINT(MT_DEBUG_SHOW, "[option]");
            DBGPRINT(MT_DEBUG_SHOW, "\t-d\t\tUsing \"kill\" command to kill all picus");
            DBGPRINT(MT_DEBUG_SHOW, "\t  \t\tUsing \"trigger\" command to trigger fw assert");
            DBGPRINT(MT_DEBUG_SHOW, "\t-c [command]\tsend command");
            DBGPRINT(MT_DEBUG_SHOW, "\t-p [path]\tOutput the file to specific dictionary");
            DBGPRINT(MT_DEBUG_SHOW, "\t-n [NO]\t\tChange the output file number");
            DBGPRINT(MT_DEBUG_SHOW, "\t-s [bytes]\tChange the output file size");
            DBGPRINT(MT_DEBUG_SHOW, "\t-f\t\tLog level: More");
            goto done;
            break;
        }
    }
    /* stpbtfwlog */
    fd = open(CUST_BT_FWLOG_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd <= 0) {
        DBGPRINT(MT_DEBUG_ERROR, "Can't open device node %s, fd: %d", CUST_BT_FWLOG_PORT, fd);
        goto done;
    } else {
        DBGPRINT(MT_DEBUG_SHOW, "Open device node successfully");
    }

    /* flock the device node */
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        DBGPRINT(MT_DEBUG_SHOW, "lock device node failed, picus already running.");
        goto done;
    }

    /* log level */
    if (logmore) {
        ret = system("echo 01 5f fc 2e 50 01 0A 00 00 00 01 00 00 C0 00 00 00 00 00 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00 01 01 01 00 01 00 00 00 01 00 00 00 00 00 00 00 > "FWLOG_DEV);    // Log More
        ;
    } else {
        ret = system("echo 01 5f fc 2e 50 01 0A 00 00 00 00 00 00 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 > "FWLOG_DEV);    // Default Level
        ;
    }
    if (ret == -1)
        DBGPRINT(MT_DEBUG_ERROR, "Set log level fail");

    /* enable it firstly */
    ret = system("echo 01 be fc 01 05 > "FWLOG_DEV);
    if (ret == -1)
        DBGPRINT(MT_DEBUG_ERROR, "Enable fail");
    DBGPRINT(MT_DEBUG_SHOW, "Log %slevel set and enabled", logmore ? "more " : "");
    usleep(10000);

    /* check already exist file under log_path */
    char temp_picus_zero_filename[36] = {0};
    DIR *p_dir = opendir(log_path);
    if (p_dir != NULL) {
        struct dirent *p_file;
        while ((p_file = readdir(p_dir)) != NULL) {
            /* ignore . and .. directory */
            if (strncmp(p_file->d_name, "..", 2) == 0
                || strncmp(p_file->d_name, ".", 1) == 0) {
                continue;
            }
            char *temp;
            if ((temp = strstr(p_file->d_name, DUMP_PICUS_NAME_EXT)) != NULL) {
                /* check filename already exist on current directory */
                dump_name_index++;
            }
            if ((temp = strstr(p_file->d_name, "0.picus")) != NULL) {
                snprintf(temp_picus_zero_filename, sizeof(temp_picus_zero_filename), "%s", p_file->d_name);
            }
        }
        closedir(p_dir);
    }
    /* remove already exist 0.picus file */
    if (remove(temp_picus_zero_filename)) {
        DBGPRINT(MT_DEBUG_SHOW, "%s can't remove", temp_picus_zero_filename);
    } else {
        DBGPRINT(MT_DEBUG_SHOW, "%s remove", temp_picus_zero_filename);
    }
    /* get current timestamp */
    time(&local_timestamp);
    strftime(timestamp_buffer, 24, "%Y%m%d%H%M%S", localtime(&local_timestamp));
    snprintf(dump_file_name, sizeof(dump_file_name), "%s/" DUMP_PICUS_NAME_PREFIX "%s_%d" DUMP_PICUS_NAME_EXT, log_path, timestamp_buffer, dump_name_index);

    if (dump_name_index == file_number) {
        dump_name_index = 0;
        snprintf(dump_file_name, sizeof(dump_file_name), "%s/" DUMP_PICUS_NAME_PREFIX "%s_%d" DUMP_PICUS_NAME_EXT, log_path, timestamp_buffer, dump_name_index);
    }
    /* combine file path and file name */
    snprintf(fw_dump_file_name, sizeof(fw_dump_file_name), "%s/" FW_DUMP_PICUS_NAME, log_path);

    /* dump file for picus log */
    if ((fscript = fopen(dump_file_name, "wb")) == NULL) {
        DBGPRINT(MT_DEBUG_ERROR, "Open script file %s fail [%s] errno %d", dump_file_name, strerror(errno), errno);
        goto done;
    } else {
        DBGPRINT(MT_DEBUG_SHOW, "%s created, dumping...", dump_file_name);
    }

    fillheader(header, sizeof(header), dump_file_seq_num);
    dump_file_seq_num++;
    fwrite(header, 1, sizeof(header), fscript);
    fwrite(padding, 1, sizeof(padding), fscript);

    ret = 0;
    retry_file_open = 0;
    file_size_remain_to_switch = file_size;

    do {
        FD_ZERO(&rset);
        FD_SET(fd,&rset);
        tv.tv_sec = 10;
        tv.tv_usec = 0;   /* timeout is 10s for select method */
        if (select(fd + 1, &rset, NULL, NULL, &tv) == 0) {
            DBGPRINT(MT_DEBUG_ERROR, "Read data timeout from stpbtfwlog, timeout is 10s");
            continue;
        }
        if (FD_ISSET(fd, &rset))
            nRead = read(fd, buffer, sizeof(buffer));
        else
            continue;

        if (nRead >= 3) {
            if ((nRead == 2 + buffer[1]) && buffer[0] == 0xFF && buffer[2] == 0x50) {
                writetofilelength = nRead - 3;
                fw_log_len = nRead - 3;
                fwrite(&buffer[3], 1, writetofilelength, fscript);
                file_size_remain_to_switch -= writetofilelength;

                if (writetofilelength % 8) {
                    fwrite(padding, 1, 8 - (fw_log_len % 8), fscript);
                    file_size_remain_to_switch -= (8 - (fw_log_len % 8));
                }

                /* switch file name if file size is over file_size */
                if (file_size_remain_to_switch <= 0) {
                    file_size_remain_to_switch = file_size;
                    fclose(fscript);
                    if (file_number - 1 > dump_name_index) {
                        dump_name_index++;
                    } else {
                        dump_name_index = 0;
                    }
                    /* remove the file before creating */
                    DIR *p_dir = opendir(log_path);
                    if (p_dir != NULL) {
                        struct dirent *p_file;
                        while ((p_file = readdir(p_dir)) != NULL) {
                            if (strncmp(p_file->d_name, "..", 2) == 0
                                || strncmp(p_file->d_name, ".", 1) == 0) {
                                continue;
                            }
                            char *temp;
                            char temp_picus_filename[24] = {0};
                            snprintf(temp_picus_filename, sizeof(temp_picus_filename), "%d.picus", dump_name_index);
                            if ((temp = strstr(p_file->d_name, temp_picus_filename)) != NULL) {
                                if (remove(p_file->d_name)) {
                                    DBGPRINT(MT_DEBUG_SHOW, "%s can't remove", p_file->d_name);
                                } else {
                                    DBGPRINT(MT_DEBUG_SHOW, "%s remove", p_file->d_name);
                                }
                            }
                        }
                        closedir(p_dir);
                    }
                    time(&local_timestamp);
                    strftime(timestamp_buffer, 24, "%Y%m%d%H%M%S", localtime(&local_timestamp));
                    snprintf(dump_file_name, sizeof(dump_file_name), "%s/" DUMP_PICUS_NAME_PREFIX "%s_%d" DUMP_PICUS_NAME_EXT, log_path, timestamp_buffer, dump_name_index);

                    while(1) {
                        if ((fscript = fopen(dump_file_name, "wb")) == NULL) {
                            DBGPRINT(MT_DEBUG_ERROR, "Open script file %s fail [%s] errno %d",
                                    dump_file_name, strerror(errno), errno);
                            if (retry_file_open >= RETRY_COUNT)
                                goto done;
                        } else {
                            DBGPRINT(MT_DEBUG_SHOW, "%s created, dumping...", dump_file_name);
                            retry_file_open = 0;
                            break;
                        }
                        ++retry_file_open;
                    }

                    fillheader(header, sizeof(header), dump_file_seq_num);
                    dump_file_seq_num++;
                    fwrite(header, 1, sizeof(header), fscript);
                }
                fflush(fscript);
            }
            else if (buffer[0] == 0x6F && buffer[1] == 0xFC) {
                /* dump file for fw dump */
                if (fw_dump_fscript == NULL) {
                    while(1) {
                        if ((fw_dump_fscript = fopen(fw_dump_file_name, "wb")) == NULL) {
                            DBGPRINT(MT_DEBUG_ERROR, "Open script file %s fail [%s] errno %d", fw_dump_file_name,
                                    strerror(errno), errno);
                            if (retry_file_open >= RETRY_COUNT)
                                goto done;
                        } else {
                            DBGPRINT(MT_DEBUG_SHOW, "%s created, dumping...", fw_dump_file_name);
                            retry_file_open = 0;
                            break;
                        }
                        ++retry_file_open;
                    }
                }
                fw_dump_writetofilelength = nRead - 4;
                if (buffer[nRead - 6] == ' ' &&
                    buffer[nRead - 5] == 'e' &&
                    buffer[nRead - 4] == 'n' &&
                    buffer[nRead - 3] == 'd') {
                    DBGPRINT(MT_DEBUG_SHOW, "FW dump end");
                }
                fwrite(&buffer[4], 1, fw_dump_writetofilelength, fw_dump_fscript);
                fflush(fw_dump_fscript);
            }
            else {
                DBGPRINT(MT_DEBUG_ERROR, "read = %d, Packet header is not fw log %02X %02X %02X %02X %02X %02X",
                        nRead, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            }
            ret = 0;
        } else {
            ++ret;
        }
    } while (cont);

done:
    if (fd) close(fd);
    if (fscript) {
        DBGPRINT(MT_DEBUG_SHOW, "release %s", dump_file_name);
        fclose(fscript);
    }
    if (fw_dump_fscript) {
        DBGPRINT(MT_DEBUG_SHOW, "release %s", fw_dump_file_name);
        fclose(fw_dump_fscript);
    }
    /* unlock the device node */
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fcntl(fd, F_SETLKW, &fl);

    return 0;
}

//---------------------------------------------------------------------------

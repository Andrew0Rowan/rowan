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


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "typedefs.h"
#include "gpt.h"
#include "crc.h"

struct part_attr nor_part[128];
struct part_attr emmc_part[128];
int nor_part_cnt = 0;
int emmc_part_cnt = 0;

bool search_nor_part_from_table(const char *name, struct part_attr *part)
{
    int name_size = strlen(name);
    int i;
    int j;
    for (i = 0; i < nor_part_cnt; i++)
        if (strncmp((nor_part[i].name), name, name_size) == 0) {
            *part = nor_part[i];
            return TRUE;
        }

    return FALSE;
}


bool search_emmc_part_from_table(const char *name, struct part_attr *part)
{
    int name_size = strlen(name);
    int i;
    int j;
    for (i = 0; i < emmc_part_cnt; i++)
        if (strncmp((emmc_part[i].name), name, name_size) == 0) {
            *part = emmc_part[i];
            return TRUE;
        }

    return FALSE;
}


int validate_mbr_partition(const struct mbr_part *part)
{
    /* invalid types and invalid status */
    if ((part->type == 0) || (part->status != 0x80 && part->status != 0x00))
        return -1;

    return 0;
}

bool is_valid_gpt(int fd)
{
    /* everytime assume file off is at 0 */
    lseek(fd, 0, SEEK_SET);

    unsigned char *mbr_buffer;
    mbr_buffer = (unsigned char *)malloc((G_BLOCK_SIZE) * sizeof(unsigned char));
    if (!mbr_buffer)
        goto error_exit;

    int read_num = read(fd, mbr_buffer, G_BLOCK_SIZE);
    if ((read_num > G_BLOCK_SIZE) || (read_num < 0))
        goto error_exit;

    /* sniff for MBR partition types */
    unsigned int i;
    int gpt_partitions_exist = 0;

    /* look for the aa55 tag */
    if (mbr_buffer[510] != 0x55 || mbr_buffer[511] != 0xaa)
        goto error_exit;

    /* see if a partition table makes sense here */
    struct mbr_part part[4];
    memcpy(part, mbr_buffer + 446, sizeof(part));

    /* check each entry to find GPT exist or not */
    for (i=0; i < 4; i++) {
        if (validate_mbr_partition(&part[i]) >= 0) {
            /* Type 0xEE indicates end of MBR and GPT partitions exist */
            if(part[i].type==0xee) {
                gpt_partitions_exist = 1;
                break;
            }
        }
    }

    if (!gpt_partitions_exist)
        goto error_exit;

    /* always make off at 0 after exit */
    lseek(fd, 0, SEEK_SET);
    return TRUE;

error_exit:
    lseek(fd, 0, SEEK_SET);
    return FALSE;
}

/*
 * Parse the gpt header and get the required header fields
 * Return 0 on valid signature
 */
int partition_parse_gpt_header(unsigned char *buffer, struct gpt_header* header)
{
    /* Check GPT Signature */
    if (((unsigned int *) buffer)[0] != GPT_SIGNATURE_2 ||
        ((unsigned int *) buffer)[1] != GPT_SIGNATURE_1) {
        goto error_exit;
    }

    header->header_size = GET_LWORD_FROM_BYTE(&buffer[HEADER_SIZE_OFFSET]);
    header->backup_header_lba = GET_LLWORD_FROM_BYTE(&buffer[BACKUP_HEADER_OFFSET]);
    header->first_usable_lba = GET_LLWORD_FROM_BYTE(&buffer[FIRST_USABLE_LBA_OFFSET]);
    header->max_partition_count = GET_LWORD_FROM_BYTE(&buffer[PARTITION_COUNT_OFFSET]);
    header->partition_entry_size = GET_LWORD_FROM_BYTE(&buffer[PENTRY_SIZE_OFFSET]);

return 0;

error_exit:
    return -1;
}

int part_table_add(struct part_attr part)
{
    switch (part.type) {
    case 0:
        emmc_part_cnt++;
        emmc_part[emmc_part_cnt - 1] = part;
        break;
    case 1:
        nor_part_cnt++;
        nor_part[nor_part_cnt - 1] = part;
        break;

    default:
        goto error_exit;
    }

    return 0; 

error_exit:
    return -1;
}

int clear_part_table(int dev_type)
{
    int i;
    int j;
    switch (dev_type) {
    case 0:
        for (i = 0; i < emmc_part_cnt; i++) {
            for (j = 0; j < MAX_GPT_NAME_SIZE; j++)
                emmc_part[i].name[j] = '0';

            emmc_part[i].type = -1;
            emmc_part[i].start_lba = 0;
            emmc_part[i].end_lba = 0;
        };
        emmc_part_cnt = 0;
        break;
    case 1:
        for (i = 0; i < nor_part_cnt; i++) {
            for (j = 0; j < MAX_GPT_NAME_SIZE; j++)
                nor_part[i].name[j] = '0';

            nor_part[i].type = -1;
            nor_part[i].start_lba = 0;
            nor_part[i].end_lba = 0;
        };
        nor_part_cnt = 0;
        break;
    default:
        return -1;
    }

    return 0;
}
int get_mbr_info()
{
   int fd = -1;
   fd = open("/dev/mmcblk0", O_RDWR);
   if (fd < 0){
		printf_e("open /dev/mmcblk0 fail\n");
        return -1;
	}
   lseek64(fd, 0, SEEK_SET);
   parse_part_info(fd, 0);
   return 0;
}
int parse_part_info(int fd, int dev_type)
{
    int err = 0;
    int count = 0;

    // clear any partitions that may have already existed
    clear_part_table(dev_type);

    /* everytime assume form begining */
    lseek(fd, 0, SEEK_SET);

    unsigned char *mbr_buffer;
    mbr_buffer = (unsigned char *)malloc((G_BLOCK_SIZE) * sizeof(unsigned char));
    if (!mbr_buffer)
        goto error_exit;

    /* sniff for MBR partition types */
    do {
        unsigned int i, j, n;
        int gpt_partitions_exist = 0;

        err = read(fd, mbr_buffer, 512);
        if (err < 0)
            goto error_exit;

        /* look for the aa55 tag */
        if (mbr_buffer[510] != 0x55 || mbr_buffer[511] != 0xaa)
            break;

        /* see if a partition table makes sense here */
        struct mbr_part part[4];
        memcpy(part, mbr_buffer + 446, sizeof(part));

        /* validate each of the partition entries */
        for (i=0; i < 4; i++) {
            if (validate_mbr_partition(&part[i]) >= 0) {
                /* Type 0xEE indicates end of MBR and GPT partitions exist */
                if(part[i].type == 0xee) {
                    gpt_partitions_exist = 1;
                    break;
                }
                count++;
            }
        }

        if (!gpt_partitions_exist)
            break;

        lseek(fd, 512, SEEK_SET);
        err = read(fd, mbr_buffer, 512);
        if (err < 0)
            goto error_exit;

        struct gpt_header gpthdr;
        err = partition_parse_gpt_header(mbr_buffer, &gpthdr);

        unsigned int part_entry_cnt = 4;
        unsigned long long partition_0 = GET_LLWORD_FROM_BYTE(&mbr_buffer[PARTITION_ENTRIES_OFFSET]);
        /* Read GPT Entries */
        for (i = 0; i < (ROUNDUP(gpthdr.max_partition_count, part_entry_cnt)) / part_entry_cnt; i++) {
            lseek(fd, (partition_0 * 512) + (i * 512), SEEK_SET);
            err = read(fd, mbr_buffer, 512);

            if (err < 0)
                break;

            for (j = 0; j < part_entry_cnt; j++) {
                unsigned char type_guid[PARTITION_TYPE_GUID_SIZE];
                unsigned char name[MAX_GPT_NAME_SIZE];
                unsigned char UTF16_name[MAX_GPT_NAME_SIZE];
                unsigned long long first_lba, last_lba, size;

                /* guid */
                memcpy(&type_guid,
                   &mbr_buffer[(j * gpthdr.partition_entry_size)],
                   PARTITION_TYPE_GUID_SIZE);
                if (type_guid[0]==0 && type_guid[1]==0) {
                    i = ROUNDUP(gpthdr.max_partition_count, part_entry_cnt);
                    break;
                }

                /* size */
                first_lba = GET_LLWORD_FROM_BYTE(&mbr_buffer[(j * gpthdr.partition_entry_size) + FIRST_LBA_OFFSET]);
                last_lba = GET_LLWORD_FROM_BYTE(&mbr_buffer[(j * gpthdr.partition_entry_size) + LAST_LBA_OFFSET]);
                size = last_lba - first_lba + 1;

                /* name */
                memset(&UTF16_name, 0x00, MAX_GPT_NAME_SIZE);
                memcpy(UTF16_name, &mbr_buffer[(j * gpthdr.partition_entry_size) +
                       PARTITION_NAME_OFFSET], MAX_GPT_NAME_SIZE);

                for (n = 0; n < MAX_GPT_NAME_SIZE / 2; n++)
                    name[n] = UTF16_name[n * 2];

                struct part_attr par;
                for (n = 0; n < MAX_GPT_NAME_SIZE / 2; n++)
                    par.name[n] = name[n];

                par.type = dev_type;
                par.start_lba = first_lba;
                par.end_lba = last_lba;
                part_table_add(par);
                count++;
            }
        }
    } while (0);

    lseek(fd, 0, SEEK_SET);
    return 0;

error_exit:
    return -1;
}

/* adjust gpt */
void patch_gpt(unsigned char *gptImage, unsigned long long density, unsigned int array_size,
          unsigned int max_part_count, unsigned int part_entry_size, unsigned int block_size)
{
    unsigned char *partition_entry_array_start;
    unsigned char *primary_gpt_header;
    unsigned char *secondary_gpt_header;
    unsigned long long card_size_sec;
    int total_part = 0, phy_last_part = 0;
    unsigned long last_part_offset;
    unsigned int crc_value;
    unsigned long long last_part_first_lba, last_part_last_lba;

    /* get size of storage */
    card_size_sec = (density) / block_size;

    /* generate second gpt header */
    memcpy(gptImage + (block_size * 2) + array_size,
           gptImage + block_size,
           block_size);

    /* patching primary header */
    primary_gpt_header = (gptImage + block_size);
    PUT_LONG_LONG(primary_gpt_header + BACKUP_HEADER_OFFSET,
              ((long long)(card_size_sec - 1)));
    PUT_LONG_LONG(primary_gpt_header + LAST_USABLE_LBA_OFFSET,
              ((long long)(card_size_sec - 34)));

    /* patching backup GPT */
    secondary_gpt_header = primary_gpt_header + block_size + array_size;
    PUT_LONG_LONG(secondary_gpt_header + PRIMARY_HEADER_OFFSET,
                    ((long long)(card_size_sec - 1)));
    PUT_LONG_LONG(secondary_gpt_header + LAST_USABLE_LBA_OFFSET,
                    ((long long)(card_size_sec - 34)));
    PUT_LONG_LONG(secondary_gpt_header + PARTITION_ENTRIES_OFFSET,
                    ((long long)(card_size_sec - 33)));
    PUT_LONG_LONG(secondary_gpt_header + BACKUP_HEADER_OFFSET,
                    ((long long)(1)));

    /* find last partition */
    while (*(primary_gpt_header + block_size + total_part * ENTRY_SIZE) != 0) {
        if (GET_LLWORD_FROM_BYTE(primary_gpt_header + block_size + total_part * ENTRY_SIZE + FIRST_LBA_OFFSET) >=
            GET_LLWORD_FROM_BYTE(primary_gpt_header + block_size + phy_last_part * ENTRY_SIZE + FIRST_LBA_OFFSET)) {
            phy_last_part = total_part;
        }
        total_part++;
    }

    /* patching last partition */
    last_part_offset = (unsigned long)(primary_gpt_header + block_size + phy_last_part * ENTRY_SIZE);
    /* last partition size should align 64KB */
    last_part_first_lba =  GET_LLWORD_FROM_BYTE(last_part_offset + PARTITION_ENTRY_FIRST_LBA);
    last_part_last_lba = (card_size_sec - 34) - (((card_size_sec - 34) - last_part_first_lba + 1) % 128);
    PUT_LONG_LONG(last_part_offset + PARTITION_ENTRY_LAST_LBA, (long long)last_part_last_lba);

    /* updating CRC of the partition entry array in both headers */
    partition_entry_array_start = primary_gpt_header + block_size;
    crc_value = (unsigned int)crc32(0x0, partition_entry_array_start,
                    max_part_count * part_entry_size);
    PUT_LONG(primary_gpt_header + PARTITION_CRC_OFFSET, crc_value);
    PUT_LONG(secondary_gpt_header + PARTITION_CRC_OFFSET, crc_value);

    /* clearing CRC fields to calculate */
    PUT_LONG(primary_gpt_header + HEADER_CRC_OFFSET, 0);
    crc_value = (unsigned int)crc32(0x0, primary_gpt_header, 92);
    PUT_LONG(primary_gpt_header + HEADER_CRC_OFFSET, crc_value);
    PUT_LONG(secondary_gpt_header + HEADER_CRC_OFFSET, 0);

    crc_value = (unsigned int)crc32(0x0, secondary_gpt_header, 92);
    PUT_LONG(secondary_gpt_header + HEADER_CRC_OFFSET, crc_value);
}

/* write gpt after adjust to partition */
int write_gpt(int fd, void *buf, unsigned long long dev_size)
{
    lseek(fd, 0, SEEK_SET);
    /* write primary */
    int ret;
    if (dev_size < 0)
        goto error_exit;

    ret = write(fd, buf, ((G_BLOCK_SIZE * 2) + MIN_PARTITION_ARRAY_SIZE));
    if (ret < 0)
        goto error_exit;

    /* write secondary */
    lseek(fd, (dev_size - (MIN_PARTITION_ARRAY_SIZE + G_BLOCK_SIZE)), SEEK_SET);
    ret = write(fd, (buf + (G_BLOCK_SIZE * 2)), (MIN_PARTITION_ARRAY_SIZE + G_BLOCK_SIZE));
    if (ret < 0)
        goto error_exit;

    return 0;

error_exit:
    return -1;
}


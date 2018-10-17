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


#define __USE_FILE_OFFSET64
#define __USE_LARGEFILE64
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include "gpt.h"
#include "file_ops.h"
#include "typedefs.h"
#include "emmc_ops.h"

/* erase, option job for emmc */
int erase_emmc(int fd, unsigned long long start, unsigned long long end)
{
    if ((start < 0) || (end < 0) || (end < start))
        return -1;

    unsigned char zero_buf[1] = {0};
    int i;
    int off = start;
	
	printf_d("start %s \n", __FUNCTION__);
	
    for (i = start; i < end; i++) {
        lseek64(fd, start, SEEK_SET);
        write(fd, zero_buf, 1);
    }
    lseek64(fd, 0, SEEK_SET);
    return 0;
}


int upgrade_emmc_mmc0boot0_partition(const unsigned char *name, const unsigned char *img_path)
{
	int fd = -1;
	int fdr = -1;
    unsigned char *img_buffer = NULL;
    unsigned char *dev_buffer = NULL;
	int img_size = file_size(img_path);
	unsigned int name_size = strlen(name);
	bool need_upgrade = FALSE;


	printf_d("start %s \n", __FUNCTION__);
	
	if (name_size > MAX_GPT_NAME_SIZE){
	   printf_e("partition name overflow %d\n",name_size);
	   goto error_exit;
	}
	

	fdr = open(img_path, O_RDONLY|O_LARGEFILE);
    if (fdr < 0){
		printf_e("open %s fail\n",img_path);
        return -1;
	}
	printf_d("open %s ok\n",img_path);

    fd = open("/dev/mmcblk0boot0", O_RDWR|O_LARGEFILE);
    if (fd < 0){
		printf_e("open /dev/mmcblk0boot0 fail\n");
        return -1;
	}
	printf_d("open mmcblk0boot0 ok\n");
	
	img_buffer = (unsigned char *)malloc((img_size) * sizeof(unsigned char));
	dev_buffer = (unsigned char *)malloc((img_size) * sizeof(unsigned char));
    if ((!img_buffer) || (!dev_buffer)){
		printf_e("buffer malloc fail \n");
        goto error_exit;
	}

	int read_num;
	lseek64(fdr, 0, SEEK_SET);
	read_num = read(fdr, img_buffer, img_size);
	if (read_num != img_size){
		printf_e("read image fail %d:%d \n",read_num,img_size);
		goto error_exit;
	}
	

    lseek64(fd, 0, SEEK_SET);
    read_num = read(fd, dev_buffer, img_size);
    if (read_num != img_size){
		printf_e("read real image fail %d:%d \n",read_num,img_size);
        goto error_exit;
	}

	#if 0
		printf_i("img_size=%d\n",img_size);
		uint i=0;
		for(i=0;i<img_size;i++){
			if(i%16 == 0) printf("\n");
			printf_i("[%2x-%2x] ",img_buffer[i],dev_buffer[i]);
		}
	#endif

	if (TRUE == is_data_same(img_buffer, dev_buffer, img_size)){
        need_upgrade = FALSE;
	}
	else need_upgrade = TRUE;
	
	printf_d("version check : need_upgrade = %s \n",((need_upgrade==TRUE)?("YES"):("NO")));
	
	if (TRUE == need_upgrade) {
		erase_emmc(fd, 0, img_size);
		int write_cnt;
		system("echo 0 > /sys/block/mmcblk0boot0/force_ro");
		lseek64(fd, 0, SEEK_SET);
		write_cnt = write(fd, img_buffer, img_size);
		if (write_cnt != img_size){
			printf_e("write fail %d:%d\n",write_cnt,img_size);
			goto error_exit;
		}
		system("echo 1 > /sys/block/mmcblk0boot0/force_ro");
		printf_d("%s upgrade done %d\n",img_path, write_cnt);
	} else {
		printf_d("%s no need upgrade\n", img_path);
	}
	
	close(fdr);
    close(fd);
    free(img_buffer);
    img_buffer = NULL;
    free(dev_buffer);
    dev_buffer = NULL;

    return 0;





error_exit:
    if (fdr > 0)
        close(fdr);
    if (fd > 0)
        close(fd);
    if (img_buffer) {
        free(img_buffer);
        img_buffer = NULL;
    }
    if (dev_buffer) {
        free(dev_buffer);
        dev_buffer = NULL;
    }

    return -1;
}


int upgrade_emmc_partition(const unsigned char *name, const unsigned char *img_path,const char force_update)
{
    unsigned int name_size = strlen(name);
    int fd = -1;
    int fdr = -1;
    unsigned char *img_buffer = NULL;
    unsigned char *dev_buffer = NULL;
    bool split_write = FALSE;
    bool need_upgrade = TRUE;
    if (name_size > MAX_GPT_NAME_SIZE){
		printf_e("partition name overflow %d\n",name_size);
        goto error_exit;
	}
	
    int img_size = file_size(img_path);
    fdr = open(img_path, O_RDONLY|O_LARGEFILE);
    if (fdr < 0){
		printf_e("open %s fail\n",img_path);
        goto error_exit;
	}

    fd = open("/dev/mmcblk0", O_RDWR|O_LARGEFILE);
    if (fd < 0){
		printf_e("open /dev/mmcblk0 fail\n");
        goto error_exit;
	}
	
    struct part_attr part;
    bool part_exist = search_emmc_part_from_table(name, &part);
    if (FALSE == part_exist){
		printf_e("upgrade %s fail ,partition not exist !\n",name);
        goto error_exit;
	}

    unsigned long long part_size = (part.end_lba - part.start_lba + 1) * G_BLOCK_SIZE;
    unsigned long long start_address = part.start_lba * G_BLOCK_SIZE;
    unsigned long long end_address = (part.end_lba + 1) * G_BLOCK_SIZE;
    if (img_size > part_size){
		printf_e("image size overflow %d :%d\n",img_size,part_size);
        goto error_exit;
	}

	//printf_d("start_address=%lld end_address=%lld \n",start_address,end_address);

    if (img_size > IMG_SPLIT_SIZE)
        split_write = TRUE;

    lseek64(fdr, 0, SEEK_SET);
    if (FALSE == split_write) {
        img_buffer = (unsigned char *)malloc((img_size) * sizeof(unsigned char));
        dev_buffer = (unsigned char *)malloc((img_size) * sizeof(unsigned char));
    } else {
        img_buffer = (unsigned char *)malloc((IMG_SPLIT_SIZE) * sizeof(unsigned char));
        dev_buffer = (unsigned char *)malloc((G_BLOCK_SIZE) * sizeof(unsigned char));
    }
    if ((!img_buffer) || (!dev_buffer)){
		printf_e("buffer malloc fail \n");
        goto error_exit;
	}
	
    if (FALSE == split_write) {
        int read_num;
        lseek64(fdr, 0, SEEK_SET);
        read_num = read(fdr, img_buffer, img_size);
        if (read_num != img_size){
			printf_e("read image fail %d:%d \n",read_num,img_size);
            goto error_exit;
		}

       // lseek64(fdr, 0, SEEK_SET);
        lseek64(fd, start_address, SEEK_SET);
        read_num = read(fd, dev_buffer, img_size);
        if (read_num != img_size){
			printf_e("read real image fail %d:%d \n",read_num,img_size);
            goto error_exit;
		}
		
        lseek64(fd, 0, SEEK_SET);

		if (TRUE == force_update)
			need_upgrade = TRUE;
		
		printf_d("version check : need_upgrade = %s \n",((need_upgrade==TRUE)?("YES"):("NO")));

		
        if (TRUE == need_upgrade) {
            int write_cnt;
            lseek64(fd, start_address, SEEK_SET);
            write_cnt = write(fd, img_buffer, img_size);
            if (write_cnt != img_size){
				printf_e("write fail %d:%d\n",write_cnt,img_size);
                goto error_exit;
			}
        } else {
            printf_d("%s no need upgrade\n", img_path);
        }
    } else {
        int read_loop_cnt = img_size / (IMG_SPLIT_SIZE);
        int read_loop_last = img_size % (IMG_SPLIT_SIZE);
        int i;
        int read_num;
        int write_cnt;
        lseek64(fdr, 0, SEEK_SET);
        lseek64(fd, start_address, SEEK_SET);
        for (i = 0; i < read_loop_cnt; i++) {
            read_num = read(fdr, img_buffer, IMG_SPLIT_SIZE);
            if (read_num != IMG_SPLIT_SIZE){
				printf_e(" split read fail %d:%d\n",read_num);
                goto error_exit;
			}
			
            write_cnt = write(fd, img_buffer, IMG_SPLIT_SIZE);
            if (write_cnt != IMG_SPLIT_SIZE){
				printf_e(" split write fail %d:%d\n",read_num);
                goto error_exit;
			}
        }
        if (read_loop_last > 0) {
			
            read_num = read(fdr, img_buffer, read_loop_last);
            if (read_num != read_loop_last){
				printf_e(" split read last loop fail %d:%d\n",read_num);
                goto error_exit;
			}
			
            write_cnt = write(fd, img_buffer, read_loop_last);
            if (write_cnt != read_loop_last){
				printf_e(" split write last loop fail %d:%d\n",write_cnt);
                goto error_exit;
			}
        }
    }

    close(fdr);
    close(fd);
    free(img_buffer);
    img_buffer = NULL;
    free(dev_buffer);
    dev_buffer = NULL;

    return 0;

error_exit:
    if (fdr > 0)
        close(fdr);
    if (fd > 0)
        close(fd);
    if (img_buffer) {
        free(img_buffer);
        img_buffer = NULL;
    }
    if (dev_buffer) {
        free(dev_buffer);
        dev_buffer = NULL;
    }

    return -1;
}

int upgrade_emmc_partitions(int upgrade_side)
{
	int ret;
	if (upgrade_side == 0) { //0---upgrade A partitions
    ret = upgrade_emmc_partition(A_BOOT_PARTITION_NAME, "/tmp/update/boot.img",FALSE);
        if (ret < 0){
			printf("upgrade boot A partition fail\n\n");
            return ret;
		}
		else
			printf("upgrade boot A partition ok\n\n");
    ret = upgrade_emmc_partition(A_ROOTFS_PARTITION_NAME, "/tmp/update/rootfs.ext4",FALSE);
        if (ret < 0){
		    printf("upgrade rootfs A partition fail\n\n");
		    return ret;
		}
		else
			printf("upgrade rootfs A partition ok\n\n");
	ret = upgrade_emmc_partition(A_TZ_PARTITION_NAME, "/tmp/update/tz.img",FALSE);
		if (ret < 0){
		     printf("upgrade tee A partition fail\n\n");
             return ret;
		}
		else
			printf("upgrade tee A partition ok\n\n");
	}
	else {  //1---upgrade B partitions
    ret = upgrade_emmc_partition(B_BOOT_PARTITION_NAME, "/tmp/update/boot.img",FALSE);
    if (ret < 0){
			printf("upgrade boot B partition fail\n\n");
			return ret;
		}
		else
			printf("upgrade boot B partition ok\n\n");
    ret = upgrade_emmc_partition(B_ROOTFS_PARTITION_NAME, "/tmp/update/rootfs.ext4",FALSE);
    if (ret < 0){
			printf("upgrade rootfs B partition fail\n\n");
			return ret;
		}
		else
			printf("upgrade rootfs B partition ok\n\n");
		ret = upgrade_emmc_partition(B_TZ_PARTITION_NAME, "/tmp/update/tz.img",FALSE);
		if (ret < 0){
			printf("upgrade tee B partition fail\n\n");
			return ret;
		}

		else
			printf("upgrade tee B partition ok\n\n");
	}
       if ( upgrade_emmc_partition("UBOOT", "/tmp/update/lk.img",FALSE) < 0)
		    printf("upgrade lk partition fail, it may because no need to upgrade lk\n\n");
	    else
		    printf("upgrade lk partition ok\n\n");
	sync();
    return ret;
}
/* upgrade emmc mbr */
int upgrade_emmc_mbr(const char *filepath)
{
    unsigned char *gpt_buffer = NULL;
    unsigned char *dev_buffer = NULL;
    int ret;
    int fdr = -1;
    int fd = -1;
    bool need_upgrade = TRUE;

    int mbr_size = file_size(filepath);
    if (mbr_size != GPT_MBR_SIZE)
        goto gpt_error_exit;

    fdr = open(filepath, O_RDONLY);
    if (fdr < 0)
        goto gpt_error_exit;

    if (FALSE == is_valid_gpt(fdr))
        goto gpt_error_exit;

    lseek64(fdr, 0, SEEK_SET);
    gpt_buffer = (unsigned char *)malloc((mbr_size + G_BLOCK_SIZE) * sizeof(unsigned char));
    dev_buffer = (unsigned char *)malloc((mbr_size + G_BLOCK_SIZE) * sizeof(unsigned char));
    if ((!dev_buffer) || (!gpt_buffer))
        goto gpt_error_exit;

    lseek64(fdr, 0, SEEK_SET);
    int read_num;
    read_num = read(fdr, gpt_buffer, mbr_size);
    if ((read_num > mbr_size) || (read_num < 0))
        goto gpt_error_exit;

    fd = open("/dev/mmcblk0", O_RDWR);
    unsigned long long emmc_size;
    ioctl(fd, BLKGETSIZE64, &emmc_size);

    lseek64(fd, 0, SEEK_SET);
    read_num = read(fd, dev_buffer, mbr_size);
    if ((read_num > mbr_size) || (read_num < 0))
        goto gpt_error_exit;

    unsigned int dev_crc1 = GET_LONG((dev_buffer + G_BLOCK_SIZE) + PARTITION_CRC_OFFSET);
    unsigned int dev_crc2 = GET_LONG((dev_buffer + G_BLOCK_SIZE) + HEADER_CRC_OFFSET);
    /* fix gpt */
    patch_gpt(gpt_buffer, emmc_size, MIN_PARTITION_ARRAY_SIZE, MAX_PARTITION_COUNT, PARTITION_ENTRY_SIZE, G_BLOCK_SIZE);
    unsigned int up_crc1 = GET_LONG((gpt_buffer + G_BLOCK_SIZE) + PARTITION_CRC_OFFSET);
    unsigned int up_crc2 = GET_LONG((gpt_buffer + G_BLOCK_SIZE) + HEADER_CRC_OFFSET);
    if ((up_crc1 == dev_crc1) && (up_crc2 == dev_crc2)) {
        need_upgrade = FALSE;
		ret = SUCCESS+1;
    }
	printf_d("version check : need_upgrade = %s \n",((need_upgrade==TRUE)?("YES"):("NO")));

    if (TRUE == need_upgrade) 
	{
        lseek64(fd, 0, SEEK_SET);
        /* erase */
        //erase_emmc(fd, 0, (G_BLOCK_SIZE * 64));
        //erase_emmc(fd, (emmc_size - (G_BLOCK_SIZE * 64)), emmc_size);
        ret = write_gpt(fd, gpt_buffer, emmc_size);
        if (ret < 0)
            goto gpt_error_exit;
		ret = SUCCESS;
    }

    close(fdr);
    close(fd);
    free(gpt_buffer);
    gpt_buffer = NULL;
    free(dev_buffer);
    dev_buffer = NULL;

    return ret;

gpt_error_exit:
    if (fdr > 0)
        close(fdr);
    if (fd > 0)
        close(fd);
    if (gpt_buffer) {
        free(gpt_buffer);
        gpt_buffer = NULL;
    }
    if (dev_buffer) {
        free(dev_buffer);
        dev_buffer = NULL;
    }

    return -1;
}


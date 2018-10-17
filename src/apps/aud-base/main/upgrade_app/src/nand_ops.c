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
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include "gpt.h"
#include "file_ops.h"
#include "typedefs.h"
#include "nand_ops.h"

#define MTD_PROC_FILENAME   "/proc/mtd"
int mtd_get_partition_info(const unsigned char *name)
{
#if 1
    char buf[2048];
    const char *bufp;
    int fd;
    int i;
    ssize_t nbytes;

    /* Open and read the file contents.
     */
    fd = open(MTD_PROC_FILENAME, O_RDONLY);
    if (fd < 0) {
		printf_e("open /proc/mtd fail\n");
        return -1;
    }
    nbytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (nbytes < 0) {
        return -1;
    }
    buf[nbytes] = '\0';

    /* Parse the contents of the file, which looks like:
     *
     *     # cat /proc/mtd
     *     dev:    size   erasesize  name
     *     mtd0: 00080000 00020000 "bootloader"
     *     mtd1: 00400000 00020000 "mfg_and_gsm"
     *     mtd2: 00400000 00020000 "0000000c"
     *     mtd3: 00200000 00020000 "0000000d"
     *     mtd4: 04000000 00020000 "system"
     *     mtd5: 03280000 00020000 "userdata"
     */
    bufp = buf;
	    int mtdnum = -1;
		int mtdsize, mtderasesize;
        int matches;
        char mtdname[64];
        mtdname[0] = '\0';
    while (nbytes > 0) {
        matches = sscanf(bufp, "mtd%d: %x %x \"%63[^\"]",
                &mtdnum, &mtdsize, &mtderasesize, mtdname);
        /* This will fail on the first line, which just contains
         * column headers.
         */
        if (matches == 4) {
		//printf_d("mtdnum = %d, erasesize = %d, mtdsize=%d , name= %s\n", mtdnum, mtderasesize, mtdsize,mtdname);
           if (strcasecmp(name, mtdname) == 0) return mtdnum;
        }
        /* Eat the line.
         */
        while (nbytes > 0 && *bufp != '\n') {
            bufp++;
            nbytes--;
        }
        if (nbytes > 0) {
            bufp++;
            nbytes--;
        }
		
    }

    return -1;
#endif
}


/* erase */
int erase_nand(int fd, int start, int end)
{
    mtd_info_t mtd_info;
    erase_info_t ei;

	printf_d("start %s start_add=%d \n", __FUNCTION__, start);
    lseek(fd, 0, SEEK_SET);
    /* get the device info */
    ioctl(fd, MEMGETINFO, &mtd_info);

    /* set the erase block size */
    ei.length = mtd_info.erasesize;
	printf_d("start %s erasesize=%d \n", __FUNCTION__,ei.length);
    /* erase */
    for (ei.start = start; ei.start < end; ei.start += ei.length) {
        //ioctl(fd, MEMUNLOCK, &ei);
        ioctl(fd, MEMERASE, &ei);
    }
    lseek(fd, 0, SEEK_SET);
	
	printf_d("end %s  end_add=%d \n", __FUNCTION__,end);
    return 0;
}



int seg_upgrade_nand_partition(const unsigned char *name,loff_t *write_location,int pt_size, int offset)
{
    extern int errno;
    unsigned int name_size = strlen(name);
	bool need_upgrade = TRUE;
    int fd = -1;
    int fdr = -1;
    unsigned char *img_buffer = NULL;
    const char *img_path ="/tmp/update/update.bin";
	/*get image info*/
    int img_size = pt_size;//read from service
	int write_size = -1;
    fdr = open(img_path, O_RDONLY);
    if (fdr < 0){
		printf_e("open %s fail\n",img_path);
		goto error_exit;
	}

	/*get this partition writesize*/
    int mtdnum = 0;
	mtdnum = mtd_get_partition_info(name);
	printf_d("get %s partition mtdnum: %d\n", name, mtdnum);
	char devname[64];
	sprintf(devname, "/dev/mtd%d", mtdnum);
    fd = open(devname, O_RDWR);
	if (fd < 0){
		printf_e("open %s fail\n",devname);
		goto error_exit;
	}

    mtd_info_t mtd_info;
    ioctl(fd, MEMGETINFO, &mtd_info);
    img_buffer = (unsigned char *)malloc((mtd_info.erasesize) * sizeof(unsigned char));
    if (!img_buffer){
		printf_e("buffer malloc fail \n");
        goto error_exit;
	}
    if(img_size > (int) mtd_info.size ){
		printf_e("image size exceeds partition szie\n");
		goto error_exit;
    	}
    //erase & write  --need to skip bad block
    if (TRUE == need_upgrade) {
		int read_num;
		int wrote = offset;
		loff_t pos = lseek(fd, 0, SEEK_CUR);
			if (pos == (loff_t) -1) {
				printf("mtd: write_block: couldn't SEEK_CUR: %s\n", strerror(errno));
				return -1;
			}
	    pos = *write_location;
		while(wrote < img_size) {
			//---read img file---
			int writesuccess = -1;
			memset(img_buffer, 0xff, mtd_info.erasesize);

			lseek(fdr, wrote, SEEK_SET);
			if( (img_size - wrote) < mtd_info.erasesize) {
				write_size = img_size - wrote;
			}
			else {
				write_size = mtd_info.erasesize;
			}
			read_num = read(fdr, img_buffer, write_size);
			if (read_num != write_size){
                printf_e("read image fail %d:%d \n",read_num,write_size);
                goto error_exit;
     		}
			//---write the img file to flash---
			while (pos + mtd_info.erasesize <= (int) mtd_info.size) {
				loff_t bpos = pos;
				if ((mtd_info.size - pos) < mtd_info.erasesize) {
					//not have enough space to write img.
					printf_e("not have enough space to write img\n");
					goto error_exit;
				}

				int ret = ioctl(fd, MEMGETBADBLOCK, &bpos);
				if (ret != 0 && !(ret == -1 && errno == EOPNOTSUPP)) {
					fprintf(stderr, "mtd: not writing bad block at 0x%08lx (ret %d): %s\n",
         	           pos, ret, strerror(errno));
					pos += mtd_info.erasesize;
					continue;  // Don't try to erase known factory-bad blocks.
        		}

				//---erase---
				erase_info_t ei;
				ei.start = pos;
				ei.length = mtd_info.erasesize;
				ioctl(fd, MEMERASE, &ei);

				//---write---
				lseek64(fd, pos, SEEK_SET);
				int write_cnt = write(fd, img_buffer, mtd_info.erasesize);
				if (write_cnt != mtd_info.erasesize){
					printf_e("write %d img to %s partition error.\n",write_size,name);
					goto error_exit;
				}
				else {
					wrote += mtd_info.erasesize;
					pos += mtd_info.erasesize;
					writesuccess = 0;
					break;
				}
			}
			if(writesuccess == -1) { //write a block fail, then judge write fail...
				goto error_exit;
			}
		}

        *write_location = pos;

		//erase the left blocks in partition
		while(pos < mtd_info.size) {
			loff_t bpos = pos;
			int write_size = -1;
			int ret = ioctl(fd, MEMGETBADBLOCK, &bpos);
			if (ret != 0 && !(ret == -1 && errno == EOPNOTSUPP)) {
				fprintf(stderr, "mtd: not writing bad block at 0x%08lx (ret %d): %s\n",
				   pos, ret, strerror(errno));
				pos += mtd_info.erasesize;
				continue;  // Don't try to erase known factory-bad blocks.
			}
			//---erase---
			erase_info_t ei;
			ei.start = pos;
			ei.length = mtd_info.erasesize;
			int erase_success = ioctl(fd, MEMERASE, &ei);
			if(erase_success == 0){
				pos += mtd_info.erasesize;
			}
			else {
				printf_e("erase error: %x  fail!\n",pos);
				goto error_exit;
			}
		}
    } else {
        printf_d("%s no need upgrade\n", img_path);
    }

	sync();

    close(fd);
    close(fdr);
    free(img_buffer);
    img_buffer = NULL;

    return 0;

error_exit:
    if (fd >= 0)
        close(fd);
    if (fdr >= 0)
        close(fdr);
    if (img_buffer) {
        free(img_buffer);
        img_buffer = NULL;
    }
    return -1;
}

int seg_upgrade_nand_partitions(int upgrade_side, char * pt_name,loff_t *write_location,int pt_size, int offset)
{
    int ret;
	if (upgrade_side == 0) { //0---upgrade A partitions
    	    if (!strcmp(pt_name, "boot.img"))
	    	{
    	      ret = seg_upgrade_nand_partition(A_BOOT_PARTITION_NAME, write_location, pt_size, offset);
    	      if (ret < 0)
			  printf("upgrade boot A partition fail\n\n");
		else
			  printf("upgrade boot A partition ok\n\n");
	    	}
		if (!strcmp(pt_name, "rootfs.ubi"))
			{
    	      ret = seg_upgrade_nand_partition(A_ROOTFS_PARTITION_NAME, write_location, pt_size, offset);
    	      if (ret < 0)
        	  printf("upgrade rootfs A partition fail\n\n");
		      else
			  printf("upgrade rootfs A partition ok\n\n");
			}
		if (!strcmp(pt_name, "tz.img"))
		    {
		      ret = seg_upgrade_nand_partition(A_TZ_PARTITION_NAME, write_location, pt_size, offset);
		      if (ret < 0)
			  printf("upgrade tee A partition fail\n\n");
		else
			  printf("upgrade tee A partition ok\n\n");
			}

	}
	else {  //1---upgrade B partitions
	    if (!strcmp(pt_name, "boot.img"))
	    	{
    	      ret = seg_upgrade_nand_partition(B_BOOT_PARTITION_NAME, write_location, pt_size, offset);
    	      if (ret < 0)
			  printf("upgrade boot B partition fail\n\n");
		else
			  printf("upgrade boot B partition ok\n\n");
	    	}
		if (!strcmp(pt_name, "rootfs.ubi"))
			{
    	      ret = seg_upgrade_nand_partition(B_ROOTFS_PARTITION_NAME, write_location, pt_size, offset);
    	      if (ret < 0)
        	  printf("upgrade rootfs B partition fail\n\n");
		      else
			  printf("upgrade rootfs B partition ok\n\n");
			}
		if (!strcmp(pt_name, "tz.img"))
		    {
		      ret = seg_upgrade_nand_partition(B_TZ_PARTITION_NAME, write_location, pt_size, offset);
		      if (ret < 0)
			  printf("upgrade tee B partition fail\n\n");
		else
			  printf("upgrade tee B partition ok\n\n");
			}
	}

	sync();
    return ret;
}
int upgrade_nand_partition(const unsigned char *name, const unsigned char *img_path)
{
    extern int errno;
    unsigned int name_size = strlen(name);
	bool need_upgrade = TRUE;
    int fd = -1;
    int fdr = -1;
    unsigned char *img_buffer = NULL;

	/*get image info*/
    int img_size = file_size(img_path);
	int write_size = -1;
    fdr = open(img_path, O_RDONLY);
    if (fdr < 0){
		printf_e("open %s fail\n",img_path);
		goto error_exit;
	}
	
	/*get this partition writesize*/
    int mtdnum = 0;
	mtdnum = mtd_get_partition_info(name);
	printf_d("get %s partition mtdnum: %d\n", name, mtdnum);
	char devname[64];
	sprintf(devname, "/dev/mtd%d", mtdnum);
    fd = open(devname, O_RDWR);
	if (fd < 0){
		printf_e("open %s fail\n",devname);
		goto error_exit;
	}

    mtd_info_t mtd_info;
    ioctl(fd, MEMGETINFO, &mtd_info);   
    img_buffer = (unsigned char *)malloc((mtd_info.erasesize) * sizeof(unsigned char));
    if (!img_buffer){
		printf_e("buffer malloc fail \n");
        goto error_exit;
	}

    if(img_size > (int) mtd_info.size ){
		printf_e("image size exceeds partition szie\n");
		goto error_exit;
    }
 
    //erase & write  --need to skip bad block
    if (TRUE == need_upgrade) {
		int read_num;
		int wrote = 0;
		loff_t pos = 0;
		while(wrote < img_size) {
			//---read img file---
			int writesuccess = -1;
			memset(img_buffer, 0xff, mtd_info.erasesize);
		
			lseek(fdr, wrote, SEEK_SET);
			if( (img_size - wrote) < mtd_info.erasesize) {
				write_size = img_size - wrote;
			}
			else {
				write_size = mtd_info.erasesize;
			}
			read_num = read(fdr, img_buffer, write_size);
			if (read_num != write_size){
   				printf_e("read image fail %d:%d \n",read_num,write_size);
        		goto error_exit;
 			}
			
			//---write the img file to flash---
			while (pos + mtd_info.erasesize <= (int) mtd_info.size) {
				loff_t bpos = pos;
				if ((mtd_info.size - pos) < mtd_info.erasesize) {
					//not have enough space to write img.
					printf_e("not have enough space to write img\n");
					goto error_exit;
				}
				
				int ret = ioctl(fd, MEMGETBADBLOCK, &bpos);
				if (ret > 0) {
				//bad block
					fprintf(stderr, "mtd: not writing bad block at 0x%08lx (ret %d): %s\n",
         	           pos, ret, strerror(errno));
					pos += mtd_info.erasesize;
					continue;  // Don't try to erase known factory-bad blocks.
        		}
				
				//---erase---
				erase_info_t ei;
				ei.start = pos;
				ei.length = mtd_info.erasesize;
				ioctl(fd, MEMERASE, &ei);
				
				//---write---
				lseek64(fd, pos, SEEK_SET);
				int write_cnt = write(fd, img_buffer, mtd_info.erasesize);
				if (write_cnt != mtd_info.erasesize){		
					printf_e("write %d img to %s partition error.\n",write_size,name);
					goto error_exit;
				}
				else {
					wrote += mtd_info.erasesize;
					pos += mtd_info.erasesize;
					writesuccess = 0;
					break;
				}
			}
			if(writesuccess == -1) { //write a block fail, then judge write fail...
				goto error_exit;
			}
		}

		//erase the left blocks in partition
		while(pos < mtd_info.size) {
			loff_t bpos = pos;
			int write_size = -1;
			int ret = ioctl(fd, MEMGETBADBLOCK, &bpos);
			if (ret > 0 ) {
				//bad block
				fprintf(stderr, "mtd: not writing bad block at 0x%08lx (ret %d): %s\n",
				   pos, ret, strerror(errno));
				pos += mtd_info.erasesize;
				continue;  // Don't try to erase known factory-bad blocks.
			}
			//---erase---
			erase_info_t ei;
			ei.start = pos;
			ei.length = mtd_info.erasesize;
			int erase_success = ioctl(fd, MEMERASE, &ei);
			if(erase_success == 0){
				pos += mtd_info.erasesize;
			}
			else {
				printf_e("erase error: %x  fail!\n",pos);	
				goto error_exit;
			}
		}
    } else {
        printf_d("%s no need upgrade\n", img_path);
    }

	sync();
	
    close(fd);
    close(fdr);
    free(img_buffer);
    img_buffer = NULL;

    return 0;

error_exit:
    if (fd >= 0)
        close(fd);
    if (fdr >= 0)
        close(fdr);
    if (img_buffer) {
        free(img_buffer);
        img_buffer = NULL;
    }
    return -1;
}

int upgrade_nand_partitions(int upgrade_side)
{
    int ret;
	if (upgrade_side == 0) { //0---upgrade A partitions
    	ret = upgrade_nand_partition(A_BOOT_PARTITION_NAME, "/tmp/update/boot.img");
        if (ret < 0){
			printf("upgrade boot A partition fail\n\n");
            return ret;
		}
		else
			printf("upgrade boot A partition ok\n\n");
    	ret = upgrade_nand_partition(A_ROOTFS_PARTITION_NAME, "/tmp/update/rootfs.ubi");
        if (ret < 0){
		    printf("upgrade rootfs A partition fail\n\n");
		    return ret;
		}
		else
			printf("upgrade rootfs A partition ok\n\n");
		ret = upgrade_nand_partition(A_TZ_PARTITION_NAME, "/tmp/update/tz.img");
		if (ret < 0){
		     printf("upgrade tee A partition fail\n\n");
             return ret;
		}
		else
			printf("upgrade tee A partition ok\n\n");
	}
	else {  //1---upgrade B partitions
    	ret = upgrade_nand_partition(B_BOOT_PARTITION_NAME, "/tmp/update/boot.img");
    	if (ret < 0){
			printf("upgrade boot B partition fail\n\n");
			return ret;
		}
		else
			printf("upgrade boot B partition ok\n\n");
    	ret = upgrade_nand_partition(B_ROOTFS_PARTITION_NAME, "/tmp/update/rootfs.ubi");
    	if (ret < 0){
			printf("upgrade rootfs B partition fail\n\n");
			return ret;
		}
		else
			printf("upgrade rootfs B partition ok\n\n");
		ret = upgrade_nand_partition(B_TZ_PARTITION_NAME, "/tmp/update/tz.img");
		if (ret < 0){
			printf("upgrade tee B partition fail\n\n");
			return ret;
		}

		else
			printf("upgrade tee B partition ok\n\n");
	}
       if ( upgrade_nand_partition("UBOOT", "/tmp/update/lk.img") < 0)
		    printf("upgrade lk partition fail, it may because no need to upgrade lk\n\n");
	    else
		    printf("upgrade lk partition ok\n\n");
	sync();
    return ret;
}


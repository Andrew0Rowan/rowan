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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <pthread.h>
#include <mtd/mtd-user.h>

#include "gpt.h"
#include "nand_ops.h"
#include "emmc_ops.h"
#include "config.h"
#include <sys/ioctl.h>
#include "upg_trigger.h"


int current_log_level=DBG;
static uint progress_status=0;
static bool ui_thread_abort=0;
static int UPG_SUCCEED =2;

#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif

typedef struct upgrade_flag_buffer {
  int last_boot;   //0-- last_boot from A.    1--last_boot from B. set the flag to lk.
  int upgA;
  int upgB;
  int now_boot;	//0-- last_boot from A.    1--last_boot from B.  get the flag from lk.
  loff_t write_location;//record write location every time
}upgrade_flag;

static const char *IMAGE_PATH = "/tmp/update";

extern int errno;

void dbg_log_self_check(int log_level)
{
	printf("current log level = %d\n",log_level);

	printf_e("printf_e test ok \n");
	printf_i("printf_i test ok \n");
	printf_d("printf_d test ok \n");
}

int check_free_zise(const char *device_mount_point)
{
	printf_d("start %s \n", __FUNCTION__);
	return SUCCESS;
}

int check_file_exist(const char *file_path)
{
	printf_d("start %s \n", __FUNCTION__);
	if(NULL == file_path){
		printf_d("invalid file path ! \n");
		return -1;
	}
	if(0 == access(file_path,F_OK)){
		printf_d("file is exist ,path :%s! \n",file_path);
		return SUCCESS;
	}
	return -1;
}

/**
*search_file() search SD card and USB device ,to check file exist or not
*
*@file_name: file name which will to find in SD/USB devie , "update.zip" here
*@slot_id: if find file ,return slot id ,to tell which slot SD/USB device plugin
*
*returns:
*    SUCCESS, on success
*    -1, on failure
*
*/
int search_file(const char *file_name,uint *slot_id)
{
	uint i;
	int ret=-1;
	int fd1,fd2;
	int check_dev_count=0;
	int check_file_count=0;
	int check_mount_count=0;
	char mount_cmd[100];
	char fsck_cmd[50];
	char file_path[100];
	char mkdir_mount_path[100];

	printf_d("start %s \n", __FUNCTION__);
	printf_d("to find %s in SD card/USB device in %d slot \n",file_name,MAX_SLOT);

	*slot_id = 0xff;

	for(i=0;i < MAX_SLOT;i++){
		printf_d("dev check round %d \n",(i+1));
		fd1=open(dev_attrs[i].dev_node, O_RDONLY);
		if (fd1 < 0){
			check_dev_count ++;
			printf_e("round - %d :no device %s \n",(i+1),dev_attrs[i].dev_node);
			close(fd1);
		}
		else{
			printf_d("round - %d :device (%s) found \n",(i+1),dev_attrs[i].dev_node);

			//prepare mount command
			snprintf(mount_cmd,100,"mount -t vfat %s %s",dev_attrs[i].dev_node,dev_attrs[i].mount_point);
			printf_i("start to mount %s to %s  using cmd: %s \n",dev_attrs[i].dev_node,dev_attrs[i].mount_point,mount_cmd);

			//create mount  point
			snprintf(mkdir_mount_path,100,"mkdir -p %s",dev_attrs[i].mount_point);
			printf_d("start to create mount point, using cmd: %s \n",mkdir_mount_path);
			system(mkdir_mount_path);

			//fsck check avoid SDcard readonly issue
			snprintf(fsck_cmd,50,"/sbin/fsck_msdos -p -f %s",dev_attrs[i].dev_node);
			printf_d("start to fsck-msdos check, using cmd: %s \n",fsck_cmd);
			system(fsck_cmd);

			//mount device
			system(mount_cmd);
			check_mount_count++;

			fd2=open(dev_attrs[i].mount_point, O_RDONLY);
			if (fd2 < 0){ //mount fail 
				printf_e("open %s fail, mount fail ? \n",dev_attrs[i].mount_point);
				close(fd1);
				close(fd2);
			}
			else{ //mount  ok  ,to check file exist
				printf_d("mount success  \n");
				snprintf(file_path,100,"%s/%s",dev_attrs[i].mount_point,file_name);

				if(check_file_exist(file_path)==SUCCESS){
					printf_d("find update file %s ! \n",file_path);
					*slot_id = i;
					printf_d("slot_id %d \n",*slot_id);
					close(fd1);
					close(fd2);
					ret = SUCCESS;
					break ;
				}
				else{
					check_file_count++;
					printf_e("NO %s found under %s ! \n",file_name,dev_attrs[i].mount_point);
					close(fd1);
					close(fd2);
				}
			}
		}
	}
	if (ret == SUCCESS){
		printf_i("serach file success, (slot_id:%d) !\n",*slot_id);
		return ret;
	}
	if (check_dev_count == MAX_SLOT){
		printf_e("NO DEVICES, Please insert SD card /USB device with update image first!!\n");
		return -NODEV;
	}

	if(check_mount_count == check_file_count){
		printf_e("NO update file found, Please put image to SD card /USB device!!\n");
		return -NOIMG;
	}

	return -UNKNOW;
}

/**
*check_free_size() to check SD/USB deivice free size, ensure there is enough space to unzip "update.zip"
*
*@device_path: which path to be checked 
*
*returns:
*    SUCCESS, space OK
*    -1, no enough space to storage unzip file
*
*/
int check_free_size(const char * device_path)
{
	int ret = 0;
	struct statfs device_info;
	char *mount_point = device_path;
	unsigned long long available_size = 0ULL;
	unsigned long long available_size_MB = 0ULL;
	
	printf_d("start %s \n", __FUNCTION__);

	ret = statfs(mount_point, &device_info);
	if(ret == -1){
		printf_e("get device info fail \n");
		return ret;
	}

	printf_d("bavail=%ld bsize=%ld  \n",device_info.f_bavail,device_info.f_bsize);

	/***
	*available_size = device_info.f_bavail * device_info.f_bsize
	*				 = (device_info.f_bavail/1024) * device_info.f_bsize KB
	*				 = ((device_info.f_bavail/1024) * device_info.f_bsize)/1024 MB
	**/
	available_size = (device_info.f_bavail/1024) * (device_info.f_bsize/512)  ;
	printf_d("available_size=%lld \n",available_size);

	available_size_MB = available_size/2;
	printf_d("available_size_MB=%lld \n",available_size_MB);
	
	//check available size, it should more than MIN_FREE_SIZE
	if (available_size_MB > MIN_FREE_SIZE){

		printf_i("available size is enough ,Check size ok \n");
		return SUCCESS;
	}

	printf_i("available size is too small \n");
	return -1;
}

/**
*decompress_img() unzip "*.zip"
*
*@mount_point: path of the zip file
*@file_name: name of the zip file to be decompress
*
*/

static const int MAX_COMMAND_LENGTH = 200;

int decompress_img(char* COMMAND_FILE)
{
	char mkdir_cmd[100];
	char decompress_cmd[100];


	//mkdir /tmp/update/
	snprintf(mkdir_cmd,100,"mkdir /tmp/update");
	printf_d("start to decompress zip file, using cmd: %s \n\n",mkdir_cmd);
	int ret = system(mkdir_cmd);
	if(ret == 0) {
		printf_d("mkdir success \n\n");
	}
	else {
		printf_d("mkdir fail \n\n");
		return -1;
	}

	//unzip the update.zip
	snprintf(decompress_cmd,100,"%s %s -d %s",UNZIP_CMD,COMMAND_FILE,IMAGE_PATH);
	printf_d("start to decompress zip file, using cmd: %s \n\n",decompress_cmd);
	ret = system(decompress_cmd);
	if(ret == 0) {
		printf_d("decompress_img success \n\n");
	}
	else {
		printf_d("decompress_img fail \n\n");
		return -1;
	}
	return 0;
}

void remove_tmp_file(const char *tmp_file_path)
{
	char rm_tmp_cmd[100];

	//printf_d("start %s \n", __FUNCTION__);
	snprintf(rm_tmp_cmd,100,"rm -rf %s",tmp_file_path);
	printf_d("rm old tmp folder, using cmd: %s 100\n",rm_tmp_cmd);
	system(rm_tmp_cmd);

}
static const int FLAG_PAGES = 1;         // number of pages to save

int get_flags(struct upgrade_flag_buffer *flag){
	//open misc partition
	int mtdnum = 0;
	int fd =0;
	mtdnum = mtd_get_partition_info("MISC");
	char devname[64];
	sprintf(devname, "/dev/mtd%d", mtdnum);
	fd = open(devname, O_RDWR);
	if (fd < 0){
		printf_e("open MISC partition:	%s fail\n",devname);
		return -1;
	}

    //get flag size
	mtd_info_t mtd_info;
	ioctl(fd, MEMGETINFO, &mtd_info);
	ssize_t flag_size = mtd_info.writesize * FLAG_PAGES;

    //get flag data
        loff_t pos = 0;
        int ret = -1;
        while (pos + mtd_info.erasesize <= (int) mtd_info.size) {
                loff_t bpos = pos;
                ret = ioctl(fd, MEMGETBADBLOCK, &bpos);
                if (ret > 0 ) {
           //bad block 
                        fprintf(stderr, "mtd: not writing bad block at 0x%08lx (ret %d): %s\n", pos, ret, strerror(errno));
                        pos += mtd_info.erasesize;
                        continue;  // Don't try to erase known factory-bad blocks.
                }
                char data[flag_size];
                lseek(fd, pos, SEEK_SET);
                ret = read(fd, data, flag_size);
                if (ret != flag_size){
                        char* mesg = strerror(errno);
                        printf_e("read flag fail, error: %s\n",mesg);
                }
                memcpy(flag, data, sizeof(*flag));
                break;

        }


	close(fd);
	return 0;
}


int set_flags(struct upgrade_flag_buffer *flag){
	//open misc partition
	int mtdnum = 0;
	int fd = -1;
	int ret = -1;
	mtdnum = mtd_get_partition_info("MISC");
	char devname[64];
	sprintf(devname, "/dev/mtd%d", mtdnum);
	fd = open(devname, O_RDWR);
	if (fd < 0){
		printf_e("open MISC partition:  %s fail\n",devname);
		return -1;
	}

    //get flag size
	mtd_info_t mtd_info;
	ioctl(fd, MEMGETINFO, &mtd_info);
	ssize_t flag_size = mtd_info.erasesize;
	loff_t pos = 0;
  	while (pos + mtd_info.erasesize <= (int) mtd_info.size) {
		loff_t bpos = pos;
		ret = ioctl(fd, MEMGETBADBLOCK, &bpos);
		if (ret > 0 ) {
           //bad block
           fprintf(stderr,
                   "mtd: not writing bad block at 0x%08lx (ret %d): %s\n",
                   pos, ret, strerror(errno));
           pos += mtd_info.erasesize;
           continue;  // Don't try to erase known factory-bad blocks.
        }

		
		//erase flag area
		erase_info_t ei;
		ei.start = pos;
		ei.length = mtd_info.erasesize;
		ioctl(fd, MEMERASE, &ei);

		//set flag data
		char data[flag_size];
		memset(data, 0, flag_size);
		memcpy(data, flag, sizeof(*flag));

		lseek(fd, pos, SEEK_SET);

		ret = write(fd, data, flag_size);
		if (ret != flag_size){
			char* mesg = strerror(errno);
			printf_e("set flag fail, error: %s\n",mesg);
		}
		break;
	}
	
	sync();
	close(fd);
	return 0;
}


int main(int argc, char **argv)
{

	uint img_update=0;
	uint slot_id = 0;
	uint index = 0;

	pthread_t p_id;
	const char *file_name = UPDATE_FILE;
	char *image_list_p;
	char *part_info;
	char sub_image_name[50];
	char nor_mbr_path[50];
	char emmc_mbr_path[50];

	upgrade_flag current_flag;
	upgrade_flag new_flag;

    //add for fenduan
    int success_wrote =0;
	int offset =0;
	int pt_size =0;
	char pt_name[20];
	//init log  level 
	current_log_level=DBG;
	dbg_log_self_check(current_log_level);

	time_t start = time(NULL);
	printf_i("Starting recovery upgrade(pid %d) on %s \n", getpid(), ctime(&start));


	get_flags(&current_flag);
	printf_d("curretn_flag: %d, %d, %d, %d\n",current_flag.last_boot, current_flag.upgA, current_flag.upgB, current_flag.now_boot);

	//set last_boot flag according from now_boot flag.
	new_flag.upgA = current_flag.upgA;
	new_flag.upgB = current_flag.upgB;
	new_flag.now_boot= current_flag.now_boot;
	new_flag.write_location= current_flag.write_location;
	if (current_flag.now_boot == 0)
		new_flag.last_boot = 0;
	else if(current_flag.now_boot == 1)
		new_flag.last_boot = 1;
	else
		new_flag.last_boot = 0;

	if(current_flag.last_boot != new_flag.last_boot) { //last_boot flag need to upgrade.
		printf_d("set the last_boot flag\n");
		set_flags(&new_flag);
	}
    int ret = -1;
	if (argc == 5)
	{
	strcpy(pt_name,argv[1]);
	printf_i("pt_name=%s\n",pt_name);
	offset = atoi(argv[2]);
	printf_i("offsize= %d\n",offset);
	pt_size = atoi(argv[3]);
	printf_i("file_size= %d\n",pt_size);
	success_wrote = atoi(argv[4]);
	printf_i("success_wrote= %d\n",success_wrote);
	}
	else if (argc == 2)
	{
	success_wrote = 1;
	ret = decompress_img(argv[1]);
	if (ret != 0){
		 goto error_exit;
	  }
	}
	else
	{
		printf_d("no update command, only set the boot flag\n");
		goto error_exit;
	}

	if (0 == offset){
		new_flag.write_location =0;
		}
	else {
		offset = 0;//let offset is  zero to keep the wrote value is zero when file dose not contain header
	}
	printf_d("current: %d, last_boot:%d\n",current_flag.last_boot,new_flag.last_boot);

	if(new_flag.last_boot == 0) { //in A,  upgrade B partition
		printf_i("starting to upgrade B partition---------\n");
		if(5 == argc)
		ret = seg_upgrade_nand_partitions(1,pt_name, &new_flag.write_location, pt_size, offset);
	    if(2 == argc)
		{
		ret = upgrade_nand_partitions(1);
		}
		if(ret == 0 && success_wrote == 1 ) {
			new_flag.upgA = -1;
			new_flag.upgB = UPG_SUCCEED;
			new_flag.write_location = 0;
			printf_d("setting flags\n");
			set_flags(&new_flag);
			printf_i("upgrade success !!!\n");
		}
		if (ret == 0){
		set_flags(&new_flag);
		}
		else
			goto error_exit;
	}
	else { //in B, upgrade A partition
		printf_i("starting to upgrade A partition---------\n");
		if(5 == argc)
		ret = seg_upgrade_nand_partitions(0,pt_name, &new_flag.write_location, pt_size, offset);
	    if(2 == argc)
		{
		ret = upgrade_nand_partitions(0);
		}
		if(ret == 0 && success_wrote == 1) {
			new_flag.upgA = UPG_SUCCEED;
			new_flag.upgB = -1;
			new_flag.write_location = 0;
			set_flags(&new_flag);
			printf_i("upgrade success !!!\n");
		}
		if (ret == 0){
		set_flags(&new_flag);
		}
		else
			goto error_exit;
	}

	remove_tmp_file(IMAGE_PATH);


	return 0;

error_exit:

	remove_tmp_file(IMAGE_PATH);
    return -1;

}

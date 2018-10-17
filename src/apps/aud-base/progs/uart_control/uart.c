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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

char device_new_console[50] = "/dev/ttyS0"; //"/dev/tty01"
void ChangeConsole(void)
{
    int fp, ret = 0;
    printf("%s, %d\n", __FUNCTION__, __LINE__);
    printf("change1 with device_new_console:%s\n", device_new_console);
    fp = open(device_new_console, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fp < 0) {
        printf("open %s failed, fd:%d?error:%d\n", device_new_console, fp, errno);
        return ;
    }
    ret = ioctl(fp, TIOCCONS); //make 123 as console
    printf("ioctl TIOCCONS with ret:%d\n", ret);
    if (ret < 0) {
        printf("ioctl TIOCCONS failed\n ");
        exit(-1) ;
    }
    close(fp);

    printf("%s, %d\n", __FUNCTION__, __LINE__);
}

int main(int argc, char **argv)
{
    int cmd = 0;
    if (argc != 2)
    {
        printf("Err: invalid param!\r\n");
        return -1;
    }

	cmd = atoi(argv[1]);
    if(cmd)
    {
        strncpy(device_new_console,"/dev/ttyS0",10);
        /*if(0 == access("/data/log_all",0))
        {
            system("rm -f /data/log_all");
        }*/
        system("echo 0 > /sys/module/printk/parameters/disable_uart");
    }
    else
    {
        strncpy(device_new_console,"/dev/tty",10);
        /*if(0 != access("/data/log_all",0))
        {
            system("touch /data/log_all");
        }*/
        system("echo 1 > /sys/module/printk/parameters/disable_uart");
    }
    printf("open device:%s!\r\n",device_new_console);

    ChangeConsole();
    
	return 0;
}


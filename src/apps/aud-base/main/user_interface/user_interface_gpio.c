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
#include <unistd.h>  
#include <fcntl.h>  
#include <poll.h>  

#include "u_gpio.h"
#include "user_interface_key.h"
  
 /**************************************************************** 
 * Constants 
 ****************************************************************/  
   
#define SYSFS_GPIO_DIR "/sys/class/gpio"  
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */  
#define MAX_BUF 64  
  
/**************************************************************** 
 * gpio_export 
 ****************************************************************/  
int gpio_export(unsigned int gpio)  
{  
    int fd, len;  
    char buf[MAX_BUF];  
   
    fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);  
    if (fd < 0) {  
        perror("gpio/export");  
        return fd;  
    }  
   
    len = snprintf(buf, sizeof(buf), "%d", gpio);  
    write(fd, buf, len);  
    close(fd);  
   
    return 0;  
}  
  
/**************************************************************** 
 * gpio_unexport 
 ****************************************************************/  
int gpio_unexport(unsigned int gpio)  
{  
    int fd, len;  
    char buf[MAX_BUF];  
   
    fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);  
    if (fd < 0) {  
        perror("gpio/export");  
        return fd;  
    }  
   
    len = snprintf(buf, sizeof(buf), "%d", gpio);  
    write(fd, buf, len);  
    close(fd);  
    return 0;  
}  
  
/**************************************************************** 
 * gpio_set_dir  
    set gpio direction  input or output
 ****************************************************************/  
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)  
{  
    int fd, len;  
    char buf[MAX_BUF];  
   
    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);  
   
    fd = open(buf, O_WRONLY);  
    if (fd < 0) {  
        perror("gpio/direction");  
        return fd;  
    }  
   
    if (out_flag)  
    {
        write(fd, "out", 4);  
    }
    else  
    {
        write(fd, "in", 3);  
    }
    close(fd);  
    return 0;  
}  
  
/**************************************************************** 
 * gpio_set_value 
 ****************************************************************/  
int gpio_set_value(unsigned int gpio, unsigned int value)  
{  
    int fd, len;  
    char buf[MAX_BUF];  
   
    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);  
   
    fd = open(buf, O_WRONLY);  
    if (fd < 0) {  
        perror("gpio/set-value");  
        return fd;  
    }  
   
    if (value)  
    {
        write(fd, "1", 2);  
    }
    else  
    {
        write(fd, "0", 2);  
    }  
    close(fd);  
    return 0;  
}  
  
/*----------------------------------------------------------------------------
 * Function: gpio_get_value
 * Description:
 *      The GPIO input reading functions. It will check the gpio and read
 *      related gpio device file.
 * Inputs:
 *      gpio: the gpio number to read.
         
 * Outputs:
 * Returns:
 *      GPIO input value.
 *---------------------------------------------------------------------------*/
int gpio_get_value(unsigned int gpio)  
{  
    int fd, len, val;  
    char buf[MAX_BUF];  
    char ch;  
  
    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);  
   
    fd = open(buf, O_RDONLY);  
    if (fd < 0) {  
        perror("gpio/get-value");  
        return fd;  
    }  
   
    read(fd, &ch, 1);  
  
    if (ch != '0') {  
        val = 1;  
    } else {  
        val = 0;  
    }  
   
    close(fd);  
    return val;  
}  
  
  
/**************************************************************** 
 * gpio_set_edge 
 ****************************************************************/  
  
int gpio_set_edge(unsigned int gpio, char *edge)  
{  
    int fd, len;  
    char buf[MAX_BUF];  
  
    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);  
   
    fd = open(buf, O_WRONLY);  
    if (fd < 0) {  
        perror("gpio/set-edge");  
        return fd;  
    }  
   
    write(fd, edge, strlen(edge) + 1);   
    close(fd);  
    return 0;  
}  
  
/**************************************************************** 
 * gpio_fd_open 
 ****************************************************************/  
  
int gpio_fd_open(unsigned int gpio)  
{  
    int fd, len;  
    char buf[MAX_BUF];  
  
    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);  
   
    fd = open(buf, O_RDONLY | O_NONBLOCK );  
    if (fd < 0) {  
        perror("gpio/fd_open");  
    }  
    return fd;  
}  
  
/**************************************************************** 
 * gpio_fd_close 
 ****************************************************************/  
  
int gpio_fd_close(int fd)  
{  
    return close(fd);  
}  


/*----------------------------------------------------------------------------
 * Function: gpio_config
 * Description:
 *   Configure GPIO pin. It sets pin as gpio¡Boutput or input mode and
 *   set output value if output mode.
 * Inputs:
 *      i4GpioNum: the gpio number to be set.
 *      i4Output:  If the integer is 0, this function will set the mode of the
 *                 gpio number as input mode, otherwise set as output mode.
 *      i4High: In output mode,if the integer is 0, this function will set the
 *              bit of the gpio number as 0, otherwise set as 1.
                If input mode, ignore it.
 * Outputs:
 * Returns:
 *---------------------------------------------------------------------------*/ 
void gpio_config(unsigned int i4GpioNum, unsigned int i4OutIn, unsigned int i4High)
{  
    i4GpioNum = GPIO_BASE_VALUE + i4GpioNum;
    gpio_export(i4GpioNum);
    gpio_set_dir(i4GpioNum, i4OutIn);
    gpio_set_value(i4GpioNum, i4High);
}


#if 0 
/**************************************************************** 
 * Main 
 ****************************************************************/  
int main(int argc, char **argv, char **envp)  
{  
    struct pollfd fdset[2];  
    int nfds = 2;  
    int gpio_fd, timeout, rc;  
    char *buf[MAX_BUF];  
    unsigned int gpio;  
    int len;  
  
  
  
    if (argc < 2) {  
        printf("Usage: gpio-int <gpio-pin>\n\n");  
        printf("Waits for a change in the GPIO pin voltage level or input on stdin\n");  
        exit(-1);  
    }  
  
    gpio = atoi(argv[1]);  
  
    gpio_export(gpio);  
    gpio_set_dir(gpio, 0);  
    gpio_set_edge(gpio, "rising");  
    gpio_fd = gpio_fd_open(gpio);  
  
    timeout = POLL_TIMEOUT;  
   
    while (1) {  
        memset((void*)fdset, 0, sizeof(fdset));  
  
        fdset[0].fd = STDIN_FILENO;  
        fdset[0].events = POLLIN;  
        
        fdset[1].fd = gpio_fd;  
        fdset[1].events = POLLPRI;  
  
        rc = poll(fdset, nfds, timeout);        
  
        if (rc < 0) {  
            printf("\npoll() failed!\n");  
            return -1;  
        }  
        
        if (rc == 0) {  
            printf(".");  
        }  
              
        if (fdset[1].revents & POLLPRI) {  
            len = read(fdset[1].fd, buf, MAX_BUF);  
            printf("\npoll() GPIO %d interrupt occurred\n", gpio);  
        }  
  
        if (fdset[0].revents & POLLIN) {  
            (void)read(fdset[0].fd, buf, 1);  
            printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);  
        }  
  
        fflush(stdout);  
    }  
  
    gpio_fd_close(gpio_fd);  
    return 0;  
}
#endif
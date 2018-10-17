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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utility.h"


/*******************************************************************************/
/* UTILITY local definitions                                                   */
/*******************************************************************************/
#define SYS_Printf printf

/*******************************************************************************/
/* UTILITY local prototypes                                                    */
/*******************************************************************************/


/*******************************************************************************/
/* UTILITY local variables                                                     */
/*******************************************************************************/


/*******************************************************************************/
/* UTILITY local functions                                                     */
/*******************************************************************************/


/*******************************************************************************/
/* UTILITY functions                                                           */
/*******************************************************************************/
ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;
  ssize_t tot = 0;
  ssize_t localcnt = count;
  //SYS_Printf("Safe read tot : %d !!\n", count);

	do {
		n = read(fd, buf, localcnt);
        //SYS_Printf("Safe read : %d, fd : %d !!\n", n, fd);
    if (n < 0)
    {
      if (errno == EINTR)
            {
        n = 0;
            }
      else
      {
        SYS_Printf("Safe read error : %s !!\n", (char *)strerror(errno));
        return n;
      }
    }
        
    tot += n;
    if (tot >= count)
        {
      return count;
        }
    else
    {
      localcnt -= n;
      buf   += n;
      //SYS_Printf("Safe read cnt : %d --- tot: %d \n", n, tot);
    }
	} while (localcnt > 0);

	return n;
}


ssize_t safe_write(int fd, const void *buf, size_t count)
{
	ssize_t n;
  ssize_t tot = 0;
  ssize_t localcnt = count;
  //SYS_Printf("Safe write tot : %d !!\n", count);

	do {
		n = write(fd, buf, localcnt);
        //SYS_Printf("Safe write : %d , fd : %d!!\n", n, fd);
    if (n < 0)
    {
      if (errno == EINTR)
            {
        n = 0;
            }
      else
      {
        SYS_Printf("Safe write error : %s !!\n", (char *)strerror(errno));
        return n;
      }
    }
        
    tot += n;
    if (tot >= count)
        {
      return count;
        }
    else
    {
      localcnt -= n;
      buf   += n;
      //SYS_Printf("Safe write cnt : %d --- tot : %d \n", n, tot);
    }
	} while (localcnt > 0);

	return n;
}


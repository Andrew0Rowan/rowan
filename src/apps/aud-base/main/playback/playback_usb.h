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


#ifndef _PLAYBACK_USB_H_
#define _PLAYBACK_USB_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"
#include "u_dbg.h"
#include "u_handle.h"
#include "u_playback_usb.h"
/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
#undef   DBG_LEVEL_MODULE
#define  DBG_LEVEL_MODULE       playback_usb_get_dbg_level()

#define PB_USB_OK                            ((INT32)0)
#define PB_USB_FAIL                          ((INT32)-1) /* abnormal return must < 0 */
#define PB_USB_INV_ARG                       ((INT32)-2)

#define MAX_WAV_FILE_LEN  1024
//#define MAIN_PCM_DEVICE_NAME "pri"
#define MAIN_PCM_DEVICE_NAME "default"
#define PLAYLIST_PATH "/tmp/wav.list"

/* application structure */
typedef struct _PLAYBACK_USB_OBJ_T
{
    HANDLE_T        h_app;
    HANDLE_T        h_iom;
    BOOL            b_app_init_ok;
    BOOL            b_recv_iom_msg;
    BOOL            b_isPause;
    BOOL            b_pb_thread_running;
    BOOL            b_isSwitch;
    UINT32          wav_num;
    UINT32          cur_file_index;
}PLAYBACK_USB_OBJ_T;


typedef struct Wav_DoubleLinkedList
{
    int   index;
    char *data;
    struct Wav_DoubleLinkedList *pre;
    struct Wav_DoubleLinkedList *next;
}Wav_Play_List;

/*-----------------------------------------------------------------------------
                    functions declarations
-----------------------------------------------------------------------------*/
BOOL playback_usb_get_auto_play(VOID);
VOID playback_usb_set_auto_play(BOOL b_auto_play);



#endif /* _PLAYBACK_USB_H_ */

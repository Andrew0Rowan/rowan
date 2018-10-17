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


/*-----------------------------------------------------------------------------
 * $RCSfile: assistant_stub_app_tts_recv.c $
 * $Revision:0.1
 * $Date: 2017/01/06
 * $Author:pingan.liu
 * $CCRevision: $
 * $SWAuthor:  $
 * $MD5HEX: $
 *
 * Description:assistance stub app tts recv thread


 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "assistant_stub_app_tts_recv.h"

#include "u_ringbuf.h"
#include "u_playback_tts.h"

#define COMPRESS_RATIO  7
#define BUFFER_TIME     20
#define BUFFER_SIZE     (BUFFER_TIME * PLAYBACK_TTS_SAMPLERATE * PLAYBACK_TTS_CHANNLE * PLAYBACK_TTS_BITWIDTH / 8 / COMPRESS_RATIO)

RINGBUF_H h_tts_stream_ringbuf;
static BOOL g_b_tts_stream_finish = TRUE;

BOOL u_assistant_stub_is_tts_stream_finish(VOID)
{
    return g_b_tts_stream_finish;
}

void _assistant_stub_tts_recv_thread(VOID* arg)
{
	FUNCTION_BEGIN
    int ret;
    int write_size;
    int server_fifo_fd;
    bin_msg msg;
    FILE * save_file;

    save_file = fopen(SAVE_FILE_NAME, "w");
    if(NULL == save_file)
	{
        printf("<ASSISTANT_STUB_APP> open save file failure!\n");
        return;
    }

START:
    if(-1 == access(SERVER_FIFO_NAME, F_OK))
	{
        if (-1 == mkfifo(SERVER_FIFO_NAME, 0777))
        {
            fprintf(stderr, "<ASSISTANT_STUB_APP> Sorry, create server fifo failure!/n");
            goto MKFIFO_ERR;
        }
    }

    printf("<ASSISTANT_STUB_APP> start open fifo!\n");
    server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
    if (-1 == server_fifo_fd)
    {
        fprintf(stderr, "<ASSISTANT_STUB_APP> Sorry, server fifo open failure!/n");
        goto OPEN_FIFO_ERR;
    }
    printf("<ASSISTANT_STUB_APP> fifo open success!\n");

    h_tts_stream_ringbuf = u_ringbuf_malloc(BUFFER_SIZE);
    if (NULL == h_tts_stream_ringbuf)
    {
        printf("<ASSISTANT_STUB_APP> u_ringbuf_malloc failed!\n");
        goto MALLOC_RINGBUF_ERR;
    }

    sleep(1);

    while (read(server_fifo_fd, &msg, sizeof(msg)) > 0)
    {
        printf("<ASSISTANT_STUB_APP>msgType is %d, dataLen is %d, requestId=%d\n",
			   msg.msgType,
			   msg.dataLen,
			   msg.requestId);

        g_b_tts_stream_finish = FALSE;
        if(msg.dataLen > 0)
		{
            //fwrite(msg.buffer, 1, msg.dataLen, save_file);
            write_size = 0;
            while (msg.dataLen)
            {
                ret = u_ringbuf_write(h_tts_stream_ringbuf, msg.buffer + write_size, msg.dataLen);
                msg.dataLen -= ret;
                write_size += ret;
            }
        }

        if(TTS_BIN_MSG_FINISH == msg.msgType)
		{
            g_b_tts_stream_finish = TRUE;
            //fclose(save_file);
        }
    }

    printf("<ASSISTANT_STUB_APP> _assistant_stub_tts_recv_thread exit \n");

    //u_ringbuf_free(h_tts_stream_ringbuf);
MALLOC_RINGBUF_ERR:
    close(server_fifo_fd);
    goto START;
OPEN_FIFO_ERR:
MKFIFO_ERR:
    fclose(save_file);
	FUNCTION_END
	pthread_exit(NULL);
}


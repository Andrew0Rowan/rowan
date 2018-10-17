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


#ifndef __U_ALSA_INTERFACE_H__
#define __U_ALSA_INTERFACE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/byteorder.h>
#include <alsa/asoundlib.h>

/* Definitions for Microsoft WAVE format */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d)    ((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)        (v)
#define LE_INT(v)        (v)
#define BE_SHORT(v)        bswap_16(v)
#define BE_INT(v)        bswap_32(v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d)    ((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)        bswap_16(v)
#define LE_INT(v)        bswap_32(v)
#define BE_SHORT(v)        (v)
#define BE_INT(v)        (v)
#else
#error "Wrong endian"
#endif

typedef long long off64_t_b;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

typedef struct PCMContainer {
	snd_pcm_t *handle;              /*need to set*/
	snd_output_t *log;
	snd_pcm_uframes_t chunk_size;   /*auto calc*/
	snd_pcm_uframes_t buffer_size;  /*auto calc*/
	snd_pcm_format_t format;        /*need to set*/
	uint16_t channels;              /*need to set*/
	size_t chunk_bytes;
	size_t bits_per_sample;
	size_t bits_per_frame;
	size_t sample_rate;	            /*need to set*/
	off64_t_b chunk_count;            /*usb wav need to set */

	uint8_t *data_buf;
} PCMContainer_t;

/*Definitions extern function*/
extern ssize_t         u_alsa_read_pcm(PCMContainer_t *sndpcm, size_t rcount);
extern ssize_t         u_alsa_write_pcm(PCMContainer_t *sndpcm, size_t wcount);
extern ssize_t         u_alsa_write_pcm_for_anysize(PCMContainer_t *sndpcm, size_t wcount);
extern int             u_alsa_set_hw_params(PCMContainer_t *pcm_params, uint32_t ui4_max_buffer_time);
extern int             u_alsa_set_hw_params_adjust_peroid_time(PCMContainer_t *pcm_params, uint32_t ui4_max_buffer_time,uint32_t proportion);
extern int             u_alsa_set_sw_params(PCMContainer_t *pcm_params, snd_pcm_uframes_t val);
extern snd_pcm_state_t u_alsa_get_pcm_state(snd_pcm_t *handle);
extern int             u_alsa_pause(snd_pcm_t *handle);
extern int             u_alsa_resume(snd_pcm_t *handle);

#endif /* #ifndef __U_ALSA_INTERFACE_H__ */

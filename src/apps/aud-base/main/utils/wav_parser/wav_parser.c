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


#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <alsa/asoundlib.h>
#include "u_dbg.h"
#include "u_wav_parser.h"


#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif

#define WAV_DEBUG(x) printf x
#define WAV_DEBUG_INFO(x) printf x


/* global data */
static off64_t_b pbrec_count = LLONG_MAX;


static struct {
    snd_pcm_format_t format;
    unsigned int channels;
    unsigned int sample_rate;
} hwparams;


#define check_wavefile_space(buffer, len, blimit) \
    if (len > blimit) { \
        blimit = len; \
        if ((buffer = realloc(buffer, blimit)) == NULL) { \
            WAV_DEBUG((WAV_DEBUG_TAG"not enough memory\n"));         \
        } \
    }

/*
 * Safe read (for pipes)
 */

static ssize_t safe_read(int fd, void *buf, size_t count)
{
    ssize_t result = 0, res;

    while (count > 0) {
        if ((res = read(fd, buf, count)) == 0)
            break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}


static size_t wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize, int line)
{
    if (*size >= reqsize)
        return *size;
    if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
        WAV_DEBUG((WAV_DEBUG_TAG"read error -- called from line %d\n",line));
    }
    return *size = reqsize;
}


char *WAV_P_FmtString(uint16_t fmt)
{
	switch (fmt) {
	case WAV_FMT_PCM:
		return "WAV_FMT_PCM";
		break;
	case WAV_FMT_IEEE_FLOAT:
		return "WAV_FMT_IEEE_FLOAT";
		break;
	case WAV_FMT_DOLBY_AC3_SPDIF:
		return "WAV_FMT_DOLBY_AC3_SPDIF";
		break;
	case WAV_FMT_EXTENSIBLE:
		return "WAV_FMT_EXTENSIBLE";
		break;
	default:
		break;
	}

	return "NON Support Fmt";
}


/*
 * test, if it's a .WAV file, > 0 if ok (and set the speed, stereo etc.)
 * == 0 if not
 * Value returned is bytes to be discarded.
 */
static ssize_t parser_wavefile(int fd, u_char *_buffer, size_t size)
{
    WaveHeader *h = (WaveHeader *)_buffer;
    u_char *buffer = NULL;
    size_t blimit = 0;
    WaveFmtBody *f;
    WaveChunkHeader *c;
    u_int type, len;

    /*RIFF chunk check*/
    if (size < sizeof(WaveHeader))
        return -1;
    if (h->magic != WAV_RIFF || h->type != WAV_WAVE)
        return -1;

	WAV_DEBUG((WAV_DEBUG_TAG"**********************************************************\n"));
	WAV_DEBUG((WAV_DEBUG_TAG"File Magic:         [%c%c%c%c]\n",
		(char)(h->magic),
		(char)(h->magic>>8),
		(char)(h->magic>>16),
		(char)(h->magic>>24)));
	WAV_DEBUG((WAV_DEBUG_TAG"File Length:        [%d]\n", h->length));
	WAV_DEBUG((WAV_DEBUG_TAG"File Type:          [%c%c%c%c]\n",
		(char)(h->type),
		(char)(h->type>>8),
		(char)(h->type>>16),
		(char)(h->type>>24)));

    if (size > sizeof(WaveHeader)) {
        check_wavefile_space(buffer, size - sizeof(WaveHeader), blimit);
        memcpy(buffer, _buffer + sizeof(WaveHeader), size - sizeof(WaveHeader));
    }
    size -= sizeof(WaveHeader);

    /*FMT chunk check*/
    while (1) {
        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = LE_INT(c->length);
        len += len % 2;
        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);

        if (type == WAV_FMT){
            break;
        }

        check_wavefile_space(buffer, len, blimit);
        wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    if (len < sizeof(WaveFmtBody)) {
        WAV_DEBUG((WAV_DEBUG_TAG"unknown length of 'fmt ' chunk -- read %u, should be %u at least\n",
         len, (u_int)sizeof(WaveFmtBody)));
		return -2;
    }
    check_wavefile_space(buffer, len, blimit);
    wavefile_read(fd, buffer, &size, len, __LINE__);
    f = (WaveFmtBody*) buffer;


	WAV_DEBUG((WAV_DEBUG_TAG"Fmt Format:         [%s]\n", WAV_P_FmtString(f->format)));
	WAV_DEBUG((WAV_DEBUG_TAG"Fmt Channels:       [%d]\n", f->channels));
	WAV_DEBUG((WAV_DEBUG_TAG"Fmt Sample_rate:    [%d](HZ)\n",f->sample_fq));
	WAV_DEBUG((WAV_DEBUG_TAG"Fmt Bytes_p_second: [%d]\n", f->byte_p_sec));
	WAV_DEBUG((WAV_DEBUG_TAG"Fmt Blocks_align:   [%d]\n", f->byte_p_spl));
	WAV_DEBUG((WAV_DEBUG_TAG"Fmt BitsPerSample:  [%d]\n", f->bit_p_spl));

    if (LE_SHORT(f->format) == WAV_FMT_EXTENSIBLE) {
        WaveFmtExtensibleBody *fe = (WaveFmtExtensibleBody*)buffer;
        if (len < sizeof(WaveFmtExtensibleBody)) {
            WAV_DEBUG((WAV_DEBUG_TAG"unknown length of extensible 'fmt ' chunk--read %u, should be %u at least\n",len, (u_int)sizeof(WaveFmtExtensibleBody)));
			return -2;
        }
        if (memcmp(fe->guid_tag, WAV_GUID_TAG, 14) != 0) {
            WAV_DEBUG((WAV_DEBUG_TAG"wrong format tag in extensible 'fmt ' chunk\n"));
			return -2;
        }
        f->format = fe->guid_format;
    }
        if (LE_SHORT(f->format) != WAV_FMT_PCM &&
            LE_SHORT(f->format) != WAV_FMT_IEEE_FLOAT) {
                WAV_DEBUG((WAV_DEBUG_TAG"can't play WAVE-file format 0x%04x which is not PCM or FLOAT encoded\n", LE_SHORT(f->format)));
				return -2;
    }
    if (LE_SHORT(f->channels) < 1) {
        WAV_DEBUG((WAV_DEBUG_TAG"can't play WAVE-files with %d tracks\n", LE_SHORT(f->channels)));
		return -2;
    }
    hwparams.channels = LE_SHORT(f->channels);

    switch (LE_SHORT(f->bit_p_spl)) {
    case 8:
        if (hwparams.format != DEFAULT_FORMAT &&
            hwparams.format != SND_PCM_FORMAT_U8)
            WAV_DEBUG((WAV_DEBUG_TAG"format is changed to U8\n"));
        	hwparams.format = SND_PCM_FORMAT_U8;
        break;
    case 16:
        if (hwparams.format != DEFAULT_FORMAT &&
         	hwparams.format != SND_PCM_FORMAT_S16_LE)
            WAV_DEBUG((WAV_DEBUG_TAG"format is changed to S16_LE\n"));
        hwparams.format = SND_PCM_FORMAT_S16_LE;
        break;
    case 24:
        switch (LE_SHORT(f->byte_p_spl) / hwparams.channels) {
        case 3:
            if (hwparams.format != DEFAULT_FORMAT &&
             	hwparams.format != SND_PCM_FORMAT_S24_3LE)
                WAV_DEBUG((WAV_DEBUG_TAG"format is changed to S24_3LE\n"));
            hwparams.format = SND_PCM_FORMAT_S24_3LE;
            break;
        case 4:
            if (hwparams.format != DEFAULT_FORMAT &&
             	hwparams.format != SND_PCM_FORMAT_S24_LE)
                WAV_DEBUG((WAV_DEBUG_TAG"format is changed to S24_LE\n"));
            hwparams.format = SND_PCM_FORMAT_S24_LE;
            break;
        default:
            WAV_DEBUG((WAV_DEBUG_TAG"can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)\n",
             LE_SHORT(f->bit_p_spl), LE_SHORT(f->byte_p_spl), hwparams.channels));
			return -2;
        }
        break;
    case 32:
            if (LE_SHORT(f->format) == WAV_FMT_PCM)
            {
                hwparams.format = SND_PCM_FORMAT_S32_LE;
				WAV_DEBUG((WAV_DEBUG_TAG"format is changed to S32_LE\n"));
            }
            else if (LE_SHORT(f->format) == WAV_FMT_IEEE_FLOAT)
            {
                hwparams.format = SND_PCM_FORMAT_FLOAT_LE;
				WAV_DEBUG((WAV_DEBUG_TAG"format is changed to FLOAT_LE\n"));
            }
        break;
    default:
        WAV_DEBUG((WAV_DEBUG_TAG" can't play WAVE-files with sample %d bits wide\n",
         LE_SHORT(f->bit_p_spl)));
		return -2;
    }
    hwparams.sample_rate = LE_INT(f->sample_fq);
	WAV_DEBUG((WAV_DEBUG_TAG"PCM Type is:  [%s]\n", snd_pcm_format_name(hwparams.format)));
    if (size > len)
        memmove(buffer, buffer + len, size - len);
    size -= len;


    while (1) {
        u_int type, len;

        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = LE_INT(c->length);

        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));

        size -= sizeof(WaveChunkHeader);

        if (type == WAV_DATA) {
            if (len < pbrec_count && len < 0x7ffffffe)
                pbrec_count = len;
            if (size > 0)
                memcpy(_buffer, buffer, size);
            free(buffer);
			WAV_DEBUG((WAV_DEBUG_TAG"Data count is:  [%lld]\n",pbrec_count));
			WAV_DEBUG((WAV_DEBUG_TAG"**********************************************************\n"));
            return size;
        }
        len += len % 2;
        check_wavefile_space(buffer, len, blimit);
        wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    /* shouldn't be reached */
    return -1;
}

int u_wave_file_get_pcm_container(int fd, PCMContainer_t * PCMContainer)
{
    size_t dta;
    ssize_t dtawave;
	u_char *audiobuf = NULL;

	assert((fd >=0) && PCMContainer);

    pbrec_count = LLONG_MAX;

    audiobuf = (u_char *)malloc(1024);

    if (NULL == audiobuf) {
        WAV_DEBUG((WAV_DEBUG_TAG"malloc audiobuf error in func %s : line %d\n",__FUNCTION__,__LINE__));
        return -1;
    }

    /* read the file header */
    dta = sizeof(WaveHeader);
    if ((size_t)safe_read(fd, audiobuf, dta) != dta) {
        WAV_DEBUG((WAV_DEBUG_TAG"read error in func %s : line %d\n",__FUNCTION__,__LINE__));
        return -2;
    }
    /* read bytes for WAVE-header */
    if ((dtawave = parser_wavefile(fd, audiobuf, dta)) >= 0) {
        WAV_DEBUG((WAV_DEBUG_TAG"%s success at line: %d\n",__FUNCTION__,__LINE__));
		PCMContainer->format = hwparams.format;
		PCMContainer->channels = hwparams.channels;
	 	PCMContainer->sample_rate = hwparams.sample_rate;
		PCMContainer->chunk_count = pbrec_count;
    } else{
        WAV_DEBUG((WAV_DEBUG_TAG"unsupport wav format in func %s : line %d\n",__FUNCTION__,__LINE__));
		return -3;
    }

	WAV_DEBUG((WAV_DEBUG_TAG"%s dtawave is %d\n",__FUNCTION__,dtawave));

    if(NULL != audiobuf)
    {
      free(audiobuf);
	  audiobuf = NULL;
    }
	return 0;
}

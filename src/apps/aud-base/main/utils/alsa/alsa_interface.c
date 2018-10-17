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


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "u_alsa_interface.h"

#define PB_ALSA_DEBUG_TAG "<playback_alsa>"

#define ALSA_OK             0
#define ALSA_FAIL          -1

#define PB_ALSA_ERR(x)  printf x
#define PB_ALSA_INFO(x) printf x

#define PCM_HANDLE_CHECK(handle) do{  \
									if(NULL == handle) \
									{  PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"%s -- Invail pcm handle fail\n",__FUNCTION__)); \
										return ALSA_FAIL;}  \
									}while(0)

/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_ReadPCM
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -ok:write pcm size  fail or pause:-1
 *---------------------------------------------------------------------------*/

ssize_t u_alsa_read_pcm(PCMContainer_t *sndpcm, size_t rcount)
{
	ssize_t r;
	size_t result = 0;
	size_t count = rcount;
	uint8_t *data = sndpcm->data_buf;

	if (count != sndpcm->chunk_size) {
		count = sndpcm->chunk_size;
	}

	while (count > 0) {
		r = snd_pcm_readi(sndpcm->handle, data, count);

		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(sndpcm->handle, 1000);
		} else if (r == -EPIPE) {
			snd_pcm_prepare(sndpcm->handle);
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n"));
		} else if (r == -ESTRPIPE) {
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>\n"));
		} else if (r < 0) {
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_readi: [%s]\n", snd_strerror(r)));
			return -1;
		}

		if (r > 0) {
			result += r;
			count -= r;
			data += r * sndpcm->bits_per_frame / 8;
		}
	}
	return rcount;
}



/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_WritePCM
 * Description      -please input samples equal with period size,if not, silence data will fill. 
 * Input arguments  -
 * Output arguments -
 * Returns          -ok:write pcm size  fail or pause:-1
 *---------------------------------------------------------------------------*/

ssize_t u_alsa_write_pcm(PCMContainer_t *sndpcm, size_t wcount)
{
	ssize_t r;
	ssize_t result = 0;
	uint8_t *data = sndpcm->data_buf;

	if (wcount < sndpcm->chunk_size) {
		snd_pcm_format_set_silence(sndpcm->format,
			data + wcount * sndpcm->bits_per_frame / 8,
			(sndpcm->chunk_size - wcount) * sndpcm->channels);
		wcount = sndpcm->chunk_size;
	}
	while (wcount > 0) {
        //PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_writei start wcount:%d data:%p\n", wcount, data));
		r = snd_pcm_writei(sndpcm->handle, data, wcount);
        //PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_writei end r:%d\n", r));
		if (r == -EAGAIN || (r >= 0 && (size_t)r < wcount)) {
            PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_writei r:%d\n", r));
			snd_pcm_wait(sndpcm->handle, 1000);
		} else if (r == -EPIPE) {
			 PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n"));
			 r = snd_pcm_prepare(sndpcm->handle);
			 if (r < 0)
			   PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Can't recovery from underrun, prepare failed: [%s]\n",snd_strerror(r)));
		} else if (r == -ESTRPIPE) {
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"<<<<<<<<<<<<<<< Need suspend [%s]>>>>>>>>>>>>>>>\n",snd_strerror(r)));
		} else if (r < 0) {
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_writei err: [%s]\n",snd_strerror(r)));
			return -1;
		}
		if (r > 0) {
			result += r;
			wcount -= r;
			data += r * sndpcm->bits_per_frame / 8;
		}
	}
	return result;
}

/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_WritePCM
 * Description      -please input any samples, which can be less than period size  
 * Input arguments  -
 * Output arguments -
 * Returns          -ok:write pcm size  fail or pause:-1
 *---------------------------------------------------------------------------*/

ssize_t u_alsa_write_pcm_for_anysize(PCMContainer_t *sndpcm, size_t wcount)
{
	ssize_t r;
	ssize_t result = 0;
	uint8_t *data = sndpcm->data_buf;

	while (wcount > 0) {
        //PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_writei start wcount:%d data:%p\n", wcount, data));
		r = snd_pcm_writei(sndpcm->handle, data, wcount);
        //PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_writei end r:%d\n", r));
		if (r == -EAGAIN || (r >= 0 && (size_t)r < wcount)) {
            PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_writei r:%d\n", r));
			snd_pcm_wait(sndpcm->handle, 1000);
		} else if (r == -EPIPE) {
			 PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n"));
			 r = snd_pcm_prepare(sndpcm->handle);
			 if (r < 0)
			   PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Can't recovery from underrun, prepare failed: [%s]\n",snd_strerror(r)));
		} else if (r == -ESTRPIPE) {
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"<<<<<<<<<<<<<<< Need suspend [%s]>>>>>>>>>>>>>>>\n",snd_strerror(r)));
		} else if (r < 0) {
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_writei err: [%s]\n",snd_strerror(r)));
			return -1;
		}
		if (r > 0) {
			result += r;
			wcount -= r;
			data += r * sndpcm->bits_per_frame / 8;
		}
	}

	return result;
}

/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_SetHWParams
 * Description      -period time is 1/4 of buffer time by default
 * Input arguments  -
 * Output arguments -
 * Returns          - OK(0)  PARAMS ERR(-1)
 *---------------------------------------------------------------------------*/

int u_alsa_set_hw_params(PCMContainer_t *pcm_params, uint32_t ui4_max_buffer_time)
{
	snd_pcm_hw_params_t *hwparams;
	uint32_t exact_rate;
	uint32_t buffer_time, period_time;
	int 	 err;

	PCM_HANDLE_CHECK(pcm_params->handle);

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);

	/* Fill it with default values */
	err = snd_pcm_hw_params_any(pcm_params->handle, hwparams);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_any : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

    /* Interleaved mode */
	err = snd_pcm_hw_params_set_access(pcm_params->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_access : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set sample format */
	err = snd_pcm_hw_params_set_format(pcm_params->handle, hwparams,pcm_params->format);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_format : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set number of channels */
	err = snd_pcm_hw_params_set_channels(pcm_params->handle, hwparams, LE_SHORT(pcm_params->channels));
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_channels : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */
	exact_rate = LE_INT(pcm_params->sample_rate);
	err = snd_pcm_hw_params_set_rate_near(pcm_params->handle, hwparams, &exact_rate, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_rate_near : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}
	if (LE_INT(pcm_params->sample_rate) != exact_rate) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"The rate %d Hz is not supported by your hardware.\n ==> Using %d Hz instead.\n",
			LE_INT(pcm_params->sample_rate), exact_rate));
	}

	err = snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_get_buffer_time_max : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_hw_params_get_buffer_time_max : %ul (us)\n",buffer_time));

	if (buffer_time > ui4_max_buffer_time)
		buffer_time = ui4_max_buffer_time;

	if (buffer_time > 0)
		period_time = buffer_time / 4;

	err = snd_pcm_hw_params_set_buffer_time_near(pcm_params->handle, hwparams, &buffer_time, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_buffer_time_near : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	err = snd_pcm_hw_params_set_period_time_near(pcm_params->handle, hwparams, &period_time, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_period_time_near : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set hw params */
	err = snd_pcm_hw_params(pcm_params->handle, hwparams);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params: %s at line->%d\n",snd_strerror(err),__LINE__));
		goto ERR_SET_PARAMS;
	}

	snd_pcm_hw_params_get_period_size(hwparams, &pcm_params->chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(hwparams, &pcm_params->buffer_size);
	if (pcm_params->chunk_size == pcm_params->buffer_size) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Can't use period equal to buffer size (%lu == %lu)\n", pcm_params->chunk_size, pcm_params->buffer_size));
		goto ERR_SET_PARAMS;
	}

	PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"chunk_size is %lu, buffer size is %lu\n", pcm_params->chunk_size, pcm_params->buffer_size));

	/*bits per sample = bits depth*/
	pcm_params->bits_per_sample = snd_pcm_format_physical_width(pcm_params->format);

	/*bits per frame = bits depth * channels*/
	pcm_params->bits_per_frame = pcm_params->bits_per_sample * LE_SHORT(pcm_params->channels);

	/*chunk byte is a better size for each write or read for alsa*/
	pcm_params->chunk_bytes = pcm_params->chunk_size * pcm_params->bits_per_frame / 8;

	/* Allocate audio data buffer */
	pcm_params->data_buf = (uint8_t *)malloc(pcm_params->chunk_bytes);
	if (!pcm_params->data_buf) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error malloc: [data_buf] at line-> %d\n",__LINE__));
		goto ERR_SET_PARAMS;
	}

	return 0;

ERR_SET_PARAMS:
	if(NULL != pcm_params->data_buf) {
		free(pcm_params->data_buf);
		pcm_params->data_buf = NULL;
	}
	snd_pcm_close(pcm_params->handle);
	pcm_params->handle = NULL;
	return -1;
}
/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_SetHWParams
 * Description      -adjust the period time proportion of buffertime by parmas porportion
 *                  -the minimum value of proportion is 2.
 * Input arguments  -
 * Output arguments -
 * Returns          - OK(0)  PARAMS ERR(-1)
 *---------------------------------------------------------------------------*/

int u_alsa_set_hw_params_adjust_peroid_time(PCMContainer_t *pcm_params, uint32_t ui4_max_buffer_time,uint32_t proportion)
{
	snd_pcm_hw_params_t *hwparams;
	uint32_t exact_rate;
	uint32_t buffer_time, period_time;
	int 	 err;

	PCM_HANDLE_CHECK(pcm_params->handle);

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);

	/* Fill it with default values */
	err = snd_pcm_hw_params_any(pcm_params->handle, hwparams);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_any : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

    /* Interleaved mode */
	err = snd_pcm_hw_params_set_access(pcm_params->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_access : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set sample format */
	err = snd_pcm_hw_params_set_format(pcm_params->handle, hwparams,pcm_params->format);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_format : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set number of channels */
	err = snd_pcm_hw_params_set_channels(pcm_params->handle, hwparams, LE_SHORT(pcm_params->channels));
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_channels : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */
	exact_rate = LE_INT(pcm_params->sample_rate);
	err = snd_pcm_hw_params_set_rate_near(pcm_params->handle, hwparams, &exact_rate, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_rate_near : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}
	if (LE_INT(pcm_params->sample_rate) != exact_rate) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"The rate %d Hz is not supported by your hardware.\n ==> Using %d Hz instead.\n",
			LE_INT(pcm_params->sample_rate), exact_rate));
	}

	err = snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_get_buffer_time_max : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_hw_params_get_buffer_time_max : %ul (us)\n",buffer_time));

	if (buffer_time > ui4_max_buffer_time)
		buffer_time = ui4_max_buffer_time;

	if (buffer_time > 0)
		period_time = buffer_time / proportion;

	err = snd_pcm_hw_params_set_buffer_time_near(pcm_params->handle, hwparams, &buffer_time, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_buffer_time_near : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	err = snd_pcm_hw_params_set_period_time_near(pcm_params->handle, hwparams, &period_time, 0);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params_set_period_time_near : %s\n",snd_strerror(err)));
		goto ERR_SET_PARAMS;
	}

	/* Set hw params */
	err = snd_pcm_hw_params(pcm_params->handle, hwparams);
	if (err < 0) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error snd_pcm_hw_params: %s at line->%d\n",snd_strerror(err),__LINE__));
		goto ERR_SET_PARAMS;
	}

	snd_pcm_hw_params_get_period_size(hwparams, &pcm_params->chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(hwparams, &pcm_params->buffer_size);
	if (pcm_params->chunk_size == pcm_params->buffer_size) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Can't use period equal to buffer size (%lu == %lu)\n", pcm_params->chunk_size, pcm_params->buffer_size));
		goto ERR_SET_PARAMS;
	}

	PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"chunk_size is %lu, buffer size is %lu\n", pcm_params->chunk_size, pcm_params->buffer_size));

	/*bits per sample = bits depth*/
	pcm_params->bits_per_sample = snd_pcm_format_physical_width(pcm_params->format);

	/*bits per frame = bits depth * channels*/
	pcm_params->bits_per_frame = pcm_params->bits_per_sample * LE_SHORT(pcm_params->channels);

	/*chunk byte is a better size for each write or read for alsa*/
	pcm_params->chunk_bytes = pcm_params->chunk_size * pcm_params->bits_per_frame / 8;

	/* Allocate audio data buffer */
	pcm_params->data_buf = (uint8_t *)malloc(pcm_params->chunk_bytes);
	if (!pcm_params->data_buf) {
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Error malloc: [data_buf] at line-> %d\n",__LINE__));
		goto ERR_SET_PARAMS;
	}

	return 0;

ERR_SET_PARAMS:
	if(NULL != pcm_params->data_buf) {
		free(pcm_params->data_buf);
		pcm_params->data_buf = NULL;
	}
	snd_pcm_close(pcm_params->handle);
	pcm_params->handle = NULL;
	return -1;
}

int u_alsa_set_sw_params(PCMContainer_t *pcm_params, snd_pcm_uframes_t val)
{
    int ret;
    snd_pcm_sw_params_t *ptr = NULL;

    snd_pcm_sw_params_alloca(&ptr);
    if (NULL == ptr)
    {
        PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_sw_params_malloc error!\n"));
        goto MALLOC_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_current(pcm_params->handle, ptr);
    if (ret < 0)
    {
        PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_sw_params_current error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params_set_start_threshold(pcm_params->handle, ptr, val);
    if (ret < 0)
    {
        PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_sw_params_set_start_threshold error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    ret = snd_pcm_sw_params(pcm_params->handle, ptr);
    if (ret < 0)
    {
        PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"snd_pcm_sw_params error ret:[%d]!\n", ret));
        goto GET_PARAMS_ERR;
    }

    return 0;

GET_PARAMS_ERR:
    snd_pcm_sw_params_free(ptr);
MALLOC_PARAMS_ERR:
    return -1;
}

/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_Get_Pcm_State
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns - pcm snd_pcm_state_t
				SND_PCM_STATE_OPEN 	              Open
				SND_PCM_STATE_SETUP 	              Setup installed
				SND_PCM_STATE_PREPARED 	       Ready to start
				SND_PCM_STATE_RUNNING     	       Running
				SND_PCM_STATE_XRUN 	              Stopped: underrun (playback) or overrun (capture) detected
				SND_PCM_STATE_DRAINING 	       Draining: running (playback) or stopped (capture)
				SND_PCM_STATE_PAUSED 	              Paused
				SND_PCM_STATE_SUSPENDED 	       Hardware is suspended
				SND_PCM_STATE_DISCONNECTED 	Hardware is disconnected
 *---------------------------------------------------------------------------*/

snd_pcm_state_t u_alsa_get_pcm_state(snd_pcm_t *handle)
{
	snd_pcm_state_t state;

	PCM_HANDLE_CHECK(handle);

	state = snd_pcm_state(handle);
	switch(state)
	{
		case SND_PCM_STATE_OPEN:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is SND_PCM_STATE_OPEN %d\n",state));
		break;
		case SND_PCM_STATE_SETUP:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is SND_PCM_STATE_SETUP %d\n",state));
		break;
		case SND_PCM_STATE_PREPARED:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is SND_PCM_STATE_PREPARED %d\n",state));
		break;
		case SND_PCM_STATE_RUNNING:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is SND_PCM_STATE_RUNNING %d\n",state));
		break;
		case SND_PCM_STATE_XRUN:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is SND_PCM_STATE_XRUN %d\n",state));
		break;
		case SND_PCM_STATE_DRAINING:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is SND_PCM_STATE_DRAINING %d\n",state));
		break;
		case SND_PCM_STATE_PAUSED:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is SND_PCM_STATE_PAUSED %d\n",state));
		break;
		case SND_PCM_STATE_SUSPENDED:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is SND_PCM_STATE_SUSPENDED %d\n",state));
		break;
		case SND_PCM_STATE_DISCONNECTED:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is SND_PCM_STATE_DISCONNECTED %d\n",state));
		break;
		default:
			PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"Current PCM state is UNKNOW %d\n",state));
			break;
	}
	return state;
}



/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_Pause
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          - OK:0 else fail
 *---------------------------------------------------------------------------*/
int u_alsa_pause(snd_pcm_t *handle)
{
	/*mtk alsa driver do not support hardware pause,so need use drop/prepare fuction*/
	int ret = ALSA_OK;
	PCM_HANDLE_CHECK(handle);
	ret = snd_pcm_drop(handle);
	if(ret < 0)
	{
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"u_alsa_pause -- drop fail %d\n",ret));
		return ret;
	}
	return ALSA_OK;
}

/*---------------------------------------------------------------------------
 * Name
 *      Playback_Alsa_Resume
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          - OK:0 else fail
 *---------------------------------------------------------------------------*/
int u_alsa_resume(snd_pcm_t *handle)
{
	/*mtk alsa driver do not support hardware pause,so need use drop/prepare fuction*/
	int ret = ALSA_OK;
	PCM_HANDLE_CHECK(handle);
	ret = snd_pcm_prepare(handle);
	if(ret < 0)
	{
		PB_ALSA_ERR((PB_ALSA_DEBUG_TAG"u_alsa_resume -- prepare fail %d\n",ret));
		return ret;
	}
	return ALSA_OK;
}


#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>

#define Printf    printf
#define x_strcpy  strcpy
#define x_memcpy  memcpy
#define x_memset  memset
#define x_strcmp  strcmp
#define x_strcat  strcat
#define x_strncmp strncmp
#define x_strlen  strlen

#include "ppc_control.h"
#include "ppc_api.h"
#ifdef PPC_EQ_MODE_ENABLE
#include "peq.h"
#endif

audio_ppc_main_struct_t *AudioPPCControl::audio_ppc_main = NULL;
AudioPPCControl *AudioPPCControl::unique_ppc_handler = NULL;

static char ppc_path[AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH];

extern "C" {
extern int get_proj_code(void);
}

AudioPPCControl *AudioPPCControl::getInstance()
{
	static pthread_mutex_t mLock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&mLock);

	if (unique_ppc_handler == NULL) {
		unique_ppc_handler = new AudioPPCControl();
	}
	pthread_mutex_unlock(&mLock);

	Printf("[Aud][PPC] AudioPPCControl getInstance\n");
	return unique_ppc_handler;
}

void AudioPPCControl::freeInstance()
{
	Printf("[Aud][PPC] AudioPPCControl freeInstance\n");
	if (unique_ppc_handler != NULL) {
		delete unique_ppc_handler;
		unique_ppc_handler = NULL;
	}
}

AudioPPCControl::AudioPPCControl()
{
	int ret;

	Printf("[Aud][PPC] AudioPPCControl constructor\n");

	ret = pthread_mutex_init(&ppc_mutex, NULL);
	if (ret < 0) {
		Printf("[Aud][PPC] Cannot init mMutex!!\n");
		return;
	}

	ret = init();
	if (ret < 0) {
		Printf("[Aud][PPC] init fail (%d)!!\n", ret);
		pthread_mutex_destroy(&ppc_mutex);
		return;
	}

}

AudioPPCControl::~AudioPPCControl()
{
	Printf("[Aud][PPC] AudioPPCControl destructor\n");
	pthread_mutex_destroy(&ppc_mutex);
	deInit();
}

int AudioPPCControl::init(void)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;
	SYS_INFO *current_info = NULL;
	SYS_INFO *volatile_info = NULL;
	AUDIO_BUF *in_buffer = NULL;
	AUDIO_BUF *out_buffer = NULL;

	audio_ppc_main = NULL;
	audio_ppc_main = (audio_ppc_main_struct_t *)calloc(1, sizeof(audio_ppc_main_struct_t));
	if (audio_ppc_main == NULL) {
		Printf("[Aud][PPC][init] mAudioPPCMain init fail\n");
		return AUD_PPC_ALLOC_PPC_MAIN_STRUCT_FAIL;
	}
	memset((void *)audio_ppc_main, 0, sizeof(audio_ppc_main_struct_t));

	audio_ppc_main->ppc_info_status = AUD_PPC_INFO_UNALLOC;
	audio_ppc_main->sys_info_status = AUD_SYS_INFO_UNALLOC;
	audio_ppc_main->ppc_hdl = NULL;
	audio_ppc_main->ppc_info = NULL;
	audio_ppc_main->ppc_info_size = 0;
	audio_ppc_main->ppc_hdl_size = 0;

	current_info = (SYS_INFO *)calloc(1, sizeof(SYS_INFO));
	if (current_info == NULL) {
		Printf("[Aud][PPC][init] current_info init fail\n");
		if (audio_ppc_main) {
			free(audio_ppc_main);
			audio_ppc_main = NULL;
		}
		return AUD_PPC_ALLOC_STRUCT_FAIL;
	}
	current_info->device = AUDIO_DEVICE_SPEAKER;
	current_info->mode = AUDIO_MODE_MOVIE;
	current_info->sample_format = AUDIO_SAMPLE_FORMAT_32BIT;
	current_info->frame_size = 64;
	current_info->ppc_frame_size = 0;
	audio_ppc_main->cur_sys_info = (void *)current_info;

	volatile_info = (SYS_INFO *)calloc(1, sizeof(SYS_INFO));
	if (volatile_info == NULL) {
		Printf("[Aud][PPC][init] volatile_info init fail\n");
		if (audio_ppc_main) {
			if (audio_ppc_main->cur_sys_info) {
				free(audio_ppc_main->cur_sys_info);
				audio_ppc_main->cur_sys_info = NULL;
			}
			free(audio_ppc_main);
			audio_ppc_main = NULL;
		}
		return AUD_PPC_ALLOC_STRUCT_FAIL;
	}
	volatile_info->device = AUDIO_DEVICE_SPEAKER;
	volatile_info->mode = AUDIO_MODE_MOVIE;
	volatile_info->sample_format = AUDIO_SAMPLE_FORMAT_32BIT;
	volatile_info->frame_size = 64;
	volatile_info->ppc_frame_size = 0;
	audio_ppc_main->volatile_sys_info = (void *)volatile_info;

	in_buffer = (AUDIO_BUF *)calloc(1, sizeof(AUDIO_BUF));
	if (in_buffer == NULL) {
		Printf("[Aud][PPC][init] in_buffer init fail\n");
		if (audio_ppc_main) {
			if (audio_ppc_main->cur_sys_info) {
				free(audio_ppc_main->cur_sys_info);
				audio_ppc_main->cur_sys_info = NULL;
			}
			if (audio_ppc_main->volatile_sys_info) {
				free(audio_ppc_main->volatile_sys_info);
				audio_ppc_main->volatile_sys_info = NULL;
			}
			free(audio_ppc_main);
			audio_ppc_main = NULL;
		}
		return AUD_PPC_ALLOC_STRUCT_FAIL;
	}
	audio_ppc_main->audio_buf_in = (void *)in_buffer;

	out_buffer = (AUDIO_BUF *)calloc(1, sizeof(AUDIO_BUF));
	if (out_buffer == NULL) {
		Printf("[Aud][PPC][init] out_buffer init fail\n");
		if (audio_ppc_main) {
			if (audio_ppc_main->cur_sys_info) {
				free(audio_ppc_main->cur_sys_info);
				audio_ppc_main->cur_sys_info = NULL;
			}
			if (audio_ppc_main->volatile_sys_info) {
				free(audio_ppc_main->volatile_sys_info);
				audio_ppc_main->volatile_sys_info = NULL;
			}
			if (audio_ppc_main->audio_buf_in) {
				free(audio_ppc_main->audio_buf_in);
				audio_ppc_main->audio_buf_in = NULL;
			}
			free(audio_ppc_main);
			audio_ppc_main = NULL;
		}
		return AUD_PPC_ALLOC_STRUCT_FAIL;
	}
	audio_ppc_main->audio_buf_out = (void *)out_buffer;

	x_strcpy(audio_ppc_main->user_cfg_path, AUD_PPC_BUILTIN_FLAG);
	audio_ppc_main->ppc_cfg_path = NULL;
	audio_ppc_main->ppc_reinit_flag = 0;
	audio_ppc_main->ppc_reset_flag = 0;
	audio_ppc_main->bypass_mode = 0;
	audio_ppc_main->ppc_tuning_flag = 0;

	return (int)audio_ppc_result;
}

void AudioPPCControl::deInit(void)
{
	char tmp_path[AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH];
	int ret = 0, pwd = 0;
	Printf("[Aud][PPC][deInit] PPC deinit begin.\n");

	if (audio_ppc_main->ppc_tuning_flag) {
		x_strcpy(tmp_path, AUD_PPC_TMP_CONFIG_PATH);
		if (ppcCheckFileExist(tmp_path))
			ret = mkdir(tmp_path, 0777);

		if (ret == 0) {
			pwd = get_proj_code();
			if (dumpConfigStatus(tmp_path, pwd) != AUD_PPC_SUCCESS)
				Printf("[Aud][PPC][deInit] Cannot save temp status to %s.\n", AUD_PPC_TMP_CONFIG_PATH);
		} else
			Printf("[Aud][PPC][deInit] Cannot create folder %s.\n", AUD_PPC_TMP_CONFIG_PATH);
	}
	ppcStructUnalloc();
	if (audio_ppc_main) {
		if (audio_ppc_main->cur_sys_info) {
			free(audio_ppc_main->cur_sys_info);
			audio_ppc_main->cur_sys_info = NULL;
		}

		if (audio_ppc_main->volatile_sys_info) {
			free(audio_ppc_main->volatile_sys_info);
			audio_ppc_main->volatile_sys_info = NULL;
		}

		if (audio_ppc_main->audio_buf_in) {
			free(audio_ppc_main->audio_buf_in);
			audio_ppc_main->audio_buf_in = NULL;
		}

		if (audio_ppc_main->audio_buf_out) {
			free(audio_ppc_main->audio_buf_out);
			audio_ppc_main->audio_buf_out = NULL;
		}

		free(audio_ppc_main);
		audio_ppc_main = NULL;
	}
}

int AudioPPCControl::set(const char *path, int ppc_mode, int out_device)
{
	SYS_INFO *volatile_info = (SYS_INFO *)audio_ppc_main->volatile_sys_info;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	Printf("[Aud][PPC][set] ppc_mode = %d, device = %d, ppc_cfg path = %s.\n", ppc_mode, out_device, path);

	pthread_mutex_lock(&ppc_mutex);
	switch(out_device)
	{
		case AUD_PPC_DEVICE_SPEAKER:
			volatile_info->device = AUDIO_DEVICE_SPEAKER;
			break;
		case AUD_PPC_DEVICE_HDMI:
			volatile_info->device = AUDIO_DEVICE_HDMI;
			break;
		default:
			pthread_mutex_unlock(&ppc_mutex);
			Printf("[Aud][PPC][set] device is illegal(%d)!\n", out_device);
			return AUD_PPC_DEVICE_NOT_SUPPORT;
	}

	switch(ppc_mode)
	{
		case AUD_PPC_MODE_MOVIE:
			volatile_info->mode = AUDIO_MODE_MOVIE;
			break;
		case AUD_PPC_MODE_MUSIC:
			volatile_info->mode = AUDIO_MODE_MUSIC;
			break;
		default:
			pthread_mutex_unlock(&ppc_mutex);
			Printf("[Aud][PPC][set] mode is illegal(%d)!\n", ppc_mode);
			return AUD_PPC_MODE_NOT_SUPPORT;
	}

	if (NULL != path && path[0] != '\0') {
		if (0 == x_strcmp(path, AUD_PPC_DEFAULT_FLAG)) {
			if (audio_ppc_main->ppc_info_status != AUD_PPC_INFO_UNALLOC && x_strcmp(path, audio_ppc_main->user_cfg_path))
				audio_ppc_main->ppc_info_status = AUD_PPC_INFO_CHANGED;
			x_strcpy(audio_ppc_main->user_cfg_path, path);
			audio_ppc_main->ppc_cfg_path = NULL;
			Printf("[Aud][PPC][set] ppc_cfg path is %s\n", AUD_PPC_DEFAULT_FLAG);
		} else if(0 == x_strcmp(path, AUD_PPC_BUILTIN_FLAG)) {
			if (audio_ppc_main->ppc_info_status != AUD_PPC_INFO_UNALLOC && x_strcmp(path, audio_ppc_main->user_cfg_path))
				audio_ppc_main->ppc_info_status = AUD_PPC_INFO_CHANGED;
			x_strcpy(audio_ppc_main->user_cfg_path, path);
			audio_ppc_main->ppc_cfg_path = NULL;
			Printf("[Aud][PPC][set] ppc_cfg path is %s\n", AUD_PPC_BUILTIN_FLAG);
		} else {
			if (x_strlen(path) > AUD_PPC_CFG_FILE_PATH_STRING_LENGTH) {
				Printf("[Aud][PPC][set] ppc_cfg path exceeds the maximum value!\n");
				audio_ppc_result = AUD_PPC_SET_CFG_PATH_FAIL;
			}
			x_memset(ppc_path, 0, AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH);
			x_strcpy(ppc_path, path);
			if (ppc_path[x_strlen(ppc_path)-1] != '/')
				x_strcat(ppc_path, "/");

			if (audio_ppc_main->ppc_info_status != AUD_PPC_INFO_UNALLOC && x_strcmp(ppc_path, audio_ppc_main->user_cfg_path))
				audio_ppc_main->ppc_info_status = AUD_PPC_INFO_CHANGED;

			x_strcpy(audio_ppc_main->user_cfg_path, ppc_path);
			audio_ppc_main->ppc_cfg_path = audio_ppc_main->user_cfg_path;
			Printf("[Aud][PPC][set] ppc_cfg path is %s\n", audio_ppc_main->ppc_cfg_path);
		}
	} else {
		Printf("[Aud][PPC][set] ppc_cfg path is illegal!\n");
		audio_ppc_result = AUD_PPC_SET_CFG_PATH_FAIL;
	}

	pthread_mutex_unlock(&ppc_mutex);

	return (int)audio_ppc_result;
}

int AudioPPCControl::open(unsigned int input_config, unsigned int sample_rate, unsigned int frame_size, unsigned int *output_config)
{
	audio_ppc_result_t audio_ppc_result;
	int sys_info_changed = 0;
	int user_info_changed = 0;
	SYS_INFO *current_info = (SYS_INFO *)audio_ppc_main->cur_sys_info;
	audio_playback_info_t playback_info;

	playback_info.input_config = input_config;
	playback_info.sample_rate = sample_rate;
	playback_info.sample_format = 32;
	playback_info.frame_size = frame_size;

	Printf("[Aud][PPC][open] PPC init begin.\n");

	pthread_mutex_lock(&ppc_mutex);
	audio_ppc_main->bypass_mode = 0;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][open] ppcStructAlloc return error(%d). PPC will be bypassed!\n",
			(int)audio_ppc_result);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return (int)audio_ppc_result;
	}

	ppcChooseConfigPath();

	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][open] ppcLoad return error(%d). PPC will be bypassed!\n",
			(int)audio_ppc_result);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return (int)audio_ppc_result;
	}

	audio_ppc_result = (audio_ppc_result_t)ppcSetSystemInfo(&playback_info, &sys_info_changed);
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][open] ppcSetSystemInfo return error(%d). PPC will be bypassed!\n",
			(int)audio_ppc_result);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return (int)audio_ppc_result;
	}

	ppcSetUserInfo(&user_info_changed);

	if ((sys_info_changed || user_info_changed) && (AUD_SYS_INFO_INITED == audio_ppc_main->sys_info_status)) {
		audio_ppc_main->sys_info_status = AUD_SYS_INFO_CHANGED;
		Printf("[Aud][PPC][open] System info changed, sys_info_changed = %d, user_info_changed = %d\n",
			sys_info_changed, user_info_changed);
	}

	audio_ppc_result = (audio_ppc_result_t)ppcCreate();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][open] ppcCreate return error(%d). PPC will be bypassed!\n",
			(int)audio_ppc_result);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return (int)audio_ppc_result;
	}

	Printf("[Aud][PPC][open] output channel config = %d\n", current_info->out_channel);
	*output_config = current_info->out_channel;

	pthread_mutex_unlock(&ppc_mutex);
	Printf("[AUD][PPC][open] PPC init end\n");
	return (int)audio_ppc_result;
}

int AudioPPCControl::reset(void)
{
	audio_ppc_result_t audio_ppc_result;
	int sys_info_changed = 0, user_info_changed = 0, i;
	unsigned int old_out_config = 0;
	SYS_INFO *current_info = (SYS_INFO *)audio_ppc_main->cur_sys_info;
	audio_playback_info_t playback_info;

	Printf("[Aud][PPC][reset] PPC reset begin.\n");

	pthread_mutex_lock(&ppc_mutex);

	playback_info.input_config = current_info->in_channel;
	for(i = 0; i < TOTAL_SAMPLING_RATE_NUM; i++) {
		if((srate_map[i].id) == (current_info->sample_rate)) {
			playback_info.sample_rate = srate_map[i].value;
			break;
		}
	}
	if (i == TOTAL_SAMPLING_RATE_NUM) {
		Printf("[Aud][PPC][reset] No matching sample rate! current_info->sample_rate = 0x%x\n",
			current_info->sample_rate);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return AUD_PPC_SAMPLERATE_UNKNOWN;
	}
	playback_info.sample_format = 32;
	playback_info.frame_size = current_info->frame_size;
	old_out_config = current_info->out_channel;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][reset] ppcStructAlloc return error(%d). PPC will be bypassed!\n",
			(int)audio_ppc_result);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return (int)audio_ppc_result;
	}

	ppcChooseConfigPath();

	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][reset] ppcLoad return error(%d). PPC will be bypassed!\n",
			(int)audio_ppc_result);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return (int)audio_ppc_result;
	}

	audio_ppc_result = (audio_ppc_result_t)ppcSetSystemInfo(&playback_info, &sys_info_changed);
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][reset] ppcSetSystemInfo return error(%d). PPC will be bypassed!\n",
			(int)audio_ppc_result);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return (int)audio_ppc_result;
	}

	ppcSetUserInfo(&user_info_changed);

	if ((sys_info_changed || user_info_changed) && (AUD_SYS_INFO_INITED == audio_ppc_main->sys_info_status)) {
		audio_ppc_main->sys_info_status = AUD_SYS_INFO_CHANGED;
		Printf("[Aud][PPC][reset] System info changed, sys_info_changed = %d, user_info_changed = %d\n",
			sys_info_changed, user_info_changed);
	}

	audio_ppc_result = (audio_ppc_result_t)ppcCreate();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][reset] ppcCreate return error(%d). PPC will be bypassed!\n",
			(int)audio_ppc_result);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return (int)audio_ppc_result;
	}

	if (old_out_config != current_info->out_channel) {
		Printf("[Aud][PPC][reset] output channel config changed! old: 0x%x new: 0x%x\n",
			old_out_config, current_info->out_channel);
		audio_ppc_main->bypass_mode = 1;
		pthread_mutex_unlock(&ppc_mutex);
		return AUD_PPC_RESET_FAIL;
	}

	pthread_mutex_unlock(&ppc_mutex);
	Printf("[AUD][PPC][reset] PPC reset end\n");
	return (int)audio_ppc_result;
}


int AudioPPCControl::process(void *input, void *output)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;
	int i = 0;
	int j = 0;
	int k = 0;
	SYS_INFO *current_info = (SYS_INFO *)audio_ppc_main->cur_sys_info;

	int* buffer_in = (int *)input;
	int* buffer_out = (int *)output;

	pthread_mutex_lock(&ppc_mutex);
	if(audio_ppc_main->bypass_mode == 1) {
		for (i = 0; i < AUDIO_PPC_CH_NUM; i++)
		{
			if ((current_info->out_channel & ch_map[i]) != 0) {
				if ((current_info->in_channel & ch_map[i]) != 0) {
					x_memcpy((void *)(buffer_out + k * current_info->frame_size), (void *)(buffer_in + j * current_info->frame_size), current_info->frame_size * 4);
					j++;
				} else
					x_memset((void *)(buffer_out + k * current_info->frame_size), 0, current_info->frame_size * 4);

				k++;
			}
		}
	} else {
		ppcGetBuffer(input, output, current_info->frame_size);
		audio_ppc_result = (audio_ppc_result_t)ppcDataProcess(current_info->frame_size, NULL);
		if(AUD_PPC_SUCCESS != audio_ppc_result) {
			Printf("[Aud][PPC][process] ppcDataProcess return error(%d). PPC will be bypassed!\n",
				(int)audio_ppc_result);
			audio_ppc_main->bypass_mode = 1;
		}
	}

	pthread_mutex_unlock(&ppc_mutex);
	return (int)audio_ppc_result;
}

int AudioPPCControl::process(void *input, void *output, unsigned int frames, int *latency)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;
	int i = 0;
	int j = 0;
	int k = 0;
	SYS_INFO *current_info = (SYS_INFO *)audio_ppc_main->cur_sys_info;

	int* buffer_in = (int *)input;
	int* buffer_out = (int *)output;

	pthread_mutex_lock(&ppc_mutex);
	if(audio_ppc_main->bypass_mode == 1) {
		for (i = 0; i < AUDIO_PPC_CH_NUM; i++)
		{
			if ((current_info->out_channel & ch_map[i]) != 0) {
				if ((current_info->in_channel & ch_map[i]) != 0) {
					x_memcpy((void *)(buffer_out + k * frames), (void *)(buffer_in + j * frames), frames * 4);
					j++;
				} else
					x_memset((void *)(buffer_out + k * frames), 0, frames * 4);

				k++;
			}
		}
		*latency = 0;
	} else {
		ppcGetBuffer(input, output, frames);
		audio_ppc_result = (audio_ppc_result_t)ppcDataProcess(frames, latency);
		if(AUD_PPC_SUCCESS != audio_ppc_result) {
			Printf("[Aud][PPC][process] ppcDataProcess return error(%d). PPC will be bypassed!\n",
				(int)audio_ppc_result);
			audio_ppc_main->bypass_mode = 1;
		}
	}

	pthread_mutex_unlock(&ppc_mutex);
	return (int)audio_ppc_result;
}

int AudioPPCControl::setParameter(unsigned int element_id, unsigned int para_id, int para_num, int* value)
{
	int i, n;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setParameter] ppcStructAlloc return error(%d). PPC can't set parameter!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_PARA_CANNOT;
	}

	Printf("[Aud][PPC][setParameter] ppcChooseConfigPath\n");
	ppcChooseConfigPath();
	Printf("[Aud][PPC][setParameter] ppcLoad\n");
	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if(AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setParameter] ppcLoad return error(%d). PPC can't set parameter!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_PARA_CANNOT;
	}

	Printf("[Aud][PPC][setParameter] element_id = 0x%08x, para_id = 0x%08x, num = %d\n",
		element_id, para_id, para_num);
	Printf("==================\n");

	return ppcSetPara(element_id, para_id, para_num, value);
}

int AudioPPCControl::getParameter(unsigned int element_id, unsigned int para_id, int para_num, int* value)
{
	int i, n;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][getParameter] ppcStructAlloc return error(%d). PPC can't get parameter!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_PARA_CANNOT;
	}

	Printf("[Aud][PPC][getParameter] ppcChooseConfigPath\n");
	ppcChooseConfigPath();

	Printf("[Aud][PPC][getParameter] ppcLoad\n");
	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][getParameter] ppcLoad return error(%d). PPC can't get parameter!\n",
			(int)audio_ppc_result);
		return AUD_PPC_GET_PARA_CANNOT;
	}

	audio_ppc_result = (audio_ppc_result_t)ppcGetPara(element_id, para_id, para_num, value);
	if (AUD_PPC_SUCCESS == audio_ppc_result) {
		Printf("[Aud][PPC][getParameter] element_id = 0x%08x, para_id = 0x%08x, num = %d\n",
			element_id, para_id, para_num);
		Printf("==================\n");
		return 0;
	} else {
		Printf("[Aud][PPC][getParameter] get para fail (element_id = 0x%08x, para_id = 0x%08x, num = %d)!\n",
			element_id, para_id, para_num);
		return AUD_PPC_GET_PARA_FAIL;
	}
}

int AudioPPCControl::setElementBypass(unsigned int element_id, int enable)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setElementBypass] ppcStructAlloc return error(%d). Cannot set element bypass!\n",
			(int)audio_ppc_result);
		return AUD_PPC_BYPASS_ELEMENT_CANNOT;
	}

	Printf("[Aud][PPC][setElementBypass] ppcChooseConfigPath\n");
	ppcChooseConfigPath();

	Printf("[Aud][PPC][setElementBypass] ppcLoad\n");
	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setElementBypass] ppcLoad return error(%d). Cannot set element bypass!\n",
			(int)audio_ppc_result);
		return AUD_PPC_BYPASS_ELEMENT_CANNOT;
	}

	audio_ppc_result = (audio_ppc_result_t)ppcBypassElement(element_id, enable);
	if (AUD_PPC_SUCCESS == audio_ppc_result)
		Printf("[Aud][PPC][setElementBypass] bypass element 0x%x success, enable = %d!\n", element_id, enable);
	else if (AUD_PPC_BYPASS_ELEMENT_NEED_RESET == audio_ppc_result)
		Printf("[Aud][PPC][setElementBypass] bypass element 0x%x success (%d) need reset!\n", element_id, enable);
	else {
		Printf("[Aud][PPC][setElementBypass] bypass element 0x%x failed(%d), enable = %d!\n",
			element_id, (int)audio_ppc_result, enable);
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::getElementBypass(unsigned int element_id, int* enabled)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][getElementBypass] ppcStructAlloc return error(%d). Cannot get element bypass!\n",
			(int)audio_ppc_result);
		return AUD_PPC_GET_ELEMENT_BYPASS_STATUS_CANNOT;
	}

	Printf("[Aud][PPC][getElementBypass] ppcChooseConfigPath\n");
	ppcChooseConfigPath();

	Printf("[Aud][PPC][getElementBypass] ppcLoad\n");
	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][getElementBypass] ppcLoad return error(%d). Cannot get PPC element bypass status!\n",
			(int)audio_ppc_result);
		return AUD_PPC_GET_ELEMENT_BYPASS_STATUS_CANNOT;
	}

	audio_ppc_result = (audio_ppc_result_t)ppcGetBypassStatus(element_id, enabled);
	if (AUD_PPC_SUCCESS == audio_ppc_result)
		Printf("[Aud][PPC][getElementBypass] element_id = %d, status = %d!\n", element_id, *enabled);
	else {
		Printf("[Aud][PPC][getElementBypass] failed(%d), can't get element(%d) bypass status!\n",
			(int)audio_ppc_result, element_id);
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::setCfgFilePath(const char *path)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setCfgFilePath] ppcStructAlloc return error(%d). Cannot set config path!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_CFG_PATH_FAIL;
	}

	Printf("[Aud][PPC][setCfgFilePath] ppcChooseConfigPath\n");
	ppcChooseConfigPath();

	Printf("[Aud][PPC][setCfgFilePath] ppcLoad\n");
	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setCfgFilePath] ppcLoad return error(%d). Cannot set config path!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_CFG_PATH_FAIL;
	}

	if (NULL != path && path[0] != '\0') {
		if (0 == x_strcmp(path, AUD_PPC_DEFAULT_FLAG)) {
			x_strcpy(audio_ppc_main->user_cfg_path, path);
			audio_ppc_main->ppc_cfg_path = NULL;
			audio_ppc_main->ppc_info_status = AUD_PPC_INFO_CHANGED;
			Printf("[Aud][PPC][setCfgFilePath] path is %s\n", AUD_PPC_DEFAULT_FLAG);
		} else if(0 == x_strcmp(path, AUD_PPC_BUILTIN_FLAG)) {
			x_strcpy(audio_ppc_main->user_cfg_path, path);
			audio_ppc_main->ppc_cfg_path = NULL;
			audio_ppc_main->ppc_info_status = AUD_PPC_INFO_CHANGED;
			Printf("[Aud][PPC][setCfgFilePath] path is %s\n", AUD_PPC_BUILTIN_FLAG);
		} else {
			x_strcpy(audio_ppc_main->user_cfg_path, path);
			audio_ppc_main->ppc_cfg_path = audio_ppc_main->user_cfg_path;
			audio_ppc_main->ppc_info_status = AUD_PPC_INFO_CHANGED;
			Printf("[Aud][PPC][setCfgFilePath] path is %s\n", audio_ppc_main->ppc_cfg_path);
		}
	} else {
		Printf("[Aud][PPC][setCfgFilePath] path is illegal!\n");
		audio_ppc_result = AUD_PPC_SET_CFG_PATH_FAIL;
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::getCfgFilePath(char *path)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	if (NULL != path) {
		switch(ppcChooseConfigPath())
		{
		case 0:
			Printf("[Aud][PPC][getCfgFilePath]Current cfg path (Built-in) is %s\n", AUD_PPC_BUILTIN_FLAG);
			x_strcpy(path, AUD_PPC_BUILTIN_FLAG);
			break;
		case 1:
			Printf("[Aud][PPC][getCfgFilePath]Current cfg path (Default) is %s\n", audio_ppc_main->ppc_cfg_path);
			x_strcpy(path, audio_ppc_main->ppc_cfg_path);
			break;
		case 2:
			Printf("[Aud][PPC][getCfgFilePath]Current cfg path is %s\n", audio_ppc_main->ppc_cfg_path);
			x_strcpy(path, audio_ppc_main->ppc_cfg_path);
			break;
		case 5:
			if(NULL != audio_ppc_main->ppc_cfg_path) {
				Printf("[Aud][PPC][getCfgFilePath]Current using cfg path is %s\n", audio_ppc_main->ppc_cfg_path);
				x_strcpy(path, audio_ppc_main->ppc_cfg_path);
			} else {
				Printf("[Aud][PPC][getCfgFilePath]Current using cfg path is %s\n", AUD_PPC_BUILTIN_FLAG);
				x_strcpy(path, AUD_PPC_BUILTIN_FLAG);
			}
		default:
			Printf("[Aud][PPC][getCfgFilePath]Error,Current cfg path is illegal!\n");
			path[0] = '\0';
			audio_ppc_result = AUD_PPC_GET_CFG_PATH_FAIL;
			break;
		}
	} else {
		Printf("[Aud][PPC]getCfgFilePath is illegal!\n");
		path[0] = '\0';
		audio_ppc_result = AUD_PPC_GET_CFG_PATH_FAIL;
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::getSystemInfo(void *ppc_sys_info)
{
	if (audio_ppc_main->sys_info_status != AUD_SYS_INFO_INITED) {
		Printf("[Aud][PPC][getSystemInfo]PPC system info is not inited!\n");
		ppc_sys_info = NULL;
		return AUD_PPC_INFO_UNINITED;
	}
	Printf("[Aud][PPC][getSystemInfo]PPC system info can be got!\n");
	x_memcpy(ppc_sys_info, audio_ppc_main->cur_sys_info, sizeof(SYS_INFO));
	return AUD_PPC_SUCCESS;
}

int AudioPPCControl::setDevice(int out_device)
{
	SYS_INFO *volatile_info = (SYS_INFO *)audio_ppc_main->volatile_sys_info;
	switch(out_device)
	{
	case AUD_PPC_DEVICE_SPEAKER:
		volatile_info->device = AUDIO_DEVICE_SPEAKER;
		Printf("[Aud][PPC][setDevice] is AUDIO_DEVICE_SPEAKER\n");
		break;
	case AUD_PPC_DEVICE_HDMI:
		volatile_info->device = AUDIO_DEVICE_HDMI;
		Printf("[Aud][PPC][setDevice] is AUDIO_DEVICE_HDMI\n");
		break;
	default:
		Printf("[Aud][PPC][setDevice] is illegal(%d)!\n", out_device);
		return AUD_PPC_DEVICE_NOT_SUPPORT;
	}
	return AUD_PPC_SUCCESS;
}

int AudioPPCControl::getDevice(void)
{
	SYS_INFO *volatile_info = (SYS_INFO *)audio_ppc_main->volatile_sys_info;
	Printf("[Aud][PPC][AudioPPCControl][getDevice] device is %d\n", (int)volatile_info->device);
	return (int)volatile_info->device;
}

int AudioPPCControl::setMode(int ppc_mode)
{
	SYS_INFO *volatile_info = (SYS_INFO *)audio_ppc_main->volatile_sys_info;

	if (ppc_mode >= AUD_PPC_MODE_MOVIE && ppc_mode <= AUD_PPC_MODE_32) {
		volatile_info->mode = (AUDIO_MODE)(1 << (ppc_mode - 1));
		Printf("[Aud][PPC][setMode] mode is %d\n", ppc_mode);
	} else {
		Printf("[Aud][PPC][setMode] mode is illegal(%d)!\n", ppc_mode);
		return AUD_PPC_MODE_NOT_SUPPORT;
	}

	return AUD_PPC_SUCCESS;
}

int AudioPPCControl::getMode(void)
{
	SYS_INFO *volatile_info = (SYS_INFO *)audio_ppc_main->volatile_sys_info;
	int i = 0;
	for (i = 0; i < AUD_PPC_MODE_32; i++) {
		if ((volatile_info->mode & (1 << i)) != 0)
			break;
	}
	if (i == AUD_PPC_MODE_32) {
		Printf("[Aud][PPC][getMode] invalid PPC mode: 0x%08x\n", (int)volatile_info->mode);
		return 0;
	}
	i++;
	Printf("[Aud][PPC][getMode] PPC mode is %d (0x%08x).\n", i, (int)volatile_info->mode);
	return i;
}

int AudioPPCControl::dumpElementData(unsigned int element_id, int enabled, char* dump_folder)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	x_memset(ppc_path, 0, AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH);
	x_strcpy(ppc_path, dump_folder);
	if (ppcChooseDumpPath(ppc_path)) {
		Printf("[Aud][PPC][dumpElementData] %s is not exist, can't dump!\n", dump_folder);
		return AUD_PPC_DUMP_PATH_NOT_EXIST;
	}

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][dumpElementData] ppcStructAlloc return error(%d). Cannot dump element(0x%08x) data!\n",
			(int)audio_ppc_result, element_id);
		return AUD_PPC_DUMP_ELEMENT_CANNOT;
	}

	Printf("[Aud][PPC][dumpElementData] ppcChooseConfigPath\n");
	ppcChooseConfigPath();

	Printf("[Aud][PPC][dumpElementData] ppcLoad\n");
	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][dumpElementData] ppcLoad return error(%d). Cannot dump element(0x%08x) data!\n",
			(int)audio_ppc_result, element_id);
		return AUD_PPC_DUMP_ELEMENT_CANNOT;
	}

	if (enabled == 0) {
		audio_ppc_result = (audio_ppc_result_t)ppcDumpElement(element_id, enabled, NULL);
		if (AUD_PPC_SUCCESS == audio_ppc_result)
			Printf("[Aud][PPC][dumpElementData] disable element_id(0x%08x) dump!\n", element_id);
		else {
			Printf("[Aud][PPC][dumpElementData] disable element_id(0x%08x) dump fail(%d)!\n",
				element_id, (int)audio_ppc_result);
		}
		return (int)audio_ppc_result;
	}

	audio_ppc_result = (audio_ppc_result_t)ppcDumpElement(element_id, 1, ppc_path);
	if (AUD_PPC_SUCCESS == audio_ppc_result) {
		Printf("[Aud][PPC][dumpElementData] enable element_id(0x%08x) dump!\n", element_id);
	} else {
		Printf("[Aud][PPC][dumpElementData] enable element_id(0x%08x) dump fail(%d)!\n",
			element_id, (int)audio_ppc_result);
	}
	return (int)audio_ppc_result;
}

int AudioPPCControl::dumpConfigStatus(char* config_path, int passwd)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	x_memset(ppc_path, 0, AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH);
	x_strcpy(ppc_path, config_path);
	if (ppcChooseDumpPath(ppc_path)) {
		Printf("[Aud][PPC][dumpConfigStatus]%s is not exist, can't dump!\n", config_path);
		return AUD_PPC_DUMP_PATH_NOT_EXIST;
	}

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][dumpConfigStatus] ppcStructAlloc return error(%d). Cannot dump config status!\n",
			(int)audio_ppc_result);
		return AUD_PPC_DUMP_STATUS_CANNOT;
	}

	Printf("[Aud][PPC][dumpConfigStatus] ppcChooseConfigPath\n");
	ppcChooseConfigPath();

	Printf("[Aud][PPC][dumpConfigStatus] ppcLoad\n");
	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][dumpConfigStatus] ppcLoad return error(%d). Cannot dump config status!\n",
			(int)audio_ppc_result);
		return AUD_PPC_DUMP_STATUS_CANNOT;
	}


	audio_ppc_result = (audio_ppc_result_t)ppcDumpStatus(ppc_path, passwd);
	if (AUD_PPC_SUCCESS == audio_ppc_result)
		Printf("[Aud][PPC][dumpConfigStatus] open ppcDumpStatus dump!\n");
	else
		Printf("[Aud][PPC][dumpConfigStatus] open ppcDumpStatus dump fail(%d)!\n", (int)audio_ppc_result);

	return (int)audio_ppc_result;
}

int AudioPPCControl::setTuningMode(bool enable)
{
	if (audio_ppc_main->ppc_tuning_flag != enable)
		audio_ppc_main->ppc_tuning_flag = enable;
	Printf("[Aud][PPC][setTuningMode] enable = %d!\n", audio_ppc_main->ppc_tuning_flag);

	return AUD_PPC_SUCCESS;
}

int AudioPPCControl::getDelay(int *delay)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;
	PPC_RESULT ret = PPC_SUCCESS;
	SYS_INFO *sys_info = (SYS_INFO *)audio_ppc_main->cur_sys_info;

	if (audio_ppc_main->sys_info_status == AUD_SYS_INFO_INITED) {
		ret = ppc_get_delay(audio_ppc_main->ppc_hdl, delay);
		if (ret != PPC_SUCCESS)
			audio_ppc_result = AUD_PPC_GET_DELAY_FAIL;
		else if (sys_info->frame_size == 0)
			*delay = 2*(*delay);
	} else {
		Printf("[Aud][PPC][getDelay] PPC system info is not inited!\n");
		audio_ppc_result = AUD_PPC_INFO_UNINITED;
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::checkVersion(int tool_version)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;
	PPC_RESULT ret = PPC_SUCCESS;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][checkVersion] ppcStructAlloc return error(%d). Cannot set check ppc version!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_PARA_CANNOT;
	}

	ppcChooseConfigPath();

	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if(AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][checkVersion] ppcLoad return error(%d). Cannot set check ppc version!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_PARA_CANNOT;
	}

	if (audio_ppc_main->ppc_info_status == AUD_PPC_INFO_INITED) {
		ret = ppc_check_version(audio_ppc_main->ppc_hdl, tool_version);
		if (ret != PPC_SUCCESS)
			audio_ppc_result = AUD_PPC_CHECK_VERSION_FAIL;
	} else {
		Printf("[Aud][PPC][checkVersion] PPC info is not inited!\n");
		audio_ppc_result = AUD_PPC_INFO_UNINITED;
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::checkSwipVersion(int type_id, int tool_version)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;
	PPC_RESULT ret = PPC_SUCCESS;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][checkSwipVersion] ppcStructAlloc return error(%d). Cannot set check swip version!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_PARA_CANNOT;
	}

	ppcChooseConfigPath();

	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if(AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][checkSwipVersion] ppcLoad return error(%d). Cannot set check swip version!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_PARA_CANNOT;
	}

	if (audio_ppc_main->ppc_info_status == AUD_PPC_INFO_INITED) {
		ret = ppc_check_swip_version(audio_ppc_main->ppc_hdl, type_id, tool_version);
		if (ret != PPC_SUCCESS)
			audio_ppc_result = AUD_PPC_CHECK_VERSION_FAIL;
	} else {
		Printf("[Aud][PPC][checkSwipVersion] PPC info is not inited!\n");
		audio_ppc_result = AUD_PPC_INFO_UNINITED;
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::setEqModePre(unsigned int element_id, int mode)
{
#ifdef PPC_EQ_MODE_ENABLE
#ifndef EQ_MODULE_NUM
	Printf("[Aud][PPC][setEqModePre] no EQ module defined!\n");
	return AUD_PPC_SET_EQ_MODE_CONNOT;
#else
	int64_t *value = NULL;
	uint32_t para_size, i, module_id;
	int (*band_num)[MAX_CHANNEL_NUM] = NULL;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setEqModePre] ppcStructAlloc return error(%d). PPC can't set EQ mode!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_EQ_MODE_CONNOT;
	}

	Printf("[Aud][PPC][setEqModePre] ppcChooseConfigPath\n");
	ppcChooseConfigPath();
	Printf("[Aud][PPC][setEqModePre] ppcLoad\n");
	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if(AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setEqModePre] ppcLoad return error(%d). PPC can't set EQ mode!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_EQ_MODE_CONNOT;
	}

	if ((element_id & 0xFFFF0000) != 0x00070000) {
		Printf("[Aud][PPC][setEqModePre] Error: element id is not EQ! element_id = 0x%08x\n", element_id);
		return AUD_PPC_SET_EQ_MODE_FAIL;
	}

	module_id = element_id & 0x0000FFFF;
	if (module_id >= EQ_MODULE_NUM) {
		Printf("[Aud][PPC][setEqModePre] Error: element_id exceeds the EQ module number! element_id = 0x%08x, EQ module num: %d\n", element_id, EQ_MODULE_NUM);
		return AUD_PPC_SET_EQ_MODE_FAIL;
	}

	band_num = eq_band_num[module_id];

	if (mode >= eq_mode_num[module_id]) {
		Printf("[Aud][PPC][setEqModePre] Error: mode exceeds the maximum number! mode: %d, maximum mode num: %d\n", mode, eq_mode_num[module_id]);
		return AUD_PPC_SET_EQ_MODE_FAIL;
	}

	para_size = MAX_CHANNEL_NUM * sizeof(int64_t);
	value = (int64_t *)malloc(para_size);
	if (value == NULL) {
		Printf("[Aud][PPC][setEqModePre] cannot allocate value buffer!\n");
		return AUD_PPC_SET_EQ_MODE_FAIL;
	}

	for (i = 0; i < MAX_CHANNEL_NUM; i++)
		*(value + i) = (int64_t)band_num[mode][i];

	audio_ppc_result = (audio_ppc_result_t)ppcSetPara(element_id, 0x0007a000, MAX_CHANNEL_NUM, (int32_t *)value);
	free(value);
	return audio_ppc_result;
#endif
#else
	Printf("[Aud][PPC][setEqModePre] EQ mode setting is not supported!\n");
	return AUD_PPC_SET_EQ_MODE_CONNOT;
#endif
}

int AudioPPCControl::setEqModePost(unsigned int element_id, int mode)
{
#ifdef PPC_EQ_MODE_ENABLE
#ifndef EQ_MODULE_NUM
	Printf("[Aud][PPC][setEqModePost] no EQ module defined!\n");
	return AUD_PPC_SET_EQ_MODE_CONNOT;
#else
	int64_t *value = NULL;
	struct mpaq_params_head *head;
	uint32_t para_id, i, j, k, module_id, para_num;
	int* (*cur_coeff_ptr)[MAX_CHANNEL_NUM][MAX_RATE_NUM] = NULL;
	int (*band_num)[MAX_CHANNEL_NUM] = NULL;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	audio_ppc_result = (audio_ppc_result_t)ppcStructAlloc();
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setEqModePost] ppcStructAlloc return error(%d). PPC can't set EQ mode!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_EQ_MODE_CONNOT;
	}

	Printf("[Aud][PPC][setEqModePost] ppcChooseConfigPath\n");
	ppcChooseConfigPath();
	Printf("[Aud][PPC][setEqModePost] ppcLoad\n");
	audio_ppc_result = (audio_ppc_result_t)ppcLoad();
	if(AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][setEqModePost] ppcLoad return error(%d). PPC can't set EQ mode!\n",
			(int)audio_ppc_result);
		return AUD_PPC_SET_EQ_MODE_CONNOT;
	}

	if ((element_id & 0xFFFF0000) != 0x00070000) {
		Printf("[Aud][PPC][setEqModePost] Error: element id is not EQ! element_id = 0x%08x\n", element_id);
		return AUD_PPC_SET_EQ_MODE_FAIL;
	}

	module_id = element_id & 0x0000FFFF;
	if (module_id >= EQ_MODULE_NUM) {
		Printf("[Aud][PPC][setEqModePost] Error: element_id exceeds the EQ module number! element_id = 0x%08x, EQ module num: %d\n", element_id, EQ_MODULE_NUM);
		return AUD_PPC_SET_EQ_MODE_FAIL;
	}

	band_num = eq_band_num[module_id];

	if (mode >= eq_mode_num[module_id]) {
		Printf("[Aud][PPC][setEqModePost] Error: mode exceeds the maximum number! mode: %d, maximum mode num: %d\n", mode, eq_mode_num[module_id]);
		return AUD_PPC_SET_EQ_MODE_FAIL;
	}

	cur_coeff_ptr = eq_coef[module_id];
	for (i = 0; i < MAX_CHANNEL_NUM; i++) {
		Printf("[Aud][PPC][setEqModePost] band num for EQ_%d mode_%d ch_%d is %d\n", module_id, mode, i, band_num[mode][i]);
		for (j = 0; j < MAX_RATE_NUM; j++) {
			if (cur_coeff_ptr[mode][i][j] != NULL) {
				int *coeff = cur_coeff_ptr[mode][i][j];
				para_num = 5 * band_num[mode][i];
				value = (int64_t *)malloc(para_num * sizeof(int64_t));
				if (value == NULL) {
					Printf("[Aud][PPC][setEqModePost] cannot allocate value buffer!\n");
					return AUD_PPC_SET_EQ_MODE_FAIL;
				}
				para_id = 0x00070000 | (j << 4) | i;
				for (k = 0; k < para_num; k++)
					*(value + k) = (int64_t)coeff[k];
				audio_ppc_result = (audio_ppc_result_t)ppcSetPara(element_id, para_id, para_num, (int32_t *)value);
				free(value);
			}
		}
	}
	return audio_ppc_result;
#endif
#else
	Printf("[Aud][PPC][setEqModePost] EQ mode setting is not supported!\n");
	return AUD_PPC_SET_EQ_MODE_CONNOT;
#endif
}

int AudioPPCControl::getParaValueSize(void)
{
	int size = sizeof(int64_t);
	Printf("[Aud][PPC][getParaValueSize] size = %d\n", size);
	return size;
}

int AudioPPCControl::getSysInfoSize(void)
{
	int size = sizeof(SYS_INFO);
	Printf("[Aud][PPC][getSysInfoSize] size = %d\n", size);
	return size;
}

int AudioPPCControl::getMaxConfigPathSize(void)
{
	int len = AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH;
	Printf("[Aud][PPC][getMaxConfigPathSize] len = %d\n", len);
	return len;
}

void AudioPPCControl::ppcGetBuffer(void *input, void *output, unsigned int frames)
{
	SYS_INFO *current_info = (SYS_INFO *)audio_ppc_main->cur_sys_info;
	AUDIO_BUF *input_buf = (AUDIO_BUF *)audio_ppc_main->audio_buf_in;
	AUDIO_BUF *output_buf = (AUDIO_BUF *)audio_ppc_main->audio_buf_out;

	int* buffer_in = (int *)input;
	int* buffer_out = (int *)output;

	int i = 0;
	int j = 0;
	int k = 0;

	for (i = 0; i < TOTAL_CH_NUM; i++)
	{
		if ((current_info->in_channel & ch_map[i]) != 0) {
			input_buf->buf[i] = (int *)(buffer_in + j * frames);
			j++;
		} else
			input_buf->buf[i] = (int *)NULL;

		if ((current_info->out_channel & ch_map[i]) != 0) {
			output_buf->buf[i] = (int *)(buffer_out + k * frames);
			k++;
		} else
			output_buf->buf[i] = (int *)NULL;

	}

	return;
}

int AudioPPCControl::ppcChooseConfigPath(void)
{
	int ret = 0;

	if (AUD_PPC_INFO_UNINIT != audio_ppc_main->ppc_info_status && AUD_PPC_INFO_CHANGED != audio_ppc_main->ppc_info_status) {
		Printf("[Aud][PPC][ppcChooseConfigPath] PPC info status is INITING or INITED!\n");
		return -1;
	}

	x_memset(ppc_path, 0, AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH);
	x_strcpy(ppc_path, AUD_PPC_TMP_CONFIG_PATH);
	x_strcat(ppc_path, AUD_PPC_CONFIG_FILE_USED_TEST);

	if (0 == ppcCheckFileExist(ppc_path)) {
		x_strcpy(audio_ppc_main->user_cfg_path, AUD_PPC_TMP_CONFIG_PATH);
		audio_ppc_main->ppc_cfg_path = audio_ppc_main->user_cfg_path;
		Printf("[Aud][PPC][set] ppc_cfg path is %s\n", audio_ppc_main->ppc_cfg_path);
	} else if (0 == x_strcmp(audio_ppc_main->user_cfg_path, AUD_PPC_DEFAULT_FLAG)) {
		Printf("[Aud][PPC][ppcChooseConfigPath] choose Default or Built-in cfg...\n");
		x_memset(ppc_path, 0, AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH);
		x_strcpy(ppc_path, AUD_PPC_DEFAULT_CONFIG_PATH00);
		x_strcat(ppc_path, AUD_PPC_CONFIG_FILE_USED_TEST);
		if (0 == ppcCheckFileExist(ppc_path)) {
			audio_ppc_main->ppc_cfg_path = AUD_PPC_DEFAULT_CONFIG_PATH00;
			Printf("[Aud][PPC][ppcChooseConfigPath] Default cfg %s will be used!\n",
				audio_ppc_main->ppc_cfg_path);
			ret = 1;
		} else {
			x_memset(ppc_path, 0, AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH);
			x_strcpy(ppc_path, AUD_PPC_DEFAULT_CONFIG_PATH01);
			x_strcat(ppc_path, AUD_PPC_CONFIG_FILE_USED_TEST);
			if (0 == ppcCheckFileExist(ppc_path)) {
				audio_ppc_main->ppc_cfg_path = AUD_PPC_DEFAULT_CONFIG_PATH01;
				Printf("[Aud][PPC][ppcChooseConfigPath] Default cfg %s will be used!\n",
					audio_ppc_main->ppc_cfg_path);
				ret = 1;
			} else {
				audio_ppc_main->ppc_cfg_path = NULL;
				Printf("[Aud][PPC][ppcChooseConfigPath] Built-in cfg will be used!\n");
				ret = 0;
			}
		}
	} else if (0 == x_strcmp(audio_ppc_main->user_cfg_path, AUD_PPC_BUILTIN_FLAG)) {
		audio_ppc_main->ppc_cfg_path = NULL;
		Printf("[Aud][PPC][ppcChooseConfigPath] Built-in cfg will be used!\n");
		ret = 0;
	} else {
		x_memset(ppc_path, 0, AUD_PPC_CFG_FILE_PATH_BUFFER_LENGTH);
		x_strcpy(ppc_path, audio_ppc_main->user_cfg_path);
		if (ppc_path[x_strlen(ppc_path)-1] != '/') {
			x_strcat(ppc_path, "/");
			x_strcat(audio_ppc_main->user_cfg_path, "/");
		}
		x_strcat(ppc_path, AUD_PPC_CONFIG_FILE_USED_TEST);
		if (0 != ppcCheckFileExist(ppc_path)) {
			Printf("[Aud][PPC][ppcChooseConfigPath] cfg file %s not exist...\n", ppc_path);
			Printf("[Aud][PPC][ppcChooseConfigPath] Built-in cfg will be used!\n");
			audio_ppc_main->ppc_cfg_path = NULL;
			ret = 0;
		} else {
			audio_ppc_main->ppc_cfg_path = audio_ppc_main->user_cfg_path;
			Printf("[Aud][PPC][ppcChooseConfigPath] cfg file %s exist and %s will be used!\n",
				ppc_path, audio_ppc_main->ppc_cfg_path);
			ret = 2;
		}
	}
	return ret;
}

int AudioPPCControl::ppcCheckFileExist(char *file_name)
{
	int *pFile = NULL;
	int ret = 0;
	Printf("[Aud][PPC][ppcCheckFileExist] Try %s exist or not...\n", file_name);

	ret = access(file_name, F_OK);
	if (ret == 0)
		Printf("[Aud][PPC][ppcCheckFileExist] %s exist\n", file_name);
	else
		Printf("[Aud][PPC][ppcCheckFileExist] %s not exist\n", file_name);

	return ret;
}

int AudioPPCControl::ppcSampleRateConvert(unsigned int rate, int *ppc_sample_rate)
{
	switch(rate)
	{
	case 8000:
		*ppc_sample_rate = AUDIO_SAMPLE_RATE_8000;
		break;
	case 11025:
		*ppc_sample_rate = AUDIO_SAMPLE_RATE_11025;
		break;
	case 16000:
		*ppc_sample_rate = AUDIO_SAMPLE_RATE_16000;
		break;
	case 22050:
		*ppc_sample_rate = AUDIO_SAMPLE_RATE_22050;
		break;
	case 32000:
		*ppc_sample_rate = AUDIO_SAMPLE_RATE_32000;
		break;
	case 44100:
		*ppc_sample_rate = AUDIO_SAMPLE_RATE_44100;
		break;
	case 48000:
		*ppc_sample_rate = AUDIO_SAMPLE_RATE_48000;
		break;
	case 96000:
		*ppc_sample_rate = AUDIO_SAMPLE_RATE_96000;
		break;
	case 192000:
		*ppc_sample_rate = AUDIO_SAMPLE_RATE_192000;
		break;
	case 88200:
	case 176400:
		return (int)AUD_PPC_SAMPLERATE_NOT_SUPPORT;
	default:
		return (int)AUD_PPC_SAMPLERATE_UNKNOWN;
	}
	return (int)AUD_PPC_SUCCESS;
}

int AudioPPCControl::ppcFormatConvert(unsigned int format, int *ppc_audio_format)
{
	switch(format)
	{
	case 16:
		*ppc_audio_format = AUDIO_SAMPLE_FORMAT_16BIT_MSB;
		break;
	case 24:
		*ppc_audio_format = AUDIO_SAMPLE_FORMAT_24BIT_MSB;
		break;
	case 32:
		*ppc_audio_format = AUDIO_SAMPLE_FORMAT_32BIT;
		break;
	default:
		return (int)AUD_PPC_FORMAT_NOT_SUPPORT;
	}
	return (int)AUD_PPC_SUCCESS;
}

int AudioPPCControl::ppcSetSystemInfo(audio_playback_info_t *playback_info, int *sys_info_changed)
{
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;
	SYS_INFO *current_info = (SYS_INFO *)audio_ppc_main->cur_sys_info;
	int rate, format;

	*sys_info_changed = 0;

	/* input channel */
	Printf("[Aud][PPC][ppcSetSystemInfo] input channel = %d\n", playback_info->input_config);
	if (playback_info->input_config != current_info->in_channel) {
		*sys_info_changed = 1;
		Printf("[Aud][PPC][ppcSetSystemInfo] sys_info input channel changed, old input channel = %d\n",
			current_info->in_channel);
		current_info->in_channel = playback_info->input_config;
	}

	/* sample rate */
	Printf("[Aud][PPC][ppcSetSystemInfo] sample_rate = %d\n", playback_info->sample_rate);
	audio_ppc_result = (audio_ppc_result_t)ppcSampleRateConvert(playback_info->sample_rate, &rate);
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][ppcSetSystemInfo] PPC_SampleRate_Convert error(%d)\n", (int)audio_ppc_result);
		*sys_info_changed = 0;
		return (int)audio_ppc_result;
	}
	if ((AUDIO_SAMPLE_RATE)rate != current_info->sample_rate) {
		*sys_info_changed = 1;
		Printf("[Aud][PPC][ppcSetSystemInfo] sys_info sample rate changed, old rate (%d), new rate (%d)\n",
			current_info->sample_rate, rate);
		current_info->sample_rate = (AUDIO_SAMPLE_RATE)rate;
	}

	/* sample format */
	Printf("[Aud][PPC][ppcSetSystemInfo] sample_format = %d\n", playback_info->sample_format);
	audio_ppc_result = (audio_ppc_result_t)ppcFormatConvert(playback_info->sample_format, &format);
	if (AUD_PPC_SUCCESS != audio_ppc_result) {
		Printf("[Aud][PPC][ppcSetSystemInfo] PPC_Format_Convert error(%d)\n", (int)audio_ppc_result);
		*sys_info_changed = 0;
		return (int)audio_ppc_result;
	}
	if ((AUDIO_SAMPLE_FORMAT)format != current_info->sample_format)
	{
		*sys_info_changed = 1;
		Printf("[Aud][PPC][ppcSetSystemInfo] sys_info sample format changed, old format = %d, new format = %d\n",
			current_info->sample_format, format);
		current_info->sample_format = (AUDIO_SAMPLE_FORMAT)format;
	}

	/* frame size */
	Printf("[Aud][PPC][ppcSetSystemInfo] frame size = %d\n", playback_info->frame_size);
	if (playback_info->frame_size != current_info->frame_size) {
		*sys_info_changed = 1;
		Printf("[Aud][PPC][ppcSetSystemInfo] sys_info frame size changed, old frame size = %d\n",
			current_info->frame_size);
		current_info->frame_size = playback_info->frame_size;
	}

	return (int)audio_ppc_result;
}

void AudioPPCControl::ppcSetUserInfo(int *user_info_changed)
{
	SYS_INFO *volatile_info = (SYS_INFO *)audio_ppc_main->volatile_sys_info;
	SYS_INFO *current_info = (SYS_INFO *)audio_ppc_main->cur_sys_info;
	*user_info_changed = 0;

	/* device */
	Printf("[Aud][PPC][ppcSetUserInfo] device = %d\n", volatile_info->device);
	if (volatile_info->device != current_info->device) {
		*user_info_changed = 1;
		Printf("[Aud][PPC][ppcSetUserInfo] sys_info device changed, old device = %d\n",
			current_info->device);
		current_info->device = volatile_info->device;
	}

	/* mode */
	Printf("[Aud][PPC][ppcSetUserInfo] mode = %d\n", volatile_info->mode);
	if (volatile_info->mode != current_info->mode) {
		*user_info_changed = 1;
		Printf("[Aud][PPC][ppcSetUserInfo] sys_info mode changed, old mode = %d\n",
			current_info->mode);
		current_info->mode = volatile_info->mode;
	}

	return;
}

int AudioPPCControl::ppcStructAlloc(void)
{
	int info_size = 0;
	int hdl_size = 0;
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	if (AUD_PPC_INFO_UNALLOC == audio_ppc_main->ppc_info_status || AUD_SYS_INFO_UNALLOC == audio_ppc_main->sys_info_status) {
		Printf("[Aud][PPC][ppcStructAlloc] call ppc_size begin.\n");
		ret = ppc_size(&info_size, &hdl_size);
		if (PPC_SUCCESS != ret) {
			audio_ppc_result = AUD_PPC_GET_PPC_SIZE_FAIL;
			Printf("[Aud][PPC][ppcStructAlloc] Get PPC size fail!\n");
			return (int)audio_ppc_result;
		}
		audio_ppc_main->ppc_info_size = info_size;
		audio_ppc_main->ppc_hdl_size = hdl_size;
		Printf("[Aud][PPC][ppcStructAlloc] Info_size = %d, Hdl_size = %d.\n", info_size, hdl_size);
	}

	if (AUD_PPC_INFO_UNALLOC == audio_ppc_main->ppc_info_status) {
		audio_ppc_main->ppc_info = (void *)calloc(1, audio_ppc_main->ppc_info_size);
		if (NULL == audio_ppc_main->ppc_info) {
			audio_ppc_result = AUD_PPC_ALLOC_PPC_INFO_FAIL;
			Printf("[Aud][PPC][ppcStructAlloc] Alloc PPC info fail!\n");
			return (int)audio_ppc_result;
		}
		audio_ppc_main->ppc_info_status = AUD_PPC_INFO_UNINIT;
	}

	if (AUD_SYS_INFO_UNALLOC == audio_ppc_main->sys_info_status)
	{
		audio_ppc_main->ppc_hdl = (void *)calloc(1, audio_ppc_main->ppc_hdl_size);
		if (NULL == audio_ppc_main->ppc_hdl) {
			audio_ppc_result = AUD_PPC_ALLOC_PPC_HANDLE_FAIL;
			Printf("[Aud][PPC][ppcStructAlloc] Alloc PPC handle fail!\n");
			free(audio_ppc_main->ppc_info);
			audio_ppc_main->ppc_info = NULL;
			audio_ppc_main->ppc_info_status = AUD_PPC_INFO_UNALLOC;
			return (int)audio_ppc_result;
		}
		audio_ppc_main->sys_info_status = AUD_SYS_INFO_UNINIT;
	}

	return (int)AUD_PPC_SUCCESS;
}

void AudioPPCControl::ppcStructUnalloc(void)
{
	Printf("[Aud][PPC][ppcStructUnalloc] ppc_destroy_info begin.\n");

	if (AUD_SYS_INFO_UNALLOC != audio_ppc_main->sys_info_status || NULL == audio_ppc_main->ppc_hdl) {
		ppc_destroy_hdl(audio_ppc_main->ppc_hdl);
		free(audio_ppc_main->ppc_hdl);
		audio_ppc_main->ppc_hdl = NULL;
		audio_ppc_main->sys_info_status = AUD_SYS_INFO_UNALLOC;
	}

	if (AUD_PPC_INFO_UNALLOC != audio_ppc_main->ppc_info_status || NULL == audio_ppc_main->ppc_info) {
		ppc_destroy_info(audio_ppc_main->ppc_info);
		free(audio_ppc_main->ppc_info);
		audio_ppc_main->ppc_info = NULL;
		audio_ppc_main->ppc_info_status = AUD_PPC_INFO_UNALLOC;
	}

	return;
}

int AudioPPCControl::ppcLoad(void)
{
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	if (AUD_PPC_INFO_UNINIT == audio_ppc_main->ppc_info_status || AUD_PPC_INFO_CHANGED == audio_ppc_main->ppc_info_status) {
		audio_ppc_main->ppc_info_status = AUD_PPC_INFO_INITING;
		Printf("[Aud][PPC][ppcLoad] ppc_destroy_info begin.\n");
		ret = ppc_destroy_info(audio_ppc_main->ppc_info);
		if (PPC_SUCCESS != ret) {
			audio_ppc_main->ppc_info_status = AUD_PPC_INFO_UNINIT;
			audio_ppc_result = AUD_PPC_DESTROY_INFO_FAIL;
			Printf("[Aud][PPC][ppcLoad] ppc_destroy_info fail(%d)\n", (int)ret);
			return (int)audio_ppc_result;
		} else
			Printf("[Aud][PPC][ppcLoad] ppc_destroy_info ok!\n");

		Printf("[Aud][PPC][ppcLoad] call ppc_load begin.\n");
		ret = ppc_load(audio_ppc_main->ppc_info, audio_ppc_main->ppc_hdl, audio_ppc_main->ppc_cfg_path);
		if (PPC_SUCCESS != ret) {
			audio_ppc_main->ppc_info_status = AUD_PPC_INFO_UNINIT;
			audio_ppc_result = AUD_PPC_LOAD_FAIL;
			Printf("[Aud][PPC][ppcLoad] ppc_load fail(%d)\n", (int)ret);
		} else {
			audio_ppc_main->ppc_info_status = AUD_PPC_INFO_INITED;
			audio_ppc_main->ppc_reinit_flag = false;
			audio_ppc_main->ppc_reset_flag = false;
			if (AUD_SYS_INFO_INITED == audio_ppc_main->sys_info_status)
				audio_ppc_main->sys_info_status = AUD_SYS_INFO_CHANGED;
			Printf("[Aud][PPC][ppcLoad] ppc_load ok!\n");
		}
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::ppcCreate(void)
{
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;
	SYS_INFO *sys_info = (SYS_INFO *)audio_ppc_main->cur_sys_info;

	if (AUD_SYS_INFO_UNINIT == audio_ppc_main->sys_info_status || AUD_SYS_INFO_CHANGED == audio_ppc_main->sys_info_status) {
		audio_ppc_main->sys_info_status = AUD_SYS_INFO_INITING;
		Printf("[Aud][PPC][ppcCreate] ppc_destroy_hdl begin\n");
		ret = ppc_destroy_hdl(audio_ppc_main->ppc_hdl);
		if (PPC_SUCCESS != ret) {
			audio_ppc_main->sys_info_status = AUD_SYS_INFO_UNINIT;
			audio_ppc_result = AUD_PPC_DESTROY_HDL_FAIL;
			Printf("[Aud][PPC][ppcCreate] ppc_destroy_hdl fail(%d)\n", (int)ret);
			return (int)audio_ppc_result;
		} else
			Printf("[Aud][PPC][ppcCreate] ppc_destroy_hdl ok!\n");

		Printf("[Aud][PPC][ppcCreate] call ppc_create begin\n");
		ret = ppc_create(audio_ppc_main->ppc_info, audio_ppc_main->ppc_hdl, sys_info);
		if (PPC_SUCCESS != ret) {
			Printf("[Aud][PPC][ppcCreate] ppc_create return error(%d)\n", (int)ret);
			audio_ppc_main->sys_info_status = AUD_SYS_INFO_UNINIT;
			audio_ppc_result = AUD_PPC_CREATE_FAIL;
			return (int)audio_ppc_result;
		} else {
			audio_ppc_main->sys_info_status = AUD_SYS_INFO_INITED;
			Printf("[Aud][PPC][ppcCreate] ppc_create return ok!\n");
			Printf("[Aud][PPC][ppcCreate] sys_info->frame_size = %d\n", sys_info->frame_size);
			Printf("[Aud][PPC][ppcCreate] sys_info->out_channel = %d\n", sys_info->out_channel);
			audio_ppc_main->ppc_reinit_flag = false;
			audio_ppc_main->ppc_reset_flag = false;
		}
	}

	if (audio_ppc_main->ppc_reinit_flag) {
		Printf("[Aud][PPC][ppcCreate] ppc_reinit!\n");
		ret = ppc_reinit(audio_ppc_main->ppc_hdl, sys_info);
		if (PPC_SUCCESS != ret) {
			Printf("[Aud][PPC][ppcCreate] ppc_reinit return error(%d)\n", (int)ret);
			audio_ppc_result = AUD_PPC_REINIT_FAIL;
		} else {
			Printf("[Aud][PPC][ppcCreate] ppc_reinit return OK!\n");
			Printf("[Aud][PPC][ppcCreate] sys_info->frame_size = %d\n", sys_info->frame_size);
			Printf("[Aud][PPC][ppcCreate] sys_info->out_channel = %d\n", sys_info->out_channel);
			audio_ppc_main->ppc_reinit_flag = false;
		}
	}

	if (audio_ppc_main->ppc_reset_flag) {
		Printf("[Aud][PPC][ppcCreate] ppc_reset!\n");
		ret = ppc_reset(audio_ppc_main->ppc_hdl);
		if(PPC_SUCCESS != ret) {
			Printf("[Aud][PPC][ppcCreate] ppc_reinit return error(%d)\n", (int)ret);
			audio_ppc_result = AUD_PPC_RESET_FAIL;
		} else {
			Printf("[Aud][PPC][ppcCreate] ppc_reset return OK!\n");
			audio_ppc_main->ppc_reset_flag = false;
		}
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::ppcDataProcess(unsigned int frames, int *latency)
{
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;
	AUDIO_BUF *input_buf = (AUDIO_BUF *)audio_ppc_main->audio_buf_in;
	AUDIO_BUF *output_buf = (AUDIO_BUF *)audio_ppc_main->audio_buf_out;

	if (latency == NULL)
		ret = ppc_process(audio_ppc_main->ppc_hdl, input_buf, output_buf);
	else
		ret = ppc_process_vf(audio_ppc_main->ppc_hdl, input_buf, output_buf, frames, latency);

	if (PPC_SUCCESS != ret) {
		audio_ppc_result = AUD_PPC_PROCESS_FAIL;
		Printf("[Aud][PPC][PPC_Process] ppc_process error(%d)\n", (int)ret);
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::ppcSetPara(unsigned int element_id, unsigned int para_id, int num, int* value)
{
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	if (AUD_PPC_INFO_INITED == audio_ppc_main->ppc_info_status) {
		ret = ppc_set_para(audio_ppc_main->ppc_hdl, element_id, para_id, num, value);
		if (PPC_SUCCESS != ret) {
			audio_ppc_result = AUD_PPC_SET_PARA_FAIL;
			Printf("[Aud][PPC][ppcSetPara] ppc_set_para error(%d)\n", (int)ret);
		} else {
			if (0 != (para_id & (1 << 15))) {
				Printf("[Aud][PPC][ppcSetPara] ppc_set_para set reinit flag!\n");
				audio_ppc_main->ppc_reinit_flag = true;
				audio_ppc_result = AUD_PPC_SET_PARA_NEED_REINIT;
			}
		}
	} else {
		audio_ppc_result = AUD_PPC_SET_PARA_CANNOT;
		Printf("[Aud][PPC][ppcSetPara] ppc info is not INITED (but in %d state), can't set para!\n",
			(int)audio_ppc_main->ppc_info_status);
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::ppcGetPara(unsigned int element_id, unsigned int para_id, int num, int* value)
{
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	if (AUD_PPC_INFO_INITED == audio_ppc_main->ppc_info_status) {
		ret = ppc_get_para(audio_ppc_main->ppc_hdl, element_id, para_id, num, value);
		if (PPC_SUCCESS != ret) {
			audio_ppc_result = AUD_PPC_GET_PARA_FAIL;
			Printf("[Aud][PPC][ppcGetPara] ppc_get_para error(%d)\n", (int)ret);
		} else
			Printf("[Aud][PPC][ppcGetPara] ppc_get_para ok!\n");
	} else {
		audio_ppc_result = AUD_PPC_GET_PARA_CANNOT;
		Printf("[Aud][PPC][ppcGetPara] ppc info is not INITED (but in %d state), can't get para!\n",
			(int)audio_ppc_main->ppc_info_status);
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::ppcBypassElement(unsigned int element_id, int enabled)
{
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	if (AUD_PPC_INFO_INITED == audio_ppc_main->ppc_info_status) {
		ret = ppc_bypass_element(audio_ppc_main->ppc_hdl, element_id, enabled);
		if (PPC_SUCCESS != ret) {
			audio_ppc_result = AUD_PPC_BYPASS_ELEMENT_FAIL;
			Printf("[Aud][PPC][ppcBypassElement] ppc_bypass_element error=%d\n", (int)ret);
		} else {
			if(0 == (element_id & (1 << 31))) {
				Printf("[Aud][PPC][ppcBypassElement] ppc_set_para set reset flag!\n");
				audio_ppc_main->ppc_reset_flag = true;
				audio_ppc_result = AUD_PPC_BYPASS_ELEMENT_NEED_RESET;
			}
		}
	} else {
		audio_ppc_result = AUD_PPC_BYPASS_ELEMENT_CANNOT;
		Printf("[Aud][PPC][ppcBypassElement] ppc info is not INITED (but in %d state), can't set element bypass!\n",
			(int)audio_ppc_main->ppc_info_status);
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::ppcGetBypassStatus(unsigned int element_id, int *enabled)
{
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	if (AUD_PPC_INFO_INITED == audio_ppc_main->ppc_info_status) {
		ret = ppc_get_bypass_status(audio_ppc_main->ppc_hdl, element_id, enabled);
		if (PPC_SUCCESS != ret) {
			audio_ppc_result = AUD_PPC_GET_ELEMENT_BYPASS_STATUS_FAIL;
			Printf("[Aud][PPC][ppcGetBypassStatus] ppc_get_bypass_status error(%d)\n", (int)ret);
		}
	} else {
		audio_ppc_result = AUD_PPC_GET_ELEMENT_BYPASS_STATUS_CANNOT;
		Printf("[Aud][PPC][ppcGetBypassStatus] ppc info is not INITED (but in %d state), can't get element bypass!\n",
			(int)audio_ppc_main->ppc_info_status);
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::ppcChooseDumpPath(char *file_path)
{
	if (0 == ppcCheckPathExist(file_path))
		return 0;
	else {
		x_strcpy(file_path, AUD_PPC_U_DISK_PATH_00);
		if (0 == ppcCheckPathExist(file_path))
			return 0;
		else {
			x_strcpy(file_path, AUD_PPC_U_DISK_PATH_01);
			if (0 == ppcCheckPathExist(file_path))
				return 0;
			else
				return -1;
		}
	}
}

int AudioPPCControl::ppcCheckPathExist(char *file_path)
{
	int *file = NULL;
	int ret = 0;
	int path_len = 0;
	Printf("[Aud][PPC][ppcCheckPathExist]Try %s exist or not...\n", file_path);

	path_len = x_strlen(file_path) + x_strlen("0x00000000_0x00000001_out.pcm");
	if (path_len > (AUD_PPC_CFG_FILE_PATH_STRING_LENGTH)) {
		Printf("[Aud][PPC][ppcCheckPathExist]The path name is too long, please use shorter path name!\n");
		return -1;
	}

	ret = access(file_path, F_OK);
	if (ret == 0)
		Printf("[Aud][PPC][ppcCheckPathExist] %s exist\n", file_path);
	else
		Printf("[Aud][PPC][ppcCheckPathExist] %s not exist\n", file_path);

	return ret;
}

int AudioPPCControl::ppcDumpStatus(char* config_path, int passwd)
{
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	if (AUD_PPC_INFO_INITED == audio_ppc_main->ppc_info_status) {
		ret = ppc_dump_status(audio_ppc_main->ppc_hdl, config_path, passwd);
 		if (PPC_SUCCESS != ret) {
			audio_ppc_result = AUD_PPC_DUMP_STATUS_FAIL;
			Printf("[Aud][PPC][ppcDumpStatus] ppc_dump_status error(%d)\n", (int)ret);
		}
	} else {
		audio_ppc_result = AUD_PPC_DUMP_STATUS_CANNOT;
		Printf("[Aud][PPC][ppcDumpStatus] ppc info is not INITED (but in %d state), can't set dump element!\n",
			(int)audio_ppc_main->ppc_info_status);
	}

	return (int)audio_ppc_result;
}

int AudioPPCControl::ppcDumpElement(unsigned int element_id, int enabled, char* dump_folder)
{
	PPC_RESULT ret = PPC_SUCCESS;
	audio_ppc_result_t audio_ppc_result = AUD_PPC_SUCCESS;

	if (AUD_PPC_INFO_INITED == audio_ppc_main->ppc_info_status) {
		ret = ppc_dump_element(audio_ppc_main->ppc_hdl, element_id, enabled, dump_folder);
		if (PPC_SUCCESS != ret) {
			audio_ppc_result = AUD_PPC_DUMP_ELEMENT_FAIL;
			Printf("[Aud][PPC][ppcDumpElement] ppc_dump_element error(%d)\n", (int)ret);
		}
	} else {
		audio_ppc_result = AUD_PPC_DUMP_ELEMENT_CANNOT;
		Printf("[Aud][PPC][ppcDumpElement] ppc info is not INITED (but in %d state), can't set dump element!\n",
			(int)audio_ppc_main->ppc_info_status);
	}
	return (int)audio_ppc_result;
}


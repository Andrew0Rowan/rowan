/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef CONFIG_SUPPORT_FFMPEG
#include "SmartAudioPlayerDriver.h"
extern "C" void* player_init(void) {
    return CreatePlayer();
}
extern "C" status_t setDataSource_url(void* handle, const char *url, int id) {
    return ((SmartAudioPlayerDriver*)handle)->setDataSource(url, id);
}
extern "C" status_t setDataSource(void* handle, int id) {
    return ((SmartAudioPlayerDriver*)handle)->setDataSource(id);
}
extern "C" status_t setPCMMode(void* handle, int id) {
    return ((SmartAudioPlayerDriver*)handle)->setPCMMode(id);
}

extern "C" status_t setOutFileName(void* handle, const char *OutFileName) {
    return ((SmartAudioPlayerDriver*)handle)->setOutFileName(OutFileName);
}

extern "C" status_t prepare(void* handle) {
    return ((SmartAudioPlayerDriver*)handle)->prepare();
}
extern "C" status_t prepareAsync(void* handle) {
    return ((SmartAudioPlayerDriver*)handle)->prepareAsync();
}
extern "C" status_t start(void* handle) {
    return ((SmartAudioPlayerDriver*)handle)->start();
}
extern "C" status_t stop(void* handle) {
    return ((SmartAudioPlayerDriver*)handle)->stop();
}
extern "C" status_t pause_l(void* handle) {
    return ((SmartAudioPlayerDriver*)handle)->pause();
}
extern "C" void setPlaybackSettings(void* handle, const AudioFormat* format) {
    ((SmartAudioPlayerDriver*)handle)->setPlaybackSettings(format);
}
extern "C" status_t seekTo(void* handle, off64_t usec) {
    return ((SmartAudioPlayerDriver*)handle)->seekTo(usec);
}
extern "C" status_t reset(void* handle) {
    return ((SmartAudioPlayerDriver*)handle)->reset();
}
extern "C" status_t setLooping(void* handle, int loop) {
    return ((SmartAudioPlayerDriver*)handle)->setLooping(loop);
}
extern "C" status_t getDuration(void* handle, off64_t *usec) {
    return ((SmartAudioPlayerDriver*)handle)->getDuration(usec);
}
extern "C" status_t getCurrentPosition(void* handle, off64_t *usec) {
    return ((SmartAudioPlayerDriver*)handle)->getCurrentPosition(usec);
}
extern "C" void player_deinit(void* handle) {
    delete ((SmartAudioPlayerDriver*)handle);
    handle = NULL;
}
extern "C" status_t writeData(void* handle, unsigned char* buffer, int size) {
    return ((SmartAudioPlayerDriver*)handle)->writeData(buffer, size, false);
}
extern "C" status_t writeDatawithEOS(void* handle, unsigned char* buffer, int size) {
    return ((SmartAudioPlayerDriver*)handle)->writeData(buffer, size, true);
}
extern "C" status_t writePCMData(void* handle, unsigned char* buffer, int size) {
    return ((SmartAudioPlayerDriver*)handle)->writePCMData(buffer, size, false);
}
extern "C" status_t writePCMDatawithEOS(void* handle, unsigned char* buffer, int size) {
    return ((SmartAudioPlayerDriver*)handle)->writePCMData(buffer, size, true);
}
extern "C" long long int getAvailiableSize(void* handle) {
    return ((SmartAudioPlayerDriver*)handle)->getAvailiableSize();
}
extern "C" void registerCallback(void* handle, callback *fn) {
    ((SmartAudioPlayerDriver*)handle)->registerCallback(fn);
}

#else
#include "ABase.h"
#include "Mutex.h"
#include "Condition.h"
#include "AUtils.h"
#include "assert0.h"
#include "Format.h"

extern "C" void* player_init(void) {
    return NULL;
}
extern "C" status_t setDataSource_url(void* handle, const char *url, int id) {
    return OK;
}
extern "C" status_t setDataSource(void* handle, int id) {
    return OK;
}
extern "C" status_t setPCMMode(void* handle, int id) {
    return OK;
}

extern "C" status_t setOutFileName(void* handle, const char *OutFileName) {
    return OK;
}

extern "C" status_t prepare(void* handle) {
    return OK;
}
extern "C" status_t prepareAsync(void* handle) {
    return OK;
}
extern "C" status_t start(void* handle) {
    return OK;
}
extern "C" status_t stop(void* handle) {
    return OK;
}
extern "C" status_t pause_l(void* handle) {
    return OK;
}
extern "C" void setPlaybackSettings(void* handle, const AudioFormat* format) {
    return;
}
extern "C" status_t seekTo(void* handle, off64_t usec) {
    return OK;
}
extern "C" status_t reset(void* handle) {
    return OK;
}
extern "C" status_t setLooping(void* handle, int loop) {
    return OK;
}
extern "C" status_t getDuration(void* handle, off64_t *usec) {
    return OK;
}
extern "C" status_t getCurrentPosition(void* handle, off64_t *usec) {
    return OK;
}
extern "C" void player_deinit(void* handle) {
    handle = NULL;
    return;
}
extern "C" status_t writeData(void* handle, unsigned char* buffer, int size) {
    return OK;
}
extern "C" status_t writeDatawithEOS(void* handle, unsigned char* buffer, int size) {
    return OK;
}
extern "C" status_t writePCMData(void* handle, unsigned char* buffer, int size) {
    return OK;
}
extern "C" status_t writePCMDatawithEOS(void* handle, unsigned char* buffer, int size) {
    return OK;
}
extern "C" long long int getAvailiableSize(void* handle) {
    return 0;
}
extern "C" void registerCallback(void* handle, callback *fn) {
    return;
}

#endif


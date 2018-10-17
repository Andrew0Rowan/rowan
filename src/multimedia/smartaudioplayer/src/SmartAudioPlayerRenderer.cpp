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

#include "SmartAudioPlayerRenderer.h"
#include <pthread.h>
#include "FFmpeg.h"
#include "Log.h"
#include "Errors.h"
#include "MASRender.h"

SmartAudioPlayer::Renderer::Renderer(AMessageQueue* &notify)
    :mNotifyQueue(notify),
     mQueue(NULL) {
    mRender = new MASRender(mNotifyQueue);
}

status_t SmartAudioPlayer::Renderer::init() {

    status_t err = mRender->OpenRender();
    if (err != OK)
        return err;

    mRender->Start();
}
void SmartAudioPlayer::Renderer::start() {

    mRender->Start();
}

SmartAudioPlayer::Renderer::~Renderer() {
    if (NULL != mRender) {
        mRender->Release();
        delete mRender;
    }
    mRender = NULL;
}

void SmartAudioPlayer::Renderer::queueBuffer(ABuffer* &ab, int index){
    mRender->QueueOutputBuffer(ab,index);
}

void SmartAudioPlayer::Renderer::queueEOS() {
    mRender->QueueOutputBuffer(NULL, BUFFER_FLAG_EOS);
}

void SmartAudioPlayer::Renderer::pause(){

    mRender->Pause();
}

void SmartAudioPlayer::Renderer::resume(){

    mRender->Resume();
}
void SmartAudioPlayer::Renderer::release(){

    mRender->Release();
}

void SmartAudioPlayer::Renderer::flush(){

    mRender->Flush();
}

status_t SmartAudioPlayer::Renderer::SetHandle(AMessageQueue* &queue) {
    mQueue = queue;
    mRender->SetMsgHandle(mQueue);
}

status_t SmartAudioPlayer::Renderer::ConfigFormat(AudioFormat format)
{
    mRender->ConfigAlsaFormat(format);
}

status_t SmartAudioPlayer::Renderer::getCurrentPosition(off64_t *mediaUs)
{
    return mRender->getCurrentPosition(mediaUs);
}

#endif

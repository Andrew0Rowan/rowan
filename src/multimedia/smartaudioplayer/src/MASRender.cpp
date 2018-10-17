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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/prctl.h>

#include "MASRender.h"
#include "FFDecoder.h"
#include "Log.h"
#include "Errors.h"
#include "FFmpeg.h"
#include "assert0.h"

using namespace std;

MASRender::MASRender(AMessageQueue* &notify)
    : mMASRenderHandle(NULL),
      mHwParams(NULL),
      mIsComponentAlive(false),
      mBufferQueue(NULL),
      mPosition(-1ll),
      mPaused(false),
      mNotifyQueue(notify)
{
    Init();
}

MASRender::~MASRender()
{
    DeInit();
}

status_t MASRender::SetMsgHandle(AMessageQueue* &queue)
{
    mBufferQueue = queue;
    return OK;
}

status_t MASRender::OpenRender()
{

    Mutex::Autolock autoLock(mRenderLock);

    status_t ret = OK;
    unsigned int buffer_time = 0;
    unsigned int period_time = 0;
    int dir = 0;
    int retryNum = 0;

    while (retryNum < 100)
    {
        ret = snd_pcm_open(&mMASRenderHandle, mOutAudioFormat.mDeviceName, SND_PCM_STREAM_PLAYBACK, 0);
        if (ret < 0)
        {
            SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("open failed %s and retry %d times", snd_strerror(ret), retryNum));
            retryNum++;
            usleep(20000);
        } else {
            break;
        }
    }
    if (retryNum >= 100)
    {
        return UNKNOWN_ERROR;
    }
    SAP_LOG(SAP_RENDER_TAG,SAP_LOG_INFO,("open render %s\n",mOutAudioFormat.mDeviceName));

    ret = snd_pcm_hw_params_malloc(&mHwParams);
    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params_malloc Failed %s", snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }

    ret = snd_pcm_hw_params_any(mMASRenderHandle, mHwParams);
    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params_any Failed %s",  snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }

    ret = snd_pcm_hw_params_set_access(mMASRenderHandle, mHwParams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params_set_access Failed %s",  snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }

    /* we need switch the ffmpeg format to alsa format, now set it to SND_PCM_FORMAT_S16_LE*/
    ret = snd_pcm_hw_params_set_format(mMASRenderHandle, mHwParams, SND_PCM_FORMAT_S16_LE);
    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params_set_format Failed %s",  snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }

    ret = snd_pcm_hw_params_set_rate_near(mMASRenderHandle, mHwParams, (unsigned int*)&mOutAudioFormat.mSampleRate, &dir);

    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params_set_rate_near Failed %s",  snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }

    ret = snd_pcm_hw_params_set_channels(mMASRenderHandle, mHwParams, mOutAudioFormat.mChannels);

    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params_set_channels Failed %s",  snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }

    ret = snd_pcm_hw_params_get_buffer_time_max(mHwParams, &buffer_time, 0);
    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params_get_buffer_time_max Failed %s", snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }


    if (buffer_time > 80000)//must align to 16x
    {
        buffer_time = 80000;
    }
    period_time = buffer_time / 4;

    SAP_LOG(SAP_RENDER_TAG,SAP_LOG_INFO,("period_time %d buffer_time %d!", period_time, buffer_time));

    ret = snd_pcm_hw_params_set_buffer_time_near(mMASRenderHandle, mHwParams, &buffer_time, 0);
    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params_set_buffer_time_near Failed %s",  snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }

    ret = snd_pcm_hw_params_set_period_time_near(mMASRenderHandle, mHwParams, &period_time, 0);
    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params_set_period_time_near Failed %s",  snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }

    ret = snd_pcm_hw_params(mMASRenderHandle, mHwParams);
    if (ret < 0)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_hw_params Failed %s", snd_strerror(ret)));
        return UNKNOWN_ERROR;
    }
    return OK;
}
void MASRender::Init()
{
    memset((VOID *)(&mOutAudioFormat), 0, sizeof(AudioFormat));
}

status_t MASRender::DeInit()
{
    SAP_LOGENTRY(SAP_RENDER_TAG,SAP_LOG_TRACE,("CloseRender"));

    Mutex::Autolock autoLock(mRenderLock);

    if (NULL != mMASRenderHandle) {
        snd_pcm_drop(mMASRenderHandle);
    }
    if (NULL != mHwParams) {
        snd_pcm_hw_params_free(mHwParams);
    }
    status_t ret = OK;
    if (NULL != mMASRenderHandle) {
        ret = snd_pcm_close(mMASRenderHandle);
        if (ret < 0) {
            SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("snd_pcm_close Failed!"));
            return UNKNOWN_ERROR;
        }
    }
    mMASRenderHandle = NULL;
    mHwParams = NULL;
    memset((VOID *)(&mOutAudioFormat), 0, sizeof(AudioFormat));
    return OK;
}

void MASRender::Flush()
{
    SAP_LOGENTRY(SAP_RENDER_TAG,SAP_LOG_TRACE,("Flush"));
    if (true == mIsComponentAlive)
    {
        Release();
    }
    else {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("component is not alive"));
    }
    FlushAllBuf();
}

void MASRender::FlushAllBuf()
{
    Mutex::Autolock autoLock(mOutputBufQLock);
    while(!mOutputBufQ.empty())
    {
       mOutputBufQ.erase(mOutputBufQ.begin());
    }
    while(!mOutputBufQIndex.empty())
    {
       mOutputBufQIndex.erase(mOutputBufQIndex.begin());
    }
}

status_t MASRender::ConfigAlsaFormat(AudioFormat OutAudioPara)
{
    mOutAudioFormat.mChannelLayout = OutAudioPara.mChannelLayout;
    mOutAudioFormat.mSampleFmt = OutAudioPara.mSampleFmt;
    mOutAudioFormat.mSampleRate = OutAudioPara.mSampleRate;
    mOutAudioFormat.mChannels = OutAudioPara.mChannels;
    strncpy(mOutAudioFormat.mDeviceName,OutAudioPara.mDeviceName, strlen(OutAudioPara.mDeviceName));
    mOutAudioFormat.mDeviceName[strlen(OutAudioPara.mDeviceName)] = '\0';
    return OK;
}

void *MASRender::MASRenderThread()
{
    SAP_LOGENTRY(SAP_RENDER_TAG,SAP_LOG_TRACE,("MASRenderThread pid %u, tid %u", getpid(), gettid()));
    while (1) {
        ABuffer *ab = NULL;
        int index = 0;
        {
            Mutex::Autolock autoLock(mOutputBufQLock);
            while ((mOutputBufQ.empty() || mOutputBufQIndex.empty())
                && true == mIsComponentAlive && false == mPaused) {
                SAP_LOG(SAP_RENDER_TAG,SAP_LOG_INFO,("Render waiting buffers"));
                mRenderCondition.wait(mOutputBufQLock);
            }
            if (false == mIsComponentAlive) {
                SAP_LOG(SAP_RENDER_TAG,SAP_LOG_INFO,("component exit"));
                return NULL;
            }
            else if (true == mPaused) {
                SAP_LOG(SAP_RENDER_TAG,SAP_LOG_INFO,("Render need Paused"));
            }
            else {
                ab = DequeueOutputBuffer(&index);
            }
        }

        {
            Mutex::Autolock autoLock(mLock);
            while(true == mPaused && true == mIsComponentAlive) {

                AMessage  msg;
                msg.type = kWhatRendererPaused;

                assert0(mNotifyQueue!=NULL);
                assert0(mNotifyQueue->SendMessage(&msg)==OK);

                SAP_LOG(SAP_RENDER_TAG,SAP_LOG_INFO,("Render Paused"));

                mCondition.wait(mLock);
            }

            if (false == mIsComponentAlive) {
                SAP_LOG(SAP_RENDER_TAG,SAP_LOG_INFO,("component exit"));
                return NULL;
            }
        }

        if (BUFFER_FLAG_EOS != index) {
            if (NULL != ab) {
               Render(ab);
            }
            else {
               SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("DequeueOutputBuffer is NULL!"));
               continue;
            }
            AMessage  msg;
            msg.type = kWhatRenderBuffer;
            msg.bufferID= index;

            assert0(mBufferQueue!=NULL);
            assert0(mBufferQueue->SendMessage(&msg)==OK);
        }
        else {
            SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("End stream from outputbuffer!"));

            {
                Mutex::Autolock autoLock(mRenderLock);
                snd_pcm_drain(mMASRenderHandle);
                snd_pcm_prepare(mMASRenderHandle);
            }

            AMessage  msg;
            msg.type = kWhatRendererEOS;

            assert0(mNotifyQueue!=NULL);
            assert0(mNotifyQueue->SendMessage(&msg)==OK);
        }
    }

    return NULL;
}

ABuffer *MASRender::DequeueOutputBuffer(int *index)
{
    ABuffer *aBuffer = NULL;
    int input_idx = -1;
    if(!mOutputBufQ.empty())
    {
        aBuffer = *(mOutputBufQ.begin());
        mOutputBufQ.erase(mOutputBufQ.begin());
    }
    if(!mOutputBufQIndex.empty())
    {
        input_idx = *(mOutputBufQIndex.begin());
        mOutputBufQIndex.erase(mOutputBufQIndex.begin());
    }
    *index = input_idx;
    return aBuffer;
}

void MASRender::QueueOutputBuffer(ABuffer *pABuffer, int index)
{
    Mutex::Autolock autoLock(mOutputBufQLock);

    mOutputBufQ.push_back(pABuffer);
    mOutputBufQIndex.push_back(index);
    mRenderCondition.signal();
}

status_t MASRender::Render(ABuffer *pABuffer)
{
    Mutex::Autolock autoLock(mRenderLock);

    int ret = 0, sampleNum;
    off64_t pts = 0;

    if (NULL == pABuffer)
    {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("Render pABuffer is NULL!"));
        return UNKNOWN_ERROR;
    }

    sampleNum = pABuffer->GetSamNum();
    pts = pABuffer->GetPts();

    {
        Mutex::Autolock autoLock(mPositionLock);
        mPosition = pts;
    }

    while (sampleNum > 0)
    {
        snd_pcm_sframes_t frames;

        frames = snd_pcm_writei(mMASRenderHandle, pABuffer->data(), sampleNum);

        if (frames >= 0)
        {
            size_t bytes = snd_pcm_frames_to_bytes(mMASRenderHandle, frames);
            sampleNum -= frames;
            pABuffer->setRange(bytes, pABuffer->size() - bytes);
        }
        else
        {
            if (frames == -EPIPE)
            {
                /* EPIPE means underrun */
                SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("underrun occurred"));
                snd_pcm_prepare(mMASRenderHandle);
            }
            else if (frames < 0)
            {
                SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("error from writei"));
            }
        }
    }
    return OK;
}

void MASRender::Start()
{
    /* Active the mIsComponentAlive */
    mIsComponentAlive = true;
    int ret = pthread_create(&RenderThread, NULL, MASRenderThreadFunc, this);
    assert0(ret == 0);
}

void MASRender::Pause()
{
    SAP_LOGENTRY(SAP_RENDER_TAG,SAP_LOG_TRACE,("Pause"));

    if (false == mIsComponentAlive) {

        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_INFO,("component is not alive"));
        AMessage  msg;
        msg.type = kWhatRendererPaused;

        assert0(mNotifyQueue!=NULL);
        assert0(mNotifyQueue->SendMessage(&msg)==OK);

        return ;
    }

    {
        Mutex::Autolock autoLock(mLock);
        mPaused = true;
    }

    {
        Mutex::Autolock autoLock(mOutputBufQLock);
        mRenderCondition.signal();
    }
}

void MASRender::Resume()
{
    SAP_LOGENTRY(SAP_RENDER_TAG,SAP_LOG_TRACE,("Resume"));
    Mutex::Autolock autoLock(mLock);
    mPaused = false;
    mCondition.signal();
}


status_t MASRender::Release()
{
    SAP_LOGENTRY(SAP_RENDER_TAG,SAP_LOG_TRACE,("Release"));

    int *pRenderReturnVal = NULL;

    if (true == mIsComponentAlive) {
        mIsComponentAlive = false;
        {
            Mutex::Autolock autoLock(mOutputBufQLock);
            mRenderCondition.signal();
        }

        {
            Mutex::Autolock autoLock(mLock);
            mCondition.signal();
        }
        pthread_join(RenderThread,(void **)&pRenderReturnVal);
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_INFO,("render thread joined"));
    }
    else {
        SAP_LOG(SAP_RENDER_TAG,SAP_LOG_ERROR,("component is not alive"));

    }
    return OK;
}
status_t MASRender::getCurrentPosition(off64_t *mediaUs) {

    Mutex::Autolock autoLock(mPositionLock);

    *mediaUs = mPosition;
    return OK;
}

void *MASRender::MASRenderThreadFunc(void *pData)
{
    return (void *) ((MASRender *)pData)->MASRenderThread();
}

#endif

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

#include "FFmpeg.h"
#include "FFDecoder.h"
#include "Log.h"
#include "Errors.h"
#include <unistd.h>

#include "assert0.h"

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

bool fgDumpData = false;

using namespace std;

/* public member variable */
/* public member function */

/* private member variable */
/* private member function */


FFDecoder::FFDecoder()
    : mPacketNum(0),
      mStartPts(0),
      mEndPts(0),
      mCodecId(AV_CODEC_ID_NONE),
      mOutBufSize(0),
      mIsComponentAlive(false),
      mCodecCtx(NULL),
      mCodec(NULL),
      mSwrConvertCtx(NULL),
      mCodecPar(NULL),
      mOutBuffer(NULL),
      mNotifyQueue(NULL),
      mIsPCMSource(false)
{
    InitDecoder();
}

FFDecoder::FFDecoder(AMessageQueue* &notify)
    : mPacketNum(0),
      mStartPts(0),
      mEndPts(0),
      mCodecCtx(NULL),
      mCodec(NULL),
      mCodecId(AV_CODEC_ID_NONE),
      mCodecPar(NULL),
      mOutBuffer(NULL),
      mOutBufSize(0),
      mIsComponentAlive(false),
      mNotifyQueue(notify),
      mIsPCMSource(false)
{
    InitDecoder();
}


void FFDecoder::InitDecoder()
{
    time_base.num = 0;
    time_base.den = 0;
    memset((void *)(&mOutAudioFormat), 0, sizeof(AudioFormat));

    /* Init the mInputBufQ */
    {
        Mutex::Autolock autoLock(mInputBufAccessUnitLock);
        for(int i = 0; i < kInputBufferQueueCount; i++)
        {
            mInputBufAccessUnit.push_back(i);
        }
    }
    /* Init the mOutputBufQ */
    {
        Mutex::Autolock autoLock(mOutputBufQLock);
        for(int i = 0; i < kOutPutBufferQueueCount; i++)
        {
            mOutputBufQ.push_back(i);
        }
    }

    AllocateBufs();
}


FFDecoder::~FFDecoder()
{
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("~FFDecoder"));
    if(true == mIsComponentAlive)
    {
        Release();
    }
    DeInitDecoder();
}

void FFDecoder::DeInitDecoder()
{
    if(!mIsPCMSource)
    {
        if (NULL != mCodecCtx)
        {
            //pfnavcodec_close(mCodecCtx);
            pfnavcodec_free_context(&mCodecCtx);
            mCodecCtx = NULL;
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("avcodec_free_context success"));
        }
        else {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("mCodecCtx is NULL"));
        }
        if(NULL != mSwrConvertCtx)
        {
            pfnswr_free(&mSwrConvertCtx);
            mSwrConvertCtx = NULL;
        }

        mPacketNum = 0;
        mStartPts = 0;
        mEndPts = 0;
        mCodecId = AV_CODEC_ID_NONE;
        time_base.num = 0;
        time_base.den = 0;
        mOutBufSize = 0;

        memset((void *)(&mOutAudioFormat), 0, sizeof(AudioFormat));

        mCodecPar = NULL;
        mCodec = NULL;
        mOutBuffer = NULL;
    }
    FreeBufs();
}

status_t FFDecoder::AllocateBufs()
{
    int i = 0;

    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("AllocateBufs start"));
    for(i = 0; i < kInputBufferQueueCount; i++)
    {
        mPacket[i] = pfnav_packet_alloc();
        if(NULL == mPacket[i])
        {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Allocate AVPacket Failed!"));
            return NO_MEMORY;
        }
        pfnav_init_packet(mPacket[i]);
    }
    for(i = 0; i < kOutPutBufferQueueCount; i++)
    {
        mOutPutBuf[i] = new ABuffer(MAX_AUDIO_FRAME_SIZE * 2);
        if(NULL == mOutPutBuf[i])
        {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Allocate ABuffer Failed!"));
            return NO_MEMORY;
        }
    }

    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("AllocateBufs end"));
    return OK;
}

status_t FFDecoder::FreeBufs()
{
    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("FreeBufs start"));
    int i = 0;
    for(i = 0; i < kInputBufferQueueCount; i++)
    {
        if (NULL != mPacket[i])
        {
            //free(mPacket[i]);
            pfnav_packet_free(&mPacket[i]);
            mPacket[i] = NULL;
        }
    }
    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("input buffer have been freed"));
    /* The output buf will free in ~ABuffer!*/
    for(i = 0; i < kOutPutBufferQueueCount; i++)
    {
        if (NULL != mOutPutBuf[i])
        {
            delete mOutPutBuf[i];
            mOutPutBuf[i] = NULL;
        }
    }
    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("output buffer have been freed"));
    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("FreeBufs end"));
    return OK;
}

int FFDecoder::DequeueInputBuffer()
{
    int input_idx = -1;

    Mutex::Autolock autoLock(mInputBufQLock);

    while((true == mInputBufQ.empty()) && (true == mIsComponentAlive))
    {
        mInputBufQCondition.wait(mInputBufQLock);
    }

    if(!mInputBufQ.empty())
    {
        input_idx = *(mInputBufQ.begin());
        mInputBufQ.erase(mInputBufQ.begin());
    }
    return input_idx;
}

void FFDecoder::QueueInputBufferfront(int index)
{
    Mutex::Autolock autoLock(mInputBufQLock);
    mInputBufQ.insert(mInputBufQ.begin(), index);
    mInputBufQCondition.signal();
}

void FFDecoder::QueueOutputBufferfront(int index)
{
    Mutex::Autolock autoLock(mOutputBufQLock);
    mOutputBufQ.insert(mOutputBufQ.begin(), index);
    mOutputBufQCondition.signal();
}


void FFDecoder::QueueInputBuffer(int index)
{
    Mutex::Autolock autoLock(mInputBufQLock);
    mInputBufQ.push_back(index);
    mInputBufQCondition.signal();
}

int FFDecoder::DequeueInputBufferAccessUnit()
{
    int input_idx = -1;
    Mutex::Autolock autoLock(mInputBufAccessUnitLock);

    if(!mInputBufAccessUnit.empty())
    {
        input_idx = *(mInputBufAccessUnit.begin());
        mInputBufAccessUnit.erase(mInputBufAccessUnit.begin());
    }

    return input_idx;
}


void FFDecoder::QueueInputBufferAccessUnit(int index)
{
    {
        Mutex::Autolock autoLock(mInputBufAccessUnitLock);
        mInputBufAccessUnit.push_back(index);
    }

    AMessage  msg;
    msg.type = kWhatInputBufferAvaliable;
    assert0(mNotifyQueue!=NULL);
    assert0(mNotifyQueue->SendMessage(&msg)==OK);
}

int FFDecoder::DequeueOutputBuffer()
{
    int input_idx = -1;

    Mutex::Autolock autoLock(mOutputBufQLock);

    if(mIsPCMSource)
    {
        while(true == mOutputBufQ.empty())
        {
            mOutputBufQCondition.wait(mOutputBufQLock);
        }
    }
    else
    {
        while((true == mOutputBufQ.empty()) && (true == mIsComponentAlive))
        {
            mOutputBufQCondition.wait(mOutputBufQLock);
        }
    }

    if(!mOutputBufQ.empty())
    {
        input_idx = *(mOutputBufQ.begin());
        mOutputBufQ.erase(mOutputBufQ.begin());
    }

    return input_idx;
}

void FFDecoder::QueueOutputBuffer(int index)
{
    Mutex::Autolock autoLock(mOutputBufQLock);
    mOutputBufQ.push_back(index);
    mOutputBufQCondition.signal();
}

int FFDecoder::DequeueOutputBufferAccessUnit()
{
    int input_idx = -1;
    Mutex::Autolock autoLock(mOutputBufAccessUnitLock);
    if(!mOutputBufAccessUnit.empty())
    {
        input_idx = *(mOutputBufAccessUnit.begin());
        mOutputBufAccessUnit.erase(mOutputBufAccessUnit.begin());
    }
    return input_idx;
}

void FFDecoder::QueueOutputBufferAccessUnit(int index)
{
    {
        Mutex::Autolock autoLock(mOutputBufAccessUnitLock);
        mOutputBufAccessUnit.push_back(index);
    }

    AMessage  msg;
    msg.type = kWhatOutputBufferAvaliable;
    assert0(mNotifyQueue!=NULL);
    assert0(mNotifyQueue->SendMessage(&msg)==OK);
}

AVPacket *FFDecoder::GetInputBuf(int index)
{
    return mPacket[index];
}

ABuffer *FFDecoder::GetOutputBuf(int index)
{
    return mOutPutBuf[index];
}

void FFDecoder::PrintAllBufStatus()
{
    {
        Mutex::Autolock autoLock(mInputBufQLock);

        printf("mInputBufQ Status: ");
        for(vector<int>::iterator it = mInputBufQ.begin(); it != mInputBufQ.end(); it++)
        {
           printf("%d ", *it);
        }
        printf("\n");
    }

    {

        Mutex::Autolock autoLock(mInputBufAccessUnitLock);
        printf("mInputBufAccessUnit Status: ");
        for(vector<int>::iterator it = mInputBufAccessUnit.begin(); it != mInputBufAccessUnit.end(); it++)
        {
           printf("%d ", *it);
        }
        printf("\n");
    }

    {
        Mutex::Autolock autoLock(mOutputBufQLock);
        printf("mOutputBufQ Status: ");
        for(vector<int>::iterator it = mOutputBufQ.begin(); it != mOutputBufQ.end(); it++)
        {
           printf("%d ", *it);
        }
        printf("\n");
    }

    {
        Mutex::Autolock autoLock(mOutputBufAccessUnitLock);
        printf("mOutputBufAccessUnit Status: ");
        for(vector<int>::iterator it = mOutputBufAccessUnit.begin(); it != mOutputBufAccessUnit.end(); it++)
        {
           printf("%d ", *it);
        }
        printf("\n");
    }
}

void FFDecoder::Flush()
{
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("Flush"));
    if(true == mIsComponentAlive)
    {
        Release();
        SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("If you want Flush Decoder, You must Release it FirST!"));
    }
    if (NULL != mCodecCtx)
        pfnavcodec_flush_buffers(mCodecCtx);

    FlushAllBuf();
}

void FFDecoder::Start()
{
    /* Active the mIsComponentAlive */
    mIsComponentAlive = true;
    int ret = pthread_create(&DecoderThread, NULL, FFecodeThreadFunc, this);
    assert0(ret == 0);
}

void FFDecoder::setPCMMode() {
    mIsPCMSource = true;
}


status_t FFDecoder::Release()
{
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("Release"));

    int *pDecoderReturnVal = NULL;

    if(mIsPCMSource)
    {
        {
            Mutex::Autolock autoLock(mInputBufQLock);
            mInputBufQCondition.signal();
        }

        {
            Mutex::Autolock autoLock(mOutputBufQLock);
            mOutputBufQCondition.signal();
        }
    }

    if(true == mIsComponentAlive)
    {
        mIsComponentAlive = false;
        {
            Mutex::Autolock autoLock(mInputBufQLock);
            mInputBufQCondition.signal();
        }
        
        {
            Mutex::Autolock autoLock(mOutputBufQLock);
            mOutputBufQCondition.signal();
        }
        pthread_join(DecoderThread,(void **)&pDecoderReturnVal);
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("decoder thread joined"));
    }
    else {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("component is not alive"));
    }
    return OK;
}

status_t FFDecoder::OpenDecoder(Track *PPTrack)
{
    status_t i4_ret = OK;

    PPTrack->GetCodecId(&mCodecId);
    mCodecPar = PPTrack->GetCodecPar();
    time_base = PPTrack->GetStream()->time_base;
    mCodec = pfnavcodec_find_decoder(mCodecId);
    if(NULL == mCodec)
    {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Couldn't Find Decoder!"));
        return ERROR_MALFORMED;
    }

    mCodecCtx = pfnavcodec_alloc_context3(mCodec);
    if (!mCodecCtx)
    {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Couldn't Allocate Codec Context!"));
        return NO_MEMORY;
    }

    i4_ret = pfnavcodec_parameters_to_context(mCodecCtx, mCodecPar);
    if (i4_ret < 0)
    {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("avcodec_parameters_to_context Failed"));
        return ERROR_MALFORMED;
    }

    pfnav_codec_set_pkt_timebase(mCodecCtx, time_base);
    mCodecCtx->framerate = PPTrack->GetStream()->avg_frame_rate;

    if((i4_ret = pfnavcodec_open2(mCodecCtx, mCodec, NULL)) < 0)
    {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Couldn't Open Decoder!"));
        return ERROR_MALFORMED;
    }
    return OK;
}

status_t FFDecoder::ConfigDecoder(AudioFormat OutAudioPara)
{
    status_t i4_ret = OK;
    int out_nb_samples = 0;
    int out_channels = 0;
    off64_t in_channel_layout = 0;

    mOutAudioFormat.mChannelLayout = OutAudioPara.mChannelLayout;
    mOutAudioFormat.mSampleFmt = OutAudioPara.mSampleFmt;
    mOutAudioFormat.mSampleRate = OutAudioPara.mSampleRate;
    mOutAudioFormat.mChannels = OutAudioPara.mChannels;
    //Out Audio Default Param for ALSA playback
    out_nb_samples = mCodecCtx->frame_size;

    out_channels = pfnav_get_channel_layout_nb_channels(mOutAudioFormat.mChannelLayout);
    if(out_channels != OutAudioPara.mChannels)
    {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("The mChannels and mChannelLayout do not match,please set it again!"));
        return ERROR_MALFORMED;
    }

    in_channel_layout = pfnav_get_default_channel_layout(mCodecCtx->channels);

    mSwrConvertCtx = pfnswr_alloc();
    if(NULL == mSwrConvertCtx)
    {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Couldn't Allocate mSwrConvertCtx!"));
        return NO_MEMORY;
    }

    pfnav_opt_set_channel_layout(mSwrConvertCtx, "in_channel_layout",  in_channel_layout, 0);
    pfnav_opt_set_channel_layout(mSwrConvertCtx, "out_channel_layout", mOutAudioFormat.mChannelLayout,  0);
    pfnav_opt_set_int(mSwrConvertCtx, "in_sample_rate", mCodecCtx->sample_rate, 0);
    pfnav_opt_set_int(mSwrConvertCtx, "out_sample_rate", mOutAudioFormat.mSampleRate, 0);
    pfnav_opt_set_sample_fmt(mSwrConvertCtx, "in_sample_fmt",  mCodecCtx->sample_fmt, 0);
    pfnav_opt_set_sample_fmt(mSwrConvertCtx, "out_sample_fmt", mOutAudioFormat.mSampleFmt,  0);

    mSwrConvertCtx = pfnswr_alloc_set_opts(mSwrConvertCtx,mOutAudioFormat.mChannelLayout, mOutAudioFormat.mSampleFmt, mOutAudioFormat.mSampleRate,
        in_channel_layout,mCodecCtx->sample_fmt , mCodecCtx->sample_rate,0, NULL);
    if(NULL == mSwrConvertCtx)
    {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Couldn't Allocate mSwrConvertCtx Set Opt!"));
        return NO_MEMORY;
    }

    i4_ret = pfnswr_init(mSwrConvertCtx);
    if(i4_ret < 0)
    {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Could not open swr context!"));
        return NO_MEMORY;
    }
    return OK;
}

void FFDecoder::InitInputBufAccessUnit()
{
    Mutex::Autolock autoLock(mInputBufQLock);
    for(int i = 0; i < kInputBufferQueueCount; i++)
    {
        mInputBufAccessUnit.push_back(i);
    }
}

void FFDecoder::InitOutputBuf()
{
    Mutex::Autolock autoLock(mOutputBufQLock);
    for(int i = 0; i < kOutPutBufferQueueCount; i++)
    {
        mOutputBufQ.push_back(i);
    }
}


void FFDecoder::FlushAllBuf()
{
    DequeueInputBufferAll();
    DequeueInputBufferAccessUnitAll();
    DequeueOutputBufferAll();
    DequeueOutputBufferAccessUnitAll();

    /* ReInit the mOutputBufQ and InputBufAccessUnit*/
    InitInputBufAccessUnit();
    InitOutputBuf();
}

void FFDecoder::DequeueOutputBufferAll()
{
    Mutex::Autolock autoLock(mOutputBufQLock);
    while(!mOutputBufQ.empty())
    {
       mOutputBufQ.erase(mOutputBufQ.begin());
    }
}

void FFDecoder::DequeueOutputBufferAccessUnitAll()
{
    Mutex::Autolock autoLock(mOutputBufAccessUnitLock);
    while(!mOutputBufAccessUnit.empty())
    {
       mOutputBufAccessUnit.erase(mOutputBufAccessUnit.begin());
    }
}

void FFDecoder::DequeueInputBufferAll()
{
    Mutex::Autolock autoLock(mInputBufQLock);
    while(!mInputBufQ.empty())
    {
       mInputBufQ.erase(mInputBufQ.begin());
    }
}

void FFDecoder::DequeueInputBufferAccessUnitAll()
{
    Mutex::Autolock autoLock(mInputBufAccessUnitLock);
    while(!mInputBufAccessUnit.empty())
    {
       mInputBufAccessUnit.erase(mInputBufAccessUnit.begin());
    }
}


status_t FFDecoder::Decoder(AVPacket *packet, ABuffer *pABuffer) {
    status_t i4_ret = OK;
    AVFrame *frame = NULL;
    int dst_nb_samples = 0;
    int nb = 0;

    frame = pfnav_frame_alloc();
    if (NULL == frame) {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Allocate AVFrame Failed!"));
        return NO_MEMORY;
    }

    mOutBuffer = pABuffer->base();

    {
        i4_ret = pfnavcodec_send_packet(mCodecCtx, packet);

        if (i4_ret < 0 && i4_ret != AVERROR_EOF) {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("avcodec_send_packet error %d", i4_ret));
            pfnav_packet_unref(packet);
            if (NULL != frame) {
                pfnav_frame_free(&frame);
            }
            return i4_ret;
        }

        i4_ret = pfnavcodec_receive_frame(mCodecCtx, frame);
        if (i4_ret < 0 && i4_ret != AVERROR(EAGAIN)) {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("avcodec_receive_frame error %d", i4_ret));
            pfnav_packet_unref(packet);
            if (NULL != frame) {
                pfnav_frame_free(&frame);
            }
            return i4_ret;
        }

        dst_nb_samples = pfnav_rescale_rnd(pfnswr_get_delay(mSwrConvertCtx, frame->sample_rate) + frame->nb_samples, mOutAudioFormat.mSampleRate, frame->sample_rate, AV_ROUND_UP);
        nb = pfnswr_convert(mSwrConvertCtx, &mOutBuffer, dst_nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
        mOutBufSize = mOutAudioFormat.mChannels * nb * pfnav_get_bytes_per_sample(mOutAudioFormat.mSampleFmt);
        pABuffer->setRange(0, mOutBufSize);
        pABuffer->setSampleNum(nb);

        packet->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : AV_TIME_BASE * frame->pts * av_q2d(time_base);
        pABuffer->setPts(packet->pts);
        if (0 == mPacketNum) {
            mStartPts = packet->pts;
        }
        //PP_DBG_MSG("---index:%d\t pts:%lld\t packet size:%d %d %d %d\n", mPacketNum, packet->pts, packet->size, dst_nb_samples, nb, mOutBufSize);
        mEndPts = packet->pts;
        mPacketNum++;
        pfnav_packet_unref(packet);
    }

    if(0) //(mPacketNum % 1000 == 0) 
    {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("mPacketNum %lld, The Buf Status is", mPacketNum));
        PrintAllBufStatus();
    }

    if (NULL != frame) {
        pfnav_frame_free(&frame);
    }

    return i4_ret;
}

void *FFDecoder::FFecodeThread()
{
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("FFecodeThread pid %u, tid %u", getpid(), gettid()));

    FILE *pFile = NULL;
    if(true == fgDumpData)
    {
        pFile = fopen("/data/dump.pcm", "wb");  //Output PCM file for debug
        if (NULL ==pFile)
        {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("fopen failed."));
        }
    }

    while (1)
    {
        Mutex::Autolock autoLock(mDecodeLock);
        // dequeue an input buffer
        int input_idx = DequeueInputBuffer();
        if (false == mIsComponentAlive)
            break;

        if ((input_idx < 0) && (BUFFER_FLAG_EOS != input_idx))
        {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("DequeueInputBuffer Failed %d!", input_idx));
            continue;
        }

        // dequeue an output buffer
        int output_idx = DequeueOutputBuffer();
        if (false == mIsComponentAlive)
            break;

        if (output_idx < 0)
        {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("DequeueOutputBuffer Failed!", output_idx));
            QueueInputBufferfront(input_idx);
            continue;
        }

        if (BUFFER_FLAG_EOS == input_idx)
        {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("BUFFER_FLAG_EOS From InputBUF!"));

            AMessage  msg;
            msg.type = kWhatDecoderEndStream;
            assert0(mNotifyQueue!=NULL);
            assert0(mNotifyQueue->SendMessage(&msg)==OK);

            QueueOutputBufferfront(output_idx);
        }
        else
        {
            // send the input/output buffers to decoder
            if(Decoder(mPacket[input_idx], mOutPutBuf[output_idx]) >= 0)
            {
                if(true == fgDumpData)
                {
                    fwrite(mOutPutBuf[output_idx]->base(), 1, mOutPutBuf[output_idx]->size(), pFile);
                }
                QueueInputBufferAccessUnit(input_idx);
                QueueOutputBufferAccessUnit(output_idx);
            }
            else
            {
                SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Decoder ERROR!"));
                QueueInputBufferAccessUnit(input_idx);   //maybe this input buf is not avaliable
                QueueOutputBufferfront(output_idx);  //the output buf we will reuse
            }
        }
    }

    if(true == fgDumpData)
    {
        fclose(pFile);
    }

    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("mStartPts : %lld mEndPts : %lld!", mStartPts, mEndPts));
    return NULL;
}

void *FFDecoder::FFecodeThreadFunc(void *pData)
{
    return (void *) ((FFDecoder *)pData)->FFecodeThread();
}

#endif



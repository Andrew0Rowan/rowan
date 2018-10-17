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

#include <pthread.h>
#include "FFmpeg.h"
#include "SmartAudioPlayerDecoder.h"
#include "Format.h"
#include "Log.h"
#include "Errors.h"
#include <unistd.h>

static const bool enableDumpInput  = false;
static const bool enableDumpOutput = false;

static unsigned int dumpIndex = 0;

SmartAudioPlayer::Decoder::Decoder(AMessageQueue* &notify,
    SmartAudioPlayer::GenericSource* &source, SmartAudioPlayer::Renderer* &render)
      :mNotifyQueue(notify),
       mSource(source),
       mRenderer(render),
       mDecoder(NULL),
       mPaused(true),
       mInputFile(NULL),
       mOutputFile(NULL),
       mResumePending(false),
       mRequestInputBuffersPending(false),
       mInputPortEOS(false),
       mQueue(NULL),
       mIsPCMSource(false) {

    if (enableDumpInput) {
        char s_name[30+1];
        snprintf(s_name, sizeof(s_name), "/data/input-%u.mp3", dumpIndex);
        mInputFile = fopen(s_name, "wb");
        if (NULL==mInputFile) {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("open input file failed"));
        }
        else SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("open input file success"));
    }
    if (enableDumpOutput) {
        char s_name[30+1];
        snprintf(s_name, sizeof(s_name), "/data/output-%u.pcm", dumpIndex);
        mOutputFile = fopen(s_name, "wb");
        if (NULL==mOutputFile) {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("open output file failed"));
        }
        else SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("open output file success"));
    }
    ++dumpIndex;
    if (dumpIndex > 10000) {
        dumpIndex = 0;
    }
}

SmartAudioPlayer::Decoder::~Decoder() {
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("shutdown now"));

    if (NULL != mQueue) {
        AMessage  msg;
        msg.type = kWhatShutDownOwn;
        assert0(mQueue->SendMessage(&msg)==OK);

        pthread_join(mDecoderThread, NULL);
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("smartaudio decoder thread joined"));

        delete mQueue;
        mQueue = NULL;
    }
    else {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("decoder Queue is null"));
    }
    if (NULL != mNotifyQueue) {
        mNotifyQueue = NULL;
    }

    if (enableDumpInput && NULL!=mInputFile) {
        fflush(mInputFile);
        fclose(mInputFile);
    }
    if (enableDumpOutput && NULL!=mOutputFile) {
        fflush(mOutputFile);
        fclose(mOutputFile);
    }
}

void SmartAudioPlayer::Decoder::setPCMMode() {
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("setPCMMode"));
    mIsPCMSource = true;
}

status_t SmartAudioPlayer::Decoder::writePCMData(unsigned char* buffer,
        int size, status_t result) {
    if (!mIsPCMSource) {
        return UNKNOWN_ERROR;
    }
    ABuffer *pABuffer;
    int output_idx;
    int nb = 0;
    if (buffer != NULL && size > 0) {
        output_idx = mDecoder->DequeueOutputBuffer();
        while (-1 == output_idx) {
            output_idx = mDecoder->DequeueOutputBuffer();
        }

        pABuffer = mDecoder->mOutPutBuf[output_idx];

        memcpy(pABuffer->base(), buffer, size);
        nb = size / mFormat.mChannels / pfnav_get_bytes_per_sample(mFormat.mSampleFmt);

        pABuffer->setRange(0, size);
        pABuffer->setSampleNum(nb);
        mDecoder->QueueOutputBufferAccessUnit(output_idx);
    }
    if (result) {
        handleAnOutputBuffer(true);
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("writePCMData EOF"));
    }
    return OK;
}

void SmartAudioPlayer::Decoder::init() {
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("init"));

    mQueue = new AMessageQueue();
    assert0(mQueue != NULL);

    mRenderer->SetHandle(mQueue);

    status_t ret = pthread_create(&mDecoderThread, NULL,
        DecoderThreadFunc,(void *)this);

    assert0(ret == 0);
}

void SmartAudioPlayer::Decoder::configure(AudioFormat &format) {

    memcpy(&mFormat, &format, sizeof(format));

    AMessage  msg;
    msg.type = kWhatConfigure;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::Decoder::onConfigureSync(AudioFormat &format) {
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("onConfigureSync"));

    memcpy(&mFormat, &format, sizeof(format));
    mDecoder = new FFDecoder(mQueue);
    mDecoder->setPCMMode();
    mPaused = false;
    return;
}



void SmartAudioPlayer::Decoder::pause() {
    //AMessage* msg = new AMessage(kWhatPause, this);

}

void SmartAudioPlayer::Decoder::signalFlush() {

    AMessage  msg;
    msg.type = kWhatFlush;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::Decoder::signalResume(bool notifyComplete) {

    AMessage  msg;
    msg.type = kWhatResume;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::Decoder::initiateShutdown() {

    AMessage  msg;
    msg.type = kWhatShutdown;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::Decoder::InitInputBuffer() {
    for (int i= 0; i< kInputBufferQueueCount; i++) {
        handleAnInputBuffer();
        if (true == mPaused) {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("paused state"));
            break;
        }
        if (true == mInputPortEOS) {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("EOS state"));
            break;
        }
    }
}

void SmartAudioPlayer::Decoder::onConfigure() {

    status_t ret = OK;
    mDecoder = new FFDecoder(mQueue);

    if(!mIsPCMSource){
        ret = mDecoder->OpenDecoder(mSource->getMediaTrack(true));
        if (ret != OK) {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Fail to open decoder"));
            goto err;
        }
        ret = mDecoder->ConfigDecoder(mFormat);
        if (ret != OK) {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("Fail to config decoder"));
            goto err;
        }

        mDecoder->Start();
        InitInputBuffer();

        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("decoder config done"));
        printf("********channel layout: %lld ***********\n", mFormat.mChannelLayout);
        printf("********sample format:  %d   ***********\n", mFormat.mSampleFmt);
        printf("********sample rate:    %d   ***********\n", mFormat.mSampleRate);
        printf("********channels:       %d   ***********\n", mFormat.mChannels);
        printf("********device name:    %s   ***********\n", mFormat.mDeviceName);
    }

    mPaused = false;

    return;

err:
    handleError(UNKNOWN_ERROR);
}

bool SmartAudioPlayer::Decoder::handleAnInputBuffer() {
    status_t ret;

    int index = mDecoder->DequeueInputBufferAccessUnit();
    AVPacket* buffer = mDecoder->GetInputBuf(index);

    if (buffer == NULL) {
        handleError(UNKNOWN_ERROR);
        return false;
    }
    //printf("handleAnInputBuffer index %d\n", index);

    ret = mSource->dequeueAccessUnit(true, buffer);

    if (ret >= 0) {
        if (enableDumpInput && NULL != mInputFile) {
            fwrite(buffer->data, 1, buffer->size, mInputFile);
        }
        mDecoder->QueueInputBuffer(index);
    }
    else if (ret == ERROR_END_OF_STREAM) {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("End of File!"));
        mInputPortEOS = true;
        mDecoder->QueueInputBuffer(BUFFER_FLAG_EOS);
    }
    else {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("fetch data error"));        
        mInputPortEOS = true;
        mDecoder->QueueInputBuffer(BUFFER_FLAG_EOS);
        //handleError(UNKNOWN_ERROR);
    }
    return true;
}

void SmartAudioPlayer::Decoder::handleAnOutputBuffer(bool isEOS) {

    //mDecoder->QueueOutputBuffer(index);

    if (mRenderer != NULL) {
        // send the buffer to renderer
        if (!isEOS) {
            ABuffer *buffer;

            int index = mDecoder->DequeueOutputBufferAccessUnit();
            buffer = mDecoder->GetOutputBuf(index);

            //printf("handleAnOutputBuffer index %d\n", index);
            if (enableDumpOutput && NULL!=mOutputFile) {
                fwrite(buffer->base(), 1, buffer->size(), mOutputFile);
            }

            mRenderer->queueBuffer(buffer, index);
        }
        else {
            SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("handleAnOutputBuffer EOS"));
            mRenderer->queueEOS();
        }
    }
    else {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_ERROR,("render gone"));
    }

}

void SmartAudioPlayer::Decoder::handleError(int err) {
    // We cannot immediately release the codec due to buffers still outstanding
    // in the renderer.  We signal to the player the error so it can shutdown/release the
    // decoder after flushing and increment the generation to discard unnecessary messages.

    //++mBufferGeneration;
    mPaused = true;

    AMessage  msg;
    msg.type = kWhatError;
    msg.err = err;

    assert0(NULL != mNotifyQueue);
    assert0(mNotifyQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::Decoder::onResume(bool notifyComplete) {
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("onResume"));

    mPaused = false;
    mInputPortEOS = false;

    mRenderer->start();

    mDecoder->Start();

    InitInputBuffer();
}

void SmartAudioPlayer::Decoder::doFlush(bool notifyComplete) {
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("doFlush"));

    if (mRenderer != NULL) {
        SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("render flush"));
        //mRenderer->release();//no need already
        mRenderer->flush();
    }

    if (mDecoder != NULL) {
        SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("decoder flush"));
        mDecoder->Release();
        mDecoder->Flush();
    }
}

void SmartAudioPlayer::Decoder::onFlush() {
    SAP_LOGENTRY(SAP_DECODE_TAG,SAP_LOG_TRACE,("onFlush"));
    mPaused = true;

    doFlush(true);

    AMessage  msg;
    msg.type = kWhatFlushCompleted;

    assert0(NULL != mNotifyQueue);
    assert0(mNotifyQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::Decoder::onShutdown(bool notifyComplete) {
    status_t err = OK;

    // if there is a pending resume request, notify complete now

    if (mDecoder != NULL) {
        delete mDecoder;
        mDecoder = NULL;
    }
    else {
        SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("Decoder is NULL"));
    }
    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("delete Decoder DONE"));
    if (notifyComplete) {
        mPaused = true;

        AMessage  msg;
        msg.type = kWhatShutdownCompleted;
        msg.err = err;

        assert0(NULL != mNotifyQueue);
        assert0(mNotifyQueue->SendMessage(&msg)==OK);
    }
}

void SmartAudioPlayer::Decoder::onRenderBuffer(int index) {

    mDecoder->QueueOutputBuffer(index);
}

void *SmartAudioPlayer::Decoder::DecoderMessageRecieve() {

    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("DecoderMessageRecieve pid %u tid %u", getpid(), gettid()));
    AMessage msg;

    while (1) {
        mQueue->ReceiveMessage(&msg);
        switch (msg.type) {
            case kWhatShutDownOwn:
            {
                SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("kWhatShutDownOwn"));
                return NULL;
            }
            case kWhatConfigure:
            {
                SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("kWhatConfigure"));
                onConfigure();
                break;
            }
            case kWhatRenderBuffer:
            {
                if (mPaused) {
                    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("paused state"));
                    break;
                }
                //SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("kWhatRenderBuffer index %d",mMSG.IndexIDFromRender));

                onRenderBuffer(msg.bufferID);
                break;
            }
            case kWhatFlush:
            {
                SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("kWhatFlush"));
                onFlush();
                break;
            }
            case kWhatResume:
            {
                SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("kWhatResume"));
                onResume(msg.notifyComplete);
                break;
            }
            case kWhatShutdown:
            {
                onShutdown(true);
                break;
            }
            case kWhatInputBufferAvaliable:
            {
                if (mPaused) {
                    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("paused state"));
                    break;
                }
                //printf("CB_INPUT_AVAILABLE\n");
                if (mInputPortEOS == false)
                    handleAnInputBuffer();
                break;
            }
            case kWhatOutputBufferAvaliable:
            {
                if (mPaused) {
                    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("paused state"));
                    break;
                }
                //printf("CB_OUTPUT_AVAILABLE\n");
                handleAnOutputBuffer();

                break;
            }
            case kWhatDecoderEndStream:
            {
                if (mPaused) {
                    SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("paused state"));
                    break;
                }
                handleAnOutputBuffer(true);
                SAP_LOG(SAP_DECODE_TAG,SAP_LOG_INFO,("decoder EOF"));
                break;
            }
            default:
                break;
        }
    }
}
void *SmartAudioPlayer::Decoder::DecoderThreadFunc(void *data) {
    return (void *) ((SmartAudioPlayer::Decoder*)data)->DecoderMessageRecieve();

}

#endif

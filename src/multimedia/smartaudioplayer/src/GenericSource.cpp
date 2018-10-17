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
#include "GenericSource.h"
#include "Log.h"
#include "Errors.h"
#include "assert0.h"
#include <unistd.h>

const int kNumListenerQueuePackets = 1000;
#define CACHE_BUFFER_ENABLE 1
SmartAudioPlayer::GenericSource::GenericSource(AMessageQueue* &notify)
      :mStarted(false),
       mIsTTSource(false),
       mFetchDataSize(0),
       mDuration(-1ll),
       mGeneration(0),
       mUri(NULL),
       mStopRead(false),
       mDemuxer(NULL),
       mAudioTrack(NULL),
       mQueue(NULL),
       mAudioSource(NULL),
       mFinalResult(OK),
       mBuffering(false),
       mNotifyQueue(notify),
       mDumpPacket(NULL){
       
    mQueue = new AMessageQueue();
    assert0(mQueue != NULL);

    time_base.num = 0;
    time_base.den = 0;

    status_t ret = pthread_create(&mSourceThread, NULL,
            GenericThreadFunc, (void *)this);
    assert0(OK == ret);

}

SmartAudioPlayer::GenericSource::~GenericSource() {
    SAP_LOGENTRY(SAP_SOURCE_TAG,SAP_LOG_TRACE,("~GenericSource"));

    //stop the onReadBuffer
    {
        Mutex::Autolock autoLock(mBufferingLock);
        mBuffering = false;
    }
    time_base.num = 0;
    time_base.den = 0;

    if (NULL != mQueue) {
        AMessage  msg;
        msg.type = kWhatShutDownOwn;
        assert0(mQueue->SendMessage(&msg)==OK);

        pthread_join(mSourceThread, NULL);
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("source thread joined"));

        delete mQueue;
        mQueue = NULL;
    }
    else {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("source queue is NULL"));
    }
    if (NULL != mNotifyQueue) {
        mNotifyQueue = NULL;
    }

    if (mAudioTrack != NULL) {
        delete mAudioTrack;
        mAudioTrack = NULL;
    }
    if (mDemuxer != NULL) {
        delete mDemuxer;
        mDemuxer = NULL;
    }

    
    if (mAudioSource != NULL) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("mAudioSource clear"));
        mAudioSource->clear();
        delete mAudioSource;
        mAudioSource = NULL;
    }

    if(NULL != mDumpPacket){
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("free mDumpPacket"));
        pfnav_packet_free(&mDumpPacket);
        mDumpPacket = NULL;
    }

    resetDataSource();
}

void SmartAudioPlayer::GenericSource::resetDataSource() {
    SAP_LOGENTRY(SAP_SOURCE_TAG,SAP_LOG_TRACE,("resetDataSource"));

    if (mUri) free(mUri);
    mUri = NULL;

    mStarted = false;
    mStopRead = true;
}

status_t SmartAudioPlayer::GenericSource::setDataSource(const char *url, int generation) {
    SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_TRACE,("setDataSource %s\n", url));

    if (NULL==url) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("url is NULL"));
        return UNKNOWN_ERROR;
    }
    mUri = (char *)malloc(strlen(url) + 1);
    memcpy((void*)mUri, url, strlen(url));

    mUri[strlen(url)] = '\0';

    mGeneration = generation;

    // delay data source creation to prepareAsync() to avoid blocking
    // the calling thread in setDataSource for any significant time.
    status_t err = initFromDataSource();

    if (err != OK) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("Failed to init from data source!"));
        return err;
    }

    return OK;
}

status_t SmartAudioPlayer::GenericSource::setDataSource(int generation) {
    SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_TRACE,("setDataSource tts"));

    mIsTTSource = true;

    mGeneration = generation;

    status_t err = initFromDataSource();

    if (err != OK) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("Failed to init from data source!"));
        return err;
    }

    // delay data source creation to prepareAsync() to avoid blocking
    // the calling thread in setDataSource for any significant time.
    return OK;
}

status_t SmartAudioPlayer::GenericSource::prepare() {
    SAP_LOGENTRY(SAP_SOURCE_TAG,SAP_LOG_TRACE,("prepare"));
#if 0
    if (mIsTTSource && (mFetchDataSize < kFetchSize)) {
        printf("[source]data not enough\n");
        return UNKNOWN_ERROR;
    }
#endif
    if (mDemuxer)
        mDemuxer->Start();

    mAudioTrack = mDemuxer->GetTrack(0);

    if (NULL == mAudioTrack) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("GetTrack fail"));
        return UNKNOWN_ERROR;
    }

    if (mAudioTrack)
        mAudioTrack->Start();
    
    time_base = mAudioTrack->GetStream()->time_base;

    return OK;
}

void SmartAudioPlayer::GenericSource::prepareAsync() {
    SAP_LOGENTRY(SAP_SOURCE_TAG,SAP_LOG_TRACE,("prepareAsync"));

    AMessage  msg;
    msg.type = kWhatPrepareAsync;

    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::GenericSource::start() {
    mStarted = true;

}

void SmartAudioPlayer::GenericSource::stop() {

}

void SmartAudioPlayer::GenericSource::pause() {
    // nothing to do
    //mStarted = false;
    mDemuxer->pause();
}

void SmartAudioPlayer::GenericSource::resume() {
    //mStarted = true;
    mDemuxer->resume();
}

void SmartAudioPlayer::GenericSource::disconnect() {
    mDemuxer->Disconnect();
}

status_t SmartAudioPlayer::GenericSource::writeData(unsigned char* buffer,
        int size, status_t result) {
    if (!mDemuxer) {
        return UNKNOWN_ERROR;
    }
    if (buffer != NULL && size > 0) {
        int sz = mDemuxer->Write(buffer, size);
        mFetchDataSize += sz;
        if (sz < 0) {
            return INVALID_OPERATION;
        }
        if (sz < size) {
            SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,
                    ("warning, write in size %ld, out size %ld", size, sz));
        }
    }
    if (result) {
        mDemuxer->signalEOS();
    }
    return OK;
}

off64_t SmartAudioPlayer::GenericSource::getAvailiableSize() {
    if (!mDemuxer) {
        return UNKNOWN_ERROR;
    }
    off64_t sz = mDemuxer->GetAvailableSize();
    if (sz < 0) {
        return INVALID_OPERATION;
    }
    return sz;
}

status_t SmartAudioPlayer::GenericSource::dequeueAccessUnit(
        bool audio, AVPacket *pkt) {
    if ((audio && !mStarted) || mAudioSource == NULL) {
        return -EWOULDBLOCK;
    }


#if CACHE_BUFFER_ENABLE
    if(false == mBuffering){
        if((NULL == mDumpPacket) || !mDemuxer->getmSaveHeader()){
            if (!haveSufficientDataOnAllTracks()) {
                postReadBuffer();
            }
        }
        else{
            postReadBuffer(); 
        }
    }

    status_t err = mAudioSource->dequeueAccessUnit(pkt);
    return err;
#else
    if (audio)
        return mAudioTrack->ReadAPacket(pkt);
    #endif
}

Track* SmartAudioPlayer::GenericSource::getMediaTrack(bool audio) {
    if (audio)
        return mAudioTrack;

    return NULL;
}

status_t SmartAudioPlayer::GenericSource::initFromDataSource() {
    SAP_LOGENTRY(SAP_SOURCE_TAG,SAP_LOG_TRACE,("initFromDataSource"));
    status_t i4_ret = 0;

    if (NULL == mDemuxer) {
        if (mUri)
            mDemuxer = new FFDemuxer(mUri);
        else
            mDemuxer = new FFDemuxer();
    }

    if (NULL == mDemuxer) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("Failed to create data source!"));
        return UNKNOWN_ERROR;
    }

    i4_ret = mDemuxer->OpenDemux();
    if (OK != i4_ret) {
        return UNKNOWN_ERROR;
    }

    return OK;
}

status_t SmartAudioPlayer::GenericSource::seekTo(off64_t seekTime) {
    status_t ret = OK;

    //stop the onReadBuffer
    {
        Mutex::Autolock autoLock(mBufferingLock);
        mBuffering = false;
        mFinalResult = OK;
    }
    
    ret = mAudioTrack->SeekByPts(seekTime);
    if (ret < 0)
    {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("SeekBySecond fail!"));
        return ret;
    }


    if (mAudioSource != NULL) {
        mAudioSource->clear();
    }
#if CACHE_BUFFER_ENABLE
    postReadBuffer();
#endif
    return OK;
}

void SmartAudioPlayer::GenericSource::setError(status_t err) {
    Mutex::Autolock autoLock(mBufferingLock);
    mFinalResult = err;
}
bool SmartAudioPlayer::GenericSource::haveSufficientDataOnAllTracks() {
    // We're going to buffer at least 10 secs worth data on all tracks before
    // starting playback (both at startup and after a seek).
    static const off64_t kMinDurationUs = 10000000ll;

    status_t err;
    off64_t durationUs;
    if (mAudioSource != NULL
            && (durationUs = mAudioSource->getBufferedDurationUs(&err) * av_q2d(time_base) * AV_TIME_BASE)
                    < kMinDurationUs
            && err == OK) {
        static off64_t LastUpdateUs = 0;
        off64_t nowUs = systemTime();
        if (nowUs - LastUpdateUs > 5000*1000*1000LL) {
            SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("audio track doesn't have enough data yet. (%.2f secs buffered)",
                (float)durationUs / AV_TIME_BASE));
            
            LastUpdateUs = nowUs;
        }
        return false;
    }

    return true;
}

status_t SmartAudioPlayer::GenericSource::postReadBuffer() {    
    {
        Mutex::Autolock autoLock(mBufferingLock);
        if (mFinalResult != OK) {
            return mFinalResult;
        }
        if (mBuffering) {
            return OK;
        }
        mBuffering = true;
    }
    SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("postReadBuffer!"));

    AMessage  msg;
    msg.type = kWhatReadBuffer;
    assert0(mQueue->SendMessage(&msg)==OK);

    return OK;
}

void SmartAudioPlayer::GenericSource::onReadBuffer() {  
    SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("onReadBuffer!"));
    for (int i = 0; i < kNumListenerQueuePackets; ++i) {
        if(true == mBuffering){                
            AVPacket* buffer = pfnav_packet_alloc();
            if (NULL == buffer) {
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("Failed to create AVPacket!"));
                mAudioSource->signalEOS(UNKNOWN_ERROR);
                setError(UNKNOWN_ERROR);
                return;
            }
            pfnav_init_packet(buffer);

            status_t ret = mAudioTrack->ReadAPacket(buffer);

            //Write
            if((NULL != mDumpPacket) && mDemuxer->getmSaveHeader()){
                pfnav_copy_packet(mDumpPacket, buffer);
                if (pfnav_interleaved_write_frame(mDemuxer->GetOutFormatCtx(), mDumpPacket) < 0) {
                    SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("Error muxing packet\n"));
                }
            }


            if (ret >= 0) {
                mAudioSource->queueAccessUnit(buffer);
            }
            else if (ret == ERROR_END_OF_STREAM) {
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("[%d]input data EOS reached.", i));
                pfnav_packet_free(&buffer);
                mAudioSource->signalEOS(ERROR_END_OF_STREAM);
                setError(ERROR_END_OF_STREAM);
                notifyEOS(ret);
                //Write
                if((NULL != mDumpPacket) && mDemuxer->getmSaveHeader()){
                    pfnav_write_trailer(mDemuxer->GetOutFormatCtx());
                    pfnav_packet_free(&mDumpPacket);
                    mDumpPacket = NULL;
                }
                break;
            }
            else {
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("[%d]unknown error.", i));
                pfnav_packet_free(&buffer);
                mAudioSource->signalEOS(ret);
                setError(ret);
                notifyEOS(ret);
                //Write
                if((NULL != mDumpPacket) && mDemuxer->getmSaveHeader()){
                    pfnav_write_trailer(mDemuxer->GetOutFormatCtx());                    
                    pfnav_packet_free(&mDumpPacket);
                    mDumpPacket = NULL;
                }
                break;
            }
        }
        else{
            break;
        }
    }
}

void SmartAudioPlayer::GenericSource::notifyEOS(status_t err) {

    AMessage  msg;
    msg.type = kWhatSourceEOS;
    msg.err = err;
    msg.generation = mGeneration;

    assert0(NULL != mNotifyQueue);
    assert0(mNotifyQueue->SendMessage(&msg)==OK);
}


void SmartAudioPlayer::GenericSource::notifyPrepared(status_t err) {

    AMessage  msg;
    msg.type = kWhatPrepared;
    msg.err = err;
    msg.generation = mGeneration;

    assert0(NULL != mNotifyQueue);
    assert0(mNotifyQueue->SendMessage(&msg)==OK);
}
void SmartAudioPlayer::GenericSource::notifyPreparedAndCleanup(status_t err) {
    SAP_LOGENTRY(SAP_SOURCE_TAG,SAP_LOG_TRACE,("notifyPreparedAndCleanup"));
    notifyPrepared(err);
}

void SmartAudioPlayer::GenericSource::onPrepareAsync() {
    SAP_LOGENTRY(SAP_SOURCE_TAG,SAP_LOG_TRACE,("onPrepareAsync"));
    assert0(mDemuxer != NULL);
    status_t err = mDemuxer->Start();
    if (err != OK) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("demuxer start fail"));
        notifyPreparedAndCleanup(UNKNOWN_ERROR);
        return;
    }

    if (mDemuxer->getDuration(&mDuration) != OK) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("get duration fail"));
    }

    mAudioTrack = mDemuxer->GetTrack(0);
    if (mAudioTrack == NULL) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("GetTrack fail"));
        notifyPreparedAndCleanup(UNKNOWN_ERROR);
        return;
    }
    mAudioTrack->Start();    
    time_base = mAudioTrack->GetStream()->time_base;

    mAudioSource = new AVPacketSource();
    if (mAudioSource == NULL) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("new AVPacketSource fail"));
        notifyPreparedAndCleanup(UNKNOWN_ERROR);
        return;
    }

#if CACHE_BUFFER_ENABLE
    postReadBuffer();
#endif
    finishPrepareAsync();
    return;

}

void SmartAudioPlayer::GenericSource::finishPrepareAsync() {
    notifyPrepared();
}

status_t SmartAudioPlayer::GenericSource::getDuration(off64_t *duration) {
    *duration = mDuration;
    return OK;
}

void *SmartAudioPlayer::GenericSource::GenericMessageReceive(void) {

    SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("GenericMessageReceive pid %u tid %u", getpid(), gettid()));
    AMessage msg;

    while (1) {
        mQueue->ReceiveMessage(&msg);
        switch (msg.type) {
            case kWhatShutDownOwn:
            {
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("kWhatShutDownOwn"));
                return NULL;
            }
            case kWhatPrepareAsync:
            {
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("kWhatPrepareAsync"));
                onPrepareAsync();
                break;
            }
            case kWhatReadBuffer:
            {
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("kWhatReadBuffer"));
                onReadBuffer();
                {
                    Mutex::Autolock autoLock(mBufferingLock);
                    mBuffering = false;
                }
                break;
            }
        }
    }
}

void *SmartAudioPlayer::GenericSource::GenericThreadFunc(void *data) {

    return (void *) ((SmartAudioPlayer::GenericSource *)data)->GenericMessageReceive();

}

status_t SmartAudioPlayer::GenericSource::SetOutFileName(const char *OutFileName) {
    SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_TRACE,("SetOutFileName %s\n", OutFileName));

    if (NULL == OutFileName) {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("OutFileName is NULL"));
        return UNKNOWN_ERROR;
    }
    mDemuxer->SetOutFileName(OutFileName,strlen(OutFileName));

    if(NULL != mDumpPacket){
        pfnav_packet_free(&mDumpPacket);
        mDumpPacket = NULL;
    }

    mDumpPacket = pfnav_packet_alloc();
    if(NULL == mDumpPacket)
    {
        SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_ERROR,("Allocate mDumpPacket Failed!"));
        return NO_MEMORY;
    }
    pfnav_init_packet(mDumpPacket);

    return OK;
}

#endif


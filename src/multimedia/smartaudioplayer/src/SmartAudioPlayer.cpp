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
#include "SmartAudioPlayerDecoder.h"
#include "SmartAudioPlayerRenderer.h"
#include "SmartAudioPlayerDriver.h"
#include "SmartAudioPlayer.h"
#include "Log.h"
#include "Errors.h"

#include <unistd.h>

//using namespace std;

void *SmartAudioPlayerMessageReceive(void *data);

struct SmartAudioPlayer::Action {
    Action() {}

    virtual void execute(SmartAudioPlayer *player) = 0;

private:
    DISALLOW_EVIL_CONSTRUCTORS(Action);
};

struct SmartAudioPlayer::SeekAction : public Action {
    SeekAction(off64_t seekTime)
        : mSeekTime(seekTime) {
    }

    virtual void execute(SmartAudioPlayer *player) {
        player->performSeek(mSeekTime);
    }

private:
    int mSeekTime;

    DISALLOW_EVIL_CONSTRUCTORS(SeekAction);
};
struct SmartAudioPlayer::ResumeDecoderAction : public Action {
    ResumeDecoderAction(bool needNotify)
        : mNeedNotify(needNotify) {
    }

    virtual void execute(SmartAudioPlayer *player) {
        player->performResumeDecoders(mNeedNotify);
    }

private:
    bool mNeedNotify;

    DISALLOW_EVIL_CONSTRUCTORS(ResumeDecoderAction);
};
struct SmartAudioPlayer::FlushDecoderAction : public Action {
    FlushDecoderAction(FlushCommand audio)
        : mAudio(audio) {
    }

    virtual void execute(SmartAudioPlayer *player) {
        player->performDecoderFlush(mAudio);
    }

private:
    FlushCommand mAudio;

    DISALLOW_EVIL_CONSTRUCTORS(FlushDecoderAction);
};

// Use this if there's no state necessary to save in order to execute
// the action.
struct SmartAudioPlayer::SimpleAction : public Action {
    typedef void (SmartAudioPlayer::*ActionFunc)();

    SimpleAction(ActionFunc func)
        : mFunc(func) {
    }

    virtual void execute(SmartAudioPlayer *player) {
        (player->*mFunc)();
    }

private:
    ActionFunc mFunc;

    DISALLOW_EVIL_CONSTRUCTORS(SimpleAction);
};

////////////////////////////////////////////////////////////////////////////////

unsigned int SmartAudioPlayer::mCount = 0;

SmartAudioPlayer::SmartAudioPlayer(pid_t pid)
    :mStarted(false),
     mPrepared(false),
     mSourceStarted(false),
     mAudioEOS(false),
     mPaused(false),
     mResetting(false),
     mResumePending(false),
     mGeneration(0),
     mSource(NULL),
     mRenderer(NULL),
     mAudioDecoder(NULL),
     mFlushingAudio(NONE),
     mQueue(NULL),
     mIsPCMSource(false) {

    SAP_LOG(SAP_TAG,SAP_LOG_TRACE,("SmartAudioPlayer Create"));

    clearFlushComplete();
    clearFormat();

    mDeferredActions.clear();

    if (mCount == 0) {
        int ret=initffmpeg();
        assert0(ret==0);
    }

    mCount++;
    mQueue = new AMessageQueue();
    assert0(mQueue != NULL);

    status_t ret = pthread_create(&mSmartAudioPlayerThread, NULL,
        PlayerThreadFunc, (void *)this);
    assert0(OK == ret);
}

SmartAudioPlayer::~SmartAudioPlayer() {
    SAP_LOG(SAP_TAG,SAP_LOG_TRACE,("SmartAudioPlayer shutdown"));

    if (NULL != mQueue) {
        AMessage  msg;
        msg.type = kWhatShutDownOwn;
        assert0(mQueue->SendMessage(&msg)==OK);

        pthread_join(mSmartAudioPlayerThread, NULL);

        delete mQueue;
        mQueue = NULL;
    }

    list<Action *>::iterator it = mDeferredActions.begin();
    while (it != mDeferredActions.end()) {
        Action* &act = *it;
        delete act;
        ++it;
    }
    mDeferredActions.clear();

    mCount--;

    if (mCount==0)
        exitffmpeg();
}

void SmartAudioPlayer::setDriver(SmartAudioPlayerDriver* driver) {
    mDriver = driver;
}

void SmartAudioPlayer::setDataSourceAsync(const char *url) {

    mSource = new GenericSource(mQueue);
    status_t err = mSource->setDataSource(url, ++mGeneration);

    if (err != OK) {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("Failed to set data source!"));
        delete mSource;
        mSource = NULL;
    }

    AMessage  msg;
    msg.type = kWhatSetDataSource;
    msg.err = err;
    assert0(mQueue->SendMessage(&msg)==OK);
}

status_t SmartAudioPlayer::setOutFileName(const char *OutFileName) {
    status_t err = mSource->SetOutFileName(OutFileName);
    return err;
}

void SmartAudioPlayer::setPCMMode() {
    SAP_LOGENTRY(SAP_TAG,SAP_LOG_TRACE,("setPCMMode"));
    mIsPCMSource = true;
    mPrepared = true;
}

void SmartAudioPlayer::setDataSourceAsync() {

    mSource = new GenericSource(mQueue);
    status_t err = mSource->setDataSource(++mGeneration);

    if (err != OK) {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("Failed to set data source!"));
        delete mSource;
        mSource = NULL;
    }

    AMessage  msg;
    msg.type = kWhatSetDataSource;
    msg.err = err;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::prepareAsync() {

    AMessage  msg;
    msg.type = kWhatPrepare;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::start() {
    AMessage  msg;
    msg.type = kWhatStart;
    msg.needNotify = true;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::start_pcm() {

    AMessage  msg;
    msg.type = kWhatStart;
    msg.needNotify = true;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::pause() {

    AMessage  msg;
    msg.type = kWhatPause;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::resetAsync() {
    if (mSource != NULL) {
        // During a reset, the data source might be unresponsive already, we need to
        // disconnect explicitly so that reads exit promptly.
        // We can't queue the disconnect request to the looper, as it might be
        // queued behind a stuck read and never gets processed.
        // Doing a disconnect outside the looper to allows the pending reads to exit
        // (either successfully or with error).
        Mutex::Autolock autoLock(mSourceLock);
        mSource->disconnect();
    }

    AMessage  msg;
    msg.type = kWhatReset;
    assert0(mQueue->SendMessage(&msg)==OK);
}

void SmartAudioPlayer::seekToAsync(off64_t seekTime, bool needNotify) {

    AMessage  msg;
    msg.type = kWhatSeek;
    msg.seekTime = seekTime;
    msg.needNotify = needNotify;
    assert0(mQueue->SendMessage(&msg)==OK);
}

status_t SmartAudioPlayer::getCurrentPosition(off64_t*mediaUs) {
    if (NULL == mRenderer) {
        return NO_INIT;
    }

    return mRenderer->getCurrentPosition(mediaUs);
}

off64_t SmartAudioPlayer::getAvailiableSize(void) {

    if (mResetting) {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("resetting player"));
        return UNKNOWN_ERROR;
    }

    assert0(mSource != NULL);
    //{
        //Mutex::Autolock autoLock(mSourceLock);
        return mSource->getAvailiableSize();
    //}
}

status_t SmartAudioPlayer::writeData(unsigned char* buffer, int size, status_t result) {

    SAP_LOGENTRY(SAP_TAG,SAP_LOG_TRACE,("writeData result %d", result));

    status_t ret = OK;
    if (mResetting) {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("resetting player"));
        return UNKNOWN_ERROR;
    }
    assert0(mSource != NULL);
    //{
        //Mutex::Autolock autoLock(mSourceLock);
        ret = mSource->writeData(buffer, size, result);
    //}

    return ret;
}

status_t SmartAudioPlayer::writePCMData(unsigned char* buffer, int size, status_t result) {
    status_t ret = OK;
    if (mResetting) {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("resetting player"));
        return UNKNOWN_ERROR;
    }
    if(NULL == mAudioDecoder){
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("writePCMData must after start!"));
        return UNKNOWN_ERROR;
    }

    ret = mAudioDecoder->writePCMData(buffer, size, result);

    return ret;
}

void SmartAudioPlayer::onStart(off64_t startPositionUs) {
    SAP_LOGENTRY(SAP_TAG,SAP_LOG_TRACE,("onStart mIsPCMSource %d", mIsPCMSource));

    if (!mSourceStarted) {
        mSourceStarted = true;
        if (mSource != NULL) {
            mSource->start();
        }
    }

    if (startPositionUs >= 0) {
        performSeek(startPositionUs);
        return;
    }

    mAudioEOS = false;
    mStarted  = true;
    mPaused   = false;

    mRenderer = new Renderer(mQueue);
    mRenderer->ConfigFormat(mFormat);
    status_t err = mRenderer->init();
    if (err != OK) {
        mSourceStarted = false;
        notifyListener(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, err);
        return;
    }

    mAudioDecoder = new Decoder(mQueue, mSource, mRenderer);
    mAudioDecoder->init();
    if(mIsPCMSource){
        mAudioDecoder->setPCMMode();
        mAudioDecoder->onConfigureSync(mFormat);
    }else{
        mAudioDecoder->configure(mFormat);
    }
}

void SmartAudioPlayer::onPause() {
    SAP_LOGENTRY(SAP_TAG,SAP_LOG_TRACE,("onPause"));

    if (mPaused) {
        return;
    }
    mPaused = true;
    if (mSource != NULL) {
        mSource->pause();
    } else {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("pause called when source is gone or not set"));
    }
    if (mRenderer != NULL) {
        mRenderer->pause();
    } else {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("pause called when renderer is gone or not set"));
    }
}
void SmartAudioPlayer::onResume() {
    SAP_LOGENTRY(SAP_TAG,SAP_LOG_TRACE,("onResume"));

    if (!mPaused || mResetting) {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("resetting, onResume discarded"));
        return;
    }
    mPaused = false;
    if (mSource != NULL) {
        mSource->resume();
    } else {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("resume called when source is gone or not set"));
    }
    // |mAudioDecoder| may have been released due to the pause timeout, so re-create it if
    // needed.
    if (mRenderer != NULL) {
        mRenderer->resume();
    } else {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("resume called when renderer is gone or not set"));
    }

}
void SmartAudioPlayer::processDeferredActions() {
    while (!mDeferredActions.empty()) {
        // We won't execute any deferred actions until we're no longer in
        // an intermediate state, i.e. one more more decoders are currently
        // flushing or shutting down.

        if (mFlushingAudio != NONE) {
            // We're currently flushing, postpone the reset until that's
            // completed.

            SAP_LOG(SAP_TAG,SAP_LOG_INFO,("postponing action mFlushingAudio=%d\n",
                  mFlushingAudio));

            break;
        }

        Action* action = *mDeferredActions.begin();
        mDeferredActions.erase(mDeferredActions.begin());

        action->execute(this);
        delete action;
    }
}

void SmartAudioPlayer::performSeek(off64_t seekTime) {
    SAP_LOG(SAP_TAG,SAP_LOG_TRACE,("performSeek seekTimeUs=(%lld usecs)",
            seekTime));

    if (mSource == NULL) {
        // This happens when reset occurs right before the loop mode
        // asynchronously seeks to the start of the stream.
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("source has gone or not set"));

        return;
    }
    mSource->seekTo(seekTime);
    
}
void SmartAudioPlayer::performDecoderFlush(FlushCommand audio){
    SAP_LOG(SAP_TAG,SAP_LOG_TRACE,("performDecoderFlush audio=%d", audio));

    if ((audio == FLUSH_CMD_NONE || mAudioDecoder == NULL)) {
        return;
    }

    if (audio != FLUSH_CMD_NONE && mAudioDecoder != NULL) {
        flushDecoder(true /* audio */, (audio == FLUSH_CMD_SHUTDOWN));
    }
}
void SmartAudioPlayer::performReset(){
    SAP_LOG(SAP_TAG,SAP_LOG_TRACE,("performReset"));

    //assert0(mAudioDecoder!= NULL);

    if (mRenderer != NULL) {
        delete mRenderer;
        mRenderer = NULL;
        //mRenderer->stop();
        //mRenderer->release();
    }

    if (mSource != NULL) {
        Mutex::Autolock autoLock(mSourceLock);
        delete mSource;
        mSource = NULL;
    }

    if (mDriver != NULL) {
        mDriver->notifyResetComplete();
    }

    //mDriver = NULL;

    mStarted = false;
    mPrepared = false;
    mResetting = false;
    mSourceStarted = false;
    mIsPCMSource = false;
}
void SmartAudioPlayer::performResumeDecoders(bool needNotify) {
    if (needNotify) {
        mResumePending = true;
        // if audio-only, we can notify seek complete now,
        // as the resume operation will be relatively fast.
        finishResume();
    }

    if (mAudioDecoder != NULL) {
        mAudioDecoder->signalResume(false /* needNotify */);
    }
}
void SmartAudioPlayer::finishResume() {
    if (mResumePending) {
        mResumePending = false;
        notifyDriverSeekComplete();
    }
}
void SmartAudioPlayer::notifyDriverSeekComplete() {
    if (mDriver != NULL) {
        mDriver->notifySeekComplete();
    }
}

void SmartAudioPlayer::notifyDriverStartComplete() {
    if (mDriver != NULL) {
        mDriver->notifyStartComplete();
    }
}

void SmartAudioPlayer::flushDecoder(bool audio, bool needShutdown) {
    SAP_LOG(SAP_TAG,SAP_LOG_TRACE,("[%s] flushDecoder needShutdown=%d",
          audio ? "audio" : "video", needShutdown));

    if (mAudioDecoder == NULL) {
        SAP_LOG(SAP_TAG,SAP_LOG_INFO,("flushDecoder %s without decoder present",
             audio ? "audio" : "video"));
        return;
    }

    // Make sure we don't continue to scan sources until we finish flushing.

    mAudioDecoder->signalFlush();

    FlushStatus newStatus =
        needShutdown ? FLUSHING_DECODER_SHUTDOWN : FLUSHING_DECODER;

    mFlushComplete[audio][false /* isDecoder */] = (mRenderer == NULL);
    mFlushComplete[audio][true /* isDecoder */] = false;
    if (audio) {
        SAP_LOG(SAP_TAG,SAP_LOG_INFO,("audio flushDecoder() is called in state %d", mFlushingAudio));
        mFlushingAudio = newStatus;
    }
}

void SmartAudioPlayer::handleFlushComplete(bool audio, bool isDecoder) {
    // We wait for both the decoder flush and the renderer flush to complete
    // before entering either the FLUSHED or the SHUTTING_DOWN_DECODER state.

    mFlushComplete[audio][isDecoder] = true;
    mFlushComplete[audio][!isDecoder] = true;//remove me when render ok

    if (!mFlushComplete[audio][!isDecoder]) {
        return;
    }

    FlushStatus *state = &mFlushingAudio;
    switch (*state) {
        case FLUSHING_DECODER:
        {
            *state = FLUSHED;
            break;
        }

        case FLUSHING_DECODER_SHUTDOWN:
        {
            *state = SHUTTING_DOWN_DECODER;

            SAP_LOG(SAP_TAG,SAP_LOG_INFO,("initiating decoder shutdown"));
            mAudioDecoder->initiateShutdown();
            break;
        }

        default:
            // decoder flush completes only occur in a flushing state.
            SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("decoder flush in invalid state %d", *state));
            break;
    }
}
void SmartAudioPlayer::finishFlushIfPossible() {
    if (mFlushingAudio != NONE && mFlushingAudio != FLUSHED
            && mFlushingAudio != SHUT_DOWN) {
        return;
    }

    SAP_LOG(SAP_TAG,SAP_LOG_INFO,("audio is flushed now."));

    mFlushingAudio = NONE;

    clearFlushComplete();

    processDeferredActions();

}
void SmartAudioPlayer::setPlaybackSettings(const AudioFormat *format) {

    mFormat.mChannelLayout = format->mChannelLayout;
    mFormat.mSampleFmt = format->mSampleFmt;
    mFormat.mSampleRate = format->mSampleRate;
    mFormat.mChannels = format->mChannels;
    strncpy(mFormat.mDeviceName,format->mDeviceName, strlen(format->mDeviceName));
    mFormat.mDeviceName[strlen(format->mDeviceName)] = '\0';
}

void SmartAudioPlayer::clearFormat() {
    mFormat.mChannelLayout = AV_CH_LAYOUT_STEREO;
    mFormat.mSampleFmt = AV_SAMPLE_FMT_S16;
    mFormat.mSampleRate = 44100;
    mFormat.mChannels = 2;
    strncpy(mFormat.mDeviceName,"default", strlen("default"));
    mFormat.mDeviceName[strlen("default")] = '\0';
}

void SmartAudioPlayer::notifyListener(int msg, int ext1, int ext2) {
    if (mDriver == NULL) {
        SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("driver have gone"));
        return;
    }
    mDriver->notifyListener(msg, ext1, ext2);
}

void *SmartAudioPlayer::PlayerMessageReceive() {

    SAP_LOG(SAP_TAG,SAP_LOG_INFO,("PlayerMessageReceive pid %u tid %u", getpid(), gettid()));
    AMessage msg;

    while (1) {
        mQueue->ReceiveMessage(&msg);
        switch (msg.type) {
            case kWhatShutDownOwn:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatShutDownOwn"));
                return NULL;
            }
            case kWhatSetDataSource:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatSetDataSource"));

                assert0(mDriver != NULL);
                mDriver->notifySetDataSourceCompleted(msg.err);
                break;
            }
#ifdef syncprepare
            case kWhatPrepare:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatPrepare"));

                assert0(mSource != NULL);
                assert0(mDriver != NULL);

                status_t err = mSource->prepare();
                if (err != OK) {

                } else {
                    mPrepared = true;
                }

                mDriver->notifyPrepareCompleted(err);

                break;
            }
#else
            case kWhatPrepare:
            {
                SAP_LOGENTRY(SAP_TAG,SAP_LOG_INFO,("kWhatPrepare"));
                off64_t nowUs = systemTime();
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("kWhatPrepare TIME %lld!", nowUs));

                assert0(mSource != NULL);
                mSource->prepareAsync();
                break;
            }
            case kWhatPrepared:
            {
                SAP_LOGENTRY(SAP_TAG,SAP_LOG_INFO,("kWhatPrepared"));
                off64_t nowUs = systemTime();
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("kWhatPrepared TIME  %lld!", nowUs));
                status_t err = msg.err;

                if (mSource == NULL || mResetting == true) {
                    // This is a stale notification from a source that was
                    // asynchronously preparing when the client called reset().
                    // We handled the reset, the source is gone.
                    break;
                }

                if (msg.generation != mGeneration) {
                    SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("stale:source %d, player %d",
                        msg.generation, mGeneration));
                    break;
                }
                if (err != OK) {

                } else {
                    mPrepared = true;
                }
                // notify duration first, so that it's definitely set when
                // the app received the "prepare complete" callback.
                off64_t durationUs;
                assert0(mDriver != NULL);

                if (mSource->getDuration(&durationUs) == OK) {
                    mDriver->notifyDuration(durationUs);
                }

                mDriver->notifyPrepareCompleted(err);
                break;
            }

#endif
            case kWhatStart:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatStart"));
                int needNotify = msg.needNotify;
                if (mStarted) {
                    onResume();
                } else {
                    onStart();
                }
                if (needNotify) {
                    notifyDriverStartComplete();
                }

                break;
            }
            case kWhatPause:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatpause"));
                onPause();
                break;
            }
            case kWhatReset:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatReset"));

                mResetting = true;

                mDeferredActions.push_back(
                        new FlushDecoderAction(FLUSH_CMD_SHUTDOWN));

                mDeferredActions.push_back(
                        new SimpleAction(&SmartAudioPlayer::performReset));

                processDeferredActions();

                break;
            }
            case kWhatSeek:
            {
                off64_t seekTime = msg.seekTime;
                int needNotify = msg.needNotify;
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatSeek seekTimeUs=%lld us, needNotify=%d",seekTime,
                        needNotify));

                if (!mStarted) {
                    // Seek before the player is started. In order to preview video,
                    // need to start the player and pause it. This branch is called
                    // only once if needed. After the player is started, any seek
                    // operation will go through normal path.
                    // Audio-only cases are handled separately.
                    onStart(seekTime);
                    if (mStarted) {
                        onPause();
                    }
                    if (needNotify) {
                        notifyDriverSeekComplete();
                    }
                    break;
                }

                mDeferredActions.push_back(
                        new FlushDecoderAction(FLUSH_CMD_FLUSH/* audio */));

                mDeferredActions.push_back(
                        new SeekAction(seekTime));

                // After a flush without shutdown, decoder is paused.
                // Don't resume it until source seek is done, otherwise it could
                // start pulling stale data too soon.
                mDeferredActions.push_back(
                        new ResumeDecoderAction(needNotify));

                processDeferredActions();

                break;
            }
            case kWhatFlushCompleted:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("decoder flush completed"));

                handleFlushComplete(true, true /* isDecoder */);
                finishFlushIfPossible();

                break;
            }
            case kWhatShutdownCompleted:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("shutdown completed"));

                if (mAudioDecoder) {
                    delete (mAudioDecoder);
                    mAudioDecoder = NULL;
                }

                assert0(int(mFlushingAudio) == int(SHUTTING_DOWN_DECODER));
                mFlushingAudio = SHUT_DOWN;
                finishFlushIfPossible();

                break;
            }
            case kWhatError:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatError"));
                status_t err;
                if (msg.err == OK) {
                    err = UNKNOWN_ERROR;
                }

                // Decoder errors can be due to Source (e.g. from streaming),
                // or from decoding corrupted bitstreams, or from other decoder
                // MediaCodec operations (e.g. from an ongoing reset or seek).
                // They may also be due to openAudioSink failure at
                // decoder start or after a format change.
                //
                // We try to gracefully shut down the affected decoder if possible,
                // rather than trying to force the shutdown with something
                // similar to performReset(). This method can lead to a hang
                // if MediaCodec functions block after an error, but they should
                // typically return INVALID_OPERATION instead of blocking.

                FlushStatus *flushing = &(mFlushingAudio);
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("received error(%#x) from decoder, flushing(%d), now shutting down",err,*flushing));

                switch (*flushing) {
                    case NONE:
                        mDeferredActions.push_back(
                            new FlushDecoderAction(FLUSH_CMD_SHUTDOWN));
                        processDeferredActions();
                        break;
                    case FLUSHING_DECODER:
                        *flushing = FLUSHING_DECODER_SHUTDOWN; // initiate shutdown after flush.
                        break; // Wait for flush to complete.
                    case FLUSHING_DECODER_SHUTDOWN:
                        break; // Wait for flush to complete.
                    case SHUTTING_DOWN_DECODER:
                        break; // Wait for shutdown to complete.
                    case FLUSHED:
                        mAudioDecoder->initiateShutdown(); // In the middle of a seek.
                        *flushing = SHUTTING_DOWN_DECODER;     // Shut down.
                        break;
                    case SHUT_DOWN:
                        finishFlushIfPossible();  // Should not occur.
                        break;                    // Finish anyways.
                    }
                    notifyListener(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, err);

                break;
            }
            case kWhatRendererEOS:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatRenderer EOS"));
                off64_t nowUs = systemTime();
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("kWhatRendererEOS TIME %lld!", nowUs));
                notifyListener(MEDIA_PLAYBACK_COMPLETE, 0, 0);
                break;

            }

            case kWhatSourceEOS:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatSourceEOS EOS"));
                off64_t nowUs = systemTime();
                SAP_LOG(SAP_SOURCE_TAG,SAP_LOG_INFO,("kWhatSourceEOS TIME %lld!", nowUs));

                notifyListener(MEDIA_CACHE_COMPLETE, 0, msg.err);
                break;

            }

            
            case kWhatRendererPaused:
            {
                SAP_LOG(SAP_TAG,SAP_LOG_INFO,("kWhatRenderer Paused"));

                assert0(mDriver != NULL);
                mDriver->notifyPauseCompleted();
                break;

            }

            default:
                break;
     }
    }
}

void *SmartAudioPlayer::PlayerThreadFunc(void *data) {

    return (void *) ((SmartAudioPlayer*)data)->PlayerMessageReceive();
}

#endif

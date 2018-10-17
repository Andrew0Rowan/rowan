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
#include "SmartAudioPlayer.h"
#include "assert0.h"
#include "Log.h"
#include <unistd.h>
bool is_lib64;

SmartAudioPlayerDriver::SmartAudioPlayerDriver(pid_t pid)
    : mState(STATE_IDLE),
      mIsAsyncPrepare(false),
      mAsyncResult(UNKNOWN_ERROR),
      mSetSurfaceInProgress(false),
      mDuration(-1),
      mPosition(-1),
      mSeekPosition(0),
      mSeekInProgress(false),
      mPlayerFlags(0),
      mAtEOS(false),
      mLooping(false),
      mAutoLoop(false),
      mId(-1),
      mCallback(NULL),
      mIsPCMSource(false) {

    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("SmartAudioPlayerDriver(%p)", this));
    mPlayer = new SmartAudioPlayer(pid);
    mPlayer->setDriver(this);
}

SmartAudioPlayerDriver::~SmartAudioPlayerDriver() {
    delete mPlayer;
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("~SmartAudioPlayerDriver(%p)", this));

}


status_t SmartAudioPlayerDriver::setDataSource(const char *url, int id) {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("setDataSource(%p) url(%s)", this, url));

    {
        Mutex::Autolock autoLock(mLock);

        if (mState != STATE_IDLE) {
            return INVALID_OPERATION;
        }

        mId = id;
        mState = STATE_SET_DATASOURCE_PENDING;
        mAtEOS = false;

        mPlayer->setDataSourceAsync(url);

        while (mState == STATE_SET_DATASOURCE_PENDING) {
            mCondition.wait(mLock);
        }
    }
    /*only for cache function test*/
    //setOutFileName("/data/dump.");

    return mAsyncResult;
}

status_t SmartAudioPlayerDriver::setOutFileName(const char *OutFileName) {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("setOutFileName(%p) OutFileName(%s)", this, OutFileName));
    Mutex::Autolock autoLock(mLock);
    status_t err = mPlayer->setOutFileName(OutFileName);
    return err;
}


status_t SmartAudioPlayerDriver::setPCMMode(int id) {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("setPCMMode(%p) mState %d", this, mState));
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_IDLE) {
        return INVALID_OPERATION;
    }

    mId = id;
    mAtEOS = false;

    mIsPCMSource = true;
    mPlayer->setPCMMode();
    mState = STATE_PREPARED;

    return OK;
}

status_t SmartAudioPlayerDriver::setDataSource(int id) {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("setDataSource(%p)", this));
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_IDLE) {
        return INVALID_OPERATION;
    }

    mId = id;
    mState = STATE_SET_DATASOURCE_PENDING;
    mAtEOS = false;

    mPlayer->setDataSourceAsync();

    while (mState == STATE_SET_DATASOURCE_PENDING) {
        mCondition.wait(mLock);
    }

    return mAsyncResult;
}


status_t SmartAudioPlayerDriver::prepare() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("prepare(%p)", this));
    Mutex::Autolock autoLock(mLock);
    return prepare_l();
}

status_t SmartAudioPlayerDriver::prepare_l() {
    switch (mState) {
        case STATE_UNPREPARED:
            mState = STATE_PREPARING;

            // Make sure we're not posting any notifications, success or
            // failure information is only communicated through our result
            // code.
            mIsAsyncPrepare = false;
            mPlayer->prepareAsync();
            while (mState == STATE_PREPARING) {
                mCondition.wait(mLock);
            }
            return (mState == STATE_PREPARED) ? OK : UNKNOWN_ERROR;
        case STATE_STOPPED:
            // this is really just paused. handle as seek to start
            mAtEOS = false;
            mState = STATE_STOPPED_AND_PREPARING;
            mIsAsyncPrepare = false;
            mPlayer->seekToAsync(0, true /* needNotify */);
            while (mState == STATE_STOPPED_AND_PREPARING) {
                mCondition.wait(mLock);
            }
            return (mState == STATE_STOPPED_AND_PREPARED) ? OK : UNKNOWN_ERROR;
        default:
            return INVALID_OPERATION;
    };
}

status_t SmartAudioPlayerDriver::prepareAsync() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("prepareAsync(%p)", this));
    Mutex::Autolock autoLock(mLock);

    switch (mState) {
        case STATE_UNPREPARED:
            mState = STATE_PREPARING;
            mIsAsyncPrepare = true;
            mPlayer->prepareAsync();
            return OK;
        case STATE_STOPPED:
            // this is really just paused. handle as seek to start
            mAtEOS = false;
            mState = STATE_STOPPED_AND_PREPARING;
            mIsAsyncPrepare = true;
            mPlayer->seekToAsync(0, true /* needNotify */);
            return OK;
        default:
            return INVALID_OPERATION;
    };
}

status_t SmartAudioPlayerDriver::start() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("start(%p), state is %d, eos is %d", this, mState, mAtEOS));
    status_t result;

    if(mIsPCMSource){
        result = start_pcm();
    }else{
        result = start_l();
    }
    return result;
}

status_t SmartAudioPlayerDriver::start_pcm() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("start_pcm(%p), state is %d, eos is %d", this, mState, mAtEOS));
    Mutex::Autolock autoLock(mLock);
    mState = STATE_START_IN_PROGRESS;
    mPlayer->start_pcm();
    while (mState == STATE_START_IN_PROGRESS) {
        mCondition.wait(mLock);
    }
    return OK;
}

status_t SmartAudioPlayerDriver::start_l() {
    Mutex::Autolock autoLock(mLock);
    switch (mState) {
        case STATE_UNPREPARED:
        {
            status_t err = prepare_l();

            if (err != OK) {
                return err;
            }
            assert0(mState == STATE_PREPARED);

            // fall through
        }

        case STATE_PAUSED:
        case STATE_STOPPED_AND_PREPARED:
        case STATE_PREPARED:
        {
            mPlayer->start();

            // fall through
        }

        case STATE_RUNNING:
        {
            if (mAtEOS) {
                mPlayer->seekToAsync(0);
                mAtEOS = false;
                mPosition = -1;
                mSeekPosition = 0;
            }
            break;
        }

        default:
            return INVALID_OPERATION;
    }

    mState = STATE_START_IN_PROGRESS;

    return OK;
}

status_t SmartAudioPlayerDriver::stop() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("stop(%p)", this));
    Mutex::Autolock autoLock(mLock);

    switch (mState) {
        case STATE_RUNNING:
            mState = STATE_PAUSE_PENDING;
            mPlayer->pause();
            while (mState == STATE_PAUSE_PENDING) {
                mCondition.wait(mLock);
            }
            // fall through

        case STATE_PAUSED:
            mState = STATE_STOPPED;
            notifyListener_l(MEDIA_STOPPED);
            break;

        case STATE_PREPARED:
        case STATE_STOPPED:
        case STATE_STOPPED_AND_PREPARING:
        case STATE_STOPPED_AND_PREPARED:
            mState = STATE_STOPPED;
            break;

        default:
            return INVALID_OPERATION;
    }

    return OK;
}

status_t SmartAudioPlayerDriver::pause() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("pause(%p)", this));
    // The NuPlayerRenderer may get flushed if pause for long enough, e.g. the pause timeout tear
    // down for audio offload mode. If that happens, the NuPlayerRenderer will no longer know the
    // current position. So similar to seekTo, update |mPositionUs| to the pause position by calling
    // getCurrentPosition here.
    off64_t unused;
    getCurrentPosition(&unused);

    Mutex::Autolock autoLock(mLock);

    switch (mState) {
        case STATE_PAUSED:
        case STATE_PREPARED:
            return OK;

        case STATE_RUNNING:
            mState = STATE_PAUSE_PENDING;
            mPlayer->pause();
            while (mState == STATE_PAUSE_PENDING) {
                mCondition.wait(mLock);
            }
            mState = STATE_PAUSED;
            notifyListener_l(MEDIA_PAUSED);
            break;

        default:
            return INVALID_OPERATION;
    }

    return OK;
}

bool SmartAudioPlayerDriver::isPlaying() {
    return mState == STATE_RUNNING && !mAtEOS;
}

status_t SmartAudioPlayerDriver::seekTo(off64_t usec) {
    SAP_LOGENTRY(SAP_DRIVER_TAG,SAP_LOG_TRACE,("seekTo(%p) %lld us at state %d", this, usec, mState));
    Mutex::Autolock autoLock(mLock);

    off64_t nowUs = systemTime();
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("seekTo TIME: %lld!", nowUs));

    if (usec > mDuration) {
        SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("seekTo(%p) [%lld] us is larger than mDuration:[%lld]", this, usec, mDuration));
        return INVALID_OPERATION;
    }

    switch (mState) {
        case STATE_PREPARED:
        case STATE_STOPPED_AND_PREPARED:
        case STATE_PAUSED:
        case STATE_RUNNING:
        {
            mAtEOS = false;
            mSeekInProgress = true;
            mPlayer->seekToAsync(usec, true /* needNotify */);
            break;
        }

        default:
            return INVALID_OPERATION;
    }

    mPosition = usec;
    mSeekPosition = usec;
    return OK;
}

status_t SmartAudioPlayerDriver::getCurrentPosition(off64_t *usec) {
    SAP_LOGENTRY(SAP_DRIVER_TAG,SAP_LOG_TRACE,("getCurrentPosition(%p)", this));
    off64_t temp = 0;
    {
        Mutex::Autolock autoLock(mLock);
        if (mSeekInProgress || (mState == STATE_PAUSED && !mAtEOS)) {
            temp = (mPosition <= 0) ? 0 : mPosition;
            *usec = (off64_t)divRound(temp, (off64_t)1);
            SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("getCurrentPosition1(%lld)", *usec));
            return OK;
        }
    }

    status_t ret = mPlayer->getCurrentPosition(&temp);

    Mutex::Autolock autoLock(mLock);
    // We need to check mSeekInProgress here because mPlayer->seekToAsync is an async call, which
    // means getCurrentPosition can be called before seek is completed. Iow, renderer may return a
    // position value that's different the seek to position.
    if (ret != OK) {
        temp = (mPosition <= 0) ? 0 : mPosition;
    } else {
        mPosition = temp + mSeekPosition;
    }
    *usec = (off64_t)divRound(mPosition, (off64_t)(1));
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("getCurrentPosition2(%lld)", *usec));
    return OK;
}

status_t SmartAudioPlayerDriver::getDuration(off64_t *usec) {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("getDuration(%p)", this));
    Mutex::Autolock autoLock(mLock);

    if (mDuration < 0) {
        return UNKNOWN_ERROR;
    }

    *usec = mDuration;

    return OK;
}

status_t SmartAudioPlayerDriver::reset() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("reset(%p) at state %d", this, mState));
    Mutex::Autolock autoLock(mLock);
    off64_t time1, time2;
    time1 = systemTime();
    switch (mState) {
        case STATE_IDLE:
            return OK;

        case STATE_SET_DATASOURCE_PENDING:
        case STATE_RESET_IN_PROGRESS:
        case STATE_PAUSE_PENDING:
            return INVALID_OPERATION;

        case STATE_PREPARING:
        {
            assert0(mIsAsyncPrepare);

            SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("reset when preparing(%p)", this));
            //notifyListener_l(MEDIA_PREPARED);
            break;
        }

        default:
            break;
    }


    mState = STATE_RESET_IN_PROGRESS;
    mPlayer->resetAsync();

    while (mState == STATE_RESET_IN_PROGRESS) {
        mCondition.wait(mLock);
    }

    mDuration = -1;
    mPosition = -1;
    mSeekPosition = 0;
    mLooping = false;
    mIsPCMSource = false;
    time2 = systemTime();
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("reset start:%lld end:%lld cost:%lld!", 
        time1, time2, time2-time1));
    return OK;
}

status_t SmartAudioPlayerDriver::setLooping(int loop) {
    mLooping = loop != 0;
    return OK;
}

void SmartAudioPlayerDriver::setPlaybackSettings(const AudioFormat *format) {
    mPlayer->setPlaybackSettings(format);
}

status_t SmartAudioPlayerDriver::writeData(unsigned char* buffer, int size, status_t result) {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("writeData(%p) %p, %d, %d", this, buffer, size, result));

    Mutex::Autolock autoLock(mWriteLock);

    if (mState == STATE_IDLE || mState == STATE_SET_DATASOURCE_PENDING) {
        return INVALID_OPERATION;
    }
    return mPlayer->writeData(buffer, size, result);

}

status_t SmartAudioPlayerDriver::writePCMData(unsigned char* buffer, int size, status_t result) {
    Mutex::Autolock autoLock(mWriteLock);

    if (mState == STATE_IDLE || mState == STATE_SET_DATASOURCE_PENDING) {
        return INVALID_OPERATION;
    }
    return mPlayer->writePCMData(buffer, size, result);

}

off64_t SmartAudioPlayerDriver::getAvailiableSize(void) {
    //printf("getAvailiableSize(%p) \n", this);

    Mutex::Autolock autoLock(mWriteLock);

    if (mState == STATE_IDLE || mState == STATE_SET_DATASOURCE_PENDING) {
        SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("mState error!"));
        return INVALID_OPERATION;
    }
    return mPlayer->getAvailiableSize();
}


void SmartAudioPlayerDriver::registerCallback(callback *fn) {
    mCallback = fn;
}

void SmartAudioPlayerDriver::notifyResetComplete() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("notifyResetComplete(%p)", this));
    Mutex::Autolock autoLock(mLock);

    assert0(mState == STATE_RESET_IN_PROGRESS);
    mState = STATE_IDLE;
    mCondition.broadcast();
}

void SmartAudioPlayerDriver::notifyStartComplete() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("notifyStartComplete(%p)", this));
    Mutex::Autolock autoLock(mLock);

    assert0(mState == STATE_START_IN_PROGRESS);
    mState = STATE_RUNNING;
    mCondition.broadcast();
    notifyListener_l(MEDIA_STARTED);
}

void SmartAudioPlayerDriver::notifyDuration(off64_t durationUs) {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("notifyDuration(%p) %lld us", this, durationUs));
    Mutex::Autolock autoLock(mLock);
    mDuration = durationUs;
}

void SmartAudioPlayerDriver::notifySeekComplete() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("notifySeekComplete(%p)", this));
    Mutex::Autolock autoLock(mLock);
    mSeekInProgress = false;
    off64_t nowUs = systemTime();
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("notifySeekComplete TIME: %lld!", nowUs));
    notifySeekComplete_l();
}

void SmartAudioPlayerDriver::notifySeekComplete_l() {
    bool wasSeeking = true;
    if (mState == STATE_STOPPED_AND_PREPARING) {
        wasSeeking = false;
        mState = STATE_STOPPED_AND_PREPARED;
        mCondition.broadcast();
        if (!mIsAsyncPrepare) {
            // if we are preparing synchronously, no need to notify listener
            return;
        }
    } else if (mState == STATE_STOPPED) {
        // no need to notify listener
        return;
    }
    notifyListener_l(wasSeeking ? MEDIA_SEEK_COMPLETE : MEDIA_PREPARED);
}

void SmartAudioPlayerDriver::notifyListener(
        int msg, int ext1, int ext2) {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_INFO,("notifyListener"));
    Mutex::Autolock autoLock(mLock);
    notifyListener_l(msg, ext1, ext2);
}

void SmartAudioPlayerDriver::notifyListener_l(
        int msg, int ext1, int ext2) {
    SAP_LOGENTRY(SAP_DRIVER_TAG,SAP_LOG_TRACE,("notifyListener_l(%p), (%d, %d, %d), loop setting(%d, %d)",
        this, msg, ext1, ext2, mAutoLoop, mLooping));
    switch (msg) {
        case MEDIA_PLAYBACK_COMPLETE:
        {
            if (mState != STATE_RESET_IN_PROGRESS) {
                if (mAutoLoop) {

                }
                if (mLooping || mAutoLoop) {
                    mPlayer->seekToAsync(0);
                    // don't send completion event when looping
                    return;
                }
            }
            // fall through
        }

        case MEDIA_ERROR:
        {
            mAtEOS = true;
            break;
        }

        default:
            break;
    }

    mLock.unlock();
    if (mCallback != NULL) {
        mCallback(msg, mId, ext1, ext2);
    }
    //sendEvent(msg, ext1, ext2);
    mLock.lock();
}

void SmartAudioPlayerDriver::notifySetDataSourceCompleted(status_t err) {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("notifySetDataSourceCompleted(%p)", this));
    Mutex::Autolock autoLock(mLock);

    assert0(mState==STATE_SET_DATASOURCE_PENDING);

    mAsyncResult = err;
    mState = (err == OK) ? STATE_UNPREPARED : STATE_IDLE;
    mCondition.broadcast();
}

void SmartAudioPlayerDriver::notifyPrepareCompleted(status_t err) {
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_PREPARING) {
        // We were preparing asynchronously when the client called
        // reset(), we sent a premature "prepared" notification and
        // then initiated the reset. This notification is stale.
        assert0(mState == STATE_RESET_IN_PROGRESS || mState == STATE_IDLE);
        return;
    }

    assert0(mState==STATE_PREPARING);

    mAsyncResult = err;

    if (err == OK) {
        // update state before notifying client, so that if client calls back into SmartAudioPlayerDriver
        // in response, SmartAudioPlayerDriver has the right state
        mState = STATE_PREPARED;
        if (mIsAsyncPrepare) {
            notifyListener_l(MEDIA_PREPARED);
        }
    } else {
        mState = STATE_UNPREPARED;
        if (mIsAsyncPrepare) {
            notifyListener_l(MEDIA_ERROR, MEDIA_ERROR_UNKNOWN, err);
        }
    }
#if 0
    sp<MetaData> meta = mPlayer->getFileMeta();
    int32_t loop;
    if (meta != NULL
            && meta->findInt32(kKeyAutoLoop, &loop) && loop != 0) {
        mAutoLoop = true;
    }
#endif
    mCondition.broadcast();
}
void SmartAudioPlayerDriver::notifyPauseCompleted() {
    SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("notifyPauseCompleted(%p)", this));
    Mutex::Autolock autoLock(mLock);

    if (mState != STATE_PAUSE_PENDING) {
        SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_ERROR,("notifyPauseCompleted(%p) no need to notify", this));
        return;
    }

    mState = STATE_PAUSED;
    mCondition.broadcast();
}

///////////////////////////////////////////////////////////////////////////////

SmartAudioPlayerDriver* CreatePlayer(void) {
    if (access("/usr/lib64/libavformat.so.57",0) == 0) {
        is_lib64 = true;
        return new SmartAudioPlayerDriver();
    } if (access("/usr/lib/libavformat.so.57",0) == 0) {
        is_lib64 = false;
        return new SmartAudioPlayerDriver();
    }
    else {
        SAP_LOG(SAP_DRIVER_TAG,SAP_LOG_TRACE,("CreatePlayer Fail"));
        return NULL;
    }
}

#endif

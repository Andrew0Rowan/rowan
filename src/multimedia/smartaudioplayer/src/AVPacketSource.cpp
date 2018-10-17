#ifdef CONFIG_SUPPORT_FFMPEG

#include "AVPacketSource.h"
#include "Log.h"

AVPacketSource::AVPacketSource():
      mEnabled(true),
      mEOSResult(OK) {

      }

status_t AVPacketSource::start() {
    return OK;
}
status_t AVPacketSource::stop() {
    return OK;
}
void AVPacketSource::clear() {
    Mutex::Autolock autoLock(mLock);

    list<AVPacket *>::iterator it = mBuffers.begin();
    while (it != mBuffers.end()) {
        AVPacket* &buffer = *it;
        pfnav_packet_free(&buffer);
        ++it;
    }
    mBuffers.clear();

    mEOSResult = OK;
}
bool AVPacketSource::hasBufferAvailable(status_t *finalResult) {
    Mutex::Autolock autoLock(mLock);
    *finalResult = OK;
    if (!mEnabled) {
        return false;
    }
    if (!mBuffers.empty()) {
        return true;
    }

    *finalResult = mEOSResult;
    return false;
}
off64_t AVPacketSource::getBufferedDurationUs(status_t *finalResult) {
    Mutex::Autolock autoLock(mLock);

    *finalResult = mEOSResult;

    if (mBuffers.empty()) {
        return 0;
    }

    off64_t time1 = -1;
    off64_t time2 = -1;
    off64_t timeUs;
    off64_t durationUs = 0;


    if ((timeUs = mBuffers.front()->pts) != AV_NOPTS_VALUE) {
            time1 = timeUs;
    }

    if ((timeUs = mBuffers.back()->pts) != AV_NOPTS_VALUE) {
            time2 = timeUs;
    }

    if(time1 != -1 && time2 != -1){
        durationUs += (time2 - time1);
    }

    return durationUs;
}

void AVPacketSource::queueAccessUnit(AVPacket* &buffer) {
    Mutex::Autolock autoLock(mLock);
    mBuffers.push_back(buffer);
    mCondition.signal();
}

void AVPacketSource::signalEOS(status_t result) {
    Mutex::Autolock autoLock(mLock);
    mEOSResult = result;
    mCondition.signal();
}

status_t AVPacketSource::dequeueAccessUnit(AVPacket* buffer) {
    Mutex::Autolock autoLock(mLock);
    while (mEOSResult == OK && mBuffers.empty()) {
        mCondition.wait(mLock);
    }

    if (!mBuffers.empty()) {
        AVPacket* src = *mBuffers.begin();

        pfnav_packet_ref(buffer, src);
        pfnav_packet_free(&src);

        mBuffers.erase(mBuffers.begin());

        return OK;
    }

    return mEOSResult;
}

void AVPacketSource::enable(bool enable) {
    Mutex::Autolock autoLock(mLock);
    mEnabled = enable;
}

AVPacketSource::~AVPacketSource() {
    clear();
}


#endif

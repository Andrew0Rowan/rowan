#ifndef AVPACKET_SOURCE_H_
#define AVPACKET_SOURCE_H_

#include "ABase.h"
#include "Errors.h"
#include "Mutex.h"
#include "Condition.h"
#include "FFmpeg.h"
#include <list>
using namespace std;

struct AVPacketSource {
    AVPacketSource();
    status_t start();
    status_t stop();
    void clear();

    bool hasBufferAvailable(status_t *finalResult);

    off64_t getBufferedDurationUs(status_t *finalResult);

    void queueAccessUnit(AVPacket* &buffer);

    void signalEOS(status_t result);

    status_t dequeueAccessUnit(AVPacket *buffer);
    void enable(bool enable);

    ~AVPacketSource();

private:

    Mutex mLock;
    Condition mCondition;

    bool mEnabled;
    list<AVPacket *> mBuffers;
    status_t mEOSResult;
    DISALLOW_EVIL_CONSTRUCTORS(AVPacketSource);
};
#endif



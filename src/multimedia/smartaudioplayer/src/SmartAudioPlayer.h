#ifndef _SMARTAUDIOPLAYER_H_
#define _SMARTAUDIOPLAYER_H_

#include "ABuffer.h"
#include "Mutex.h"
#include "Condition.h"
#include "AUtils.h"
#include "assert0.h"
#include "Format.h"

#include "AMessageQueue.h"

#include <list>
using namespace std;

struct ABuffer;
struct SmartAudioPlayerDriver;
struct MASRender;

class SmartAudioPlayer
{
public:

    SmartAudioPlayer(pid_t pid);

    void setDriver(SmartAudioPlayerDriver* driver);

    void setDataSourceAsync(const char *url);
    void setPCMMode();

    void setDataSourceAsync();
    status_t setOutFileName(const char *OutFileName);

    void prepareAsync();

    //status_t setPlaybackSettings(const AudioPlaybackSetting &setting);
    //status_t getPlaybackSettings(AudioPlaybackSetting *setting /* nonnull */);

    void start();
    void start_pcm();

    void pause();

    // Will notify the driver through "notifyResetComplete" once finished.
    void resetAsync();

    // Will notify the driver through "notifySeekComplete" once finished
    // and needNotify is true.
    void seekToAsync(off64_t seekTime, bool needNotify = false);

    status_t getCurrentPosition(off64_t *mediaUs);
    void setPlaybackSettings(const AudioFormat *format);

    status_t writeData(unsigned char* buffer, int size, status_t result = false);
    status_t writePCMData(unsigned char* buffer, int size, status_t result = false);
    off64_t getAvailiableSize(void);
    ~SmartAudioPlayer();

    private:
    void onStart(off64_t startPositionUs=-1);
    void onPause();
    void onResume();

    struct Decoder;
    struct Renderer;
    struct GenericSource;
    struct Action;
    struct SeekAction;
    struct ResumeDecoderAction;
    struct FlushDecoderAction;
    struct SimpleAction;

    enum FlushStatus {
        NONE,
        FLUSHING_DECODER,
        FLUSHING_DECODER_SHUTDOWN,
        SHUTTING_DOWN_DECODER,
        FLUSHED,
        SHUT_DOWN,
    };

    enum FlushCommand {
        FLUSH_CMD_NONE,
        FLUSH_CMD_FLUSH,
        FLUSH_CMD_SHUTDOWN,
    };
    void performSeek(off64_t seekTime);
    void performDecoderFlush(FlushCommand audio);
    void performReset();
    void performResumeDecoders(bool needNotify);
    void processDeferredActions();
    void flushDecoder(bool audio, bool needShutdown);
    void handleFlushComplete(bool audio, bool isDecoder);
    void finishFlushIfPossible();
    void notifyListener(int msg, int ext1, int ext2);
    void finishResume();
    void notifyDriverSeekComplete();
    void notifyDriverStartComplete();

    void clearFormat();

    // Status of flush responses from the decoder and renderer.
    bool mFlushComplete[2][2];

    FlushStatus mFlushingAudio;

    inline void clearFlushComplete() {
        mFlushComplete[0][0] = false;
        mFlushComplete[0][1] = false;
        mFlushComplete[1][0] = false;
        mFlushComplete[1][1] = false;
    }

    list<Action *> mDeferredActions;

    AudioFormat mFormat;

    Mutex mSourceLock;  // guard |mSource|.

    GenericSource* mSource;
    Renderer*      mRenderer;
    Decoder*       mAudioDecoder;

    pthread_t mSmartAudioPlayerThread;
    AMessageQueue*  mQueue;

    void *PlayerMessageReceive();
    static void *PlayerThreadFunc(void *data);

    bool mStarted;
    bool mPrepared;
    bool mAudioEOS;
    bool mPaused;
    bool mSourceStarted;
    bool mResetting;
    bool mResumePending;
    int  mGeneration;

    static unsigned int mCount;
    private:

    SmartAudioPlayerDriver* mDriver;
    bool mIsPCMSource;

    DISALLOW_EVIL_CONSTRUCTORS(SmartAudioPlayer);

};

#endif

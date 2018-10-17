#ifndef _SMARTAUDIOPLAYERDRIVER_H_
#define _SMARTAUDIOPLAYERDRIVER_H_

#include "ABase.h"
#include "Mutex.h"
#include "Condition.h"
#include "AUtils.h"
#include "assert0.h"
#include "Format.h"

struct SmartAudioPlayer;


struct SmartAudioPlayerDriver {
    SmartAudioPlayerDriver(pid_t pid =0);

    status_t setDataSource(const char *url, int id = -1);
    status_t setDataSource(int id = -1);
    status_t setPCMMode(int id = -1);
    status_t setOutFileName(const char *OutFileName);
    status_t prepare();
    status_t prepareAsync();
    status_t start();
    status_t start_pcm();
    status_t stop();
    status_t pause();
    bool isPlaying();
    void setPlaybackSettings(const AudioFormat *format);
    //virtual status_t getPlaybackSettings(AudioPlaybackRate *rate);
    status_t seekTo(off64_t usec);
    status_t getCurrentPosition(off64_t *usec);
    status_t getDuration(off64_t *usec);
    status_t reset();
    status_t setLooping(int loop);

    status_t writeData(unsigned char* buffer, int size, status_t result = false);
    status_t writePCMData(unsigned char* buffer, int size, status_t result = false);
    off64_t getAvailiableSize(void);

    void registerCallback(callback* fn);

    void notifySetDataSourceCompleted(status_t err);
    void notifyPrepareCompleted(status_t err);
    void notifyPauseCompleted();
    void notifyResetComplete();
    void notifyStartComplete();
    void notifyDuration(off64_t durationUs);
    void notifySeekComplete();
    void notifySeekComplete_l();
    void notifyListener(int msg, int ext1 = 0, int ext2 = 0);

public:
    virtual ~SmartAudioPlayerDriver();

    status_t prepare_l();
    status_t start_l();
    void notifyListener_l(int msg, int ext1 = 0, int ext2 = 0);

private:
    enum State {
        STATE_IDLE,
        STATE_SET_DATASOURCE_PENDING,
        STATE_UNPREPARED,
        STATE_PREPARING,
        STATE_PREPARED,
        STATE_RUNNING,
        STATE_PAUSED,
        STATE_RESET_IN_PROGRESS,
        STATE_STOPPED,                  // equivalent to PAUSED
        STATE_STOPPED_AND_PREPARING,    // equivalent to PAUSED, but seeking
        STATE_STOPPED_AND_PREPARED,     // equivalent to PAUSED, but seek complete
        STATE_PAUSE_PENDING,
        STATE_START_IN_PROGRESS,
    };

    mutable Mutex mLock;
    Condition mCondition;

    mutable Mutex mWriteLock;

    State mState;

    bool mIsAsyncPrepare;
    status_t mAsyncResult;

    // The following are protected through "mLock"
    // >>>
    bool mSetSurfaceInProgress;
    off64_t mDuration;
    off64_t mPosition;
    off64_t mSeekPosition;
    bool mSeekInProgress;
    // <<<

    SmartAudioPlayer* mPlayer;
    off64_t mPlayerFlags;

    bool mAtEOS;
    bool mLooping;
    bool mAutoLoop;

    int mId;
    callback* mCallback;

    bool mIsPCMSource;

    DISALLOW_EVIL_CONSTRUCTORS(SmartAudioPlayerDriver);
};

SmartAudioPlayerDriver* CreatePlayer(void);

#endif

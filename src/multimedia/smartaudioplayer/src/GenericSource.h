#ifndef GENERIC_SOURCE_H_

#define GENERIC_SOURCE_H_

#include "SmartAudioPlayer.h"
#include "FFDemuxer.h"
#include "AMessageQueue.h"
#include "AVPacketSource.h"

struct ABuffer;
struct FFDemuxer;

struct SmartAudioPlayer::GenericSource {

public:

    enum Flags {
        FLAG_CAN_PAUSE          = 1,
        FLAG_CAN_SEEK_BACKWARD  = 2,  // the "10 sec back button"
        FLAG_CAN_SEEK_FORWARD   = 4,  // the "10 sec forward button"
        FLAG_CAN_SEEK           = 8,  // the "seek bar"
        FLAG_DYNAMIC_DURATION   = 16,
        FLAG_SECURE             = 32,
        FLAG_PROTECTED          = 64,
    };

    GenericSource(AMessageQueue* &notify);

    status_t setDataSource(int generation = -1);
    status_t setDataSource(const char *url, int generation = -1);

    void prepareAsync();

    status_t prepare();

    void start();
    void stop();
    void pause();
    void resume();
    //void release();

    void disconnect();

    //status_t feedMoreTSData();

    // sp<MetaData> getFileFormatMeta() const;
    //status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit);
    //status_t getDuration(INT64 *durationUs);
    //UINT64 getTrackCount() const;
    //sp<MetaData> getTrackInfo(UINT64 trackIndex) const;
    //UINT64 getSelectedTrack(media_track_type type) const;
    //status_t seekTo(UINT64 seekTimeUs);
    status_t dequeueAccessUnit(bool audio, AVPacket *pkt);
    Track*   getMediaTrack(bool audio);
    status_t seekTo(off64_t seekTime);
    status_t writeData(unsigned char* buffer, int size, status_t result = false);
    off64_t getAvailiableSize(void);
    status_t getDuration(off64_t *duration);
    status_t postReadBuffer();
    
    status_t SetOutFileName(const char *OutFileName);

    ~GenericSource();

private:
    void *GenericMessageReceive(void);
    static void *GenericThreadFunc(void *data);

    pthread_t mSourceThread;
    AMessageQueue*  mQueue;
    AMessageQueue*  mNotifyQueue;

    char* mUri;
    bool mStarted;
    bool mStopRead;
    FFDemuxer* mDemuxer;
    Track* mAudioTrack;
    AVPacketSource* mAudioSource;
    
    bool   mIsTTSource;
    int mFetchDataSize;
    off64_t mDuration;
    int  mGeneration;

    status_t mFinalResult;
    bool mBuffering;
    Mutex mBufferingLock;
    AVRational time_base;

    void setError(status_t err);
    bool haveSufficientDataOnAllTracks();
    void onReadBuffer();

    void notifyEOS(status_t err = OK);
    void notifyPrepared(status_t err = OK);
    void resetDataSource();
    void finishPrepareAsync();
    void onPrepareAsync();
    void notifyPreparedAndCleanup(status_t err);
    status_t initFromDataSource();

    //for dump source data
    AVPacket* mDumpPacket;

    DISALLOW_EVIL_CONSTRUCTORS(GenericSource);
};
#endif  // GENERIC_SOURCE_H_


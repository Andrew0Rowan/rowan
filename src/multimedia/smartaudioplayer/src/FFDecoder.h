#ifndef _FFDECODER_H_
#define _FFDECODER_H_
#include <vector>
#include <semaphore.h>
#include <pthread.h>

#include "FFmpeg.h"
#include "FFDemuxer.h"
#include "ABuffer.h"
#include "pp_msg_q.h"
#include "ABase.h"

#include "AMessageQueue.h"

using namespace std;

#define kInputBufferQueueCount 10
#define kOutPutBufferQueueCount 15

class FFDecoder
{
public:
    /* public member variable */

    /* public member function */
    FFDecoder();
    FFDecoder(AMessageQueue* &notify);
    virtual ~FFDecoder();
    status_t OpenDecoder(Track *PPTrack);
    status_t ConfigDecoder(AudioFormat OutAudioPara);
    status_t Decoder(AVPacket *packet, ABuffer *pABuffer);
    void setPCMMode();

    int  DequeueInputBuffer();
    void QueueInputBuffer(int index);
    void QueueInputBufferfront(int index);
    int  DequeueInputBufferAccessUnit();
    void QueueInputBufferAccessUnit(int index);
    int  DequeueOutputBuffer();
    void QueueOutputBuffer(int index);
    void QueueOutputBufferfront(int index);
    int  DequeueOutputBufferAccessUnit();
    void QueueOutputBufferAccessUnit(int index);
    void Start();
    void Stop();
    void Play();
    void Flush();
    AVPacket *GetInputBuf(int index);
    ABuffer *GetOutputBuf(int index);
    void PrintAllBufStatus();
    void FlushAllBuf();
    status_t Release();
    ABuffer *mOutPutBuf[kOutPutBufferQueueCount];

private:
    /* private member variable */
    unsigned long long int mPacketNum;
    unsigned long long int mStartPts;
    unsigned long long int mEndPts;
    enum AVCodecID mCodecId;
    AVRational time_base;
    int mOutBufSize;
    AudioFormat mOutAudioFormat;
    bool mIsComponentAlive;
    AVCodecContext  *mCodecCtx;
    AVCodec         *mCodec;
    struct SwrContext *mSwrConvertCtx;
    AVCodecParameters *mCodecPar;
    unsigned char *mOutBuffer;

    AVPacket *mPacket[kInputBufferQueueCount];

    pthread_t DecoderThread;
    Mutex mDecodeLock;

    Mutex mInputBufQLock;
    Condition mInputBufQCondition;

    Mutex mInputBufAccessUnitLock;
    vector<int> mInputBufQ;
    vector<int> mInputBufAccessUnit;

    Mutex mOutputBufQLock;
    Condition mOutputBufQCondition;

    Mutex mOutputBufAccessUnitLock;
    vector<int> mOutputBufQ;
    vector<int> mOutputBufAccessUnit;

    AMessageQueue* mNotifyQueue;

    /* private member function */
    static void *FFecodeThreadFunc(void *pData);
    void *FFecodeThread();
    void DequeueInputBufferAll();
    void DequeueInputBufferAccessUnitAll();
    void DequeueOutputBufferAll();
    void DequeueOutputBufferAccessUnitAll();
    void InitInputBufAccessUnit();
    void InitOutputBuf();
    void InitDecoder();
    void DeInitDecoder();
    status_t AllocateBufs();
    status_t FreeBufs();

    bool mIsPCMSource;

    DISALLOW_EVIL_CONSTRUCTORS(FFDecoder);
};

#endif



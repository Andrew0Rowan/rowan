#ifndef _MASRENDER_H_
#define _MASRENDER_H_
#include <vector>
#include <pthread.h>

#include "alsa/asoundlib.h"
#include "FFDemuxer.h"
#include "os.h"
#include "ABuffer.h"

#include "Mutex.h"
#include "Condition.h"

#include "AMessageQueue.h"

using namespace std;

class MASRender
{
public:
    MASRender(AMessageQueue* &notify);
    virtual ~MASRender();
    status_t ConfigAlsaFormat(AudioFormat OutAudioPara);
    status_t OpenRender();
    void Start();
    void Pause();
    void Resume();
    status_t Release();
    status_t Render(ABuffer *pABuffer);
    void QueueOutputBuffer(ABuffer *pABuffer, int index);
    void Flush();
    status_t SetMsgHandle(AMessageQueue* &queue);
    status_t getCurrentPosition(off64_t *mediaUs);

private:
    /* private member variable */
    snd_pcm_t *mMASRenderHandle;
    snd_pcm_hw_params_t *mHwParams;
    AudioFormat mOutAudioFormat;
    bool mIsComponentAlive;

    pthread_t RenderThread;
    AMessageQueue*  mBufferQueue;
    AMessageQueue*  mNotifyQueue;

    vector<ABuffer *> mOutputBufQ;
    vector<int> mOutputBufQIndex;
    off64_t mPosition;
    bool mPaused;

    Mutex mLock;
    Condition mCondition;

    Mutex mOutputBufQLock;
    Condition mRenderCondition;

    Mutex mRenderLock;
    Mutex mPositionLock;

    /* private member function */
    static void *MASRenderThreadFunc(void *pData);
    void *MASRenderThread();
    ABuffer *DequeueOutputBuffer(int *index);
    status_t DeInit();
    void FlushAllBuf();
    void Init();
    DISALLOW_EVIL_CONSTRUCTORS(MASRender);
};

#endif




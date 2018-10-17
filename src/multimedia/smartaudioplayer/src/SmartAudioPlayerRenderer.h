#ifndef _SMARTAUDIOPLAYER_RENDERER_H_
#define _SMARTAUDIOPLAYER_RENDERER_H_

#include "SmartAudioPlayer.h"
#include "ABuffer.h"

#include "SmartAudioPlayerDecoder.h"

#include "AMessageQueue.h"

class SmartAudioPlayer::Renderer{
public:
    /* public member variable */
    Renderer(AMessageQueue* &notify);
    virtual ~Renderer();
    status_t init();
    void queueBuffer(ABuffer* &ab, int index);
    void queueEOS();
    status_t SetHandle(AMessageQueue* &queue);
    status_t ConfigFormat(AudioFormat format);
    void pause();
    void resume();
    void flush();
    void release();
    void start();
    status_t getCurrentPosition(off64_t *mediaUs);
    /* public member function */

private:
    /* private member variable */
    AMessageQueue*  mQueue;
    AMessageQueue*  mNotifyQueue;
    MASRender *mRender;
    /* private member function */
    DISALLOW_EVIL_CONSTRUCTORS(Renderer);
};

#endif

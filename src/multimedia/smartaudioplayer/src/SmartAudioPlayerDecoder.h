#ifndef SMARTAUDIOPLAYER_DECODER_BASE_H_
#define SMARTAUDIOPLAYER_DECODER_BASE_H_

#include "SmartAudioPlayer.h"
#include "GenericSource.h"
#include "FFDecoder.h"
#include "SmartAudioPlayerRenderer.h"
#include "Format.h"

#include "AMessageQueue.h"

struct ABuffer;

struct SmartAudioPlayer::Decoder {

public:

    Decoder(AMessageQueue* &notify,SmartAudioPlayer::GenericSource* &source, SmartAudioPlayer::Renderer* &render);
    void configure(AudioFormat &format);
    void onConfigureSync(AudioFormat &format);
    void init();

    virtual ~Decoder();

    void signalFlush();
    void signalResume(bool notifyComplete);
    void initiateShutdown();
    void setPCMMode();
    status_t writePCMData(unsigned char* buffer, int size, status_t result);

private:

    pthread_t mDecoderThread;
    AMessageQueue*  mQueue;
    AMessageQueue*  mNotifyQueue;

    AudioFormat mFormat;

    GenericSource*  mSource;
    FFDecoder*      mDecoder;
    Renderer*       mRenderer;

    bool mPaused;
    bool mResumePending;
    bool mRequestInputBuffersPending;
    bool mInputPortEOS;

    FILE *mInputFile;
    FILE *mOutputFile;

    void InitInputBuffer();

    void onConfigure();
    bool handleAnInputBuffer();
    void handleAnOutputBuffer(bool isEOS = false);
    void handleError(int err);
    void pause();
    void onResume(bool notifyComplete);
    void doFlush(bool notifyComplete);
    void onFlush();
    void onShutdown(bool notifyComplete);
    void onRenderBuffer(int index);
    void *DecoderMessageRecieve();
    static void *DecoderThreadFunc(void *data);

    bool mIsPCMSource;

    DISALLOW_EVIL_CONSTRUCTORS(Decoder);
};

#endif

#ifndef _AMESSAGEQUEUE_H_
#define _AMESSAGEQUEUE_H_

#include "pp_msg_q.h"
#include "ABase.h"
#include "Errors.h"

typedef struct {
    int type;
    int err;
    int bufferID;
    int generation;
    int notifyComplete;
    int needNotify;
    off64_t seekTime;
    int size;
    unsigned char* buffer;
} AMessage;

enum BufferFlags {
    BUFFER_FLAG_SYNCFRAME   = -100,
    BUFFER_FLAG_CODECCONFIG = -101,
    BUFFER_FLAG_EOS         = -102,
};

enum {
    kWhatCodecNotify         = 'cdcN',
    kWhatRenderBuffer        = 'rndr',
    kWhatSetVideoSurface     = 'sSur',

    kWhatInputDiscontinuity  = 'inDi',
    kWhatVideoSizeChanged    = 'viSC',
    kWhatFlushCompleted      = 'flsC',
    kWhatShutdownCompleted   = 'shDC',
    kWhatResumeCompleted     = 'resC',
    kWhatEOS                 = 'eos ',
    kWhatError               = 'err ',

    kWhatConfigure           = 'conf',
    kWhatSetParameters       = 'setP',
    kWhatSetRenderer         = 'setR',
    kWhatPause               = 'paus',
    kWhatGetInputBuffers     = 'gInB',
    kWhatRequestInputBuffers = 'reqB',
    kWhatShutdown            = 'shuD',

    kWhatQueueBuffer         = 'queB',
    kWhatQueueEOS            = 'qEOS',
    kWhatFlush               = 'flus',

    kWhatSetDataSource              = '=DaS',
    kWhatPrepare                    = 'prep',
    kWhatSetAudioSink               = '=AuS',
    kWhatConfigPlayback             = 'cfPB',
    kWhatConfigSync                 = 'cfSy',
    kWhatGetPlaybackSettings        = 'gPbS',
    kWhatGetSyncSettings            = 'gSyS',
    kWhatStart                      = 'strt',
    kWhatScanSources                = 'scan',
    kWhatVideoNotify                = 'vidN',
    kWhatAudioNotify                = 'audN',
    kWhatReset                      = 'rset',
    kWhatResume                     = 'rsme',

    kWhatClosedCaptionNotify        = 'capN',
    kWhatRendererNotify             = 'renN',
    kWhatSeek                       = 'seek',
    kWhatPollDuration               = 'polD',
    kWhatSourceNotify               = 'srcN',
    kWhatGetTrackInfo               = 'gTrI',
    kWhatGetSelectedTrack           = 'gSel',
    kWhatSelectTrack                = 'selT',

    kWhatPrepared                   = 'prpd',
    kWhatPrepareAsync               = 'prpa',
    kWhatShutDownOwn                = 'stdo',
    kWhatRendererEOS                = 'wrde',
    kWhatRendererPaused             = 'wrdp',
    kWhatInputBufferAvaliable       = 'ibfa',
    kWhatOutputBufferAvaliable      = 'obfa',
    kWhatDecoderEndStream           = 'deds',
    kWhatReadBuffer                 = 'rdbf',    
    kWhatSourceEOS                  = 'wsre',
};

class AMessageQueue
{
public:
    AMessageQueue();
    status_t SendMessage(AMessage *message);
    void ReceiveMessage(AMessage *message);
    virtual ~AMessageQueue();

private:
    HANDLE_T mHandle;

    DISALLOW_EVIL_CONSTRUCTORS(AMessageQueue);
};

#endif


#ifndef _FFSTREAMER_H_
#define _FFSTREAMER_H_

#include "FFmpeg.h"
#include "Mutex.h"
#include "Condition.h"
#include "ABase.h"

class RingBuf;

class FFStreamer
{
public:
    /* public member variable */
    /* public member function */
    FFStreamer();
    status_t SetDataSource(const char *Url, int size);
    status_t OpenAvio();
    status_t CloseAvio();
    int Read(void *opaque, unsigned char *data, int size);
    int Write(void *opaque, unsigned char *data, int size);
    unsigned long long int GetFileSize();
    AVIOContext *GetAVIOCtx();
    unsigned int GetAvailableSize();
    void signalEOS();
    void Disconnect();
    bool getStatus();
    virtual ~FFStreamer();
    int readBuffer(uint8_t *buf, int buf_size);
    static int read_buffer(void *opaque, unsigned char *buf, int buf_size);
    void pause();
    void resume();
private:
    /* private member variable */
    AVIOContext *mContext;      /* Bytestream IO Context,FFMPEG internal type*/
    unsigned long long int mOffset;             /* Current offset */
    unsigned long long int mFileSize;               /* file size */
    char *mUrl;                 /* file adress,such as "/data/1.mp3" or "http:/.." */
    unsigned char *mAvioBuffer;
    RingBuf *mRingBuf;
    Mutex mLock;
    Condition mCondition;
    bool mEOSResult;
    bool mStop;
    bool mTtsPause;
    /* private member function */

    DISALLOW_EVIL_CONSTRUCTORS(FFStreamer);
};

class RingBuf
{
public:
    /* public member variable */

    /* public member function */
    RingBuf();
    virtual ~RingBuf();
    void Clear();
    int Write(void *pdata, unsigned int ui4Size);
    int Read(void *pdata, unsigned int ui4Size);
    unsigned int GetUsedSize();
    unsigned int GetAvailableSize();

private:
    /* private member variable */
    void *mBuf;
    unsigned int mSize;
    unsigned int mWrite;
    unsigned int mRead;
    /* private member function */

    void WriteData(void *pdata, unsigned int ui4Size);
    void WriteDataLoopback(void *pdata, unsigned int ui4Size);
    void ReadData(void *pdata, unsigned int ui4Size);
    void ReadDataLoopback(void *pdata, unsigned int ui4Size);

    DISALLOW_EVIL_CONSTRUCTORS(RingBuf);
};

#endif


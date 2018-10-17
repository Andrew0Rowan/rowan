#ifndef _FFDEMUXER_H_
#define _FFDEMUXER_H_

#include "FFmpeg.h"
#include "FFStreamer.h"
#include "refptr.h"
#include "Format.h"
#include "ABase.h"


class Track;

class FFDemuxer
{
public:
    /* public member variable */
    /* public member function */
    FFDemuxer();
    FFDemuxer(const char *Url);
    virtual ~FFDemuxer();
    status_t SetDataSource(const char *Url, int size);
    status_t Demux(AVPacket *pkt);
    unsigned long long int GetFileSize();
    int CountTracks();
    Track* GetTrack(int mIndex);
    AVFormatContext* GetFormatCtx();
    int Write(unsigned char *data, int size);
    status_t Start();
    void Stop();
    void  signalEOS();
    bool getStatus();
    void Disconnect();
    status_t getDuration(off64_t *duration);
    unsigned int GetAvailableSize();    
    void pause();
    void resume();

    status_t SetOutFileName(const char *OutFileName, int size);
    AVFormatContext* GetOutFormatCtx();
    bool getmSaveHeader();
    status_t OpenDemux();

private:
    /* private member variable */
    AVFormatContext	*mFormatCtx;/* Format I/O context,FFMPEG internal type*/
    FFStreamer *mStreamer;/* the stream */
    unsigned long long int mOffset;/* Current offset */
    char *mUrl;/* file adress,such as "/data/1.mp3" or "http:/.." */
    off64_t mTrackNum;/* The number of track */
    off64_t mDuration;/* mDuration */
    off64_t mDurationBySecs;/* mDuration by seconds */
    bool  mStarted;
    bool  mStop;

    char *mOutFileName;/* output file name,such as "/data/dump.mp3" */
    bool mDumpSource;
    AVFormatContext *mOutFormatCtx;
    AVOutputFormat *mOutFmt;
    bool  mSaveHeader;
    /* private member function */

    status_t CloseDemux();
    DISALLOW_EVIL_CONSTRUCTORS(FFDemuxer);
};


class Track
{
public:
    /* public member variable */
    /* public member function */
    Track(FFDemuxer *mDemuxer, int mIndex);
    virtual ~Track();
    status_t Start();
    status_t Stop();
    status_t ReadAPacket(AVPacket *pkt);
    status_t SeekBySecond(int mSeekSecond);
    status_t SeekByPts(off64_t seekPts);
    status_t GetTrackFormat();
    AVStream *GetStream();
    AVCodecParameters *GetCodecPar();
    status_t GetCodecType(enum AVMediaType *CodecType);
    status_t GetCodecId(enum AVCodecID *CodecId);

private:
    /* private member variable */
    AVStream *mStream;
    AVCodecParameters *mCodecPar;
    enum AVMediaType mCodecType;
    enum AVCodecID   mCodecId;
    FFDemuxer *mFFDemuxer;/* the Demuxer */
    int mTrackIndex;
    AudioFormat mFormat;
    off64_t mLastPts;
    /* private member function */
    DISALLOW_EVIL_CONSTRUCTORS(Track);
};
#endif


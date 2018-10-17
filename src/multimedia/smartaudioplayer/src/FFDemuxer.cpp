/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef CONFIG_SUPPORT_FFMPEG


#include "FFStreamer.h"
#include "FFmpeg.h"
#include "Log.h"
#include "Errors.h"
#include "FFDemuxer.h"

/* public member variable */
/* public member function */

static int demuxer_interrupt_cb(void *ctx)
{
    FFDemuxer *demuxer = (FFDemuxer *)ctx;
    return demuxer->getStatus();
}


FFDemuxer::FFDemuxer()
    : mFormatCtx(NULL),
      mStreamer(NULL),
      mOffset(0),
      mUrl(NULL),
      mTrackNum(0),
      mDuration(0),
      mStarted(false),
      mStop(false),
      mDurationBySecs(0),
      mOutFileName(NULL),
      mDumpSource(false),
      mOutFormatCtx(NULL),
      mOutFmt(NULL),
      mSaveHeader(false)
{
}

FFDemuxer::FFDemuxer(const char *Url)
    : mFormatCtx(NULL),
      mStreamer(NULL),
      mOffset(0),
      mTrackNum(0),
      mDuration(0),
      mStarted(false),
      mStop(false),
      mDurationBySecs(0),
      mOutFileName(NULL),
      mDumpSource(false),
      mOutFormatCtx(NULL),
      mOutFmt(NULL),
      mSaveHeader(false)
{
    if(NULL == Url)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("The Url is NULL!"));
    }
    else
    {
        size_t size = strlen(Url);
        mUrl = (char *)malloc(size + 1);
        memcpy(mUrl, Url, size);
        mUrl[size] = '\0';
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("After FFDemuxer mUrl is %s!", mUrl));
    }
}

FFDemuxer::~FFDemuxer()
{
    if(NULL != mUrl)
    {
       SAP_LOGENTRY(SAP_DEMUXER_TAG,SAP_LOG_INFO,("free url"));
       free(mUrl);
       mUrl = NULL;
    }
    if(NULL != mOutFileName)
    {
       SAP_LOGENTRY(SAP_DEMUXER_TAG,SAP_LOG_INFO,("free mOutFileName"));
       free(mOutFileName);
       mOutFileName = NULL;
    }
    mDumpSource = false;
    mSaveHeader = false;
    CloseDemux();
}

status_t FFDemuxer::OpenDemux()
{
    status_t i4_ret = 0, i = 0;

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("OpenDemux Start!"));
    mFormatCtx = pfnavformat_alloc_context();
    if(NULL == mFormatCtx)
    {
        i4_ret = AVERROR(ENOMEM);
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("pfnavformat_alloc_context Failed!"));
        return NO_MEMORY;
    }
    mFormatCtx->interrupt_callback.callback = demuxer_interrupt_cb;
    mFormatCtx->interrupt_callback.opaque = this;

    mStreamer = new FFStreamer();
    if(NULL != mUrl)
    {
        mStreamer->SetDataSource(mUrl,strlen(mUrl));
    }
    i4_ret = mStreamer->OpenAvio();
    if (OK != i4_ret) {
        return UNKNOWN_ERROR;
    }

    mFormatCtx->pb = mStreamer->GetAVIOCtx();

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("OpenDemux End Success!"));
    return OK;
}

status_t FFDemuxer::CloseDemux()
{
    SAP_LOGENTRY(SAP_DEMUXER_TAG,SAP_LOG_INFO,("CloseDemux"));

    if(NULL != mFormatCtx)
    {
        SAP_LOGENTRY(SAP_DEMUXER_TAG,SAP_LOG_INFO,("avformat_close_input"));
        if (mStarted)
        {
            pfnavformat_close_input(&mFormatCtx);
            SAP_LOGENTRY(SAP_DEMUXER_TAG,SAP_LOG_INFO,("avformat_close_input success"));
        }
        else
        {
            pfnavformat_free_context(mFormatCtx);
            SAP_LOGENTRY(SAP_DEMUXER_TAG,SAP_LOG_INFO,("avformat_free_context success"));
        }
        mStarted = false;
        mFormatCtx = NULL;
    }
    else
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("Demux has been closed!"));
    }

    if(NULL != mStreamer)
    {
        SAP_LOGENTRY(SAP_DEMUXER_TAG,SAP_LOG_INFO,("close streamer"));
        delete mStreamer;
        mStreamer = NULL;
    }

    if(NULL != mOutFormatCtx)
    {
        SAP_LOGENTRY(SAP_DEMUXER_TAG,SAP_LOG_INFO,("avformat_free_context"));
        if (mOutFormatCtx && !(mOutFmt->flags & AVFMT_NOFILE))
        {
            pfnavio_close(mOutFormatCtx->pb);
            mOutFormatCtx->pb = NULL;
        }
        pfnavformat_free_context(mOutFormatCtx);
        mOutFormatCtx = NULL;
    }


    return OK;
}

status_t FFDemuxer::Demux(AVPacket *pkt)
{
    return pfnav_read_frame(mFormatCtx, pkt);
}


status_t FFDemuxer::SetDataSource(const char *Url, int size)
{
    if(NULL != mUrl)
    {
        free(mUrl);
        mUrl = NULL;
    }

    if(NULL == Url)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("The Url is NULL!\n"));
        return UNKNOWN_ERROR;
    }
    else
    {
        mUrl = (char *)malloc(size + 1);
        memcpy(mUrl, Url, size);
        mUrl[size] = '\0';
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("After SetDataSource mUrl is %s!", mUrl));
        return OK;
    }
}


unsigned long long int FFDemuxer::GetFileSize()
{
    return mStreamer->GetFileSize();
}

int FFDemuxer::CountTracks()
{
    return mTrackNum;
}

Track *FFDemuxer::GetTrack(int mIndex)
{
    Track *mTrack = new Track(this, mIndex);
    if(NULL == mTrack)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("New Track Failed!"));
        return NULL;
    }
    else
    {
        return mTrack;
    }
}

AVFormatContext *FFDemuxer::GetFormatCtx()
{
    return mFormatCtx;
}

int FFDemuxer::Write(unsigned char *data, int size)
{
//PP_DEMUXER_DBG_MSG("FFDemuxer Write \n");
    return mStreamer->Write(NULL, data, size);
}


unsigned int FFDemuxer::GetAvailableSize()
{
    return mStreamer->GetAvailableSize();
}

status_t FFDemuxer::Start()
{
    status_t i4_ret = 0, audioStream = -1, i = 0;

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("FFDemuxer Start!"));

    if (NULL == mStreamer->GetAVIOCtx()) {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("AVIO is NULL!"));
        return UNKNOWN_ERROR;
    }

    AVInputFormat *fmt = NULL;
    if (mUrl == NULL)
    {
        i4_ret = pfnav_probe_input_buffer(mStreamer->GetAVIOCtx(), &fmt, NULL, NULL, 0, 2048);
    }
    else
    {
        i4_ret = pfnav_probe_input_buffer(mStreamer->GetAVIOCtx(), &fmt, NULL, NULL, 0, 64 * 1024);
    }

    if (i4_ret < 0)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("Couldn't Probe Format! %d", i4_ret));
        return UNKNOWN_ERROR;
    }
    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("Probe success, format: %s[%s]", fmt->name, fmt->long_name));

    AVDictionary *opt = NULL;
    if (mUrl != NULL &&(strlen(mUrl) > strlen("http")) &&(strncmp(mUrl, "http", strlen("http")) == 0)) {
        pfnav_dict_set(&opt, "rw_timeout", "3000000", 0);
        pfnav_dict_set(&opt, "timeout", "3000000", 0);
        pfnav_dict_set(&opt, "listen_timeout", "10000", 0);
        pfnav_dict_set(&opt, "reconnect", "0", 0);
        pfnav_dict_set(&opt, "reconnect_at_eof", "0", 0);
        pfnav_dict_set(&opt, "reconnect_streamed", "1", 0);
        pfnav_dict_set(&opt, "multiple_requests", "1", 0);
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("http set timeout"));
    }

    i4_ret = pfnavformat_open_input(&mFormatCtx, mUrl, fmt, &opt);
    if (i4_ret != 0)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("Couldn't Open Input Stream! %d", i4_ret));
        goto fail;
    }

    mStarted = true;

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("pfnavformat_find_stream_info!"));

    if((i4_ret = pfnavformat_find_stream_info(mFormatCtx, NULL)) < 0)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("Couldn't find stream information!"));
        goto fail;
    }

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("after pfnavformat_find_stream_info!"));

    mDuration = mFormatCtx->duration + (mFormatCtx->duration <= INT64_MAX - 5000 ? 5000 : 0);
    mDurationBySecs  = mDuration / AV_TIME_BASE;
    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("Input stream mDuration %lld, mDurationBySecs %lld, start time %lld",
        mDuration, mDurationBySecs, mFormatCtx->start_time));

    mTrackNum = mFormatCtx->nb_streams;
    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("Input stream mTrackNum %lld!", mTrackNum));

    audioStream = -1;
    for(i = 0; i < mFormatCtx->nb_streams; i++)
    {
        if(mFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStream = i;
            SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("Find a audio stream,Index is %d!",audioStream));
            break;
        }
    }
    if(audioStream == -1)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("Didn't find a audio stream !"));
        goto fail;
    }

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("FFDemuxer End !"));
    if (opt != NULL)
        pfnav_dict_free(&opt);

    /* dump source header*/
    if(true == mDumpSource){
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("mDumpSource Header Start !"));

        strcat(mOutFileName,mFormatCtx->iformat->name);
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("mOutFileName is %s!\n", mOutFileName));

        i4_ret = pfnavformat_alloc_output_context2(&mOutFormatCtx, NULL, NULL, mOutFileName);
        if (0 != i4_ret) {
            SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("Could not Create mOutFormatCtx, %d %s!\n", i4_ret, av_error2str(i4_ret)));
            goto ok;
        }
        mOutFmt = mOutFormatCtx->oformat;

        AVStream *out_stream = pfnavformat_new_stream(mOutFormatCtx, NULL);
        if (!out_stream) {
            SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("Failed allocating output stream!\n"));
            goto ok;
        }
        //Copy the settings of AVCodecContext
        if (pfnavcodec_copy_context(out_stream->codec, mFormatCtx->streams[audioStream]->codec) < 0){
            SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("Failed to copy codec context!\n"));
            goto ok;
        }

        out_stream->codec->codec_tag = 0;
        /* Some formats want stream headers to be separate. */
        if (mOutFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

        //Open output file
        if (!(mOutFmt->flags & AVFMT_NOFILE)){
            if (pfnavio_open(&mOutFormatCtx->pb, mOutFileName, AVIO_FLAG_WRITE) < 0){
                SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("Could not open output file '%s'!", mOutFileName));
                goto ok;
            }
        }
        //Write file header
        if (pfnavformat_write_header(mOutFormatCtx, NULL) < 0){
            SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("Error occurred when avformat_write_header!\n"));
            goto ok;
        }

        out_stream = mOutFormatCtx->streams[audioStream];
        out_stream->time_base = mFormatCtx->streams[audioStream]->time_base;

        mSaveHeader = true;

        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("mDumpSource Header End !"));
    }

ok:
    return OK;

fail:
    if (opt != NULL)
        pfnav_dict_free(&opt);
    return UNKNOWN_ERROR;
}

void FFDemuxer::Disconnect() {
    mStop = true;
    mStreamer->Disconnect();
}

bool FFDemuxer::getStatus() {
    return mStop;
}

void FFDemuxer::signalEOS() {
    mStreamer->signalEOS();
}

status_t FFDemuxer::getDuration(off64_t *duration) {
    if(mDuration < 0)
        return UNKNOWN_ERROR;

    *duration = mDuration;
    return OK;
}

// pause and resume only for tts
void FFDemuxer::pause()
{
    mStreamer->pause();
}

void FFDemuxer::resume()
{
    mStreamer->resume();
}

status_t FFDemuxer::SetOutFileName(const char *OutFileName, int size)
{
    if(NULL != mOutFileName)
    {
        free(mOutFileName);
        mOutFileName = NULL;
    }

    if(NULL == OutFileName)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("The OutFileName is NULL!\n"));
        return UNKNOWN_ERROR;
    }
    else
    {
        /* the malloc size must be large enough for strcat */
        mOutFileName = (char *)malloc(size + 20);
        memcpy(mOutFileName, OutFileName, size);
        mOutFileName[size] = '\0';
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("After SetOutFileName mOutFileName is %s!", mOutFileName));
    }

    mDumpSource = true;
    return OK;
}

AVFormatContext *FFDemuxer::GetOutFormatCtx()
{
    return mOutFormatCtx;
}

bool FFDemuxer::getmSaveHeader() {
    return mSaveHeader;
}

/* private member function */



/* public member variable */
/* public member function */
Track::Track(FFDemuxer *mDemuxer, int mIndex)
    : mStream(NULL),
      mTrackIndex(0),
      mCodecType(AVMEDIA_TYPE_UNKNOWN),
      mCodecId(AV_CODEC_ID_NONE),
      mCodecPar(NULL),
      mLastPts(0)
{
    mFFDemuxer = mDemuxer;
    mTrackIndex = mIndex;
    memset((void *)(&mFormat), 0, sizeof(AudioFormat));
}

Track::~Track()
{
   /* mStream and mCodecPar are all free by the demuxer*/
}

status_t Track::Start()
{
    mStream = mFFDemuxer->GetFormatCtx()->streams[mTrackIndex];

    mCodecPar = mStream->codecpar;
    mCodecId = mCodecPar->codec_id;
    mCodecType = mCodecPar->codec_type;

    mFormat.mChannelLayout = mCodecPar->channel_layout;
    mFormat.mSampleFmt = (enum AVSampleFormat)mCodecPar->format;
    mFormat.mSampleRate =  mCodecPar->sample_rate;
    mFormat.mChannels =  mCodecPar->channels;

    return OK;
}

status_t Track::Stop()
{
    return OK;
}

status_t Track::SeekByPts(off64_t seekPts)
{
#if 1 /*quik seek method*/
    double seekSecond = (double)seekPts / AV_TIME_BASE;

    double pos = pfnavio_seek(mFFDemuxer->GetFormatCtx()->pb, 0, SEEK_CUR);
    double incr = seekSecond - pos * 8.0 / mFFDemuxer->GetFormatCtx()->bit_rate;

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("seekSecond:[%d] lastSecond:[%lf] incr:[%lf]!\n", seekSecond, pos * 8.0 / mFFDemuxer->GetFormatCtx()->bit_rate, incr));

    if (mFFDemuxer->GetFormatCtx()->bit_rate) {
        incr *= mFFDemuxer->GetFormatCtx()->bit_rate / 8.0;
    } else {
        incr *= 180000.0;
    }
    pos += incr;

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("bitrate:[%lld] pos:[%lf] incr:[%lf]!\n", mFFDemuxer->GetFormatCtx()->bit_rate, pos, incr));

    int64_t seek_min = incr > 0 ? pos - incr + 2 : INT64_MIN;
    int64_t seek_max = incr < 0 ? pos - incr - 2 : INT64_MAX;

    status_t ret = pfnavformat_seek_file(mFFDemuxer->GetFormatCtx(), -1, seek_min, pos, seek_max, AVSEEK_FLAG_BYTE);
    if (ret < 0)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("avformat_seek_file fail:[%d]!", ret));
        return ret;
    }

    /* after seek the data maybe is error, so we lost 10 packets */
    int i;
    AVPacket* buffer = pfnav_packet_alloc();
    if (NULL == buffer) {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("Failed to create AVPacket!"));
        return NO_MEMORY;
    }
    pfnav_init_packet(buffer);

    for(i = 0; i < 10; i++)
    {
        pfnav_read_frame(mFFDemuxer->GetFormatCtx(), buffer);
        pfnav_packet_unref(buffer);
    }
    pfnav_packet_free(&buffer);

    return OK;
#else
    status_t ret = OK;
    off64_t TargetPts = 0;
    if(seekPts > mFFDemuxer->GetFormatCtx()->duration)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("The mPts is bigger than the Stream duration!"));
        return UNKNOWN_ERROR;
    }

    TargetPts = seekPts / av_q2d(mStream->time_base) / AV_TIME_BASE;

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("The target PTS is %lld!", TargetPts));
    ret = pfnav_seek_frame(mFFDemuxer->GetFormatCtx(), mTrackIndex, TargetPts, 0);
    if (ret < 0)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("av_seek_frame fail!"));
        return ret;
    }
    return OK;
#endif
}

status_t Track::SeekBySecond(int seekSecond)
{
    status_t ret = OK;
    off64_t TargetPts = 0;
    if (seekSecond * AV_TIME_BASE > mFFDemuxer->GetFormatCtx()->duration)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("The mSeekSecond is bigger than the file duration!"));
        return UNKNOWN_ERROR;
    }

    TargetPts = seekSecond / av_q2d(mStream->time_base);

    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("The target PTS is %lld!", TargetPts));
    ret = pfnav_seek_frame(mFFDemuxer->GetFormatCtx(), mTrackIndex, TargetPts, AVSEEK_FLAG_BACKWARD);
    if (ret < 0)
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("av_seek_frame fail!"));
        return ret;
    }
    return OK;
}


status_t Track::ReadAPacket(AVPacket *pkt)
{
    status_t ret = OK;
    if(NULL != pkt)
    {
        do
        {
            ret = mFFDemuxer->Demux(pkt);
            //SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_INFO,("mLastPts : %lld, stream duration : %lld",mLastPts, mStream->duration));

            if(ret < 0)
            {
                SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("mLastPts : %lld, stream duration : %lld ret= %s",
                    mLastPts, mStream->duration, av_error2str(ret)));

                if (ret == AVERROR_EOF) {
                    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("End of File"));
                    return ERROR_END_OF_STREAM;
                }
                else if (ret == AVERROR(EAGAIN)) {
                    SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("Continue"));
                    continue;
                }
#if 0
                if((mStream->duration - mLastPts) < AV_TIME_BASE)
                {
                    SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("ERROR_END_OF_STREAM!"));
                    return ERROR_END_OF_STREAM;
                }
#endif
                SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("ReadAPacket fail ret= %s", av_error2str(ret)));
                return UNKNOWN_ERROR;
            }
            else
            {
                mLastPts = pkt->pts;
            }
        }while(pkt->stream_index != mTrackIndex);

        if(ret >= 0)
        {
            return ret;
        }
    }
    else
    {
        SAP_LOG(SAP_DEMUXER_TAG,SAP_LOG_ERROR,("ReadAPacket FAIL, The pkt is NULL!"));
        return UNKNOWN_ERROR;
    }
}

int Track::GetTrackFormat()
{
    return OK;
}

AVStream *Track::GetStream()
{
    return mStream;
}

AVCodecParameters *Track::GetCodecPar()
{
    return mCodecPar;
}

int Track::GetCodecType(enum AVMediaType *CodecType)
{
    *CodecType = mCodecType;
    return OK;
}

int Track::GetCodecId(enum AVCodecID *CodecId)
{
    *CodecId = mCodecId;
    return OK;
}

/* private member variable */

/* private member function */

#endif


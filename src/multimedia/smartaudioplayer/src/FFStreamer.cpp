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
#include <unistd.h>

#define AVIO_BUFFER_SIZE 4096

/* public member variable */

static int streamer_interrupt_cb(void *ctx)
{
    FFStreamer *streamer = (FFStreamer *)ctx;
    return streamer->getStatus();
}


/* public member function */

FFStreamer::FFStreamer()
    : mContext(NULL),
      mOffset(0),
      mFileSize(0),
      mUrl(NULL),
      mEOSResult(false),
      mStop(false),
      mTtsPause(false),
      mAvioBuffer(NULL),
      mRingBuf(NULL)
{
}

FFStreamer::~FFStreamer()
{
    CloseAvio();
}

status_t FFStreamer::SetDataSource(const char *Url, int size)
{
    if(NULL != mUrl)
    {
        free(mUrl);
        mUrl = NULL;
    }

    if(NULL == Url)
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("The Url is NULL, It is TTS mode!"));

        return UNKNOWN_ERROR;
    }
    else
    {
        mUrl = (char *)malloc(size + 1);
        memcpy(mUrl, Url, size);
        mUrl[size] = '\0';
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("After SetDataSource mUrl is %s!", mUrl));
        return OK;
    }
}



status_t FFStreamer::OpenAvio()
{
    status_t ret = 0;
    off64_t size = 0;
    AVDictionary *opt = NULL;

    SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("OpenAvio Start!"));
    if(NULL == mUrl)
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("You must set URL first or It maybe TTS MODE!"));
        mAvioBuffer = (unsigned char *)pfnav_malloc(AVIO_BUFFER_SIZE);
        mRingBuf = new RingBuf();
    }

    if(NULL != mContext)
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("You must CloseAvio the Avio first!"));
        return UNKNOWN_ERROR;
    }

    if(NULL == mUrl)  /* for tts mode*/
    {
        mContext = pfnavio_alloc_context(mAvioBuffer, AVIO_BUFFER_SIZE, 0, this, read_buffer, NULL, NULL);
        //mContext = pfnavio_alloc_context(mAvioBuffer, AVIO_BUFFER_SIZE, 0, NULL, NULL, NULL, NULL);

        if(ret < 0)
        {
            SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Failed to Open pen url:%s, ret:%d!", mUrl , ret));
            return UNKNOWN_ERROR;
        }
    }
    else /* for url mode*/
    {
        AVIOInterruptCB cb = {
            .callback = streamer_interrupt_cb,
            .opaque = this,
        };

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

        ret = pfnavio_open2(&mContext, mUrl, AVIO_FLAG_READ, &cb, &opt);
        if(ret < 0)
        {
            SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Failed to Open url:%s, ret:%s!", mUrl , av_error2str(ret)));
            goto fail;
        }
        size = pfnavio_size(mContext);
        if(size < 0)
        {
            SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Failed to get avio size, the size is :%d! ", size));
            goto fail;
        }

        mFileSize = size > 0 ? size : 0;
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("File Size is %lld!", mFileSize));
    }

    SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("OpenAvio End Success!"));
    if (opt != NULL)
        pfnav_dict_free(&opt);
    return OK;
fail:
    if (opt != NULL)
        pfnav_dict_free(&opt);
    return UNKNOWN_ERROR;
}

status_t FFStreamer::CloseAvio()
{
    status_t ret;

    SAP_LOGENTRY(SAP_STREAMER_TAG,SAP_LOG_INFO,("CloseAvio"));
    if(NULL != mContext)
    {
        if(NULL == mUrl)
        {
            if (NULL != mContext->buffer)
            {
                pfnav_freep(&mContext->buffer);
            }
            pfnav_freep(&mContext);
            mAvioBuffer = NULL;
#if 0
            PP_DBG_MSG("Begin free mAvioBuffer!\n");
            if (mAvioBuffer) pfnav_freep(mAvioBuffer);
            mAvioBuffer = NULL;
            pfnav_freep(mContext);
#endif
        }
        else
        {
            ret = pfnavio_close(mContext);
            if(ret < 0)
            {
                SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Failed to close Avio!"));
                return UNKNOWN_ERROR;
            }
        }
        mContext = NULL;
    }
    else
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Avio has been closed!"));
    }

    SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("CloseAvio doing"));

    if(NULL != mUrl)
    {
        free(mUrl);
        mUrl = NULL;
    }

    if(NULL != mRingBuf)
    {
        delete mRingBuf;
        mRingBuf = NULL;
    }

    return OK;
}

int FFStreamer::Read(void *opaque, unsigned char *data, int size)
{
    return mRingBuf->Read((void *)data, size);
}

unsigned int FFStreamer::GetAvailableSize()
{
    if (true == getStatus()) {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("streamer stopped"));
        return INVALID_OPERATION;
    }

    return mRingBuf->GetAvailableSize();
}

int FFStreamer::Write(void *opaque, unsigned char *data, int size)
{
    if (true == getStatus()) {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("streamer stopped"));
        return INVALID_OPERATION;
    }

    Mutex::Autolock autoLock(mLock);
    int ret = mRingBuf->Write((void *)data, size);
    mCondition.signal();
    return ret;
}

unsigned long long int FFStreamer::GetFileSize()
{
    return mFileSize;
}

AVIOContext *FFStreamer::GetAVIOCtx()
{
    return mContext;
}

void FFStreamer::Disconnect() {
    Mutex::Autolock autoLock(mLock);
    mStop = true;
    mCondition.signal();
}

bool FFStreamer::getStatus() {
    return mStop;
}

void FFStreamer::signalEOS()
{
    Mutex::Autolock autoLock(mLock);
    if (true == mEOSResult) {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("already EOS"));
        return;
    }
    mEOSResult = true;
    mCondition.signal();
}
const static nsecs_t kAcquireWaitTimeout = 2000000000; // 2 seconds

int FFStreamer::readBuffer(unsigned char *buf, int buf_size) {

    Mutex::Autolock autoLock(mLock);
    int ret = 0;
    while ((ret=Read(NULL, buf, buf_size))==0 && mEOSResult==false &&
            getStatus()==false) {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("waiting for input stream, mTtsPause : %d!", mTtsPause));
        if(mTtsPause){
            mCondition.wait(mLock);
        }
        else{
            if (NO_ERROR != mCondition.waitRelative(mLock, kAcquireWaitTimeout)) {
                SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Timed out waiting for input stream"));
                return AVERROR(EIO);
            }
       }
    }
    SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("read_buffer,read size %d, return size %d, eos %d status %d",
        buf_size, ret, mEOSResult, getStatus()));
    if (true == getStatus())
        return AVERROR_EXIT;
    if (ret > 0)
        return ret;
    else if (mEOSResult)
        return AVERROR_EOF;
}

int FFStreamer::read_buffer(void *opaque, unsigned char *buf, int buf_size)
{
    return ((FFStreamer *)opaque)->readBuffer(buf, buf_size);
}

// pause and resume only for tts
void FFStreamer::pause()
{
    Mutex::Autolock autoLock(mLock);
    mTtsPause = true;
    mCondition.signal();
}

void FFStreamer::resume()
{
    Mutex::Autolock autoLock(mLock);
    mTtsPause = false;
    mCondition.signal();
}


/* private member function */



/* RingBuf */

#define RING_BUFFER_SIZE 512 * 1024

/* public member function */
RingBuf::RingBuf()
{
    //malloc mSize + 1:ringbuf must reserve 1 byte to calc space
    mSize = RING_BUFFER_SIZE;
    mBuf = calloc(mSize + 1, sizeof(signed char));
    if (NULL == mBuf)
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Malloc RingBuf Failed! size:%d", mSize));
    }
    mSize = mSize + 1;
    mWrite = 0;
    mRead = 0;

    SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_INFO,("mSize(%d) mWrite(%d) mRead(%d)\n", mSize, mWrite, mRead));
}



RingBuf::~RingBuf()
{
    Clear();
    if (NULL != mBuf)
    {
        free(mBuf);
        mBuf = NULL;
    }
}

void RingBuf::Clear()
{
    memset(mBuf, 0, mSize);
    mRead = 0;
    mWrite = 0;
}



int RingBuf::Write(void *pdata, unsigned int ui4Size)
{
    unsigned int ui4_available_size;

    if (NULL == pdata)
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Arg pdata is not valid"));
        return 0;
    }

    if (0 == ui4Size)
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Size is 0!"));
        return ui4Size;
    }


    ui4_available_size = GetAvailableSize();
    if (ui4Size > ui4_available_size)
    {
        //PP_STREAMER_DBG_INFO("write size(%d) is bigger than available size(%d)\n", ui4Size, ui4_available_size);
        ui4Size = ui4_available_size;
    }

    if (mSize >= (mWrite + ui4Size))
    {
        WriteData(pdata, ui4Size);
    }
    else
    {
        WriteDataLoopback(pdata, ui4Size);
    }

    return ui4Size;
}

int RingBuf::Read(void *pdata, unsigned int ui4Size)
{
    unsigned int ui4_data_size;

    if (NULL == pdata)
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Arg pdata is not valid"));
        return 0;
    }

    if (0 == ui4Size)
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("Size is 0!"));
        return ui4Size;
    }

    ui4_data_size = GetUsedSize();
    if (ui4Size > ui4_data_size)
    {
        SAP_LOG(SAP_STREAMER_TAG,SAP_LOG_ERROR,("read size(%d) is bigger than data size(%d)", ui4Size, ui4_data_size));
        ui4Size = ui4_data_size;
    }

    if (mSize >= (mRead + ui4Size))
    {
        ReadData(pdata, ui4Size);
    }
    else
    {
        ReadDataLoopback(pdata, ui4Size);
    }

    return ui4Size;
}


unsigned int RingBuf::GetAvailableSize()
{
    unsigned int ui4_write = mWrite;
    unsigned int ui4_read  = mRead;
    unsigned int ui4_size  = mSize;

    if (ui4_write >= ui4_read)
    {
        return ui4_size - (ui4_write - ui4_read) - 1;
    }
    else
    {
        return ui4_read - ui4_write - 1;
    }
}
/* private member function */

void RingBuf::WriteData(void *pdata, unsigned int ui4Size)
{
    memcpy(mBuf + mWrite, pdata, ui4Size);
    mWrite += ui4Size;
    if (mWrite == mSize)
    {
        mWrite = 0;
    }
}

void RingBuf::WriteDataLoopback(void *pdata, unsigned int ui4Size)
{
    unsigned int ui4_first_size = mSize - mWrite;
    unsigned int ui4_second_size = ui4Size - ui4_first_size;

    memcpy(mBuf + mWrite, pdata, ui4_first_size);
    memcpy(mBuf, pdata + ui4_first_size, ui4_second_size);
    mWrite = ui4_second_size;
}

void RingBuf::ReadData(void *pdata, unsigned int ui4Size)
{
    memcpy(pdata, mBuf + mRead, ui4Size);
    mRead += ui4Size;
    if (mRead == mSize)
    {
        mRead = 0;
    }
}

void RingBuf::ReadDataLoopback(void *pdata, unsigned int ui4Size)
{
    unsigned int ui4_first_size = mSize - mRead;
    unsigned int ui4_second_size = ui4Size - ui4_first_size;

    memcpy(pdata, mBuf + mRead, ui4_first_size);
    memcpy(pdata + ui4_first_size, mBuf, ui4_second_size);
    mRead = ui4_second_size;
}

unsigned int RingBuf::GetUsedSize()
{
    return mSize - 1 - GetAvailableSize();
}

#endif

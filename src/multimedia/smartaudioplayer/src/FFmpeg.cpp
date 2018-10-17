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

#include <dlfcn.h>
#include "FFmpeg.h"
#include "Log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>
#include "sched.h"
#include <syslog.h>

extern bool is_lib64;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void log_callback(void *ptr, int level, const char *fmt, va_list vl) {
    pthread_mutex_lock(&mutex);

    va_list vl2;
    char line[1024];
    static int print_prefix = 1;
    va_copy(vl2, vl);
    pfnav_log_format_line(ptr, level, fmt, vl2, line, sizeof(line), &print_prefix);

    if (access("/data/log_all",0) == 0) {
        syslog(LOG_WARNING, "%s %s", "<SAPlayer-FFMPEG>", line);
        fprintf(stdout, "%s %s", "<SAPlayer-FFMPEG>\n", line);
    }
    else {
        syslog(LOG_WARNING, "%s %s", "<SAPlayer-FFMPEG>", line);
    }
    va_end(vl2);
    pthread_mutex_unlock(&mutex);
}

void *libAvformatEntryhandle;
void *libAvcodecEntryhandle;
void *libAvutiltEntryhandle;
void *libAvswresampleEntryhandle;

void (*pfnav_register_all)(void);
int (*pfnavformat_network_init)(void);
int (*pfnavformat_network_deinit)(void);
AVFormatContext *(*pfnavformat_alloc_context)(void);
int (*pfnavformat_open_input)(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options);
int (*pfnavformat_find_stream_info)(AVFormatContext *ic, AVDictionary **options);
void (*pfnav_dump_format)(AVFormatContext *ic, int index, const char *url, int is_output);
AVCodec *(*pfnavcodec_find_decoder)(enum AVCodecID id);
AVCodecContext *(*pfnavcodec_alloc_context3)(const AVCodec *codec);
int (*pfnavcodec_parameters_to_context)(AVCodecContext *codec, const AVCodecParameters *par);
void (*pfnav_codec_set_pkt_timebase)(AVCodecContext *avctx, AVRational val);
int  (*pfnavcodec_open2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
int (*pfnav_get_channel_layout_nb_channels)(uint64_t channel_layout);
int (*pfnav_samples_get_buffer_size)(int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align);
void *(*pfnav_malloc)(size_t size);
AVFrame *(*pfnav_frame_alloc)(void);
int64_t (*pfnav_get_default_channel_layout)(int nb_channels);
av_cold struct SwrContext *(*pfnswr_alloc)(void);
struct SwrContext *(*pfnswr_alloc_set_opts)(struct SwrContext *s,
                                      int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
                                      int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate,
                                      int log_offset, void *log_ctx);
av_cold int (*pfnswr_init)(struct SwrContext *s);
int (*pfnav_read_frame)(AVFormatContext *s, AVPacket *pkt);
int (*pfnavformat_flush)(AVFormatContext *s);
int  (*pfnavcodec_send_packet)(AVCodecContext *avctx, const AVPacket *avpkt);
void (*pfnav_packet_unref)(AVPacket *pkt);
int (*pfnav_packet_ref)(AVPacket *dst, const AVPacket *src);
void (*pfnav_init_packet)(AVPacket *pkt);
void (*pfnav_packet_free)(AVPacket **pkt);
AVPacket* (*pfnav_packet_alloc)(void);
int  (*pfnavcodec_receive_frame)(AVCodecContext *avctx, AVFrame *frame);
int  (*pfnswr_convert)(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in , int in_count);
av_cold void (*pfnswr_free)(SwrContext **ss);
void *(*pfnav_free)(void *ptr);
void *(*pfnav_freep)(void *ptr);
av_cold int (*pfnavcodec_close)(AVCodecContext *avctx);
void (*pfnavformat_close_input)(AVFormatContext **ps);
void (*pfnav_frame_free)(AVFrame **frame);
int (*pfnavio_close)(AVIOContext *s);
void (*pfnavformat_free_context)(AVFormatContext *s);
AVIOContext *(*pfnavio_alloc_context)(
                      unsigned char *buffer,
                      int buffer_size,
                      int write_flag,
                      void *opaque,
                      int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                      int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                      int64_t (*seek)(void *opaque, int64_t offset, int whence));
int (*pfnav_strerror)(int errnum, char *errbuf, size_t errbuf_size);
unsigned (*pfnavformat_version)(void);
int (*pfnav_opt_set_channel_layout)(void *obj, const char *name, int64_t cl, int search_flags);
int (*pfnav_opt_set_int)(void *obj, const char *name, int64_t val, int search_flags);
int (*pfnav_opt_set_sample_fmt)(void *obj, const char *name, enum AVSampleFormat fmt, int search_flag);
int64_t (*pfnav_rescale_rnd)(int64_t a, int64_t b, int64_t c, enum AVRounding rnd);
int64_t (*pfnswr_get_delay)(struct SwrContext *s, int64_t base);
int (*pfnav_get_bytes_per_sample)(enum AVSampleFormat sample_fmt);

int (*pfnavio_open2)(AVIOContext **s, const char *url, int flags,
               const AVIOInterruptCB *int_cb, AVDictionary **options);
int64_t (*pfnavio_size)(AVIOContext *s);
int (*pfnavio_read)(AVIOContext *s, unsigned char *buf, int size);
int (*pfnavio_write)(AVIOContext *s, unsigned char *buf, int size);
void (*pfnavio_flush)(AVIOContext *s);

int64_t (*pfnavio_seek)(AVIOContext *s, int64_t offset, int whence);
int (*pfnav_seek_frame)(AVFormatContext *s, int stream_index,int64_t timestamp, int
flags);

int (*pfnav_thread_message_queue_alloc)(AVThreadMessageQueue **mq,
                                  unsigned nelem,
                                  unsigned elsize);
void (*pfnav_thread_message_queue_free)(AVThreadMessageQueue **mq);
int (*pfnav_thread_message_queue_send)(AVThreadMessageQueue *mq,
                                 void *msg,
                                 unsigned flags);
int (*pfnav_thread_message_queue_recv)(AVThreadMessageQueue *mq,
                                 void *msg,
                                 unsigned flags);
void (*pfnav_thread_message_queue_set_err_send)(AVThreadMessageQueue *mq,
                                          int err);
void (*pfnav_thread_message_queue_set_err_recv)(AVThreadMessageQueue *mq,
                                          int err);
void (*pfnav_thread_message_queue_set_free_func)(AVThreadMessageQueue *mq,
                                           void (*free_func)(void *msg));
void (*pfnav_thread_message_flush)(AVThreadMessageQueue *mq);

int (*pfnavio_feof)(AVIOContext *s);

void (*pfnav_log_set_level)(int level);
int (*pfnav_log_get_level)(void);
void (*pfnav_log_set_callback)(void (*callback)(void*, int, const char*, va_list));
void (*pfnav_log_format_line)(void *ptr, int level, const char *fmt, va_list vl,
                        char *line, int line_size, int *print_prefix);

int (*pfnav_probe_input_buffer)(AVIOContext *pb, AVInputFormat **fmt,
                          const char *filename, void *logctx,
                          unsigned int offset, unsigned int max_probe_size);

void (*pfnavcodec_flush_buffers)(AVCodecContext *avctx);

int (*pfnav_dict_set)(AVDictionary **pm, const char *key, const char *value, int flags);
void (*pfnav_dict_free)(AVDictionary **m);

void (*pfnavcodec_free_context)(AVCodecContext **avctx);

int (*pfnavformat_alloc_output_context2)(AVFormatContext **ctx, AVOutputFormat *oformat,
                                   const char *format_name, const char *filename);
int (*pfnav_copy_packet)(AVPacket *dst, const AVPacket *src);
int (*pfnav_interleaved_write_frame)(AVFormatContext *s, AVPacket *pkt);
int (*pfnav_write_trailer)(AVFormatContext *s);
AVStream *(*pfnavformat_new_stream)(AVFormatContext *s, const AVCodec *c);
int (*pfnavcodec_copy_context)(AVCodecContext *dest, const AVCodecContext *src);
int (*pfnavformat_write_header)(AVFormatContext *s, AVDictionary **options);
int (*pfnavio_open)(AVIOContext **s, const char *url, int flags);
int (*pfnavformat_seek_file)(AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags);

int initffmpeg(void)
{
    if (is_lib64)
    {
        libAvformatEntryhandle = dlopen("/usr/lib64/libavformat.so.57",RTLD_NOW);
        if (NULL == libAvformatEntryhandle)
        {
            SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("fail to load library libavformat.so."));
            return -1;
        }
        libAvcodecEntryhandle = dlopen("/usr/lib64/libavcodec.so.57",RTLD_NOW);
        if (NULL == libAvcodecEntryhandle)
        {
            SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("fail to load library libavcodec.so."));
            return -1;
        }
        libAvutiltEntryhandle = dlopen("/usr/lib64/libavutil.so.55",RTLD_NOW);
        if (NULL == libAvutiltEntryhandle)
        {
            SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("fail to load library libavutil.so."));
            return -1;
        }
        libAvswresampleEntryhandle = dlopen("/usr/lib64/libswresample.so.2",RTLD_NOW);
        if (NULL == libAvswresampleEntryhandle)
        {
            SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("fail to load library libswresample.so."));
            return -1;
        }
    } 
    else
    {
        libAvformatEntryhandle = dlopen("/usr/lib/libavformat.so.57",RTLD_NOW);
        if (NULL == libAvformatEntryhandle)
        {
            SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("fail to load library libavformat.so."));
            return -1;
        }
        libAvcodecEntryhandle = dlopen("/usr/lib/libavcodec.so.57",RTLD_NOW);
        if (NULL == libAvcodecEntryhandle)
        {
            SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("fail to load library libavcodec.so."));
            return -1;
        }
        libAvutiltEntryhandle = dlopen("/usr/lib/libavutil.so.55",RTLD_NOW);
        if (NULL == libAvutiltEntryhandle)
        {
            SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("fail to load library libavutil.so."));
            return -1;
        }
        libAvswresampleEntryhandle = dlopen("/usr/lib/libswresample.so.2",RTLD_NOW);
        if (NULL == libAvswresampleEntryhandle)
        {
            SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("fail to load library libswresample.so."));
            return -1;
        }
    }
    SAP_LOG(SAP_TAG,SAP_LOG_INFO,("Loading ffmpeg library success."));

    #define MAP_SYM(fn, handle) \
         do \
         { \
                  *(void **) (&pfn##fn) = dlsym(handle,#fn); \
                   if(pfn##fn == 0) \
                   { \
                           SAP_LOG(SAP_TAG,SAP_LOG_ERROR,("Can't get mapping for method:%s in shared library\n",#fn)); \
                           return -1; \
                   } \
         }while(0) \

    MAP_SYM(av_register_all, libAvformatEntryhandle);
    MAP_SYM(avformat_network_init, libAvformatEntryhandle);
    MAP_SYM(avformat_network_deinit, libAvformatEntryhandle);
    MAP_SYM(avformat_alloc_context, libAvformatEntryhandle);
    MAP_SYM(avformat_open_input, libAvformatEntryhandle);
    MAP_SYM(avformat_find_stream_info, libAvformatEntryhandle);
    MAP_SYM(av_dump_format, libAvformatEntryhandle);
    MAP_SYM(av_read_frame, libAvformatEntryhandle);
    MAP_SYM(avformat_flush, libAvformatEntryhandle);
    MAP_SYM(avformat_close_input, libAvformatEntryhandle);
    MAP_SYM(avformat_free_context, libAvformatEntryhandle);
    MAP_SYM(avio_alloc_context, libAvformatEntryhandle);
    MAP_SYM(avformat_version, libAvformatEntryhandle);
    MAP_SYM(avio_open2, libAvformatEntryhandle);
    MAP_SYM(avio_size, libAvformatEntryhandle);
    MAP_SYM(avio_close, libAvformatEntryhandle);
    MAP_SYM(avio_read, libAvformatEntryhandle);
    MAP_SYM(avio_write, libAvformatEntryhandle);
    MAP_SYM(avio_seek, libAvformatEntryhandle);
    MAP_SYM(avio_flush, libAvformatEntryhandle);
    MAP_SYM(avio_feof, libAvformatEntryhandle);
    MAP_SYM(av_seek_frame, libAvformatEntryhandle);
    MAP_SYM(av_probe_input_buffer, libAvformatEntryhandle);

    MAP_SYM(avcodec_find_decoder, libAvcodecEntryhandle);
    MAP_SYM(avcodec_alloc_context3, libAvcodecEntryhandle);
    MAP_SYM(avcodec_parameters_to_context, libAvcodecEntryhandle);
    MAP_SYM(av_codec_set_pkt_timebase, libAvcodecEntryhandle);
    MAP_SYM(avcodec_open2, libAvcodecEntryhandle);
    MAP_SYM(avcodec_send_packet, libAvcodecEntryhandle);
    MAP_SYM(av_packet_unref, libAvcodecEntryhandle);
    MAP_SYM(av_packet_ref, libAvcodecEntryhandle);
    MAP_SYM(av_init_packet, libAvcodecEntryhandle);
    MAP_SYM(av_packet_free, libAvcodecEntryhandle);
    MAP_SYM(av_packet_alloc, libAvcodecEntryhandle);
    MAP_SYM(avcodec_receive_frame, libAvcodecEntryhandle);
    MAP_SYM(avcodec_close, libAvcodecEntryhandle);
    MAP_SYM(avcodec_flush_buffers, libAvcodecEntryhandle);
    MAP_SYM(avcodec_free_context, libAvcodecEntryhandle);

    MAP_SYM(av_get_channel_layout_nb_channels, libAvutiltEntryhandle);
    MAP_SYM(av_samples_get_buffer_size, libAvutiltEntryhandle);
    MAP_SYM(av_malloc, libAvutiltEntryhandle);
    MAP_SYM(av_frame_alloc, libAvutiltEntryhandle);
    MAP_SYM(av_get_default_channel_layout, libAvutiltEntryhandle);
    MAP_SYM(av_free, libAvutiltEntryhandle);
    MAP_SYM(av_freep, libAvutiltEntryhandle);
    MAP_SYM(av_strerror, libAvutiltEntryhandle);
    MAP_SYM(av_opt_set_channel_layout, libAvutiltEntryhandle);
    MAP_SYM(av_opt_set_int, libAvutiltEntryhandle);
    MAP_SYM(av_opt_set_sample_fmt, libAvutiltEntryhandle);
    MAP_SYM(av_samples_get_buffer_size, libAvutiltEntryhandle);
    MAP_SYM(av_get_bytes_per_sample, libAvutiltEntryhandle);
    MAP_SYM(av_log_set_level, libAvutiltEntryhandle);
    MAP_SYM(av_log_get_level, libAvutiltEntryhandle);
    MAP_SYM(av_log_set_callback, libAvutiltEntryhandle);
    MAP_SYM(av_log_format_line, libAvutiltEntryhandle);
    MAP_SYM(av_dict_set, libAvutiltEntryhandle);
    MAP_SYM(av_dict_free, libAvutiltEntryhandle);
    MAP_SYM(av_rescale_rnd, libAvutiltEntryhandle);

    MAP_SYM(swr_alloc, libAvswresampleEntryhandle);
    MAP_SYM(swr_alloc_set_opts, libAvswresampleEntryhandle);
    MAP_SYM(swr_init, libAvswresampleEntryhandle);
    MAP_SYM(swr_convert, libAvswresampleEntryhandle);
    MAP_SYM(av_frame_free, libAvswresampleEntryhandle);
    MAP_SYM(swr_free, libAvswresampleEntryhandle);
    MAP_SYM(swr_get_delay, libAvswresampleEntryhandle);

    MAP_SYM(avformat_alloc_output_context2, libAvformatEntryhandle);
    MAP_SYM(av_copy_packet, libAvcodecEntryhandle);
    MAP_SYM(av_interleaved_write_frame, libAvformatEntryhandle);
    MAP_SYM(av_write_trailer, libAvformatEntryhandle);
    MAP_SYM(avformat_write_header, libAvformatEntryhandle);
    MAP_SYM(avformat_new_stream, libAvformatEntryhandle);
    MAP_SYM(avcodec_copy_context, libAvcodecEntryhandle);
    MAP_SYM(avio_open, libAvformatEntryhandle);
    MAP_SYM(avformat_seek_file, libAvformatEntryhandle);


    pfnav_register_all();
    pfnavformat_network_init();

    SAP_LOG(SAP_TAG,SAP_LOG_INFO,("ffmpeg log level %d", pfnav_log_get_level()));

    pfnav_log_set_callback(log_callback);
    return 0;
}

void exitffmpeg(void)
{
   pfnavformat_network_deinit();
   dlclose(libAvformatEntryhandle);
   dlclose(libAvcodecEntryhandle);
   dlclose(libAvutiltEntryhandle);
   dlclose(libAvswresampleEntryhandle);
   libAvformatEntryhandle = NULL;
   libAvcodecEntryhandle = NULL;
   libAvutiltEntryhandle = NULL;
   libAvswresampleEntryhandle = NULL;
   SAP_LOG(SAP_TAG,SAP_LOG_INFO,("dlclose ffmpeg library success."));
}

static char errorbuffer[AV_ERROR_MAX_STRING_SIZE+1];

char *av_error2str(int errnum) {
    if (errorbuffer) {
        pfnav_strerror(errnum, errorbuffer, AV_ERROR_MAX_STRING_SIZE);
        errorbuffer[AV_ERROR_MAX_STRING_SIZE]='\0';
        return errorbuffer;
    }
    else {
        return NULL;
    }
}
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

pid_t gettid(void) {

    return syscall(__NR_gettid);
}

#endif

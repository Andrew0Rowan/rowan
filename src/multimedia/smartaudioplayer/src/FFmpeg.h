#ifndef __FFMPEG_H__
#define __FFMPEG_H__


#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/fifo.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/dict.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/avstring.h"
#include "libavutil/imgutils.h"
#include "libavutil/timestamp.h"
#include "libavutil/bprint.h"
#include "libavutil/time.h"
#include "libavutil/threadmessage.h"
#include "libavutil/error.h"

extern void *libAvformatEntryhandle;
extern void *libAvcodecEntryhandle;
extern void *libAvutiltEntryhandle;
extern void *libAvswresampleEntryhandle;

extern void (*pfnav_register_all)(void);
extern int (*pfnavformat_network_init)(void);
extern int (*pfnavformat_network_deinit)(void);
extern AVFormatContext *(*pfnavformat_alloc_context)(void);
extern int (*pfnavformat_open_input)(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options);
extern int (*pfnavformat_find_stream_info)(AVFormatContext *ic, AVDictionary **options);
extern void (*pfnav_dump_format)(AVFormatContext *ic, int index, const char *url, int is_output);
extern AVCodec *(*pfnavcodec_find_decoder)(enum AVCodecID id);
extern AVCodecContext *(*pfnavcodec_alloc_context3)(const AVCodec *codec);
extern int (*pfnavcodec_parameters_to_context)(AVCodecContext *codec, const AVCodecParameters *par);
extern void (*pfnav_codec_set_pkt_timebase)(AVCodecContext *avctx, AVRational val);
extern int  (*pfnavcodec_open2)(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
extern int (*pfnav_get_channel_layout_nb_channels)(uint64_t channel_layout);
extern int (*pfnav_samples_get_buffer_size)(int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align);
extern void *(*pfnav_malloc)(size_t size);
extern AVFrame *(*pfnav_frame_alloc)(void);
extern int64_t (*pfnav_get_default_channel_layout)(int nb_channels);
extern av_cold struct SwrContext *(*pfnswr_alloc)(void);
extern struct SwrContext *(*pfnswr_alloc_set_opts)(struct SwrContext *s,
        int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
        int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate,
        int log_offset, void *log_ctx);
extern av_cold int (*pfnswr_init)(struct SwrContext *s);
extern int (*pfnav_read_frame)(AVFormatContext *s, AVPacket *pkt);
extern int (*pfnavformat_flush)(AVFormatContext *s);
extern int  (*pfnavcodec_send_packet)(AVCodecContext *avctx, const AVPacket *avpkt);
extern void (*pfnav_packet_unref)(AVPacket *pkt);
extern int (*pfnav_packet_ref)(AVPacket *dst, const AVPacket *src);
extern void (*pfnav_init_packet)(AVPacket *pkt);
extern void (*pfnav_packet_free)(AVPacket **pkt);
extern AVPacket* (*pfnav_packet_alloc)(void);
extern int  (*pfnavcodec_receive_frame)(AVCodecContext *avctx, AVFrame *frame);
extern int  (*pfnswr_convert)(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in , int in_count);
extern av_cold void (*pfnswr_free)(SwrContext **ss);
extern void *(*pfnav_free)(void *ptr);
extern void *(*pfnav_freep)(void *ptr);
extern av_cold int (*pfnavcodec_close)(AVCodecContext *avctx);
extern void (*pfnavformat_close_input)(AVFormatContext **ps);
extern void (*pfnav_frame_free)(AVFrame **frame);
extern int (*pfnavio_close)(AVIOContext *s);
extern void (*pfnavformat_free_context)(AVFormatContext *s);
extern AVIOContext *(*pfnavio_alloc_context)(
		unsigned char *buffer,
		int buffer_size,
		int write_flag,
		void *opaque,
		int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
		int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
		int64_t (*seek)(void *opaque, int64_t offset, int whence));
extern int (*pfnav_strerror)(int errnum, char *errbuf, size_t errbuf_size);
extern unsigned (*pfnavformat_version)(void);
extern int (*pfnav_opt_set_channel_layout)(void *obj, const char *name, int64_t cl, int search_flags);
extern int (*pfnav_opt_set_int)(void *obj, const char *name, int64_t val, int search_flags);
extern int (*pfnav_opt_set_sample_fmt)(void *obj, const char *name, enum AVSampleFormat fmt, int search_flag);
extern int64_t (*pfnav_rescale_rnd)(int64_t a, int64_t b, int64_t c, enum AVRounding rnd);
extern int64_t (*pfnswr_get_delay)(struct SwrContext *s, int64_t base);
extern int (*pfnav_samples_get_buffer_size)(int *linesize, int nb_channels, int nb_samples,
                               enum AVSampleFormat sample_fmt, int align);
extern int (*pfnav_get_bytes_per_sample)(enum AVSampleFormat sample_fmt);

extern int initffmpeg(void);
extern void exitffmpeg(void);

extern int (*pfnavio_open2)(AVIOContext **s, const char *url, int flags,
               const AVIOInterruptCB *int_cb, AVDictionary **options);
extern int64_t (*pfnavio_size)(AVIOContext *s);
extern int (*pfnavio_read)(AVIOContext *s, unsigned char *buf, int size);
extern int (*pfnavio_write)(AVIOContext *s, unsigned char *buf, int size);
extern int64_t (*pfnavio_seek)(AVIOContext *s, int64_t offset, int whence);
extern void (*pfnavio_flush)(AVIOContext *s);
extern int (*pfnav_seek_frame)(AVFormatContext *s, int stream_index,int64_t timestamp, int flags);

extern int (*pfnavio_feof)(AVIOContext *s);

extern int (*pfnav_log_get_level)(void);

extern void (*pfnav_log_set_level)(int level);

extern void (*pfnav_log_set_callback)(void (*callback)(void*, int, const char*, va_list));

extern void (*pfnav_log_format_line)(void *ptr, int level, const char *fmt, va_list vl,
                        char *line, int line_size, int *print_prefix);

extern int (*pfnav_probe_input_buffer)(AVIOContext *pb, AVInputFormat **fmt,
                          const char *filename, void *logctx,
                          unsigned int offset, unsigned int max_probe_size);

extern void (*pfnavcodec_flush_buffers)(AVCodecContext *avctx);

extern int (*pfnav_dict_set)(AVDictionary **pm, const char *key, const char *value, int flags);
extern void (*pfnav_dict_free)(AVDictionary **m);

extern void (*pfnavcodec_free_context)(AVCodecContext **avctx);

extern char *av_error2str(int errnum);

extern pid_t gettid(void);

extern int (*pfnavformat_alloc_output_context2)(AVFormatContext **ctx, AVOutputFormat *oformat,
                                   const char *format_name, const char *filename);
extern int (*pfnav_copy_packet)(AVPacket *dst, const AVPacket *src);

extern int (*pfnav_interleaved_write_frame)(AVFormatContext *s, AVPacket *pkt);
extern int (*pfnav_write_trailer)(AVFormatContext *s);
extern AVStream *(*pfnavformat_new_stream)(AVFormatContext *s, const AVCodec *c);
extern int (*pfnavcodec_copy_context)(AVCodecContext *dest, const AVCodecContext *src);
extern int (*pfnavformat_write_header)(AVFormatContext *s, AVDictionary **options);
extern int (*pfnavio_open)(AVIOContext **s, const char *url, int flags);
extern int (*pfnavformat_seek_file)(AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags);

#endif

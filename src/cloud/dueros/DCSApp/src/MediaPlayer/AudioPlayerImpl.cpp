/*
 * AudioPlayerImpl.cpp
 *
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <LoggerUtils/DcsSdkLogger.h>
#include "MediaPlayer/AudioPlayerImpl.h"

namespace duerOSDcsApp {
namespace mediaPlayer {
using application::ThreadPoolExecutor;

static const std::string TAG("AudioPlayerImpl");

#define RESAMPLE_OUTPUT_BUFFER_SIZE 192000
#define PCM_PLAYBUFFER_SIZE 48000
#define PLAYFINISH_DELAY_TIME_MS 400000L
#define PLAY_LOOP_DELAY_TIME_MS 30000L
#define PAUSE_CHECK_INTERVAL_MS 200000L
#define BUFFER_CHECK_INTERVAL_MS 300000L
#define BUFFER_REFILL_MIN_SIZE 15
#define OUTPUT_SAMPLE_RATE 48000
#define NEAR_FINISH_TIME_LEFT 8000
#define MAX_RETRY_OPEN_TIME 3
#define TIMEOUT_MS 15000
#define DECODE_FINISH_TIME_THRESHOLD_MS 15000
#define RETRY_DECODE_TIMEINTERVAL_MS 200000
#define RETRY_INTERVAL 30
#define RETRY_OPEN_TIMEINTERVAL_MS 100000
#define MAX_RETRY_DECODE_NUM 8
#define RETRY_DECODE_TIME_UNIT 300000

AudioPlayerImpl::AudioPlayerImpl(const std::string &audio_dev) : m_formatCtx(NULL),
                                                                 m_codecCtx(NULL),
                                                                 m_codecParam(NULL),
                                                                 m_pcmBuffer(NULL),
                                                                 m_frameBuffer(NULL),
                                                                 m_alsaController(NULL),
                                                                 m_listener(NULL),
                                                                 m_status(AUDIOPLAYER_STATUS_IDLE),
                                                                 m_decodeThread(0),
                                                                 m_playThread(0),
                                                                 m_progressMs(0),
                                                                 m_durationMs(0),
                                                                 m_bytesPersecond(0),
                                                                 m_reportIntervalMs(3000),
                                                                 m_startOffsetMs(0),
                                                                 m_frameTimestamp(0),
                                                                 m_outChannel(1),
                                                                 m_progressStartMs(0),
                                                                 m_sampleRate(0),
                                                                 m_bytesPersample(0),
                                                                 m_seekable(false),
                                                                 m_pauseFlag(false),
                                                                 m_stopFlag(false),
                                                                 m_finishFlag(false),
                                                                 m_firstPackFlag(true),
                                                                 m_nearlyFinishFlag(false),
                                                                 m_shouldBreakFlag(false),
                                                                 m_threadAlive(true) {
#ifdef MTK8516
    m_outChannel = 2;
#else
    m_outChannel = 1;
#endif

    PcmBufPool::getInstance()->setStreamListener(this);
    m_executor = ThreadPoolExecutor::getInstance();
    m_alsaController = new AlsaController(audio_dev);
    m_alsaController->init(48000, m_outChannel);
    pthread_mutex_init(&m_decodeMutex, NULL);
    pthread_mutex_init(&m_playMutex, NULL);
    pthread_mutex_init(&m_alsaMutex, NULL);
    pthread_cond_init(&m_decodeCond, NULL);
    pthread_cond_init(&m_playCond, NULL);
    av_log_set_callback(ffmpegLogOutput);
    av_log_set_level(AV_LOG_INFO);
    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    m_pcmBuffer = (uint8_t *) av_malloc(PCM_PLAYBUFFER_SIZE);
    m_frameBuffer = (uint8_t *) av_malloc(RESAMPLE_OUTPUT_BUFFER_SIZE);
    pthread_create(&m_decodeThread, NULL, decodeFunc, this);
    pthread_create(&m_playThread, NULL, playFunc, this);
}

AudioPlayerImpl::~AudioPlayerImpl() {
    m_threadAlive = false;
    m_stopFlag = true;

    pthread_cond_signal(&m_playCond);
    pthread_cond_signal(&m_decodeCond);

    void *play_thread_return = NULL;
    if (m_playThread != 0) {
        pthread_join(m_playThread, &play_thread_return);
    }
    void *decode_thread_return = NULL;
    if (m_decodeThread != 0) {
        pthread_join(m_decodeThread, &decode_thread_return);
    }

    if (m_pcmBuffer != NULL) {
        av_free(m_pcmBuffer);
        m_pcmBuffer = NULL;
    }
    if (m_frameBuffer != NULL) {
        av_free(m_frameBuffer);
        m_frameBuffer = NULL;
    }

    m_alsaController->aslaAbort();
    m_alsaController->alsaClose();
    if (m_alsaController) {
        delete m_alsaController;
        m_alsaController = NULL;
    }

    pthread_mutex_destroy(&m_alsaMutex);
    pthread_mutex_destroy(&m_decodeMutex);
    pthread_mutex_destroy(&m_playMutex);
    pthread_cond_destroy(&m_decodeCond);
    pthread_cond_destroy(&m_playCond);
    m_status = AUDIOPLAYER_STATUS_IDLE;
}

void AudioPlayerImpl::registerListener(AudioPlayerListener *notify) {
    m_listener = notify;
}

void *AudioPlayerImpl::decodeFunc(void *arg) {
    AudioPlayerImpl *player = (AudioPlayerImpl *) arg;
    while (player->m_threadAlive) {
        pthread_mutex_lock(&player->m_decodeMutex);
        while (player->m_url.empty()) {
            pthread_cond_wait(&player->m_decodeCond, &player->m_decodeMutex);
            if (!player->m_threadAlive) {
                return NULL;
            }
        }

        player->m_alsaController->alsaPrepare();
        PcmBufPool *pcm_pool = PcmBufPool::getInstance();

        //Open the url
        APP_INFO("==open input");
        int open_ret = 0;
        unsigned int l_try_open_count = 0;
        bool l_open_success_flag = false;
        while (l_try_open_count++ < MAX_RETRY_OPEN_TIME) {
            player->m_formatCtx = avformat_alloc_context();
            player->m_formatCtx->interrupt_callback.callback = interruptCallback;
            player->m_formatCtx->interrupt_callback.opaque = player;
            open_ret = avformat_open_input(&player->m_formatCtx,
                                           player->m_url.c_str(),
                                           NULL,
                                           NULL);
            if (open_ret == 0) {
                l_open_success_flag = true;
                break;
            } else {
                player->freeFormatContext();
            }
        }

        if (!l_open_success_flag) {
            player->freeFormatContext();
            player->m_url = "";
            pcm_pool->setEndFlag(true);
            pthread_mutex_unlock(&player->m_decodeMutex);
            player->executePlaybackError();
            continue;
        }

        if (avformat_find_stream_info(player->m_formatCtx, NULL) < 0) {
            player->freeFormatContext();
            player->m_url = "";
            pcm_pool->setEndFlag(true);
            pthread_mutex_unlock(&player->m_decodeMutex);
            player->executePlaybackError();
            continue;
        }
        av_dump_format(player->m_formatCtx, 0, player->m_url.c_str(), false);
        player->m_durationMs = (player->m_formatCtx->duration) / 1000;
        if (player->m_durationMs > 0) {
            player->m_seekable = true;
        } else {
            player->m_seekable = false;
        }

        int audio_stream = -1;
        for (unsigned int i = 0; i < player->m_formatCtx->nb_streams; i++) {
            if (player->m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                audio_stream = i;
                break;
            }
        }

        if (audio_stream == -1) {
            player->freeFormatContext();
            player->m_url = "";
            pcm_pool->setEndFlag(true);
            pthread_mutex_unlock(&player->m_decodeMutex);
            player->executePlaybackError();
            continue;
        }

        AVStream *a_stream = player->m_formatCtx->streams[audio_stream];
        player->m_codecParam = a_stream->codecpar;
        AVCodec *codec = avcodec_find_decoder(player->m_codecParam->codec_id);
        player->m_codecCtx = avcodec_alloc_context3(codec);
        int error_code = 0;
        if ((error_code = avcodec_parameters_to_context(player->m_codecCtx, player->m_codecParam)) < 0) {
            player->freeCodecContext();
            player->freeFormatContext();
            player->m_url = "";
            pcm_pool->setEndFlag(true);
            pthread_mutex_unlock(&player->m_decodeMutex);
            player->executePlaybackError();
            continue;
        }

        if (avcodec_open2(player->m_codecCtx, codec, NULL) < 0) {
            player->freeCodecContext();
            player->freeFormatContext();
            player->m_url = "";
            pcm_pool->setEndFlag(true);
            pthread_mutex_unlock(&player->m_decodeMutex);
            player->executePlaybackError();
            continue;
        }

        player->m_sampleRate = player->m_codecCtx->sample_rate;
        player->m_bytesPersample = av_get_bytes_per_sample(player->m_codecCtx->sample_fmt);
        player->m_bytesPersecond = OUTPUT_SAMPLE_RATE * player->m_outChannel * 2;

        unsigned int out_channels = player->m_outChannel;
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
        uint64_t out_channel_layout = AV_CH_LAYOUT_MONO;
        if (out_channels == 1) {
            out_channel_layout = AV_CH_LAYOUT_MONO;
        } else if (out_channels == 2) {
            out_channel_layout = AV_CH_LAYOUT_STEREO;
        } else {
        }

        int bytes_per_sample = av_get_bytes_per_sample(out_sample_fmt);
        int64_t in_channel_layout = av_get_default_channel_layout(player->m_codecCtx->channels);

        struct SwrContext *convert_ctx = swr_alloc();
        convert_ctx = swr_alloc_set_opts(convert_ctx,
                                         out_channel_layout,
                                         out_sample_fmt,
                                         OUTPUT_SAMPLE_RATE,
                                         in_channel_layout,
                                         player->m_codecCtx->sample_fmt,
                                         player->m_codecCtx->sample_rate,
                                         0,
                                         NULL);
        swr_init(convert_ctx);
        if (player->m_seekable && player->m_progressMs != 0) {
            //seek to position
            long int seek_pos = 0;
            //player->_progress_ms = player->_progress_ms > 0 ? player->_progress_ms : 3000;
            seek_pos = player->m_progressMs / 1000 * AV_TIME_BASE;
            if (player->m_formatCtx->start_time != AV_NOPTS_VALUE) {
                seek_pos += player->m_formatCtx->start_time;
            }
            if (av_seek_frame(player->m_formatCtx, -1, seek_pos, AVSEEK_FLAG_BACKWARD) < 0) {
                player->freeCodecContext();
                player->freeFormatContext();
                swr_free(&convert_ctx);
                player->m_url = "";
                pcm_pool->setEndFlag(true);
                pthread_mutex_unlock(&player->m_decodeMutex);
                player->executePlaybackError();
                continue;
            }
        }

        APP_INFO("==decode begin");
        int pos = 0;
        int readRetryCount = 0;
        AVPacket *packet = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();
        while (!player->m_stopFlag) {
            if (player->m_pauseFlag) {
                usleep(PAUSE_CHECK_INTERVAL_MS);
                continue;
            }

            player->m_frameTimestamp = TimerUtil::currentTimeMs();
            int ret = av_read_frame(player->m_formatCtx, packet);
            if (ret >= 0) {
                readRetryCount = 0;
                avcodec_send_packet(player->m_codecCtx, packet);
                error_code = avcodec_receive_frame(player->m_codecCtx, frame);
                if (error_code == 0) {
                    uint8_t *target_ptr = player->m_pcmBuffer + pos;
                    memset(player->m_frameBuffer, 0, RESAMPLE_OUTPUT_BUFFER_SIZE);
                    int len = swr_convert(convert_ctx,
                                          &player->m_frameBuffer,
                                          RESAMPLE_OUTPUT_BUFFER_SIZE,
                                          (const uint8_t **) frame->data,
                                          frame->nb_samples);
                    unsigned int resample_buffer_size = len * out_channels * bytes_per_sample;
                    if (pos + resample_buffer_size >= PCM_PLAYBUFFER_SIZE) {
                        int offset_ms = (int) (frame->pts * av_q2d(a_stream->time_base) * 1000);
                        pcm_pool->pushPcmChunk(player->m_pcmBuffer, pos, offset_ms);
                        memset(player->m_pcmBuffer, 0, PCM_PLAYBUFFER_SIZE);
                        pos = 0;
                        target_ptr = player->m_pcmBuffer;
                        memcpy(target_ptr, player->m_frameBuffer, resample_buffer_size);
                        pos += resample_buffer_size;
                    } else {
                        memcpy(target_ptr, player->m_frameBuffer, resample_buffer_size);
                        pos += resample_buffer_size;
                    }
                } else {
                    if (error_code == AVERROR_EOF) {
                        av_frame_unref(frame);
                        av_packet_unref(packet);
                        APP_INFO("==eof, decode finish.");
                        break;
                    }
                }
                av_frame_unref(frame);
                av_packet_unref(packet);
            } else {
                //if (ret == AVERROR_EOF || player->_should_break_flag) {
                if (ret == AVERROR_EOF) {
                    int timeDiff = player->m_durationMs - player->m_progressMs;
                    if (timeDiff >= DECODE_FINISH_TIME_THRESHOLD_MS) {
                        APP_INFO("==decode not finish(%d/%d), seek to target pos.",
                                         player->m_progressMs, player->m_durationMs);
                        unsigned int retryCount = 0;
                        bool retryRet = false;
                        while (retryCount++ < MAX_RETRY_DECODE_NUM) {
                            usleep((retryCount + 1) * RETRY_DECODE_TIME_UNIT);
                            retryRet = player->retryDecode();
                            if (retryRet) {
                                break;
                            }
                        }

                        if (retryRet) {
                            APP_INFO("==retry to decode success.");
                            continue;
                        } else {
                            APP_INFO("==retry to decode failed. decode finish.");
                            break;
                        }
                    }
                    APP_INFO("==decode finish.");
                    break;
                } else {
                    usleep(RETRY_DECODE_TIMEINTERVAL_MS);
                    if (readRetryCount >= RETRY_INTERVAL) {
                        APP_INFO("==retry to decode begin.");
                        while (!player->m_stopFlag && !player->retryDecode()) {
                            usleep(RETRY_OPEN_TIMEINTERVAL_MS);
                        }
                        readRetryCount = 0;
                        APP_INFO("==retry to decode end.");
                    }
                    readRetryCount++;
                    continue;
                }
            }
        }

        player->m_url = "";
        av_packet_free(&packet);
        av_frame_free(&frame);
        swr_free(&convert_ctx);
        player->freeCodecContext();
        player->freeFormatContext();

        pcm_pool->setEndFlag(true);
        pthread_mutex_unlock(&player->m_decodeMutex);
    }
    return NULL;
}

bool AudioPlayerImpl::retryDecode() {
    freeCodecContext();
    freeFormatContext();
    m_formatCtx = avformat_alloc_context();
    m_formatCtx->interrupt_callback.callback = interruptCallback;
    m_formatCtx->interrupt_callback.opaque = this;
    int open_ret = avformat_open_input(&m_formatCtx, m_url.c_str(), NULL, NULL);
    if (open_ret < 0) {
        freeFormatContext();
        return false;
    }

    if (avformat_find_stream_info(m_formatCtx, NULL) < 0) {
        freeFormatContext();
        return false;
    }
    av_dump_format(m_formatCtx, 0, m_url.c_str(), false);
    m_durationMs = (m_formatCtx->duration) / 1000;
    if (m_durationMs > 0) {
        m_seekable = true;
    } else {
        m_seekable = false;
    }

    int audio_stream = -1;
    for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream = i;
            break;
        }
    }

    if (audio_stream == -1) {
        freeFormatContext();
        return false;
    }

    AVStream *a_stream = m_formatCtx->streams[audio_stream];
    m_codecParam = a_stream->codecpar;
    AVCodec *codec = avcodec_find_decoder(m_codecParam->codec_id);
    m_codecCtx = avcodec_alloc_context3(codec);
    int error_code = 0;
    if ((error_code = avcodec_parameters_to_context(m_codecCtx, m_codecParam)) < 0) {
        freeCodecContext();
        freeFormatContext();
        return false;
    }

    if (avcodec_open2(m_codecCtx, codec, NULL) < 0) {
        freeCodecContext();
        freeFormatContext();
        return false;
    }

    m_sampleRate = m_codecCtx->sample_rate;
    m_bytesPersample = av_get_bytes_per_sample(m_codecCtx->sample_fmt);
    m_bytesPersecond = OUTPUT_SAMPLE_RATE * m_outChannel * 2;

    if (m_seekable && m_progressMs != 0) {
        long int seek_pos = m_progressMs / 1000 * AV_TIME_BASE;
        if (m_formatCtx->start_time != AV_NOPTS_VALUE) {
            seek_pos += m_formatCtx->start_time;
        }
        if (av_seek_frame(m_formatCtx, -1, seek_pos, AVSEEK_FLAG_BACKWARD) < 0) {
            freeCodecContext();
            freeFormatContext();
            return false;
        }
    }

    return true;
}

void AudioPlayerImpl::freeFormatContext() {
    if (m_formatCtx) {
        avformat_close_input(&m_formatCtx);
        avformat_free_context(m_formatCtx);
        m_formatCtx = NULL;
    }
}

void AudioPlayerImpl::freeCodecContext() {
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = NULL;
    }
}

void AudioPlayerImpl::handleHungryState() {
    PcmBufPool* pcm_pool = PcmBufPool::getInstance();
    if (pcm_pool->isDirty()) {
        APP_INFO("===audio player underbuffer");
        executePlaybackUnderbuffer(false);
        while (true) {
            usleep(BUFFER_CHECK_INTERVAL_MS);
            if (m_stopFlag || pcm_pool->getEndFlag()) {
                break;
            }
            if (pcm_pool->size() > BUFFER_REFILL_MIN_SIZE) {
                APP_INFO("===audio player bufferrefilled");
                executePlaybackBufferrefill();
                break;
            }
        }
    }
}

void* AudioPlayerImpl::playFunc(void *arg) {
    AudioPlayerImpl* player = (AudioPlayerImpl*)arg;
    PcmBufPool* pcm_pool = PcmBufPool::getInstance();
    bool shouldReportFinishEvent = false;
    while (player->m_threadAlive) {
        pthread_mutex_lock(&player->m_playMutex);
        while (player->m_url.empty()) {
            pthread_cond_wait(&player->m_playCond, &player->m_playMutex);
            if (!player->m_threadAlive) {
                return NULL;
            }
        }

        shouldReportFinishEvent = false;
        while (!player->m_stopFlag) {
            if (!player->m_pauseFlag && !player->m_finishFlag) {
                PcmChunk *chunk = pcm_pool->popPcmChunk();
                if (chunk != NULL && chunk->dat != NULL) {
                    player->pushStream(player->m_outChannel, chunk->dat, chunk->len);

                    av_free(chunk->dat);
                    chunk->dat = NULL;
                    av_free(chunk);
                    chunk = NULL;
                } else {
                    if (pcm_pool->getEndFlag()) {
                        usleep(PLAYFINISH_DELAY_TIME_MS);
                        player->m_status = AUDIOPLAYER_STATUS_FINISHED;
                        player->m_finishFlag = true;

                        shouldReportFinishEvent = true;
                        break;
                    } else {
                        player->handleHungryState();
                    }
                }
            }
            usleep(PLAY_LOOP_DELAY_TIME_MS);
        }

        pthread_mutex_unlock(&player->m_playMutex);
        if (shouldReportFinishEvent) {
            player->executePlaybackFinished();
        }
    }
    return NULL;
}

void AudioPlayerImpl::audioPlay(const std::string &url,
                                 RES_FORMAT format,
                                 unsigned int offset,
                                 unsigned int report_interval) {
    APP_INFO("audioPlay called, call stop first");
    audioStop();
    APP_INFO("audioPlay called");
    pthread_mutex_lock(&m_decodeMutex);
    pthread_mutex_lock(&m_playMutex);
    PcmBufPool::getInstance()->clear();
    m_startOffsetMs = offset;
    m_progressMs = m_startOffsetMs;

    unsigned int current_time = TimerUtil::currentTimeMs();
    m_progressStartMs = current_time;
    m_frameTimestamp = current_time;
    m_reportIntervalMs = report_interval;
    m_seekable = false;
    m_pauseFlag = false;
    m_finishFlag = false;
    m_shouldBreakFlag = false;
    m_firstPackFlag = true;
    m_durationMs = 0;
    m_stopFlag = false;
    m_url = url;
    m_progressMs = offset;
    m_nearlyFinishFlag = false;
    m_status = AUDIOPLAYER_STATUS_PLAY;
    executePlaybackUnderbuffer(true);
    pthread_cond_signal(&m_playCond);
    pthread_cond_signal(&m_decodeCond);
    pthread_mutex_unlock(&m_playMutex);
    pthread_mutex_unlock(&m_decodeMutex);
    APP_INFO("audioPlay called finish");
}

void AudioPlayerImpl::pushStream(unsigned int channels,
                                  const void *buffer,
                                  unsigned long buff_size) {
    pthread_mutex_lock(&m_alsaMutex);
    if (m_pauseFlag || m_stopFlag) {
        pthread_mutex_unlock(&m_alsaMutex);
        return;
    }

    if (m_firstPackFlag) {
        executePlaybackStarted();
        m_firstPackFlag = false;
    }

    m_alsaController->writeStream(channels, buffer, buff_size);
    m_progressMs += (unsigned long) ((buff_size * 1000.0f) / m_bytesPersecond);

    unsigned int current_time = TimerUtil::currentTimeMs();
    if (m_listener != NULL && current_time - m_progressStartMs >= m_reportIntervalMs) {
        m_listener->playbackProgress(m_progressMs);
        m_progressStartMs = current_time;
    }
    if (!m_nearlyFinishFlag && (m_durationMs - m_progressMs <= NEAR_FINISH_TIME_LEFT)) {
        m_nearlyFinishFlag = true;
        executePlaybackNearlyFinished();
    }

    pthread_mutex_unlock(&m_alsaMutex);
}

void AudioPlayerImpl::audioStop() {
    APP_INFO("audioStop called");
    if (m_status == AUDIOPLAYER_STATUS_IDLE || m_status == AUDIOPLAYER_STATUS_FINISHED) {
        APP_INFO("audioStop return, status is idle");
        return;
    }

    m_stopFlag = true;
    PcmBufPool::getInstance()->clear();
    pthread_mutex_lock(&m_decodeMutex);
    pthread_mutex_lock(&m_playMutex);
    if (!m_alsaController->isAccessable()) {
        freeCodecContext();
        freeFormatContext();
        pthread_mutex_unlock(&m_playMutex);
        pthread_mutex_unlock(&m_decodeMutex);
        APP_INFO("audioStop return, alsa is not accessable");
        return;
    }

    freeCodecContext();
    freeFormatContext();

    m_alsaController->alsaClear();
    m_status = AUDIOPLAYER_STATUS_IDLE;
    executePlaybackStopped();

    pthread_mutex_unlock(&m_playMutex);
    pthread_mutex_unlock(&m_decodeMutex);
    APP_INFO("audioStop called finish");
}

void AudioPlayerImpl::audioPause() {
    pthread_mutex_lock(&m_alsaMutex);
    if (m_status == AUDIOPLAYER_STATUS_IDLE || m_status == AUDIOPLAYER_STATUS_FINISHED) {
        pthread_mutex_unlock(&m_alsaMutex);
        return;
    }

    if (!m_alsaController->isAccessable()) {
        pthread_mutex_unlock(&m_alsaMutex);
        return;
    }
    m_pauseFlag = true;

    if (m_status != AUDIOPLAYER_STATUS_STOP) {
        if (!m_alsaController->alsaPause()) {
            pthread_mutex_unlock(&m_alsaMutex);
            return;
        }
    }
    executePlaybackPaused();
    m_status = AUDIOPLAYER_STATUS_STOP;
    pthread_mutex_unlock(&m_alsaMutex);
}

void AudioPlayerImpl::audioResume() {
    if (m_status == AUDIOPLAYER_STATUS_IDLE || m_status == AUDIOPLAYER_STATUS_FINISHED) {
        return;
    }

    if (!m_alsaController->isAccessable()) {
        return;
    }
    m_pauseFlag = false;

    if (!m_alsaController->alsaResume()) {
        return;
    }
    executePlaybackResumed();
    m_status = AUDIOPLAYER_STATUS_PLAY;
}

bool AudioPlayerImpl::seekBy(unsigned int offsetMs) {
    if (m_status == AUDIOPLAYER_STATUS_IDLE || m_status == AUDIOPLAYER_STATUS_FINISHED) {
        return false;
    }
    if (!m_seekable || m_formatCtx == NULL) {
        return false;
    }
    //seek to target position.
    long int seek_pos = (m_progressMs + offsetMs) / 1000 * AV_TIME_BASE;
    if (m_formatCtx->start_time != AV_NOPTS_VALUE) {
        seek_pos += m_formatCtx->start_time;
    }
    if (av_seek_frame(m_formatCtx, -1, seek_pos, AVSEEK_FLAG_BACKWARD) < 0) {
        return false;
    }
    return true;
}

unsigned long AudioPlayerImpl::getProgress() {
    return m_progressMs;
}

unsigned long AudioPlayerImpl::getDuration() {
    return m_durationMs;
}

void AudioPlayerImpl::ffmpegLogOutput(void *ptr,
                                      int level,
                                      const char *fmt,
                                      va_list vl) {
}

int AudioPlayerImpl::interruptCallback(void *ctx) {
    AudioPlayerImpl *this_ptr = (AudioPlayerImpl *) ctx;
    long time_diff = TimerUtil::currentTimeMs() - this_ptr->m_frameTimestamp;
    if (this_ptr->m_stopFlag) {
        return -1;
    } else if (time_diff >= TIMEOUT_MS) {
        this_ptr->m_shouldBreakFlag = true;
        return -1;
    } else {
        return 0;
    }
}

void AudioPlayerImpl::executePlaybackStarted() {
    m_executor->submit(
            [this] () {
                if (m_listener != NULL) {
                    m_listener->playbackStarted(m_startOffsetMs);
                }
            }
    );
}

void AudioPlayerImpl::executePlaybackUnderbuffer(bool beforeStart) {
    m_executor->submit(
            [this, beforeStart] () {
                PlayProgressInfo playProgressInfo;
                if (beforeStart) {
                    playProgressInfo = PlayProgressInfo::BEFORE_START;
                } else {
                    playProgressInfo = PlayProgressInfo::DURING_PLAY;
                }
                if (m_listener != NULL) {
                    m_listener->playbackBufferunderrun(playProgressInfo);
                }
            }
    );
}

void AudioPlayerImpl::executePlaybackBufferrefill() {
    m_executor->submit(
            [this] () {
                if (m_listener != NULL) {
                    m_listener->playbackBufferefilled();
                }
            }
    );
}

void AudioPlayerImpl::executePlaybackError() {
    m_executor->submit(
            [this] () {
                if (m_listener != NULL) {
                    m_listener->playbackError();
                }
            }
    );
}

void AudioPlayerImpl::executePlaybackStopped() {
    m_executor->submit(
            [this] () {
                if (m_listener != NULL) {
                    m_listener->playbackStopped(m_progressMs);
                }
            }
    );
}

void AudioPlayerImpl::executePlaybackPaused() {
    m_executor->submit(
            [this] () {
                if (m_listener != NULL) {
                    m_listener->playbackPaused(m_progressMs);
                }
            }
    );
}

void AudioPlayerImpl::executePlaybackResumed() {
    m_executor->submit(
            [this] () {
                if (m_listener != NULL) {
                    m_listener->playbackResumed(m_startOffsetMs);
                }
            }
    );
}

void AudioPlayerImpl::executePlaybackFinished() {
    m_executor->submit(
            [this] () {
                if (NULL != m_listener) {
                    m_listener->playbackFinished(m_progressMs, AudioPlayerFinishStatus::AUDIOPLAYER_FINISHSTATUS_END);
                }
            }
    );
}

void AudioPlayerImpl::executePlaybackNearlyFinished() {
    m_executor->submit(
            [this] () {
                if (NULL != m_listener) {
                    m_listener->playbackNearlyFinished(m_progressMs);
                }
            }
    );
}

void AudioPlayerImpl::onStreamInterrupt() {
    m_executor->submit(
            [this] () {
                if (NULL != m_listener) {
                    m_listener->playbackStreamUnreach();
                }
            }
    );
}

}  // mediaPlayer
}  // duerOSDcsApp

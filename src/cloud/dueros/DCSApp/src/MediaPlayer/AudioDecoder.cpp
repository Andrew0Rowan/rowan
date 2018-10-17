/*
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

#include "MediaPlayer/AudioDecoder.h"

namespace duerOSDcsApp {
namespace mediaPlayer {
AudioDecoder::AudioDecoder(unsigned int channel,
                           unsigned int samplerate) : m_decodeMode(DECODE_MODE_NORMAL),
                                                      m_formatCtx(NULL),
                                                      m_codecCtx(NULL),
                                                      m_packet(NULL),
                                                      m_frame(NULL),
                                                      m_repeatTimes(1),
                                                      m_currentTimes(0),
                                                      m_convertCtx(NULL),
                                                      m_outputChannel(1),
                                                      m_outputSamplerate(48000),
                                                      m_bytesPersample(0),
                                                      c_outputFmt(AV_SAMPLE_FMT_S16) {
    m_outputChannel = channel;
    m_outputSamplerate = samplerate;
    av_register_all();
    avformat_network_init();
    av_log_set_level(AV_LOG_FATAL);

    m_packet = av_packet_alloc();
    m_frame = av_frame_alloc();
}

AudioDecoder::~AudioDecoder() {
    if (m_packet != NULL) {
        av_packet_free(&m_packet);
        m_packet = NULL;
    }

    if (m_frame != NULL) {
        av_frame_free(&m_frame);
        m_frame = NULL;
    }

    avformat_network_deinit();
}

void AudioDecoder::setDecodeMode(DecodeMode mode, int val) {
    m_decodeMode = mode;
    m_repeatTimes = val;
    m_currentTimes = 0;
}

bool AudioDecoder::open(const std::string &url) {
    m_bytesPersample = 0;
    m_currentTimes = 0;
    if (avformat_open_input(&m_formatCtx,
                            url.c_str(),
                            NULL,
                            NULL) != 0) {
        return false;
    }
    if (avformat_find_stream_info(m_formatCtx, NULL) < 0) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = NULL;
        return false;
    }
    av_dump_format(m_formatCtx, 0, url.c_str(), false);

    int audio_stream = -1;
    for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream = i;
            break;
        }
    }

    if (audio_stream == -1) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = NULL;
        return false;
    }

    AVStream *a_stream = m_formatCtx->streams[audio_stream];
    AVCodecParameters *codec_par = a_stream->codecpar;
    AVCodec *codec = avcodec_find_decoder(codec_par->codec_id);
    m_codecCtx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(m_codecCtx, codec_par) < 0) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = NULL;
    }
    if (codec == NULL) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = NULL;
        return false;
    }

    if (avcodec_open2(m_codecCtx, codec, NULL) < 0) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = NULL;
        return false;
    }

    uint64_t out_channel_layout = AV_CH_LAYOUT_MONO;
    if (m_outputChannel == 1) {
        out_channel_layout = AV_CH_LAYOUT_MONO;
    } else if (m_outputChannel == 2) {
        out_channel_layout = AV_CH_LAYOUT_STEREO;
    }

    m_bytesPersample = av_get_bytes_per_sample(c_outputFmt);
    int64_t in_channel_layout = av_get_default_channel_layout(m_codecCtx->channels);

    m_convertCtx = swr_alloc();
    m_convertCtx = swr_alloc_set_opts(m_convertCtx,
                                      out_channel_layout,
                                      c_outputFmt,
                                      m_outputSamplerate,
                                      in_channel_layout,
                                      m_codecCtx->sample_fmt,
                                      m_codecCtx->sample_rate,
                                      0,
                                      NULL);
    swr_init(m_convertCtx);
    return true;
}

StreamReadResult AudioDecoder::readFrame(uint8_t **frameBuffer, unsigned int bufferLen, unsigned int *len) {
    int ret = av_read_frame(m_formatCtx, m_packet);
    if (ret >= 0) {
        avcodec_send_packet(m_codecCtx, m_packet);
        int error_code = avcodec_receive_frame(m_codecCtx, m_frame);
        if (error_code == 0) {
            int frame_size = swr_convert(m_convertCtx,
                                         frameBuffer,
                                         bufferLen,
                                         (const uint8_t **) m_frame->data,
                                         m_frame->nb_samples);
            *len = frame_size * m_outputChannel * m_bytesPersample;
            av_frame_unref(m_frame);
            av_packet_unref(m_packet);
            return READ_SUCCEED;
        } else {
            av_frame_unref(m_frame);
            av_packet_unref(m_packet);
            return READ_FAILED;
        }
    } else {
        if (m_decodeMode == DECODE_MODE_NORMAL) {
            if (ret == AVERROR_EOF) {
                return READ_END;
            } else {
                return READ_FAILED;
            }
        } else if (m_decodeMode == DECODE_MODE_LOOP) {
            if (ret == AVERROR_EOF) {
                if (m_repeatTimes == 0) {
                    av_seek_frame(m_formatCtx, -1, 0L, AVSEEK_FLAG_BACKWARD);
                    return READ_FAILED;
                } else {
                    if (++m_currentTimes < m_repeatTimes) {
                        av_seek_frame(m_formatCtx, -1, 0L, AVSEEK_FLAG_BACKWARD);
                        return READ_FAILED;
                    } else {
                        return READ_END;
                    }
                }
            } else {
                return READ_FAILED;
            }
        }
    }
    return READ_FAILED;
}

unsigned int AudioDecoder::calculateResampleSize(int frameSize,
                                                 int inSampleRate, int outSampleRate,
                                                 int inChannel, int outChannel,
                                                 int inFmtSize, int outFmtSize) {
    double samplerateRatio = (outSampleRate * 1.0) / inSampleRate;
    double channelRatio = (outChannel * 1.0) / inChannel;
    double fmtRatio = (outFmtSize * 1.0) / inFmtSize;
    samplerateRatio = ceil(samplerateRatio);
    channelRatio = ceil(channelRatio);
    fmtRatio = ceil(fmtRatio);
    return (unsigned int)(frameSize * samplerateRatio * channelRatio * fmtRatio);
}

bool AudioDecoder::close() {
    if (NULL != m_convertCtx) {
        swr_free(&m_convertCtx);
        m_convertCtx = NULL;
    }
    if (NULL != m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
        avcodec_close(m_codecCtx);
        m_codecCtx = NULL;
    }
    if (NULL != m_formatCtx) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = NULL;
    }
    return true;
}

}  // mediaPlayer
}  // duerOSDcsApp
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

#ifndef DUEROS_DCS_APP_MEDIAPLAYER_AUDIODECODER_H_
#define DUEROS_DCS_APP_MEDIAPLAYER_AUDIODECODER_H_

#include <string>
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
};
#endif

namespace duerOSDcsApp {
namespace mediaPlayer {

enum StreamReadResult {
    READ_SUCCEED,
    READ_FAILED,
    READ_END
};

enum DecodeMode {
    DECODE_MODE_NORMAL,
    DECODE_MODE_LOOP
};

class AudioDecoder {
public:
    AudioDecoder(unsigned int channel, unsigned int samplerate);

    ~AudioDecoder();

    void setDecodeMode(DecodeMode mode, int val);

    bool open(const std::string &url);

    StreamReadResult readFrame(uint8_t **frameBuffer, unsigned int bufferLen, unsigned int *len);

    bool close();

private:
    DecodeMode m_decodeMode;
    AVFormatContext *m_formatCtx;
    AVCodecContext *m_codecCtx;
    AVPacket *m_packet;
    AVFrame *m_frame;
    unsigned int m_repeatTimes;
    unsigned int m_currentTimes;
    struct SwrContext *m_convertCtx;
    unsigned int m_outputChannel;
    unsigned int m_outputSamplerate;
    unsigned int m_bytesPersample;
    const AVSampleFormat c_outputFmt;

private:
    AudioDecoder(const AudioDecoder &);

    AudioDecoder &operator=(const AudioDecoder &);

    unsigned int calculateResampleSize(int frameSize,
                                       int inSampleRate, int outSampleRate,
                                       int inChannel, int outChannel,
                                       int inFmt, int outFmt);
};

}  // mediaPlayer
}  // duerOSDcsApp

#endif  // DUEROS_DCS_APP_MEDIAPLAYER_AUDIODECODER_H_

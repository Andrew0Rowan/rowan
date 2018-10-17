//
//  pal_sample.c
//  pal_sdk
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "portaudio.h"
#include "pal.h"

#define OPUS_SAMPLE_FILE  "sample.opus"
#define PCM_SAMPLE_FILE  "sample.pcm"

#define PLAY_CMD_FILE "play.cmd"

#define EXTERNAL_PLAYER_CMD "./playertest 1 "

static const int NUM_INPUT_CHANNELS = 8;
static const int NUM_OUTPUT_CHANNELS = 0;
static const double SAMPLE_RATE = 16000;
static const unsigned long PREFERRED_SAMPLES_PER_CALLBACK = paFramesPerBufferUnspecified;

#define BUF_SIZE 512
//char g_tts_whatTheHell[BUF_SIZE];
char g_cmd_whatTheHell[BUF_SIZE];
char g_uri_whatTheHell[BUF_SIZE];

int g_exec_this_cmd = 0;

static void douglas_asr_test(int format)
{
    for (int i = 0; i < 1; i++) {
        int ret = pal_asr_start();
        FILE *fp = NULL;
        if (format == PAL_AUDIO_FORMAT_OPUS) {
            fp = fopen(OPUS_SAMPLE_FILE, "rb");
        } else if (format == PAL_AUDIO_FORMAT_PCM){
            fp = fopen(PCM_SAMPLE_FILE, "rb");
        } else {
            printf("unsupport format\n");
        }
        if (!fp) {
            printf("No file opened\n");
            return;
        }
        int file_size;
        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        int raw_len = 640;
        printf("====================================================== begin\n");
        unsigned char *raw = malloc(raw_len * sizeof(unsigned char));
        int read;
        int sum = 0;
        int count = 0;
        while ((read = fread(raw, sizeof(unsigned char), raw_len, fp)) != 0) {
            sum += read;
            usleep(10 * 1000);

            int ret = pal_asr_send_buffer((char *)raw, read * sizeof(unsigned char));
            if (ret == PAL_VAD_STATUS_STOP) {
                printf("Detect VAD STOP\n");
                break;
            }
        }
        printf("===================================================== end sum: %d\n", sum);
        free(raw);
        fclose(fp);
        struct pal_rec_result *result = pal_asr_stop();
        if (result) {
            printf("MY>>> result->status=%d\n", result->status);
            printf("MY>>> result->should_restore_player_status=%d\n", result->should_restore_player_status);
            printf("MY>>> result->asr_result=%s\n", result->asr_result);
            printf("MY>>> result->task_status=%d\n", result->task_status);
            printf("MY>>> result->tts=%s\n", result->tts);
            printf("MY>>> result->raw=%s\n", result->raw);

            // Add quotation marks around tts string.
            // This will make sure that player gets correct uri
            sprintf(g_uri_whatTheHell, "\"%s\"\0", result->tts);
            char tts_test[512];
            sprintf(tts_test, "%s %s", EXTERNAL_PLAYER_CMD, g_uri_whatTheHell);
            system(tts_test);

            pal_rec_result_destroy(result);
        }
        sleep(3);
    }
}

static int totalSample = 0;

int portAudioCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long numSamples,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {

    totalSample += numSamples;
    printf("Incoming samples:%d, total samples:%d\n", numSamples, totalSample);

    FILE *record = fopen(PCM_SAMPLE_FILE, "a+");
#if 1
    // 8 channel -> 1 channel
    const void *pw = inputBuffer;
    for (int i = 0; i < numSamples; ++ i) {
        fwrite(pw, 2, 1, record);
        pw += 2 * NUM_INPUT_CHANNELS;
    }
#else
    // 16 bits per sample
    //fwrite(inputBuffer, 2 * NUM_INPUT_CHANNELS, numSamples, record);
#endif
    fflush(record);
    fclose(record);

    return paContinue;
}

static int voice_in_test_record() {
    PaError err;
    PaStream* paStream;
    err = Pa_Initialize();
    if (err != paNoError) {
        printf("Failed to initialize PortAudio");
        return -1;
    }
    /*paInt16 -> 16bits per sample -> 2byte*/
    err = Pa_OpenDefaultStream(
        &paStream,
        NUM_INPUT_CHANNELS,
        NUM_OUTPUT_CHANNELS,
        paInt16,
        SAMPLE_RATE,
        PREFERRED_SAMPLES_PER_CALLBACK,
        portAudioCallback,
        NULL);
    if (err != paNoError) {
        printf("Failed to open PortAudio default stream");
        return -1;
    }

    printf("Press any key to start recording...\n");
    getchar();

    err = Pa_StartStream(paStream);
    if (err != paNoError) {
        printf("Failed to start PortAudio stream");
        return -1;
    }

    printf("Press any key to stop recording...\n");
    getchar();

    err = Pa_StopStream(paStream);
    if (err != paNoError) {
        printf("Failed to stop PortAudio stream");
        return -1;
    }
    printf("End of recording\n");
    return 0;
}

void play_tts_test() {

}

int play_cmd_test() {
    int retry = 500;
    while(0 == g_exec_this_cmd) {
        usleep(10 * 1000);
        --retry;
        if (retry < 0) {
            printf("play_cmd_test wait timeout\n");
            g_exec_this_cmd = 0;
            return -1;
        }
    }
    printf("MY>>> play_cmd_test wait end. retry=%d\n", retry);
    FILE *cmdFile = fopen(PLAY_CMD_FILE, "r");
    if (!cmdFile) {
        printf("MY>>> No cmd file\n");
        return -1;
    }
    fgets(g_cmd_whatTheHell, BUF_SIZE, cmdFile);
    fclose(cmdFile);
    printf("MY>>> cmd = %s\n", g_cmd_whatTheHell);

    sleep(1);

    char *start = strstr(g_cmd_whatTheHell, "http:");
    char *end = strstr(start, "\"");
    int length = end - start;
    memcpy(g_uri_whatTheHell, start, length);
    // end of string
    g_uri_whatTheHell[length] = '\0';

    printf("MY>>> uri: %s\n", g_uri_whatTheHell);

    char tts_test[512];
    sprintf(tts_test, "%s %s", EXTERNAL_PLAYER_CMD, g_uri_whatTheHell);
    system(tts_test);

    g_exec_this_cmd = 0;
}


static void pal_callback_fn(const char *cmd, int cmd_type, char *buffer, int buffer_capacity, void *user)
{
    printf("MY>>> cmd %s\n", cmd);
    printf("MY>>> length=%d\n", strlen(cmd));
    // only exec play cmd
    // ignore all the others
    if (strstr(cmd, "play")) {
        remove(PLAY_CMD_FILE);
        FILE *cmdFile = fopen(PLAY_CMD_FILE, "w+");
        fprintf(cmdFile, "%s", cmd);
        fclose(cmdFile);
        g_exec_this_cmd = 1;
    } else {
        printf("MY>>> ignore cmd\n");
    }
}


int main(int argc, const char * argv[]) {
    if (argc < 2) {
        printf("Usage: <index>\n \
            Test items:\n \
            1. douglas_asr_test\n \
            2. tts_test\n \
            3. voice_in_test\n \
            4. play_cmd_test\n \
            5. ...\n \
            ...\n \
            9. ALL: 3-1-4 \n");
        return -1;
    }

    int index = atoi(argv[1]);
    struct pal_config config;
    memset(&config, 0, sizeof(struct pal_config));
    config.ca_file_path = "ca.pem"; //ca.pem路径，如/tmp／ca.pem
    int format = PAL_AUDIO_FORMAT_PCM;
    //int format = PAL_AUDIO_FORMAT_OPUS;
    config.format = format;
    config.sample_rate = SAMPLE_RATE;
    config.channels = 1; // 8 channel not supported?
    config.bits_per_sample = 16;
    config.callback = pal_callback_fn;

    pal_set_debug_uuid("981019BA5A5B341E9087C61A063294A5");
    int ret = pal_init(&config);

    if (1 == index) {
        douglas_asr_test(format);
    } else

    if (2 == index) {
        struct pal_rec_result* result =  pal_get_tts("我要听刘德华的歌");
        if (result) {
            printf("===== %s\n", result->raw);

            printf("MY>>> result->status=%d\n", result->status);
            printf("MY>>> result->should_restore_player_status=%d\n", result->should_restore_player_status);
            printf("MY>>> result->asr_result=%s\n", result->asr_result);
            printf("MY>>> result->task_status=%d\n", result->task_status);
            printf("MY>>> result->tts=%s\n", result->tts);
            printf("MY>>> result->raw=%s\n", result->raw);
        } else {
            printf("get_tts failed\n");
        }
    } else

    if (3 == index) {
        remove(PCM_SAMPLE_FILE);
        voice_in_test_record();
    } else

    if (4 == index) {
        g_exec_this_cmd = 1;
        play_cmd_test();
    } else

    if (9 == index) {
        remove(PCM_SAMPLE_FILE);
        voice_in_test_record();
        printf("MY>>> wait a little bit for callback to finish its job.\n");
        sleep(1);
        douglas_asr_test(format);
        play_cmd_test();
    }

    pal_destroy();

    //opus_codec_destroy();
    return 0;
}


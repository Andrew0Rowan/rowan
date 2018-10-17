#ifndef AUDIO_FORMAT_H
#define AUDIO_FORMAT_H


/* audio sampling rate */
#define TOTAL_SAMPLING_RATE_NUM 9
typedef enum
{
    AUDIO_SAMPLE_RATE_8000  = 0x00000001,
    AUDIO_SAMPLE_RATE_11025 = 0x00000002,
    AUDIO_SAMPLE_RATE_16000 = 0x00000004,
    AUDIO_SAMPLE_RATE_22050 = 0x00000008,
    AUDIO_SAMPLE_RATE_32000 = 0x00000010,
    AUDIO_SAMPLE_RATE_44100 = 0x00000020,
    AUDIO_SAMPLE_RATE_48000 = 0x00000040,
    AUDIO_SAMPLE_RATE_96000 = 0x00000080,
    AUDIO_SAMPLE_RATE_192000 = 0x00000100
} AUDIO_SAMPLE_RATE;

typedef struct srate_map
{
    AUDIO_SAMPLE_RATE id;
    int value;
} SRATE_MAP;

static const SRATE_MAP srate_map[] =
{
    {AUDIO_SAMPLE_RATE_8000, 8000},
    {AUDIO_SAMPLE_RATE_11025, 11025},
    {AUDIO_SAMPLE_RATE_16000, 16000},
    {AUDIO_SAMPLE_RATE_22050, 22050},
    {AUDIO_SAMPLE_RATE_32000, 32000},
    {AUDIO_SAMPLE_RATE_44100, 44100},
    {AUDIO_SAMPLE_RATE_48000, 48000},
    {AUDIO_SAMPLE_RATE_96000, 96000},
    {AUDIO_SAMPLE_RATE_192000, 192000}
};

/* audio channel */
#define TOTAL_CH_NUM 16
typedef enum
{
    AUDIO_CHANNEL_FRONT_LEFT   = 0x0001,
    AUDIO_CHANNEL_FRONT_RIGHT  = 0x0002,
    AUDIO_CHANNEL_FRONT_CENTER = 0x0004,
    AUDIO_CHANNEL_BACK_LEFT    = 0x0008,
    AUDIO_CHANNEL_BACK_RIGHT   = 0x0010,
    AUDIO_CHANNEL_BACK_CENTER  = 0x0020,
    AUDIO_CHANNEL_SIDE_LEFT    = 0x0040,
    AUDIO_CHANNEL_SIDE_RIGHT   = 0x0080,
    AUDIO_CHANNEL_EXTEND_0     = 0x0100,
    AUDIO_CHANNEL_EXTEND_1     = 0x0200,
    AUDIO_CHANNEL_EXTEND_2     = 0x0400,
    AUDIO_CHANNEL_EXTEND_3     = 0x0800,
    AUDIO_CHANNEL_EXTEND_4     = 0x1000,
    AUDIO_CHANNEL_EXTEND_5     = 0x2000,
    AUDIO_CHANNEL_EXTEND_6     = 0x4000,
    AUDIO_CHANNEL_LOW_FREQUENCY =0x8000
} AUDIO_CHANNEL;

// mono channel
#define AUDIO_CHANNEL_MONO AUDIO_CHANNEL_FRONT_CENTER

// stereo channel
#define AUDIO_CHANNEL_STEREO  (AUDIO_CHANNEL_FRONT_LEFT | \
                               AUDIO_CHANNEL_FRONT_RIGHT)

// 3 channel
#define AUDIO_CHANNEL_THREE (AUDIO_CHANNEL_STEREO | \
	                          AUDIO_CHANNEL_FRONT_CENTER)

#define AUDIO_CHANNEL_THR_SUR    (AUDIO_CHANNEL_STEREO | \
	                          AUDIO_CHANNEL_BACK_CENTER)

// 4 channel
// quad channel
#define AUDIO_CHANNEL_QUAD    (AUDIO_CHANNEL_FRONT_LEFT | \
                               AUDIO_CHANNEL_FRONT_RIGHT| \
                               AUDIO_CHANNEL_BACK_LEFT  | \
                               AUDIO_CHANNEL_BACK_RIGHT )

// surround channel
#define AUDIO_CHANNEL_SUR     (AUDIO_CHANNEL_FRONT_LEFT  | \
                                AUDIO_CHANNEL_FRONT_RIGHT | \
                                AUDIO_CHANNEL_FRONT_CENTER| \
                                AUDIO_CHANNEL_BACK_CENTER)

// 5 channel
#define AUDIO_CHANNEL_FIVE     (AUDIO_CHANNEL_FRONT_LEFT | \
                                AUDIO_CHANNEL_FRONT_RIGHT | \
                                AUDIO_CHANNEL_FRONT_CENTER | \
                                AUDIO_CHANNEL_BACK_LEFT | \
                                AUDIO_CHANNEL_BACK_RIGHT)

#define AUDIO_CHANNEL_FIVE_SIDE (AUDIO_CHANNEL_FRONT_LEFT | \
                                AUDIO_CHANNEL_FRONT_RIGHT | \
                                AUDIO_CHANNEL_FRONT_CENTER | \
                                AUDIO_CHANNEL_SIDE_LEFT | \
                                AUDIO_CHANNEL_SIDE_RIGHT)

// 6 channel
#define AUDIO_CHANNEL_SIX     (AUDIO_CHANNEL_FIVE | \
	                           AUDIO_CHANNEL_BACK_CENTER)

#define AUDIO_CHANNEL_SIX_SIDE   (AUDIO_CHANNEL_FIVE_SIDE | \
	                           AUDIO_CHANNEL_BACK_CENTER)

// 7 channel
#define AUDIO_CHANNEL_SEVEN  (AUDIO_CHANNEL_FRONT_LEFT | \
                                AUDIO_CHANNEL_FRONT_RIGHT | \
                                AUDIO_CHANNEL_FRONT_CENTER | \
                                AUDIO_CHANNEL_BACK_LEFT | \
                                AUDIO_CHANNEL_BACK_RIGHT | \
                                AUDIO_CHANNEL_SIDE_LEFT | \
                                AUDIO_CHANNEL_SIDE_RIGHT)


static const AUDIO_CHANNEL ch_map[TOTAL_CH_NUM] =
{
    AUDIO_CHANNEL_FRONT_LEFT,
    AUDIO_CHANNEL_FRONT_RIGHT,
    AUDIO_CHANNEL_FRONT_CENTER,
    AUDIO_CHANNEL_BACK_LEFT,
    AUDIO_CHANNEL_BACK_RIGHT,
    AUDIO_CHANNEL_BACK_CENTER,
    AUDIO_CHANNEL_SIDE_LEFT,
    AUDIO_CHANNEL_SIDE_RIGHT,
    AUDIO_CHANNEL_EXTEND_0,
    AUDIO_CHANNEL_EXTEND_1,
    AUDIO_CHANNEL_EXTEND_2,
    AUDIO_CHANNEL_EXTEND_3,
    AUDIO_CHANNEL_EXTEND_4,
    AUDIO_CHANNEL_EXTEND_5,
    AUDIO_CHANNEL_EXTEND_6,
    AUDIO_CHANNEL_LOW_FREQUENCY,
};

/* audio buf to define channel buffer address
 * index is according to ch_map
 */
typedef struct audio_buf{
	int* buf[TOTAL_CH_NUM];
}AUDIO_BUF;

/* sample format */
typedef enum
{
    AUDIO_SAMPLE_FORMAT_16BIT_MSB = 0x0001,
    AUDIO_SAMPLE_FORMAT_24BIT_MSB = 0x0002,
    AUDIO_SAMPLE_FORMAT_32BIT     = 0x0004
} AUDIO_SAMPLE_FORMAT;

/* audio device */
typedef enum
{
    AUDIO_DEVICE_SPEAKER = 0x00000001,
    AUDIO_DEVICE_HDMI    = 0x00000002
} AUDIO_DEVICE;

/*
 * audio scenario mode
 */
typedef enum
{
    AUDIO_MODE_MOVIE   = 0x00000001,
    AUDIO_MODE_MUSIC   = 0x00000002,
    AUDIO_MODE_3       = 0x00000004,
    AUDIO_MODE_4       = 0x00000008,
    AUDIO_MODE_5       = 0x00000010,
    AUDIO_MODE_6       = 0x00000020,
    AUDIO_MODE_7       = 0x00000040,
    AUDIO_MODE_8       = 0x00000080,
    AUDIO_MODE_9       = 0x00000100,
    AUDIO_MODE_10      = 0x00000200,
    AUDIO_MODE_11      = 0x00000400,
    AUDIO_MODE_12      = 0x00000800,
    AUDIO_MODE_13      = 0x00001000,
    AUDIO_MODE_14      = 0x00002000,
    AUDIO_MODE_15      = 0x00004000,
    AUDIO_MODE_16      = 0x00008000,
    AUDIO_MODE_17      = 0x00010000,
    AUDIO_MODE_18      = 0x00020000,
    AUDIO_MODE_19      = 0x00040000,
    AUDIO_MODE_20      = 0x00080000,
    AUDIO_MODE_21      = 0x00100000,
    AUDIO_MODE_22      = 0x00200800,
    AUDIO_MODE_23      = 0x00400000,
    AUDIO_MODE_24      = 0x00800000,
    AUDIO_MODE_25      = 0x01000000,
    AUDIO_MODE_26      = 0x02000000,
    AUDIO_MODE_27      = 0x04000000,
    AUDIO_MODE_28      = 0x08000000,
    AUDIO_MODE_29      = 0x10000000,
    AUDIO_MODE_30      = 0x20000000,
    AUDIO_MODE_31      = 0x40000000,
    AUDIO_MODE_32      = 0x80000000,
} AUDIO_MODE;

/*
 * system information
 */
typedef struct
{
    // fill in by system
    unsigned int in_channel;
    unsigned int out_channel;
    AUDIO_SAMPLE_RATE sample_rate;
    AUDIO_SAMPLE_FORMAT sample_format;
    AUDIO_DEVICE device;
    AUDIO_MODE mode;
    // system frame size
    int frame_size;
    // ppc frame size, filled by ppc
    int ppc_frame_size;
} SYS_INFO;

#endif

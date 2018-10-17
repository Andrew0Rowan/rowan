//MTK : PPC Tool 
//(DO NOT EDIT: machine generated header)
#ifndef ELEMENT_TYPE_H
#define ELEMENT_TYPE_H
#include "id.h"
/*
 * The following sections define parameter information
 * and default parameter for each element
 */
//input element parameter
#define input_para_num 3
#define input_data_num 13

PARA_INFO input_para_info[input_para_num] = {
    {"PARA_INPUT_SAMPLE_RATE", PARA_INPUT_SAMPLE_RATE, "support sample rate", TYPE_INT, 0, 1 },
    {"PARA_INPUT_SAMPLE_FORMAT", PARA_INPUT_SAMPLE_FORMAT, "support sample type", TYPE_INT, 1, 1 },
    {"PARA_INPUT_SUPPORT_CH", PARA_INPUT_SUPPORT_CH, "support channel", TYPE_INT, 2, 11 },
};

int input_para[input_data_num] = {112,7,4,3,7,27,39,31,223,199,63,231,32967};

//output element parameter
#define output_para_num 1
#define output_data_num 1

PARA_INFO output_para_info[output_para_num] = {
    {"PARA_OUTPUT_CH_MODE", PARA_OUTPUT_CH_MODE, "channel mode", TYPE_INT, 0, 1 },
};

int output_para[output_data_num] = {1};

//add element parameter
#define add_para_num 1
#define add_data_num 16

PARA_INFO add_para_info[add_para_num] = {
    {"PARA_ADD_WEIGHT", PARA_ADD_WEIGHT, "channel weight", TYPE_INT, 0, 16 },
};

int add_para[add_data_num] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//channel_mix element parameter
#define channel_mix_para_num 45
#define channel_mix_data_num 23

PARA_INFO channel_mix_para_info[channel_mix_para_num] = {
    {"PARA_CHMIX_OUTPUT_CH", PARA_CHMIX_OUTPUT_CH, "output channel", TYPE_INT, 0, 1 },
    {"PARA_CHMIX_COEFF_MONO", PARA_CHMIX_COEFF_MONO, "coeff for mono", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_STEREO", PARA_CHMIX_COEFF_STEREO, "coeff for stereo", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_THREE", PARA_CHMIX_COEFF_THREE, "coeff for three", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_THR_SUR", PARA_CHMIX_COEFF_THR_SUR, "coeff for thr sur", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_QUAD", PARA_CHMIX_COEFF_QUAD, "coeff for quad", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_SUR", PARA_CHMIX_COEFF_SUR, "coeff for sur", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_FIVE", PARA_CHMIX_COEFF_FIVE, "coeff for five", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_FIVE_SIDE", PARA_CHMIX_COEFF_FIVE_SIDE, "coeff for five side", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_SIX", PARA_CHMIX_COEFF_SIX, "coeff for six", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_SIX_SIDE", PARA_CHMIX_COEFF_SIX_SIDE, "coeff for six side", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_SEVEN", PARA_CHMIX_COEFF_SEVEN, "coeff for seven", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_MONO_LFE", PARA_CHMIX_COEFF_MONO_LFE, "coeff for mono with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_STEREO_LFE", PARA_CHMIX_COEFF_STEREO_LFE, "coeff for stereo with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_THREE_LFE", PARA_CHMIX_COEFF_THREE_LFE, "coeff for three with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_THR_SUR_LFE", PARA_CHMIX_COEFF_THR_SUR_LFE, "coeff for thr sur with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_QUAD_LFE", PARA_CHMIX_COEFF_QUAD_LFE, "coeff for quad with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_SUR_LFE", PARA_CHMIX_COEFF_SUR_LFE, "coeff for sur with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_FIVE_LFE", PARA_CHMIX_COEFF_FIVE_LFE, "coeff for five with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_FIVE_SIDE_LFE", PARA_CHMIX_COEFF_FIVE_SIDE_LFE, "coeff for five side with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_SIX_LFE", PARA_CHMIX_COEFF_SIX_LFE, "coeff for six with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_SIX_SIDE_LFE", PARA_CHMIX_COEFF_SIX_SIDE_LFE, "coeff for six side with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_COEFF_SEVEN_LFE", PARA_CHMIX_COEFF_SEVEN_LFE, "coeff for seven with LFE", TYPE_INT, 1, 0 },
    {"PARA_CHMIX_ENABLE_MONO", PARA_CHMIX_ENABLE_MONO, "enable for mono", TYPE_INT, 1, 1 },
    {"PARA_CHMIX_ENABLE_STEREO", PARA_CHMIX_ENABLE_STEREO, "enable for stereo", TYPE_INT, 2, 1 },
    {"PARA_CHMIX_ENABLE_THREE", PARA_CHMIX_ENABLE_THREE, "enable for three", TYPE_INT, 3, 1 },
    {"PARA_CHMIX_ENABLE_THR_SUR", PARA_CHMIX_ENABLE_THR_SUR, "enable for thr sur", TYPE_INT, 4, 1 },
    {"PARA_CHMIX_ENABLE_QUAD", PARA_CHMIX_ENABLE_QUAD, "enable for quad", TYPE_INT, 5, 1 },
    {"PARA_CHMIX_ENABLE_SUR", PARA_CHMIX_ENABLE_SUR, "enable for sur", TYPE_INT, 6, 1 },
    {"PARA_CHMIX_ENABLE_FIVE", PARA_CHMIX_ENABLE_FIVE, "enable for five", TYPE_INT, 7, 1 },
    {"PARA_CHMIX_ENABLE_FIVE_SIDE", PARA_CHMIX_ENABLE_FIVE_SIDE, "enable for five side", TYPE_INT, 8, 1 },
    {"PARA_CHMIX_ENABLE_SIX", PARA_CHMIX_ENABLE_SIX, "enable for six", TYPE_INT, 9, 1 },
    {"PARA_CHMIX_ENABLE_SIX_SIDE", PARA_CHMIX_ENABLE_SIX_SIDE, "enable for six side", TYPE_INT, 10, 1 },
    {"PARA_CHMIX_ENABLE_SEVEN", PARA_CHMIX_ENABLE_SEVEN, "enable for seven", TYPE_INT, 11, 1 },
    {"PARA_CHMIX_ENABLE_MONO_LFE", PARA_CHMIX_ENABLE_MONO_LFE, "enable for mono with LFE", TYPE_INT, 12, 1 },
    {"PARA_CHMIX_ENABLE_STEREO_LFE", PARA_CHMIX_ENABLE_STEREO_LFE, "enable for stereo with LFE", TYPE_INT, 13, 1 },
    {"PARA_CHMIX_ENABLE_THREE_LFE", PARA_CHMIX_ENABLE_THREE_LFE, "enable for three with LFE", TYPE_INT, 14, 1 },
    {"PARA_CHMIX_ENABLE_THR_SUR_LFE", PARA_CHMIX_ENABLE_THR_SUR_LFE, "enable for thr sur with LFE", TYPE_INT, 15, 1 },
    {"PARA_CHMIX_ENABLE_QUAD_LFE", PARA_CHMIX_ENABLE_QUAD_LFE, "enable for quad with LFE", TYPE_INT, 16, 1 },
    {"PARA_CHMIX_ENABLE_SUR_LFE", PARA_CHMIX_ENABLE_SUR_LFE, "enable for sur with LFE", TYPE_INT, 17, 1 },
    {"PARA_CHMIX_ENABLE_FIVE_LFE", PARA_CHMIX_ENABLE_FIVE_LFE, "enable for five with LFE", TYPE_INT, 18, 1 },
    {"PARA_CHMIX_ENABLE_FIVE_SIDE_LFE", PARA_CHMIX_ENABLE_FIVE_SIDE_LFE, "enable for five side with LFE", TYPE_INT, 19, 1 },
    {"PARA_CHMIX_ENABLE_SIX_LFE", PARA_CHMIX_ENABLE_SIX_LFE, "enable for six with LFE", TYPE_INT, 20, 1 },
    {"PARA_CHMIX_ENABLE_SIX_SIDE_LFE", PARA_CHMIX_ENABLE_SIX_SIDE_LFE, "enable for six side with LFE", TYPE_INT, 21, 1 },
    {"PARA_CHMIX_ENABLE_SEVEN_LFE", PARA_CHMIX_ENABLE_SEVEN_LFE, "enable for seven with LFE", TYPE_INT, 22, 1 },
};

int channel_mix_para[channel_mix_data_num] = {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//GainControl element parameter
#define GainControl_para_num 2
#define GainControl_data_num 17

PARA_INFO GainControl_para_info[GainControl_para_num] = {
    {"PARA_GAIN_CONTROL_SET_GAIN", PARA_GAIN_CONTROL_SET_GAIN, "set gain for all channel", TYPE_INT, 0, 16 },
    {"PARA_GAIN_CONTROL_SMOOTH_ORDER", PARA_GAIN_CONTROL_SMOOTH_ORDER, "set smooth order", TYPE_INT, 16, 1 },
};

int GainControl_para[GainControl_data_num] = {67108864,67108864,67108864,67108864,67108864,67108864,67108864,67108864,67108864,67108864,67108864,67108864,67108864,67108864,67108864,67108864,512};

//filter element parameter
#define filter_para_num 17
#define filter_data_num 96

PARA_INFO filter_para_info[filter_para_num] = {
    {"PARA_FILTER_SET_ORDER", PARA_FILTER_SET_ORDER, "set filter order", TYPE_INT, 0, 16 },
    {"PARA_FILTER_SET_CH0_PARA_TOOL", PARA_FILTER_SET_CH0_PARA_TOOL, "filter tool param", TYPE_INT, 16, 5 },
    {"PARA_FILTER_SET_CH1_PARA_TOOL", PARA_FILTER_SET_CH1_PARA_TOOL, "filter tool param", TYPE_INT, 21, 5 },
    {"PARA_FILTER_SET_CH2_PARA_TOOL", PARA_FILTER_SET_CH2_PARA_TOOL, "filter tool param", TYPE_INT, 26, 5 },
    {"PARA_FILTER_SET_CH3_PARA_TOOL", PARA_FILTER_SET_CH3_PARA_TOOL, "filter tool param", TYPE_INT, 31, 5 },
    {"PARA_FILTER_SET_CH4_PARA_TOOL", PARA_FILTER_SET_CH4_PARA_TOOL, "filter tool param", TYPE_INT, 36, 5 },
    {"PARA_FILTER_SET_CH5_PARA_TOOL", PARA_FILTER_SET_CH5_PARA_TOOL, "filter tool param", TYPE_INT, 41, 5 },
    {"PARA_FILTER_SET_CH6_PARA_TOOL", PARA_FILTER_SET_CH6_PARA_TOOL, "filter tool param", TYPE_INT, 46, 5 },
    {"PARA_FILTER_SET_CH7_PARA_TOOL", PARA_FILTER_SET_CH7_PARA_TOOL, "filter tool param", TYPE_INT, 51, 5 },
    {"PARA_FILTER_SET_CH8_PARA_TOOL", PARA_FILTER_SET_CH8_PARA_TOOL, "filter tool param", TYPE_INT, 56, 5 },
    {"PARA_FILTER_SET_CH9_PARA_TOOL", PARA_FILTER_SET_CH9_PARA_TOOL, "filter tool param", TYPE_INT, 61, 5 },
    {"PARA_FILTER_SET_CH10_PARA_TOOL", PARA_FILTER_SET_CH10_PARA_TOOL, "filter tool param", TYPE_INT, 66, 5 },
    {"PARA_FILTER_SET_CH11_PARA_TOOL", PARA_FILTER_SET_CH11_PARA_TOOL, "filter tool param", TYPE_INT, 71, 5 },
    {"PARA_FILTER_SET_CH12_PARA_TOOL", PARA_FILTER_SET_CH12_PARA_TOOL, "filter tool param", TYPE_INT, 76, 5 },
    {"PARA_FILTER_SET_CH13_PARA_TOOL", PARA_FILTER_SET_CH13_PARA_TOOL, "filter tool param", TYPE_INT, 81, 5 },
    {"PARA_FILTER_SET_CH14_PARA_TOOL", PARA_FILTER_SET_CH14_PARA_TOOL, "filter tool param", TYPE_INT, 86, 5 },
    {"PARA_FILTER_SET_CH15_PARA_TOOL", PARA_FILTER_SET_CH15_PARA_TOOL, "filter tool param", TYPE_INT, 91, 5 },
};

int filter_para[filter_data_num] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1,46334,6553600,0,0,1};

//MasterVolume element parameter
#define MasterVolume_para_num 2
#define MasterVolume_data_num 2

PARA_INFO MasterVolume_para_info[MasterVolume_para_num] = {
    {"PARA_MASTERVOL_SET_GAIN", PARA_MASTERVOL_SET_GAIN, "set master volume", TYPE_INT, 0, 1 },
    {"PARA_MASTERVOL_SMOOTH_ORDER", PARA_MASTERVOL_SMOOTH_ORDER, "set smooth order", TYPE_INT, 1, 1 },
};

int MasterVolume_para[MasterVolume_data_num] = {67108864,512};

//EQ element parameter
#define EQ_para_num 3
#define EQ_data_num 33

PARA_INFO EQ_para_info[EQ_para_num] = {
    {"PARA_EQ_SET_BAND", PARA_EQ_SET_BAND, "set EQ band number", TYPE_INT, 0, 16 },
    {"PARA_EQ_SET_MODE", PARA_EQ_SET_MODE, "set current mode", TYPE_INT, 16, 1 },
    {"PARA_EQ_SET_ENABLE", PARA_EQ_SET_ENABLE, "set ch enable", TYPE_INT, 17, 16 },
};

int EQ_para[EQ_data_num] = {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//delay element parameter
#define delay_para_num 1
#define delay_data_num 16

PARA_INFO delay_para_info[delay_para_num] = {
    {"PARA_DELAY_SET_DELAY", PARA_DELAY_SET_DELAY, "channel delay", TYPE_INT, 0, 16 },
};

int delay_para[delay_data_num] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//MBDRC element parameter
#define MBDRC_para_num 11
#define MBDRC_data_num 123

PARA_INFO MBDRC_para_info[MBDRC_para_num] = {
    {"PARA_MB_DRC_SET_NUM_BANDS", PARA_MB_DRC_SET_NUM_BANDS, "band number", TYPE_INT, 0, 1 },
    {"PARA_MB_DRC_SET_CROSSOVER_FREQ", PARA_MB_DRC_SET_CROSSOVER_FREQ, "crossover frequency", TYPE_INT, 1, 7 },
    {"PARA_MB_DRC_SET_DRC_TH", PARA_MB_DRC_SET_DRC_TH, "DRC threshold", TYPE_INT, 8, 40 },
    {"PARA_MB_DRC_SET_DRC_GAIN", PARA_MB_DRC_SET_DRC_GAIN, "DRC gain", TYPE_INT, 48, 40 },
    {"PARA_MB_DRC_SET_SB_GAIN", PARA_MB_DRC_SET_SB_GAIN, "sub-band gain", TYPE_INT, 88, 8 },
    {"PARA_MB_DRC_SET_SB_MODE", PARA_MB_DRC_SET_SB_MODE, "sub-band mode", TYPE_INT, 96, 8 },
    {"PARA_MB_DRC_SET_ATT_TIME", PARA_MB_DRC_SET_ATT_TIME, "attack time", TYPE_INT, 104, 8 },
    {"PARA_MB_DRC_SET_REL_TIME", PARA_MB_DRC_SET_REL_TIME, "release time", TYPE_INT, 112, 8 },
    {"PARA_MB_DRC_SET_LIM_TH", PARA_MB_DRC_SET_LIM_TH, "limiter threshold", TYPE_INT, 120, 1 },
    {"PARA_MB_DRC_SET_LIM_GAIN", PARA_MB_DRC_SET_LIM_GAIN, "limiter gain", TYPE_INT, 121, 1 },
    {"PARA_MB_DRC_SET_LIM_CONST", PARA_MB_DRC_SET_LIM_CONST, "limiter constant", TYPE_INT, 122, 1 },
};

int MBDRC_para[MBDRC_data_num] = {8,300,500,1000,2000,4000,8000,16000,-15360,-12800,-10240,-7680,0,-15360,-12800,-10240,-7680,0,-15360,-12800,-10240,-7680,0,-15360,-12800,-10240,-7680,0,-15360,-12800,-10240,-7680,0,-15360,-12800,-10240,-7680,0,-15360,-12800,-10240,-7680,0,-15360,-12800,-10240,-7680,0,6144,6144,6144,6144,0,6144,6144,6144,6144,0,6144,6144,6144,6144,0,6144,6144,6144,6144,0,6144,6144,6144,6144,0,6144,6144,6144,6144,0,6144,6144,6144,6144,0,6144,6144,6144,6144,0,-10240,-7680,-5120,-768,-768,0,0,0,1,1,1,1,1,1,1,1,64,64,32,32,32,164,164,164,6400,6400,16400,16400,16400,16400,16400,16400,32767,32767,4};

const ELEMENT_TYPE default_ele_pool[] = {
  {
    "input",
    0x0000,
    "Turn input data to supported format",
    PPC_TRUE,
    PPC_FALSE,
    14,
    268,
    0x80ff,
    0x80ff,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    input_para_num,
    input_para_info,
    input_data_num,
    input_para
  },
  {
    "output",
    0x7fff,
    "Turn data to setting output format",
    PPC_TRUE,
    PPC_FALSE,
    5,
    176,
    0x80ff,
    0x80ff,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    output_para_num,
    output_para_info,
    output_data_num,
    output_para
  },
  {
    "add",
    0x0002,
    "weighted add multiple input to one output",
    PPC_TRUE,
    PPC_FALSE,
    12,
    76,
    0x80ff,
    0x0004,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    add_para_num,
    add_para_info,
    add_data_num,
    add_para
  },
  {
    "channel_mix",
    0x0004,
    "mix channel with channel matrix",
    PPC_TRUE,
    PPC_FALSE,
    7,
    3256,
    0x80ff,
    0x80ff,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    channel_mix_para_num,
    channel_mix_para_info,
    channel_mix_data_num,
    channel_mix_para
  },
  {
    "GainControl",
    0x0005,
    "Apply gain on each channel",
    PPC_TRUE,
    PPC_TRUE,
    2,
    340,
    0x80ff,
    0x80ff,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    GainControl_para_num,
    GainControl_para_info,
    GainControl_data_num,
    GainControl_para
  },
  {
    "filter",
    0x0006,
    "filter",
    PPC_TRUE,
    PPC_TRUE,
    4,
    37224,
    0x80ff,
    0x80ff,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    filter_para_num,
    filter_para_info,
    filter_data_num,
    filter_para
  },
  {
    "MasterVolume",
    0x0001,
    "Apply master volume on all channels",
    PPC_TRUE,
    PPC_TRUE,
    2,
    340,
    0x80ff,
    0x80ff,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    MasterVolume_para_num,
    MasterVolume_para_info,
    MasterVolume_data_num,
    MasterVolume_para
  },
  {
    "EQ",
    0x0007,
    "equalizer",
    PPC_TRUE,
    PPC_TRUE,
    19,
    102700,
    0x80ff,
    0x80ff,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    EQ_para_num,
    EQ_para_info,
    EQ_data_num,
    EQ_para
  },
  {
    "delay",
    0x000d,
    "delay for each channel",
    PPC_TRUE,
    PPC_TRUE,
    1,
    2396,
    0x80ff,
    0x80ff,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    delay_para_num,
    delay_para_info,
    delay_data_num,
    delay_para
  },
  {
    "MBDRC",
    0x000f,
    "Multi-band DRC",
    PPC_TRUE,
    PPC_FALSE,
    60,
    90032,
    0x0003,
    0x0003,
    AUDIO_SAMPLE_RATE_32000|AUDIO_SAMPLE_RATE_44100|AUDIO_SAMPLE_RATE_48000,
    MBDRC_para_num,
    MBDRC_para_info,
    MBDRC_data_num,
    MBDRC_para
  },
};
const int default_ele_type_num=sizeof(default_ele_pool)/sizeof(ELEMENT_TYPE);

#endif

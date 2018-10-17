#ifndef COMMON_H
#define COMMON_H
#include "audio_format.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#define print_log
#define print_must printf

unsigned int map_ch_index(AUDIO_CHANNEL ch);
unsigned int map_ch_buf(unsigned int ch_all, AUDIO_CHANNEL ch);
unsigned int ch_num(unsigned int ch_all);

#endif

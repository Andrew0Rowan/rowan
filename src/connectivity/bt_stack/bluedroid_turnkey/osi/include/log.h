/******************************************************************************
 *
 *  Copyright (C) 2014 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#pragma once

/*
 * TODO(armansito): Work-around until we figure out a way to generate logs in a
 * platform-independent manner.
 */
#if defined(OS_GENERIC)

/* syslog didn't work well here since we would be redefining LOG_DEBUG. */
#include <stdio.h>

#ifdef MTK_BLUEDROID_PATCH
#include "mdroid_buildcfg.h"
#endif

#if defined(MTK_LINUX_MW_STACK_LOG2FILE) && (MTK_LINUX_MW_STACK_LOG2FILE == TRUE)
#if defined(MTK_BT_SYS_LOG)

#include <syslog.h>
extern unsigned int ui4_enable_all_log;

/*control by /data/log_all file exist or not*/
#define printf(format, args...)                \
do{                                            \
    if (0 == ui4_enable_all_log)               \
    {                                          \
        syslog(LOG_WARNING, format, ##args);   \
    }                                          \
    else                                       \
    {                                          \
        syslog(LOG_WARNING, format, ##args);   \
        printf(format, ##args);                \
    }                                          \
}while(0)
#endif

enum {
    LOG_LVL_ERROR,
    LOG_LVL_WARN,
    LOG_LVL_INFO,
    LOG_LVL_DEBUG,
    LOG_LVL_VERBOSE,

    LOG_LVL_END,
};

int log_getLevel();

void save_log2file_init(bool log2file, bool save_last,
    const char *log_path, int log_level, bool only2file);
void log_write(const char *format, ...);
void log_deinit();
#define LOGWRAPPER(tag, fmt, args...) log_write("%s: " fmt "\n", tag, ## args)
#define LOG_VERBOSE(...) do{if (log_getLevel() >= LOG_LVL_VERBOSE)    LOGWRAPPER(__VA_ARGS__);}while(0)
#define LOG_DEBUG(...) do{if (log_getLevel() >= LOG_LVL_DEBUG)    LOGWRAPPER(__VA_ARGS__);}while(0)
#define LOG_INFO(...) do{if (log_getLevel() >= LOG_LVL_INFO)    LOGWRAPPER(__VA_ARGS__);}while(0)
#define LOG_WARN(...) do{if (log_getLevel() >= LOG_LVL_WARN)    LOGWRAPPER(__VA_ARGS__);}while(0)
#define LOG_ERROR(...) do{if (log_getLevel() >= LOG_LVL_ERROR)    LOGWRAPPER(__VA_ARGS__);}while(0)
/*end of MTK_LINUX_MW_STACK_LOG2FILE*/
#elif defined(MTK_LINUX_STACK_LOG2FILE) && (MTK_LINUX_STACK_LOG2FILE == TRUE)
#if defined(MTK_BT_SYS_LOG)

#include <syslog.h>
extern unsigned int ui4_enable_all_log;

/*control by /data/log_all file exist or not*/
#define printf(format, args...)                \
do{                                            \
    if (0 == ui4_enable_all_log)               \
    {                                          \
        syslog(LOG_WARNING, format, ##args);   \
    }                                          \
    else                                       \
    {                                          \
        syslog(LOG_WARNING, format, ##args);   \
        printf(format, ##args);                \
    }                                          \
}while(0)
#endif

void log_init();
void log_write(const char *format, ...);
void log_deinit();
#define LOGWRAPPER(tag, fmt, args...) log_write("%s: " fmt "\n", tag, ## args)
/*end of MTK_LINUX_STACK_LOG2FILE*/
#elif defined(MTK_COMMON) && (MTK_COMMON == TRUE)
#define LOGWRAPPER(tag, fmt, args...) printf("%s: " fmt "\n", tag, ## args)
#else
#define LOGWRAPPER(tag, fmt, args...) fprintf(stderr, "%s: " fmt "\n", tag, ## args)
#endif/*#if defined(MTK_LINUX_MW_STACK_LOG2FILE) && (MTK_LINUX_MW_STACK_LOG2FILE == TRUE)*/

#if defined(MTK_LINUX_MW_STACK_LOG2FILE) && (MTK_LINUX_MW_STACK_LOG2FILE == TRUE)
#else
#define LOG_VERBOSE(...) LOGWRAPPER(__VA_ARGS__)
#define LOG_DEBUG(...) LOGWRAPPER(__VA_ARGS__)
#define LOG_INFO(...) LOGWRAPPER(__VA_ARGS__)
#define LOG_WARN(...) LOGWRAPPER(__VA_ARGS__)
#define LOG_ERROR(...) LOGWRAPPER(__VA_ARGS__)
#endif

#else  /* !defined(OS_GENERIC) */

#include <cutils/log.h>

#if LOG_NDEBUG
#define LOG_VERBOSE(...) ((void)0)
#else  // LOG_NDEBUG
#define LOG_VERBOSE(tag, fmt, args...) ALOG(LOG_VERBOSE, tag, fmt, ## args)
#endif  // !LOG_NDEBUG
#define LOG_DEBUG(tag, fmt, args...)   ALOG(LOG_DEBUG, tag, fmt, ## args )
#define LOG_INFO(tag, fmt, args...)    ALOG(LOG_INFO, tag, fmt, ## args)
#define LOG_WARN(tag, fmt, args...)    ALOG(LOG_WARN, tag, fmt, ## args)
#define LOG_ERROR(tag, fmt, args...)   ALOG(LOG_ERROR, tag, fmt, ## args)

#endif  /* defined(OS_GENERIC) */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include "stack_config.h"
#include "osi/include/osi.h"
//#if defined(MTK_LINUX_STACK_LOG2FILE) && (MTK_LINUX_STACK_LOG2FILE == TRUE)
#include "osi/include/log.h"
//#endif

typedef void (*bg_log_func)(char *log_str);

bg_log_func s_log_cb = NULL;

static int g_log_fd = INVALID_FD;
static bool fg_only2file = true;
static char g_ts_buffer[24];

#define TIMESTAMP_TYPE_FILE_NAME   1
#define TIMESTAMP_TYPE_LOG_PREFIX  2

static char* log_timestamp(int type)
{
    char buffer[24] = {0};
    struct timeval tv;

    gettimeofday(&tv, NULL);
    struct tm *ltime = localtime(&tv.tv_sec);

    memset(g_ts_buffer, 0, sizeof(g_ts_buffer));
    if (TIMESTAMP_TYPE_FILE_NAME == type)
    {
        strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", ltime);
        snprintf(g_ts_buffer, sizeof(g_ts_buffer), "%s%03ld", buffer, tv.tv_usec / 1000);
    }
    else
    {
        int today_sec = tv.tv_sec % 86400;
        int hour = today_sec / 3600;
        int min = today_sec % 3600 / 60;
        int sec = today_sec % 60;
        snprintf(g_ts_buffer, sizeof(g_ts_buffer), "<%02d:%02d:%02d.%06ld>", hour, min, sec, tv.tv_usec);
    }

    return g_ts_buffer;
}

#if defined(MTK_LINUX_MW_STACK_LOG2FILE) && (MTK_LINUX_MW_STACK_LOG2FILE == TRUE)
EXPORT_SYMBOL int g_log_level = LOG_LVL_WARN;
int log_getLevel()
{
    return g_log_level;
}

void log_setLevel(unsigned int lvl)
{
    if (lvl > LOG_LVL_VERBOSE)
    {
        g_log_level = LOG_LVL_VERBOSE;
        return;
    }

    g_log_level = lvl;
    return;
}

EXPORT_SYMBOL void save_log2file_init(bool log2file, bool save_last,
    const char *log_path, int log_level, bool only2file)
{
    char last_log_path[PATH_MAX] = {0};

    log_setLevel(log_level);

    if (false == log2file)
    {
        g_log_fd = INVALID_FD;
        printf("%d@%s no need save log2file\n", __LINE__, __func__);
        if (0 == access(log_path, F_OK))
        {
            unlink(log_path);
            printf("%d@%s remove the existed log file successful\n", __LINE__, __func__);
        }
        return;
    }

    if (NULL == log_path)
    {
        printf("%d@%s null file name\n", __LINE__, __func__);
        return;
    }

    if (true == save_last)
    {
        snprintf(last_log_path, PATH_MAX, "%s.%s", log_path, log_timestamp(TIMESTAMP_TYPE_FILE_NAME));
        if (!rename(log_path, last_log_path) && errno != ENOENT)
        {
            printf("%s unable to rename '%s' to '%s': %s\n", __func__, log_path, last_log_path, strerror(errno));
        }
    }
    printf("%s %d:%d@%s\n", log_path, g_log_fd, __LINE__, __func__);

    g_log_fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (INVALID_FD == g_log_fd)
    {
        printf("%s unable to open '%s': %s\n", __func__, log_path, strerror(errno));
    }
    printf("%d@%s fd:%d\n", __LINE__, __func__, g_log_fd);

    fg_only2file = only2file;
}

#else
EXPORT_SYMBOL void log_init()
{
    const char *log_path = stack_config_get_interface()->get_btstack_log_path();
    char last_log_path[PATH_MAX] = {0};

    if (false == stack_config_get_interface()->get_btstack_log2file_turned_on())
    {
        g_log_fd = INVALID_FD;
        printf("%d@%s\n", __LINE__, __func__);
        return;
    }

    if (true == stack_config_get_interface()->get_btstack_should_save_last())
    {
        snprintf(last_log_path, PATH_MAX, "%s.%s", log_path, log_timestamp(TIMESTAMP_TYPE_FILE_NAME));
        if (!rename(log_path, last_log_path) && errno != ENOENT)
        {
            printf("%s unable to rename '%s' to '%s': %s\n", __func__, log_path, last_log_path, strerror(errno));
        }
    }
    printf("%s %d:%d@%s\n", log_path, g_log_fd, __LINE__, __func__);

    g_log_fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (INVALID_FD == g_log_fd)
    {
        printf("%s unable to open '%s': %s\n", __func__, log_path, strerror(errno));
    }
    printf("%d@%s %d\n", __LINE__, __func__, g_log_fd);
}
#endif

EXPORT_SYMBOL void log_reg(void (*log_cb)(char *log_str))
{
    s_log_cb = log_cb;
}

EXPORT_SYMBOL void log_write(const char *format, ...)
{
    va_list args;
    char msg[500] = {0};
    char msg_ts[500] = {0};

    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);
    if (g_log_fd != INVALID_FD)
    {
        snprintf(msg_ts, sizeof(msg_ts), "%s %s", log_timestamp(TIMESTAMP_TYPE_LOG_PREFIX), msg);
        if (false == fg_only2file)
        {
            printf("%s", msg_ts);
        }
        write(g_log_fd, msg_ts, strlen(msg_ts));
    }
    else
    {
        if (s_log_cb)
        {
            snprintf(msg_ts, sizeof(msg_ts), "%s %s", log_timestamp(TIMESTAMP_TYPE_LOG_PREFIX), msg);
            s_log_cb(msg_ts);
        }
        else
        {
            printf("%s %s", log_timestamp(TIMESTAMP_TYPE_LOG_PREFIX), msg);
        }
    }
}

EXPORT_SYMBOL void log_deinit()
{
    printf("%d@%s %d\n", __LINE__, __func__, g_log_fd);
    if (g_log_fd != INVALID_FD)
    {
        close(g_log_fd);
        g_log_fd = INVALID_FD;
    }
}


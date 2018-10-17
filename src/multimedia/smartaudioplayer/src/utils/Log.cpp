#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "pthread.h"
#include "sched.h"
#include <syslog.h>

#include "Log.h"

static int log_enable = 0;
#define LOGGING_FUNC_SIZE 512

const char* GetUserString(int userId)
{
    switch(userId)
    {
    case SAP_TAG:
        return "<SAPlayer>";
    case SAP_DRIVER_TAG:
        return "<SAPlayer-DRIVER>";
    case SAP_DECODE_TAG:
        return "<SAPlayer-FFDECODER>";
    case SAP_DEMUXER_TAG:
        return "<SAPlayer-DEMUXER>";
    case SAP_STREAMER_TAG:
        return "<SAPlayer-STREAMER>";
    case SAP_RENDER_TAG:
        return "<SAPlayer-RENDER>";

    default:
        return "<SAPlayer>";
    }

    return "<SAPlayer>";
}

Trace::Trace(const int userId, const int level, const char* func) :
mLine(0),
mUserId(userId),
mLevel(level),
mFunc(func),
mEnable(true)
{
    /*init output log type*/
    if(access("/data/log_all",0)==0)
    {
        log_enable = 1;
    } else
    {
        log_enable = 0;
    }
}

Trace::Trace(const int userId, const int level, const char* func, int line) :
mLine(line),
mUserId(userId),
mLevel(level),
mFunc(func),
mEnable(true)
{
    /*init output log type*/
    if(access("/data/log_all",0)==0)
    {
        log_enable = 1;
    } else
    {
        log_enable = 0;
    }

    log_enable = 1;
}


Trace::~Trace()
{
    if (mEnable && (0 == mLine))
    {
        if (0 == log_enable) {
            syslog(LOG_WARNING, "%s LEAVING %s ", GetUserString(mUserId), mFunc);
        } else {
            syslog(LOG_WARNING, "%s LEAVING %s ", GetUserString(mUserId), mFunc);
            printf("%s ", GetUserString(mUserId));
            printf("LEAVING %s", mFunc);
            printf("\n");
        }
    }
}

void Trace::LogEntry()
{
    if (mEnable) {
        if (0 == log_enable) {
            syslog(LOG_WARNING, "%s ENTERING %s ", GetUserString(mUserId), mFunc);
        } else {
            syslog(LOG_WARNING, "%s ENTERING %s ", GetUserString(mUserId), mFunc);
            printf("%s ", GetUserString(mUserId));
            printf("ENTERING %s", mFunc);
            printf("\n");
        }
    }
}

void Trace::LogEntry(const char* fmt, ...)
{
    if (mEnable) {
        if (0 == log_enable) {
            va_list va;
            va_start(va, fmt);
            char buffer[LOGGING_FUNC_SIZE];
            vsnprintf(buffer, LOGGING_FUNC_SIZE, fmt, va);
            syslog(LOG_WARNING, "%s ENTERING %s %s ", GetUserString(mUserId), mFunc, buffer);
            va_end(va);
        } else {
            printf("%s ", GetUserString(mUserId));
            printf("ENTERING %s ", mFunc);

            // printf fmt
            {
            va_list va;
            va_start(va, fmt);
            char buffer[LOGGING_FUNC_SIZE];
            vsnprintf(buffer, LOGGING_FUNC_SIZE, fmt, va);
            syslog(LOG_WARNING, "%s ENTERING %s %s ", GetUserString(mUserId), mFunc, buffer);
            printf("%s", buffer);
            va_end(va);
            }

            printf("\n");
        }
    }
}

void Trace::LogExit(const char* fmt, ...)
{
    if (mEnable) {
        if (0 == log_enable) {
            va_list va;
            va_start(va, fmt);
            char buffer[LOGGING_FUNC_SIZE];
            vsnprintf(buffer, LOGGING_FUNC_SIZE, fmt, va);
            syslog(LOG_WARNING, "%s LEAVING %s %s ", GetUserString(mUserId), mFunc, buffer);
            va_end(va);
        } else {
            printf("%s ", GetUserString(mUserId));
            printf("LEAVING %s ", mFunc);

            // printf fmt
            {
            va_list va;
            va_start(va, fmt);
            char buffer[LOGGING_FUNC_SIZE];
            vsnprintf(buffer, LOGGING_FUNC_SIZE, fmt, va);
            syslog(LOG_WARNING, "%s LEAVING %s %s ", GetUserString(mUserId), mFunc, buffer);
            printf("%s", buffer);
            va_end(va);
            }

            printf("\n");
        }
    }
}



void Trace::Log(const char* fmt, ...)
{
    if (mEnable) {
        if (0 == log_enable) {
            va_list va;
            va_start(va, fmt);
            char buffer[LOGGING_FUNC_SIZE];
            vsnprintf(buffer, LOGGING_FUNC_SIZE, fmt, va);
            syslog(LOG_WARNING, "%s %s(%d) %s ", GetUserString(mUserId), mFunc, mLine, buffer);
            va_end(va);
        } else {
            printf("%s ", GetUserString(mUserId));
            printf("%s(%d) ", mFunc, mLine);

            // printf fmt
            {
            va_list va;
            va_start(va, fmt);
            char buffer[LOGGING_FUNC_SIZE];
            vsnprintf(buffer, LOGGING_FUNC_SIZE, fmt, va);
            syslog(LOG_WARNING, "%s %s(%d) %s ", GetUserString(mUserId), mFunc, mLine, buffer);
            printf("%s", buffer);
            va_end(va);
            }

            printf("\n");
        }
    }
}


#if 0
int main(int argc, char **argv)
{
		int i = 100;
    SAP_LOGENTRY(SAP_TAG, 1, ("I am %d", i));


    SAP_LOG(SAP_TAG, 1, ("I am %d", i));
    SAP_LOG(SAP_DECODE_TAG, 1, ("I am %d", i));

    SAP_LOG(SAP_DEMUXER_TAG, 1, ("I am %d", i));
}

#endif


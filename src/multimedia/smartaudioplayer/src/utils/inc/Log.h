#ifndef __SMP_LOG_H__
#define __SMP_LOG_H__

/**
 * Print no output.
 */
#define SAP_LOG_QUIET    -8

/**
 * Something went really wrong and we will crash now.
 */
#define SAP_LOG_PANIC     0

/**
 * Something went wrong and recovery is not possible.
 * For example, no header was found for a format which depends
 * on headers or an illegal combination of parameters is used.
 */
#define SAP_LOG_FATAL     8

/**
 * Something went wrong and cannot losslessly be recovered.
 * However, not all future data is affected.
 */
#define SAP_LOG_ERROR    16

/**
 * Something somehow does not look correct. This may or may not
 * lead to problems. An example would be the use of '-vstrict -2'.
 */
#define SAP_LOG_WARNING  24

/**
 * Standard information.
 */
#define SAP_LOG_INFO     32

/**
 * Detailed information.
 */
#define SAP_LOG_VERBOSE  40

/**
 * Stuff which is only useful for libav* developers.
 */
#define SAP_LOG_DEBUG    48

/**
 * Extremely verbose debugging, useful for libav* development.
 */
#define SAP_LOG_TRACE    56


enum SAP_LOG_USER_ID
{
	SAP_TAG,
    SAP_DRIVER_TAG,
	SAP_DECODE_TAG,
	SAP_SOURCE_TAG,
	SAP_DEMUXER_TAG,
	SAP_STREAMER_TAG,
	SAP_RENDER_TAG,
};




class Trace
{
public :
    Trace(const int userId, const int level, const char* func);
    Trace(const int userId, const int level, const char* func, int line);

    /// The method scope is left, log messages
    ~Trace();
    
    bool isEnabled()
    {
    		return mEnable;
    }

    /// Log method entry without a message
    void LogEntry();

    /// Log method entry messages
    void LogEntry(const char* fmt, ...) __attribute__((format(printf, 2, 3)));

    /// The method scope is left, log messages
    void LogExit(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
    
    /// The method scope is left, log messages
    void Log(const char* fmt, ...) __attribute__((format(printf, 2, 3)));

private:
    int mLine; 			// log line, __LINE__
    int mUserId;			// for user module, SAP_LOG_USER_ID
    int mLevel;  			// for future use
    const char* mFunc; // loc function, __FUNCTION__ or __PRETTY_FUNCTION__
    bool mEnable;			// for future use

private:
    /// Copy Constructor
    Trace(const Trace& tr);
    
    /// Operator =
    Trace& operator=(const Trace& tr);
};


// it will print a log in function entry
// it will print the function name automatic
// and it will print a log in function exit automatic
#define SAP_LOGENTRY(cid, mask, args) \
    Trace ___trace((cid), (mask), __FUNCTION__); \
    if (___trace.isEnabled()) { \
        ___trace.LogEntry args; \
    } \
    do { } while (false)


// print a log, it will print the function name and line 
#define SAP_LOG(cid, mask, args) \
    { \
    Trace ___trace((cid), (mask), __FUNCTION__, __LINE__); \
    if (___trace.isEnabled()) { \
        ___trace.Log args; \
    } \
    } \
    do { } while (false)






#endif // #ifndef __SMP_LOG_H__


/*
 * Logger
 */

#include "NtcLogger.h"

#include "Utils.h"

namespace fw {

void Logger::syslog(int logLevel, const char *file, int line, const char *strFmt, ...) const noexcept
{
    va_list ap;

    std::string msg;

    va_start(ap, strFmt);

    try {
        msg = fw::utils::vformat(strFmt, ap);

        for (const auto &lmsg : fw::utils::split(msg)) {
            ::syslog(logLevel, fw::utils::format("%s:%d: %s", file, line, lmsg.c_str()).c_str());
        }
    }
    catch (const std::bad_alloc &e) {
        ::syslog(LOG_ERR, "memory allocation failure for syslog");
    }

    ::va_end(ap);
}

void Logger::setup(int maxLogLevel_)
{
    syslog(LOG_NOTICE, __FILE__, __LINE__, "change log level from %d to %d", maxLogLevel, maxLogLevel_);

    maxLogLevel = maxLogLevel_;

    ::setlogmask(LOG_UPTO(maxLogLevel));

    for (int i = LOG_EMERG; i <= LOG_DEBUG; ++i)
        syslog(i, __FILE__, __LINE__, "loglevel checkpoint (level=%d)", i);
}

Logger::Logger() : name(program_invocation_short_name), maxLogLevel(LOG_DEBUG)
{
    ::openlog(name.c_str(), LOG_PID, LOG_DAEMON);
    ::setlogmask(LOG_UPTO(maxLogLevel));

    syslog(LOG_NOTICE, __FILE__, __LINE__, "start...");
}

Logger::~Logger()
{
    syslog(LOG_NOTICE, __FILE__, __LINE__, "finished.");
    ::closelog();
}

}; // namespace fw

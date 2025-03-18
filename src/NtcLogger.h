#ifndef NTCLOGGER_H_12554303012024
#define NTCLOGGER_H_12554303012024

/*
 * Logger
 */

#include <syslog.h>

#include "NtcUtils.h"

#define log(logLevel, ...) fw::Logger::getInstance().syslog(logLevel, __FILE__, __LINE__, __VA_ARGS__)

namespace fw {

class Logger : public fw::utils::Singleton<Logger>
{
  public:
    Logger();
    ~Logger();

    void setup(int maxLogLevel_);
    void syslog(int logLevel, const char *file, int line, const char *strFmt, ...) const noexcept __attribute__((format(printf, 5, 6)));

  private:
    std::string name;
    int maxLogLevel;
};

}; // namespace fw

#endif // NTCLOGGER_H_12554303012024

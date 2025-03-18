/*
 * Utilities.
 */

#include "NtcUtils.h"

namespace fw {
namespace utils {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
std::string vformat(const char *format, va_list args) noexcept
{
    char *buf = nullptr;

    ScopeDeletor bufDeletor([&buf] {
        ::free(buf);
    });

    int n = ::vasprintf(&buf, format, args);

    std::string retVal;

    if (n > 0) {
        retVal = std::string(buf, n);
    }

    return std::string(retVal);
}

template <typename T>
std::basic_string<T> format(const T *format, ...) noexcept
{
    va_list ap;
    ::va_start(ap, format);
    std::basic_string<T> retval(vformat(format, ap));
    ::va_end(ap);

    return retval;
}

template std::string format(const char *format, ...) noexcept;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Parse (split) a string in C++ using delimiter
std::vector<std::string> split(std::string_view str, const char delim)
{
    std::vector<std::string> retVal;

    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string_view::npos) {
        retVal.emplace_back(str.substr(previous, current - previous));
        previous = current + sizeof(delim);
        current = str.find(delim, previous);
    }
    retVal.emplace_back(str.substr(previous, current - previous));

    return retVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Concatenate string vector into string using delimiter
std::string concat(const std::vector<std::string> &v, const std::string &sep)
{
    std::string retVal;
    for (auto s = v.cbegin(), e = v.cend(), i = s; i != e; ++i) {
        if (i != s) {
            retVal.append(sep);
        }
        retVal.append(*i);
    }

    return retVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string replace(const std::string &str, const std::string_view &toSearch, const std::string_view &replaceStr)
{
    std::string retVal = str;

    size_t pos = retVal.find(toSearch);

    while (pos != std::string::npos) {
        retVal.replace(pos, toSearch.size(), replaceStr);
        pos = retVal.find(toSearch, pos + replaceStr.size());
    }

    return retVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SysCmdResult SysCmd::exec(const std::string &command) {
    int exitCode = 0;
    std::array<char, 1024> buffer {};
    std::string result;

    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        throw std::runtime_error("popen() failed!");
    }
    try {
        std::size_t bytesread;
        while ((bytesread = std::fread(buffer.data(), sizeof(buffer.at(0)), sizeof(buffer), pipe)) != 0) {
            result += std::string(buffer.data(), bytesread);
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    exitCode = WEXITSTATUS(pclose(pipe));
    return SysCmdResult{exitCode, result};
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}; // namespace utils
}; // namespace fw

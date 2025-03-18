#ifndef NTCUTILS_H_12022803012024
#define NTCUTILS_H_12022803012024

/*
 * Utilities.
 */

#include <string>
#include <functional>
#include <mutex>
#include <stdarg.h>
#include <map>
#include <string_view>

namespace fw {
namespace utils {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// default value map class
template <typename TKey, typename TValue>
    class DefValueMap : public std::map<TKey, TValue>
{
    public:
        using std::map<TKey, TValue>::map;

        /**
         * @brief Get the value by key.
         *
         * @param key key to search for.
         * @param def default value when the key does not exist.
         * @return TValue value when the key is found. Otherwise, def.
         */
        TValue get(const TKey &key, const TValue &def) const
        {
            TValue retVal = def;

            auto it = std::map<TKey, TValue>::find(key);

            if (it != std::map<TKey, TValue>::end()) {
                retVal = it->second;
            }

            return retVal;
        }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::basic_string<T> vformat(const T *format, va_list args) noexcept;

template <typename T>
std::basic_string<T> format(const T *format, ...) noexcept __attribute__((format(printf, 1, 2)));

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Scope deletor
class ScopeDeletor
{
  public:
    typedef std::function<void(void)> deletor_t;

    ScopeDeletor(const ScopeDeletor &o) = delete;
    ScopeDeletor(ScopeDeletor &&o) = delete;
    ScopeDeletor &operator=(const ScopeDeletor &o) = delete;
    ScopeDeletor &operator=(ScopeDeletor &&o) = delete;

    /**
     * @brief Construct a new Scope Deletor object
     *
     * @param deletor_ functor that is called when the scope is deleted.
     */
    explicit ScopeDeletor(deletor_t deletor_) noexcept : deletor(deletor_) {}
    ~ScopeDeletor()
    {
        deletor();
    }

  private:
    deletor_t deletor;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class Singleton
{
  public:
    /**
     * @brief Get the singleton object.
     *
     * @return T& singleton object.
     */
    static T &getInstance()
    {
        static T instance;

        return instance;
    }

    Singleton() = default;
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Parse (split) a string in C++ using delimiter
std::vector<std::string> split(std::string_view str, const char delim = '\n');

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Concatenate string vector into string using delimiter
std::string concat(const std::vector<std::string> &v, const std::string &sep = "");

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string replace(const std::string &str, const std::string_view &toSearch, const std::string_view &replaceStr);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// System command result
struct SysCmdResult {
    int rc; // return code
    std::string output;

    SysCmdResult() : rc(-1), output("") { }
    SysCmdResult(int _returnCode, const std::string &_output) : rc(_returnCode), output(_output) { }
    SysCmdResult(int _returnCode, const char *_output) : SysCmdResult(_returnCode, std::string(_output)) { }

    bool operator==(const SysCmdResult &rhs) const {
        return output == rhs.output &&
            rc == rhs.rc;
    }
    bool operator!=(const SysCmdResult &rhs) const {
        return !(rhs == *this);
    }
};

// System command
class SysCmd {
  public:
    /**
     * Execute system command and get STDOUT result.
     * Regular system() only gives back exit status, this gives back output as well.
     * @param command system command to execute
     * @return commandResult containing STDOUT (not stderr) output & rc(return code)
     * of command. Empty if command failed (or has no output). If you want stderr,
     * use shell redirection (2&>1).
     */
    static SysCmdResult exec(const std::string &command);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}; // namespace utils
}; // namespace fw

#endif // NTCUTILS_H_12022803012024

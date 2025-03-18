#ifndef NTCUCI_H_10431830012024
#define NTCUCI_H_10431830012024

/*
 * Openwrt UCI C++ wrapper
 */

#include "NtcUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <libubox/blobmsg_json.h>
#include <libubox/blob.h>
#include <libubox/blobmsg.h>

#ifdef __cplusplus
}
#endif

#include <libubus.h>

#include <map>
#include <set>
#include <functional>
#include <bitset>

#include "json/json.h"

namespace uci {

/*### Classes for UCI name ###*/
///////////////////////////////////////////////////////////////////////////////////////////////
class UciNameTypeBase
{
  public:
    enum class NameType : std::uint8_t
    {
        UNKNOWN,
        PACKAGE,
        SECTION,
        OPTION
    };

    UciNameTypeBase() = delete;

    std::string getPackageName() const noexcept { return _package; }
    std::string getSectionName() const noexcept { return _section; }
    std::string getOptionName() const noexcept { return _option; }
    std::string getFullpath() const noexcept { return _fullpath; }
    NameType getUciNameType() const noexcept { return _nameType; }

  protected:
    UciNameTypeBase(const std::string &pkg, const std::string &sec, const std::string &opt) noexcept
        : _nameType(NameType::UNKNOWN), _fullpath(), _package(pkg), _section(sec), _option(opt) { }
    NameType _nameType;
    std::string _fullpath;

  private:
    std::string _package;
    std::string _section;
    std::string _option;
};

class UciPkgNameType : public UciNameTypeBase
{
  public:
    UciPkgNameType(const std::string &pkg): UciNameTypeBase(pkg, "", "")
    {
        _nameType = NameType::PACKAGE;
        _fullpath = pkg;
    }
};

class UciSecNameType : public UciNameTypeBase
{
  public:
    UciSecNameType(const std::string &pkg, const std::string &sec): UciNameTypeBase(pkg, sec, "")
    {
        _nameType = NameType::SECTION;
        _fullpath = pkg + "." + sec;
    }
};

class UciOptNameType : public UciNameTypeBase
{
  public:
    UciOptNameType(const std::string &pkg, const std::string &sec, const std::string &opt): UciNameTypeBase(pkg, sec, opt)
    {
        _nameType = NameType::OPTION;
        _fullpath = pkg + "." + sec + "." + opt;
    }
};
///////////////////////////////////////////////////////////////////////////////////////////////

/*### Classes for UCI value ###*/
///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min)                                       \
  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif

#ifndef __THROW
# if defined __cplusplus && (__GNUC_PREREQ (2,8) || __clang_major__ >= 4)
#  define __THROW       throw ()
# else
#  define __THROW
# endif
#endif

#ifndef __glibc_clang_has_attribute
#if defined __clang__ && defined __has_attribute
# define __glibc_clang_has_attribute(name) __has_attribute (name)
#else
# define __glibc_clang_has_attribute(name) 0
#endif
#endif

#ifndef __nonnull
# if __GNUC_PREREQ (3,3) || __glibc_clang_has_attribute (__nonnull__)
#  define __nonnull(params) __attribute__ ((__nonnull__ params))
# else
#  define __nonnull(params)
# endif
#endif

/* ========================================================================================= */
template <typename T>
struct IntegerTraits {};

template <>
struct IntegerTraits<unsigned short int> : public std::numeric_limits<unsigned short int> {
    static unsigned short int strToT(const char *__restrict __nptr, char **__restrict __endptr, int __base) __THROW __nonnull((1))
    {
        return ::strtol(__nptr, __endptr, __base);
    }
};

template <>
struct IntegerTraits<int> : public std::numeric_limits<int> {
    static int strToT(const char *__restrict __nptr, char **__restrict __endptr, int __base) __THROW __nonnull((1))
    {
        return ::strtol(__nptr, __endptr, __base);
    }
};

template <>
struct IntegerTraits<unsigned int> : public std::numeric_limits<unsigned int> {
    static unsigned int strToT(const char *__restrict __nptr, char **__restrict __endptr, int __base) __THROW __nonnull((1))
    {
        return ::strtoul(__nptr, __endptr, __base);
    }
};

template <>
struct IntegerTraits<long int> : public std::numeric_limits<long int> {
    static long int strToT(const char *__restrict __nptr, char **__restrict __endptr, int __base) __THROW __nonnull((1))
    {
        return ::strtol(__nptr, __endptr, __base);
    }
};

template <>
struct IntegerTraits<unsigned long> : public std::numeric_limits<unsigned long> {
    static unsigned long int strToT(const char *__restrict __nptr, char **__restrict __endptr, int __base) __THROW __nonnull((1))
    {
        return ::strtoul(__nptr, __endptr, __base);
    }
};

template <>
struct IntegerTraits<long long int> : public std::numeric_limits<long long int> {
    static long long int strToT(const char *__restrict __nptr, char **__restrict __endptr, int __base) __THROW __nonnull((1))
    {
        return ::strtoll(__nptr, __endptr, __base);
    }
};

template <>
struct IntegerTraits<unsigned long long int> : public std::numeric_limits<unsigned long long int> {
    static unsigned long long int strToT(const char *__restrict __nptr, char **__restrict __endptr, int __base) __THROW __nonnull((1))
    {
        return ::strtoull(__nptr, __endptr, __base);
    }
};
/* ========================================================================================= */

/* ========================================================================================= */
template <typename T = int>
T sToT(const std::string &str, T def = 0) noexcept
{
    T retVal = def;

    if (str.size() > 0) {
        char *newPtr;
        const char *oldPtr = str.c_str();

        T v = IntegerTraits<T>::strToT(oldPtr, &newPtr, 0);

        if (newPtr == oldPtr) {
            retVal = def;
        } else if (((v == IntegerTraits<T>::min()) || (v == IntegerTraits<T>::max())) && (errno == ERANGE)) {
            retVal = def;
        } else {
            retVal = v;
        }
    }

    return static_cast<T>(retVal);
}
/* ========================================================================================= */

class UciValue
{
  public:

    // create UciValue
    UciValue() : _val() {}

    UciValue(std::string val) : _val(std::make_shared<std::string>(std::move(val))) {}
    UciValue(std::string_view val) : _val(std::make_shared<std::string>(std::move(val))) {}
    UciValue(const char* val) : _val(std::make_shared<std::string>(val)) {}
    UciValue(bool flag) : UciValue(std::to_string(flag ? 1 : 0)) {}

    UciValue(std::tuple<std::string_view, std::string_view> states, bool flag) : UciValue(std::move(flag ? std::get<0>(states) : std::get<1>(states)))
    {}

    template <typename T, typename Tp = void>
    using enable_if_t_is_integral = typename std::enable_if_t<std::is_integral_v<T>, Tp>;

    template <typename T, enable_if_t_is_integral<T, int> = 0>
    UciValue(T number) : UciValue(std::to_string(number))
    {}

    bool isAvail() const noexcept
    {
        return (bool)_val;
    }

    bool isSet() const noexcept
    {
        return (bool)_val && (_val->size() > 0);
    }

    void reset() noexcept
    {
        _val = nullptr;
    }

    std::string toStdString(const std::string &def = std::string("")) const noexcept;
    std::string_view toStringView(const std::string_view &def = std::string_view("")) const noexcept;
    const char *toCharString(const char *def = "") const noexcept;

    operator std::string() const noexcept
    {
        return toStdString();
    }

    operator const char *() const noexcept
    {
        return toCharString();
    }

    template <typename T = int>
    T toInt(T def = 0) const noexcept
    {
        if (!_val) {
            return def;
        }

        return sToT<T>(*_val, def);
    }

    template <typename T>
    operator T() const noexcept
    {
        return toInt<T>();
    }

    bool toBool(bool def = false) const noexcept
    {
        return toInt<int>(def) != 0;
    }

    bool toBool(std::tuple<std::string_view, std::string_view> states, bool def = false) const noexcept
    {
        if(!_val) {
            return def;
        }

        if (!def) {
            return *_val == std::get<0>(states);
        }

        return *_val != std::get<1>(states);
    }

    operator bool() const noexcept
    {
        return toInt<int>() != 0;
    }

    bool operator==(const UciValue &o) const noexcept;
    bool operator!=(const UciValue &o) const noexcept
    {
        return !(*this == o);
    }

  private:
    std::shared_ptr<std::string> _val;
};
///////////////////////////////////////////////////////////////////////////////////////////////

/*### Classes for UCI subscription ###*/
///////////////////////////////////////////////////////////////////////////////////////////////
template <int N>
class TokenPool
{
  public:
    TokenPool() : _tokenPool(), _nextToken(1) {}
    int getNewToken() {
        if(_tokenPool.all()) // All token used
            throw std::runtime_error("Full of token pool");

        for(int i = 0; i < _tokenPoolSize; ++i ) {
            int next = _tokenPoolSize < _nextToken + i ? (_nextToken + i - _tokenPoolSize) : (_nextToken + i);
            if(!_tokenPool.test(next - 1)) {
                _tokenPool.set(next - 1);
                _nextToken = next + 1;
                return next;
            }
        }
        return 0;
    }

    void releaseToken(int tok) {
        if (tok >= 1 && tok <= _tokenPoolSize)
            _tokenPool.reset(tok - 1);
    }

  private:
    const int _tokenPoolSize = N;
    std::bitset<N> _tokenPool;
    int _nextToken;
};

using subscribeCallback_t = std::function<void(void)>;

class SubscribeCallbackWrapper
{
  public:
    SubscribeCallbackWrapper() = delete;
    SubscribeCallbackWrapper(subscribeCallback_t cb): _func(cb) { _classToken = _tokenPool.getNewToken(); }
    ~SubscribeCallbackWrapper() { _tokenPool.releaseToken(_classToken); }

    void operator() () { if (_func) _func(); }

    int getToken() { return _classToken; }

    static constexpr int _tokenPoolSize = 4096;

  private:
    int _classToken;
    subscribeCallback_t _func;
    static TokenPool<_tokenPoolSize> _tokenPool;
};

class SubscribeList
{
  public:

    /* "map" of UciName and UciValue */
    typedef std::map<std::string /*uciName*/, UciValue /*uciValue*/> uciNameValueMap_t;

    /* "multimap" of UciName and callback */
    typedef std::multimap<std::string /*uciName*/, SubscribeCallbackWrapper /*callback wrapper*/> subscribeListMap_t;

    /* subscribe callback type */
    typedef int subscribeToken_t;

    SubscribeList() noexcept : _classMutex(), _uciValueMap(), _subListMap() {}

    subscribeToken_t registerSubscription(const std::string &uciName, subscribeCallback_t cb, const std::string &initialVal = std::string());
    void deregisterSubscription(const subscribeToken_t &token);
    bool isSubscribed(const subscribeToken_t &token);
    void clearSubscription();

    std::string getUciValueOnDB(const std::string &uciName);
    void updateUciValueOnDB(const std::string &uciName, const std::string &uciValue);

    uciNameValueMap_t &getUciNameValueMap() noexcept { return _uciValueMap; }
    subscribeListMap_t &getSubscribeListMap() noexcept { return _subListMap; }

  private:
    std::mutex _classMutex;
    uciNameValueMap_t _uciValueMap;
    subscribeListMap_t _subListMap;
};
///////////////////////////////////////////////////////////////////////////////////////////////


/*### Classes for UBUS context ###*/
///////////////////////////////////////////////////////////////////////////////////////////////
class Context : public fw::utils::Singleton<Context>
{
  public:
    struct ubus_context* getUbusCtx() const noexcept { return _ubusCtx; }

    Json::StreamWriterBuilder &getJsonWbuilder() noexcept { return _jsonWriterBuilder; }

    Json::CharReaderBuilder &getJsonRbuilder() noexcept { return _jsonReaderBuilder; }

    std::string &getUbusSessId() noexcept { return _sessionId; }

    SubscribeList &getSubscribeObj() noexcept { return _subscribeObj; }

    Context();
    ~Context();

    Context(const Context &o) = delete;
    Context &operator=(const Context &o) = delete;

  private:
    /** data members **/
    // jsoncpp writer builder object.
    Json::StreamWriterBuilder _jsonWriterBuilder;

    // jsoncpp reader builder object.
    Json::CharReaderBuilder   _jsonReaderBuilder;

    // UBUS context object
    struct ubus_context *_ubusCtx;

    std::string _sessionId; // UBUS rpc session id
    std::string _sessionUser; // UBUS rpc session session username
    std::string _sessionPass; // UBUS rpc session session password
    uint32_t _sessionTimeout; // UBUS rpc session session timeout
    static constexpr char _sessionOwner[] = "gatt_server";

    // static object on SubscribeList
    static SubscribeList _subscribeObj;

    /** function members **/
    std::string _createSession(const std::string &user, const std::string &pass, int timeout, const std::string &owner);
    void _destroySession(const std::string &sessId);
    std::string _loginSession(const std::string &user, const std::string &pass, int timeout);
};
///////////////////////////////////////////////////////////////////////////////////////////////

/*### Classes for UCI interface ###*/
///////////////////////////////////////////////////////////////////////////////////////////////
/* <UCI data structure>
 *
 * - Human-friendly('uci export' form):
 *      package PackageName
 *
 *      config SectionType [SectionName]
 *          option OptionName OptionValue
 *          list   ListName   ListValue1
 *          list   ListName   ListValue2
 *
 * - Programmable('uci show' form):
 *    > Named Section
 *      PackageName.SectionName.='SectionType'
 *      PackageName.SectionName.OptionName='OptionValue'
 *      PackageName.SectionName.ListName='ListValue1' 'ListValue2'
 *
 *    > Unnamed Section
 *      PackageName.@SectionType[0]='SectionType'
 *      PackageName.@SectionType[0].OptionName='OptionValue'
 *      PackageName.@SectionType[0].ListName='ListValue1' 'ListValue2'
 *      Or,
 *      PackageName.SectionCFGID='SectionType'
 *      PackageName.SectionCFGID.OptionName='OptionValue'
 *      PackageName.SectionCFGID.ListName='ListValue1' 'ListValue2'
 */
class UciHandle
{
  public:
    /*! @brief get UCI value
     *
     * @param[in] uciName UCI name ["p.s"|"p.s.o"]
     * @param[in] delim delimiter for list value
     *
     * @return UCI value in string
     * @note 1. section field on uciName parameter represents "SectionName".
     *      - Named Section: SectionName
     *      - Unnamed Section: "@SectionType[#]" or SectionCFGID
     * @note 2. This function works for only "p.s" and "p.s.o" format.
     *      - "p.s" format returns section type
     *      - "p.s.o" format returns option/list value.
     */
    UciValue get(const UciSecNameType &uciName, const std::string &delim = std::string(" "))
    {
        return _get(static_cast<const UciNameTypeBase &>(uciName), delim);
    }
    UciValue get(const UciOptNameType &uciName, const std::string &delim = std::string(" "))
    {
        return _get(static_cast<const UciNameTypeBase &>(uciName), delim);
    }

    /*! @brief get UCI list
     *
     * @param[in] uciName UCI name ["p.s.o"]
     * @param[out] output string vector of list
     *
     * @return return false, if the "p.s.o" is not a list or does not exist
     * @note 1. section field on uciName parameter represents "SectionName".
     *      - Named Section: SectionName
     *      - Unnamed Section: "@SectionType[#]" or SectionCFGID
     */
    bool getList(const UciOptNameType &uciName, std::vector<std::string> &output)
    {
        return _getList(static_cast<const UciNameTypeBase &>(uciName), output);
    }

    /*! @brief set UCI option/list value
     *
     * @param[in] uciName UCI name ["p.s.o"]
     * @param[in] setVal string value to set("p.s.o"="value" or "p.s.o"=string_vector)
     * @param[in] commitFlag trigger ubus 'config.change' event.
     * @param[in] createFlag create UCI if it does not exist.
     *
     * @return true or false
     * @note 1. section field on uciName parameter represents "SectionName".
     *      - Named Section: SectionName
     *      - Unnamed Section: "@SectionType[#]" or SectionCFGID
     * @note 2. In the case that createFlag is true, it is not possible to create new UCI, if package or section does not exist.
     */
    bool set(const UciOptNameType &uciName, const std::string &setVal, bool commitFlag = false, bool createFlag = false)
    {
        return _set(static_cast<const UciNameTypeBase &>(uciName), setVal, commitFlag, createFlag);
    }
    bool set(const UciOptNameType &uciName, const std::vector<std::string> &setList, bool commitFlag = false, bool createFlag = false)
    {
        return _set(static_cast<const UciNameTypeBase &>(uciName), setList, commitFlag, createFlag);
    }
    /* Note: */
    /* Below version is not implemented, because it could change LIST type to Option type of UCI value by force. */
    /* bool set(const UciOptNameType &uciName, const UciValue &setVal, bool commitFlag = false, bool createFlag = false) */

    /*! @brief set UCI list
     *
     * @param[in] uciName UCI name ["p.s.o"]
     * @param[in] input string vector of list
     * @param[in] commitFlag trigger ubus 'config.change' event.
     * @param[in] createFlag create UCI if it does not exist.
     *
     * @return true or false
     * @note 1. section field on uciName parameter represents "SectionName".
     *      - Named Section: SectionName
     *      - Unnamed Section: "@SectionType[#]" or SectionCFGID
     */
    bool setList(const UciOptNameType &uciName, const std::vector<std::string> &input, bool commitFlag = false, bool createFlag = false)
    {
        return _set(static_cast<const UciNameTypeBase &>(uciName), input, commitFlag, createFlag);
    }

    /*! @brief commit UCI package
     *
     * @param[in] uciName UCI name ["p"|"p.s"|"p.s.o"]
     *
     * @return true or false
     */
    bool commit(const UciPkgNameType &uciName)
    {
        return _commit(static_cast<const UciPkgNameType &>(uciName));
    }
    bool commit(const UciSecNameType &uciName)
    {
        return _commit(static_cast<const UciSecNameType &>(uciName));
    }
    bool commit(const UciOptNameType &uciName)
    {
        return _commit(static_cast<const UciOptNameType &>(uciName));
    }

    /*! @brief check whether there are any changes which are not committed in the package.
     *
     * @param[in] uciName UCI name ["p"|"p.s"|"p.s.o"]
     *
     * @return true or false
     * @note 1. section field on uciName parameter represents "SectionName".
     *      - Named Section: SectionName
     *      - Unnamed Section: "@SectionType[#]" or SectionCFGID
     */
    bool isChangedButNotCommitted(const UciPkgNameType &uciName)
    {
        return _isChangedButNotCommitted(static_cast<const UciPkgNameType &>(uciName));
    }
    bool isChangedButNotCommitted(const UciSecNameType &uciName)
    {
        return _isChangedButNotCommitted(static_cast<const UciSecNameType &>(uciName));
    }
    bool isChangedButNotCommitted(const UciOptNameType &uciName)
    {
        return _isChangedButNotCommitted(static_cast<const UciOptNameType &>(uciName));
    }

    /*! @brief add UCI Unnamed section
     *
     * @param[in] config UCI config name
     * @param[in] secType UCI section type
     *
     * @return If success, returns SectionCFGID. Otherwise, retures empty string.
     */
    std::string addUnnamedSection(const std::string &config, const std::string &secType)
    {
        return _addSection(config, secType, "");
    }

    /*! @brief add UCI Named section
     *
     * @param[in] config UCI config name
     * @param[in] secType UCI section type
     * @param[in] secName UCI section name
     *
     * @return If success, returns SectionName. Otherwise, retures empty string.
     */
    std::string addNamedSection(const std::string &config, const std::string &secType, const std::string &secName)
    {
        return _addSection(config, secType, secName);
    }

    /*! @brief delete UCI element
     *
     * @param[in] uciName UCI name ["p.s"|"p.s.o"]
     *
     * @return true or false
     * @note 1. section field on uciName parameter represents "SectionName".
     *      - Named Section: SectionName
     *      - Unnamed Section: "@SectionType[#]" or SectionCFGID
     */
    bool deleteElem(const UciSecNameType &uciName)
    {
        return _deleteElem(static_cast<const UciNameTypeBase &>(uciName));
    }
    bool deleteElem(const UciOptNameType &uciName)
    {
        return _deleteElem(static_cast<const UciNameTypeBase &>(uciName));
    }

    /*! @brief register a subscription for UCI value changes
     *
     * @param[in] uciname uci name ["p.s.o"]
     * @param[in] cb notification callback
     *
     * @return token for registered subscription
     */
    SubscribeList::subscribeToken_t subscribe(const UciOptNameType &uciName, subscribeCallback_t cb) {
        UciValue initialVal = get(uciName);
        return _context().getSubscribeObj().registerSubscription(uciName.getFullpath(), cb, initialVal);
    }

    /*! @brief deregister a subscription for UCI value changes
     *
     * @param[in] token subscription token to deregister.
     *
     */
    void unsubscribe(const SubscribeList::subscribeToken_t &token) {
        _context().getSubscribeObj().deregisterSubscription(token);
    }

    /*! @brief check whether the subscription is registered
     *
     * @param[in] token subscription token to deregister.
     *
     */
    bool isSubscribed(const SubscribeList::subscribeToken_t &token) {
        return _context().getSubscribeObj().isSubscribed(token);
    }

    /*! @brief remove all subscriptions
     */
    void resetSubscription() {
        _context().getSubscribeObj().clearSubscription();
    }

    /*! @brief trigger registered callback if there is change.
     */
    void pollSubscription();

  private:
    static constexpr auto _context = Context::getInstance;

    UciValue _get(const UciNameTypeBase &uciName, const std::string &delim);
    bool _getList(const UciNameTypeBase &uciName, std::vector<std::string> &output);
    bool _set(const UciNameTypeBase &uciName, const std::string &setVal, bool commitFlag, bool createFlag);
    bool _set(const UciNameTypeBase &uciName, const std::vector<std::string> &setList, bool commitFlag, bool createFlag);
    bool _commit(const UciNameTypeBase &uciName);
    bool _isChangedButNotCommitted(const UciNameTypeBase &uciName);
    std::string _addSection(const std::string &config, const std::string &secType, const std::string &secName = std::string());
    bool _deleteElem(const UciNameTypeBase &uciName);

};
///////////////////////////////////////////////////////////////////////////////////////////////

}; // namespace uci

#endif // NTCUCI_H_10431830012024

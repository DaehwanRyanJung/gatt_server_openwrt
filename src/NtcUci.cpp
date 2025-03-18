/*
 * Openwrt UCI C++ wrapper
 */

#include "NtcUci.h"

#include <vector>
#include <numeric>
#include <stdexcept>

#include "NtcUtils.h"
#include "NtcLogger.h"

namespace {
typedef struct
{
    /* enum ubus_msg_status { */
    /*   UBUS_STATUS_OK,                // 0 */
    /*   UBUS_STATUS_INVALID_COMMAND,   // 1 */
    /*   UBUS_STATUS_INVALID_ARGUMENT,  // 2 */
    /*   UBUS_STATUS_METHOD_NOT_FOUND,  // 3 */
    /*   UBUS_STATUS_NOT_FOUND,         // 4 */
    /*   UBUS_STATUS_NO_DATA,           // 5 */
    /*   UBUS_STATUS_PERMISSION_DENIED, // 6 */
    /*   UBUS_STATUS_TIMEOUT,           // 7 */
    /*   UBUS_STATUS_NOT_SUPPORTED,     // 8 */
    /*   UBUS_STATUS_UNKNOWN_ERROR,     // 9 */
    /*   UBUS_STATUS_CONNECTION_FAILED, // 10 */
    /*   __UBUS_STATUS_LAST             // 11 */
    /* }; */
    int reqStatusCode;

    /* enum ubus_msg_type { */
    /*     UBUS_MSG_HELLO,         // 0: initial server message */
    /*     UBUS_MSG_STATUS,        // 1: generic command response */
    /*     UBUS_MSG_DATA,          // 2: data message response */
    /*     UBUS_MSG_PING,          // 3: ping request */
    /*     UBUS_MSG_LOOKUP,        // 4: look up one or more objects */
    /*     UBUS_MSG_INVOKE,        // 5: invoke a method on a single object */
    /*     UBUS_MSG_ADD_OBJECT,    // 6: */
    /*     UBUS_MSG_REMOVE_OBJECT, // 7: */
    /*     UBUS_MSG_SUBSCRIBE,     // 8: subscribe to object notifications */
    /*     UBUS_MSG_UNSUBSCRIBE,   // 9: unsubscribe to object notifications(sent from ubusd when the object disappears) */
    /*     UBUS_MSG_NOTIFY,        // 10: send a notification to all subscribers of an object. when sent from the server, it indicates a subscription status change */
    /*     UBUS_MSG_MONITOR,       // 11 */
    /*     __UBUS_MSG_LAST,        // 12 */
    /* }; */
    int ubusMsgType;
    std::string parsedStr;
} vendor_data_t;

// Callback for ubus_invoke()
void _ubus_invoke_cb(struct ubus_request *req, int msgType, struct blob_attr *msg)
{
    char *str = nullptr;
    vendor_data_t *userdata = static_cast<vendor_data_t *>(req->priv);

    fw::utils::ScopeDeletor bufDeletor([&str] {
        free(str);
    });

    if (userdata) {
        userdata->reqStatusCode = req->status_code;
        userdata->ubusMsgType = msgType;
    }

    if (!msg) {
        return;
    }

    str = blobmsg_format_json(msg, true);
    if (str && userdata) {
        userdata->parsedStr = str;
    }
}

/*! @brief parse Json string to Json::Value object
 *
 * @param[in] builder JSONCPP reader builder object
 * @param[in] jsonStr json string
 * @param[out] jObj output in Json::Value object
 *
 * @return true or false
*/
bool parseJsonStr(const Json::CharReaderBuilder &builder, const std::string &jStr,  Json::Value &jObj)
{
    JSONCPP_STRING err;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

    jObj.clear();
    if (!reader->parse(jStr.data(), jStr.data() + jStr.size(), &jObj, &err)) {
        log(LOG_ERR, "Invalid JSON form:: %s", err.c_str());
        return false;
    }
    return true;
}

/*
  > return code
UBUS_STATUS_OK,                // 0
UBUS_STATUS_INVALID_COMMAND,   // 1
UBUS_STATUS_INVALID_ARGUMENT,  // 2
UBUS_STATUS_METHOD_NOT_FOUND,  // 3
UBUS_STATUS_NOT_FOUND,         // 4
UBUS_STATUS_NO_DATA,           // 5
UBUS_STATUS_PERMISSION_DENIED, // 6
UBUS_STATUS_TIMEOUT,           // 7
UBUS_STATUS_NOT_SUPPORTED,     // 8
UBUS_STATUS_UNKNOWN_ERROR,     // 9
UBUS_STATUS_CONNECTION_FAILED, // 10
__UBUS_STATUS_LAST             // 11
*/
int ntc_ubus_invoke(struct ubus_context* ctx, const std::string &path, const std::string &method,
        struct blob_attr *msg, ubus_data_handler_t cb, void *priv, int timeout=3000 /*milliseconds*/)
{
    uint32_t id;
    int rc = UBUS_STATUS_UNKNOWN_ERROR;

    rc = ubus_lookup_id(ctx, path.c_str(), &id);
    if (rc == UBUS_STATUS_OK) {
        rc = ubus_invoke(ctx, id, method.c_str(), msg, cb, priv, timeout);
    }

    return rc;
}
}; // unnamed namespace

namespace uci {

/*### Classes for UCI value ###*/
///////////////////////////////////////////////////////////////////////////////////////////////
const char *UciValue::toCharString(const char *def) const noexcept
{
    const char *retVal = def;

    if (_val) {
        retVal = _val->c_str();
    }

    return retVal;
}

std::string_view UciValue::toStringView(const std::string_view &def) const noexcept
{
    std::string_view retVal = def;

    if (_val) {
        retVal = *_val;
    }

    return retVal;
}

std::string UciValue::toStdString(const std::string &def) const noexcept
{
    std::string retVal = def;

    if (_val) {
        retVal = *_val;
    }

    return retVal;
}

bool UciValue::operator==(const UciValue &o) const noexcept
{
    if (_val == o._val)
        return true;

    if ((_val == nullptr) || (o._val == nullptr))
        return false;

    return *_val == *o._val;
}
///////////////////////////////////////////////////////////////////////////////////////////////

/*### Classes for UCI subscription ###*/
///////////////////////////////////////////////////////////////////////////////////////////////

/* SubscribeCallbackWrapper class static data members */
TokenPool<SubscribeCallbackWrapper::_tokenPoolSize> SubscribeCallbackWrapper::_tokenPool;

/* SubscribeList class function members */
SubscribeList::subscribeToken_t SubscribeList::registerSubscription(const std::string &uciName, subscribeCallback_t cb, const std::string &initialVal)
{
    std::lock_guard<std::mutex> guard(_classMutex);

    /* update _uciValueMap */
    if (_uciValueMap.find(uciName) == _uciValueMap.end()) { /* Not existing */
        _uciValueMap.emplace(uciName, initialVal);
    }

    /* update _subListMap */
    auto it = _subListMap.emplace(uciName, SubscribeCallbackWrapper(cb));
    return it->second.getToken();
}

void SubscribeList::deregisterSubscription(const subscribeToken_t &token)
{
    std::string uciName;

    if (!isSubscribed(token))
        return;

    std::lock_guard<std::mutex> guard(_classMutex);
    for ( auto it = _subListMap.begin(); it != _subListMap.end(); ++it) {
        if (it->second.getToken() == token) {
            uciName = it->first;
            _subListMap.erase(it);
            break;
        }
    }

    if (!uciName.empty()) {
        auto uciValIt = _uciValueMap.find(uciName);
        if (_subListMap.count(uciName) == 0 && uciValIt != _uciValueMap.end()) {
            _uciValueMap.erase(uciValIt);
        }
    }
}

bool SubscribeList::isSubscribed(const subscribeToken_t &token)
{
    std::lock_guard<std::mutex> guard(_classMutex);
    for (auto it = _subListMap.begin(); it != _subListMap.end(); ++it) {
        if (it->second.getToken() == token) {
            return true;
        }
    }
    return false;
}

void SubscribeList::clearSubscription()
{
    std::lock_guard<std::mutex> guard(_classMutex);
    _uciValueMap.clear();
    _subListMap.clear();
}

std::string SubscribeList::getUciValueOnDB(const std::string &uciName)
{
    auto uciValIt = _uciValueMap.find(uciName);
    if (uciValIt != _uciValueMap.end()) {
        return uciValIt->second;
    }
    return "";
}

void SubscribeList::updateUciValueOnDB(const std::string &uciName, const std::string &uciValue)
{
    auto uciValIt = _uciValueMap.find(uciName);
    if (uciValIt != _uciValueMap.end()) {
        uciValIt->second = uciValue;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

/*### Classes for UBUS context ###*/
///////////////////////////////////////////////////////////////////////////////////////////////
// Initialize static member
SubscribeList Context::_subscribeObj;

Context::Context() : _ubusCtx(nullptr), _sessionId(), _sessionUser(), _sessionPass(), _sessionTimeout(0)
{
    _ubusCtx = ubus_connect(nullptr);
    if (!_ubusCtx) {
        throw std::runtime_error("Failed to connect to ubus");
    }

    _jsonWriterBuilder["indentation"] = ""; //to omits default indentations character('\t')

    // TODO:: Need to add UBUS rpc session username/password management.
    _sessionUser = "root";
    _sessionPass = "";
    _sessionTimeout = 0; // permanent session

    _sessionId = _createSession(_sessionUser, _sessionPass, _sessionTimeout, std::string(_sessionOwner));
}

Context::~Context()
{
    _destroySession(_sessionId);
    ubus_free(_ubusCtx);
}

std::string Context::_createSession(const std::string &user, const std::string &pass, int timeout, const std::string &owner)
{
    std::string sessId;
    struct blob_buf b;
    vendor_data_t userdata;
    void *ptr1, *ptr2;

    Json::Value jRoot;
    std::vector<std::string> sessIdVec;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    /* 1. get all UBUS RPC session ids */
    /* Note:
     *   Json string which is returned from ubus_invoke("session", "list", ..) is ill-formed,
     *   so the Json string cannot be parsed with blobmsg or JSONCPP library.
     *   That is, it is not possible to use ubus_invoke() library function to get all UBUS RPC session id list.
     *   Here, the system call is used to get all session ids, instead.
     */
    std::string cmd = R"(echo "[$(ubus call session list | grep ubus_rpc_session | sed 's/[[:space:]]*"ubus_rpc_session"://g')]")";
    fw::utils::SysCmdResult cmdResult = fw::utils::SysCmd::exec(cmd);
    if (cmdResult.rc == 0) {
        if (parseJsonStr(getJsonRbuilder(), cmdResult.output, jRoot) && jRoot.isArray()) {
            for (auto &elem : jRoot) {
                if (elem.asString() != "00000000000000000000000000000000") // skip UBUS internal session id
                    sessIdVec.push_back(elem.asString());
            }
        }
    }

    /* 2. Destroy existing UBUS RPC session owned by Gatt server */
    for(auto &elem : sessIdVec) {
        memset(&b, 0, sizeof(struct blob_buf));
        blob_buf_init(&b, 0);
        blobmsg_add_string(&b, "ubus_rpc_session", elem.c_str());

        if (ntc_ubus_invoke(getUbusCtx(), "session", "get", b.head, _ubus_invoke_cb, &userdata, 3000) == UBUS_STATUS_OK
            && userdata.reqStatusCode == UBUS_STATUS_OK && userdata.ubusMsgType == UBUS_MSG_DATA
            && parseJsonStr(getJsonRbuilder(), userdata.parsedStr, jRoot)) {
            if (jRoot.isMember("values") && jRoot["values"].isObject()
                && jRoot["values"].isMember("owner") && jRoot["values"]["owner"] == owner) {
                _destroySession(elem);
            }
        }
        blob_buf_free(&b);
    }

    /* 3. create new UBUS RPC session */
    sessId = _loginSession(user, pass, timeout);

    /* 4. grant all UCI access permission and set UBUS RPC session owner */
    if (!sessId.empty()) {
        /* revoke all uci permision */
        /* ubus call session revoke '{"ubus_rpc_session": "session_id", "scope": "uci"}' */
        memset(&b, 0, sizeof(struct blob_buf));
        blob_buf_init(&b, 0);
        blobmsg_add_string(&b, "ubus_rpc_session", sessId.c_str());
        blobmsg_add_string(&b, "scope", "uci");

        ntc_ubus_invoke(getUbusCtx(), "session", "revoke", b.head, nullptr, nullptr, 3000);
        blob_buf_free(&b);

        /* grant all UCI read/write permission */
        /* ubus call session grant '{"ubus_rpc_session": "session_id", "scope": "uci", "objects": [["*", "read"], ["*", "write"]]}' */
        memset(&b, 0, sizeof(struct blob_buf));
        blob_buf_init(&b, 0);
        blobmsg_add_string(&b, "ubus_rpc_session", sessId.c_str());
        blobmsg_add_string(&b, "scope", "uci");

        ptr1 = blobmsg_open_array(&b, "objects");

        ptr2 = blobmsg_open_array(&b, nullptr);
        blobmsg_add_string(&b, nullptr, "*");
        blobmsg_add_string(&b, nullptr, "read");
        blobmsg_close_array(&b, ptr2);

        ptr2 = blobmsg_open_array(&b, nullptr);
        blobmsg_add_string(&b, nullptr, "*");
        blobmsg_add_string(&b, nullptr, "write");
        blobmsg_close_array(&b, ptr2);

        blobmsg_close_array(&b, ptr1);

        ntc_ubus_invoke(getUbusCtx(), "session", "grant", b.head, nullptr, nullptr, 3000);
        blob_buf_free(&b);

        /* ubus call session set '{"ubus_rpc_session":"session_id","values":{"owner":"gatt_server"}}' */
        memset(&b, 0, sizeof(struct blob_buf));
        blob_buf_init(&b, 0);
        blobmsg_add_string(&b, "ubus_rpc_session", sessId.c_str());
        ptr1 = blobmsg_open_table(&b, "values");
        blobmsg_add_string(&b, "owner", owner.c_str());
        blobmsg_close_table(&b, ptr1);

        ntc_ubus_invoke(getUbusCtx(), "session", "set", b.head, nullptr, nullptr, 3000);
        blob_buf_free(&b);
    }

    return sessId;
}

std::string Context::_loginSession(const std::string &user, const std::string &pass, int timeout)
{
    struct blob_buf b;
    vendor_data_t userdata;
    Json::Value jRoot;
    std::string sessId;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "username", user.c_str());
    blobmsg_add_string(&b, "password", pass.c_str());
    blobmsg_add_u32(&b, "timeout", timeout);

    if (ntc_ubus_invoke(getUbusCtx(), "session", "login", b.head, _ubus_invoke_cb, &userdata, 3000) != UBUS_STATUS_OK
        || userdata.reqStatusCode != UBUS_STATUS_OK || userdata.ubusMsgType != UBUS_MSG_DATA
        || !parseJsonStr(getJsonRbuilder(), userdata.parsedStr, jRoot)
        || !jRoot.isMember("ubus_rpc_session") || !jRoot["ubus_rpc_session"].isString()
       ) {
        return sessId;
    }

    return jRoot["ubus_rpc_session"].asString();
}

void Context::_destroySession(const std::string &sessId)
{
    struct blob_buf b;

    if(sessId.empty())
        return;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "ubus_rpc_session", sessId.c_str());

    ntc_ubus_invoke(getUbusCtx(), "session", "destroy", b.head, nullptr, nullptr, 3000);
}
///////////////////////////////////////////////////////////////////////////////////////////////

/*### Classes for UCI interface ###*/
///////////////////////////////////////////////////////////////////////////////////////////////
void UciHandle::pollSubscription()
{
    SubscribeList::uciNameValueMap_t uciMap = _context().getSubscribeObj().getUciNameValueMap();
    SubscribeList::subscribeListMap_t subMap = _context().getSubscribeObj().getSubscribeListMap();

    for(auto const &[name, value] : uciMap) {
        auto names = fw::utils::split(name, '.');

        UciValue curVal = get(UciOptNameType{names[0], names[1], names[2]});
        if (curVal != value) {
            _context().getSubscribeObj().updateUciValueOnDB(name, curVal);

            auto [l, u] = subMap.equal_range(name);
            if (l == u) {
                continue;
            }

            for (auto &it = l; it != u; ++it) {
                it->second();
            }
        }
    }
}

UciValue UciHandle::_get(const UciNameTypeBase &uciName, const std::string &delim)
{
    struct blob_buf b;
    vendor_data_t userdata;
    std::string retVal;

    Json::Value jRoot;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    if (!uciName.getPackageName().empty())
        blobmsg_add_string(&b, "config", uciName.getPackageName().c_str());
    if (!uciName.getSectionName().empty())
        blobmsg_add_string(&b, "section", uciName.getSectionName().c_str());
    if (!uciName.getOptionName().empty())
        blobmsg_add_string(&b, "option", uciName.getOptionName().c_str());
    if (!_context().getUbusSessId().empty())
        blobmsg_add_string(&b, "ubus_rpc_session", _context().getUbusSessId().c_str());

    if (ntc_ubus_invoke(_context().getUbusCtx(), "uci", "get", b.head, _ubus_invoke_cb, &userdata, 3000) != UBUS_STATUS_OK
        || userdata.reqStatusCode != UBUS_STATUS_OK || userdata.ubusMsgType != UBUS_MSG_DATA
        || !parseJsonStr(_context().getJsonRbuilder(), userdata.parsedStr, jRoot)
       ) {
        return UciValue(); // failure
    }

    switch(uciName.getUciNameType()) {
        case UciNameTypeBase::NameType::SECTION:
            if (!jRoot.isMember("values") || !jRoot["values"].isObject() || !jRoot["values"].isMember(".type")) {
                return UciValue(); // failure
            }
            retVal = jRoot["values"][".type"].asString();
            break;

        case UciNameTypeBase::NameType::OPTION:
            if (!jRoot.isMember("value")) {
                return UciValue(); // failure
            }
            if (jRoot["value"].isArray()) { // list type
                retVal.clear();
                for (auto s = jRoot["value"].begin(), e = jRoot["value"].end(), i = s; i != e; ++i) {
                    if (i != s) {
                        retVal.append(delim);
                    }
                    retVal.append(i->asString());
                }

            } else if(jRoot["value"].isString()) { // option type
                retVal = jRoot["value"].asString();
            }
            break;
        case UciNameTypeBase::NameType::PACKAGE:
        case UciNameTypeBase::NameType::UNKNOWN:
            break;
    }

    return UciValue(retVal);
}

bool UciHandle::_getList(const UciNameTypeBase &uciName, std::vector<std::string> &output)
{
    struct blob_buf b;
    vendor_data_t userdata;

    Json::Value jRoot;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "config", uciName.getPackageName().c_str());
    blobmsg_add_string(&b, "section", uciName.getSectionName().c_str());
    blobmsg_add_string(&b, "option", uciName.getOptionName().c_str());
    if (!_context().getUbusSessId().empty())
        blobmsg_add_string(&b, "ubus_rpc_session", _context().getUbusSessId().c_str());

    if (ntc_ubus_invoke(_context().getUbusCtx(), "uci", "get", b.head, _ubus_invoke_cb, &userdata, 3000) != UBUS_STATUS_OK
        || userdata.reqStatusCode != UBUS_STATUS_OK || userdata.ubusMsgType != UBUS_MSG_DATA
        || !parseJsonStr(_context().getJsonRbuilder(), userdata.parsedStr, jRoot)
       ) {
        return false;
    }

    if (jRoot["value"].isArray()) { // list type
        for(auto &elem : jRoot["value"]) {
            if (elem.isString())
                output.push_back(elem.asString());
        }
        return true;
    }

    return false;
}

bool UciHandle::_set(const UciNameTypeBase &uciName, const std::string &setVal, bool commitFlag, bool createFlag)
{
    struct blob_buf b;
    void *tbl;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    if (!uciName.getPackageName().empty())
        blobmsg_add_string(&b, "config", uciName.getPackageName().c_str());
    if (!uciName.getSectionName().empty())
        blobmsg_add_string(&b, "section", uciName.getSectionName().c_str());
    if (!uciName.getOptionName().empty()) {
        tbl = blobmsg_open_table(&b, "values");
        blobmsg_add_string(&b, uciName.getOptionName().c_str(), setVal.c_str());
        blobmsg_close_table(&b, tbl);
    }
    if (!_context().getUbusSessId().empty())
        blobmsg_add_string(&b, "ubus_rpc_session", _context().getUbusSessId().c_str());

    if(ntc_ubus_invoke(_context().getUbusCtx(), "uci", "set", b.head, nullptr, nullptr, 3000) != UBUS_STATUS_OK) {
        return false;
    }

    if (commitFlag && _isChangedButNotCommitted(uciName)) {
        return _commit(uciName);
    }
    return true;
}

bool UciHandle::_set(const UciNameTypeBase &uciName, const std::vector<std::string> &setList, bool commitFlag, bool createFlag)
{
    struct blob_buf b;
    void *tbl, *arr;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    if (!uciName.getPackageName().empty())
        blobmsg_add_string(&b, "config", uciName.getPackageName().c_str());
    if (!uciName.getSectionName().empty())
        blobmsg_add_string(&b, "section", uciName.getSectionName().c_str());
    if (!uciName.getOptionName().empty()) {
        tbl = blobmsg_open_table(&b, "values");
        arr = blobmsg_open_array(&b, uciName.getOptionName().c_str());
        for (auto &elem : setList) {
            blobmsg_add_string(&b, nullptr, elem.c_str());
        }
        blobmsg_close_array(&b, arr);
        blobmsg_close_table(&b, tbl);
    }
    if (!_context().getUbusSessId().empty())
        blobmsg_add_string(&b, "ubus_rpc_session", _context().getUbusSessId().c_str());

    if(ntc_ubus_invoke(_context().getUbusCtx(), "uci", "set", b.head, nullptr, nullptr, 3000) != UBUS_STATUS_OK) {
        return false;
    }

    if (commitFlag && _isChangedButNotCommitted(uciName)) {
        return _commit(uciName);
    }
    return true;
}

bool UciHandle::_commit(const UciNameTypeBase &uciName)
{
    struct blob_buf b;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    if (!uciName.getPackageName().empty())
        blobmsg_add_string(&b, "config", uciName.getPackageName().c_str());
    if (!_context().getUbusSessId().empty())
        blobmsg_add_string(&b, "ubus_rpc_session", _context().getUbusSessId().c_str());

    return ntc_ubus_invoke(_context().getUbusCtx(), "uci", "commit", b.head, nullptr, nullptr, 3000) == UBUS_STATUS_OK;
}

bool UciHandle::_isChangedButNotCommitted(const UciNameTypeBase &uciName)
{
    struct blob_buf b;
    vendor_data_t userdata;

    Json::Value jRoot;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    if (!uciName.getPackageName().empty())
        blobmsg_add_string(&b, "config", uciName.getPackageName().c_str());
    if (!_context().getUbusSessId().empty())
        blobmsg_add_string(&b, "ubus_rpc_session", _context().getUbusSessId().c_str());

    if (ntc_ubus_invoke(_context().getUbusCtx(), "uci", "changes", b.head, _ubus_invoke_cb, &userdata, 3000) != UBUS_STATUS_OK
        || userdata.reqStatusCode != UBUS_STATUS_OK || userdata.ubusMsgType != UBUS_MSG_DATA
        || !parseJsonStr(_context().getJsonRbuilder(), userdata.parsedStr, jRoot)
       ) {
        return false;
    }

    if (!jRoot.isMember("changes") || !jRoot["changes"].isArray() || jRoot["changes"].size() < 1) {
        return false;
    }
    return true;
}

std::string UciHandle::_addSection(const std::string &config, const std::string &secType, const std::string &secName)
{
    struct blob_buf b;
    vendor_data_t userdata;
    std::string retVal;

    Json::Value jRoot;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "config", config.c_str());
    blobmsg_add_string(&b, "type", secType.c_str());
    if (!secName.empty())
        blobmsg_add_string(&b, "name", secName.c_str());
    if (!_context().getUbusSessId().empty())
        blobmsg_add_string(&b, "ubus_rpc_session", _context().getUbusSessId().c_str());

    if (ntc_ubus_invoke(_context().getUbusCtx(), "uci", "add", b.head, _ubus_invoke_cb, &userdata, 3000) != UBUS_STATUS_OK
        || userdata.reqStatusCode != UBUS_STATUS_OK || userdata.ubusMsgType != UBUS_MSG_DATA
        || !parseJsonStr(_context().getJsonRbuilder(), userdata.parsedStr, jRoot)
       ) {
        return retVal;
    }

    if (jRoot.isMember("section") && jRoot["section"].isString()) {
        retVal = jRoot["section"].asString();
    }
    return retVal;
}

bool UciHandle::_deleteElem(const UciNameTypeBase &uciName)
{
    struct blob_buf b;

    /* "struct blob_buf b" MUST be freed, when escaping from a scope */
    b.buf = nullptr;
    fw::utils::ScopeDeletor bufDeletor([&b] {
        blob_buf_free(&b);
    });

    memset(&b, 0, sizeof(struct blob_buf));
    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "config", uciName.getPackageName().c_str());
    blobmsg_add_string(&b, "section", uciName.getSectionName().c_str());
    if (!uciName.getOptionName().empty())
        blobmsg_add_string(&b, "option", uciName.getOptionName().c_str());
    if (!_context().getUbusSessId().empty())
        blobmsg_add_string(&b, "ubus_rpc_session", _context().getUbusSessId().c_str());

    return ntc_ubus_invoke(_context().getUbusCtx(), "uci", "delete", b.head, nullptr, nullptr, 3000) == UBUS_STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////////////////////////

}; // namespace uci

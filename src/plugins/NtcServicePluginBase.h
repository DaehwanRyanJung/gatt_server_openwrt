#ifndef NTCSERVICEPLUGINBASE_H_15244704122023
#define NTCSERVICEPLUGINBASE_H_15244704122023

/*
 * GATT server interface service plugin base
 */

#include "Server.h"
#include "ServerUtils.h"
#include "Utils.h"
#include "Globals.h"
#include "DBusObject.h"
#include "DBusInterface.h"
#include "GattProperty.h"
#include "GattService.h"
#include "GattUuid.h"
#include "GattCharacteristic.h"
#include "GattDescriptor.h"
#include "Logger.h"

#include "NtcLogger.h"
#include "NtcUci.h"
#include "uuids.h" // our custom UUIDs

#include "json/json.h"

namespace ggk {

class NtcServicePluginBase
{
  public:
    NtcServicePluginBase()
    {
        jsonWriterBuilder["indentation"] = ""; //to omits default indentations character('\t')
    }

    virtual ~NtcServicePluginBase() {};

  protected:
    /** type members **/
    using jsonReaderPtr = std::unique_ptr<Json::CharReader>;

    /** data members **/
    // jsoncpp writer object.
    static Json::StreamWriterBuilder jsonWriterBuilder;
    // jsoncpp reader object.
    static Json::CharReaderBuilder   jsonReaderBuilder;

    // UCI handle on plugin domain
    static uci::UciHandle            uciHdl;
};

#define DEFINE_PLUGIN(classType)        \
   class classType;                     \
   static classType *_servicePlugin;

#define INIT_PLUGIN()                   \
   _servicePlugin = this

#define PLUGIN    _servicePlugin

// Macro to define uci subscription token
#define DEFINE_UCI_SUBSCRIBE_TOKEN(token)           \
  uci::SubscribeList::subscribeToken_t token

// Macro to register uci subscription on onStartNotify method of GattCharacteristic class.
#define UCI_SUBSCRIBE_FOR_CHANGE(token, pkg, sec, opt)         \
  if(!uciHdl.isSubscribed(PLUGIN->token)) {                            \
    PLUGIN->token = uciHdl.subscribe({pkg, sec, opt}, [&self]() {      \
      ggkNofifyUpdatedCharacteristic(self.getPath().c_str());  \
      });                                                      \
  }

// Macro to deregister uci subscription on onStopNotify method of GattCharacteristic class.
#define UCI_UNSUBSCRIBE(token)        \
  if (uciHdl.isSubscribed(PLUGIN->token)) {   \
     uciHdl.unsubscribe(PLUGIN->token);       \
  }

}; // namespace ggk

#endif // NTCSERVICEPLUGINBASE_H_15244704122023

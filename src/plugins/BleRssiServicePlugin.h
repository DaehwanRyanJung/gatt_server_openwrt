#ifndef BLERSSISERVICEPLUGIN_H_14300614122023
#define BLERSSISERVICEPLUGIN_H_14300614122023

/*
 * GATT BLE Signal Strength service plugin
 */

#include "NtcServicePluginBase.h"

#include <bluetooth/bluetooth.h>

namespace ggk {

class BleRssiServicePlugin : public NtcServicePluginBase
{
  public:
    BleRssiServicePlugin(DBusObject &obj);

  private:
    /** type members **/

    /** data members **/
    int hciDevId = -1;

    /** function members **/
    bool getConnectedBdAddr(bdaddr_t *conAddr);
    std::string getBleRssi();
};

}; // namespace ggk

#endif // BLERSSISERVICEPLUGIN_H_14300614122023

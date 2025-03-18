#ifndef DEVICEPLUGIN_H_13231805122023
#define DEVICEPLUGIN_H_13231805122023

/*
 * GATT device service plugin
 */

#include "NtcServicePluginBase.h"

namespace ggk {

class DeviceServicePlugin : public NtcServicePluginBase
{
  public:
    DeviceServicePlugin(DBusObject &obj);
};

}; // namespace ggk

#endif // DEVICEPLUGIN_H_13231805122023

/*
 * GATT main service plugin
 */

#include "NtcServicePluginBase.h"
#include "DeviceServicePlugin.h"
#include "DeviceInfoServicePlugin.h"
#include "BleRssiServicePlugin.h"


namespace ggk {

std::vector<std::shared_ptr<NtcServicePluginBase>> plugins;
void Server::registerServicePlugins()
{
    plugins.push_back(std::shared_ptr<NtcServicePluginBase>(new DeviceServicePlugin(objects.back())));
    plugins.push_back(std::shared_ptr<NtcServicePluginBase>(new DeviceInfoServicePlugin(objects.back())));
    plugins.push_back(std::shared_ptr<NtcServicePluginBase>(new BleRssiServicePlugin(objects.back())));

}

}; // namespace ggk

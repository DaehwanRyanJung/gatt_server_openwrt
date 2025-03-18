/*
 * GATT Device Information service
 */
#include "DeviceInfoServicePlugin.h"

namespace ggk {

DEFINE_PLUGIN(DeviceInfoServicePlugin)

/*** Class member function definitions ***/
// initialize
void DeviceInfoServicePlugin::initServicePlugin()
{
    Json::Value jRoot;
    jRoot.clear(); // clear all object member

    jRoot["GATT"] = "2.0";
    jRoot["Hardware"] = "v01.00";
    jRoot["Software"] = "v01.00";
    jRoot["Firmware"] = "v01.00";
#ifdef V_GATT_SERVER_AUTH_y
    jRoot["Authentication"] = "Required";
#else
    jRoot["Authentication"] = "Unrequired";
#endif
    devVersion = Json::writeString(jsonWriterBuilder, jRoot);

    jRoot.clear(); // clear all object member

    jRoot["Serial Number"] = "1234567890";
    jRoot["IMEI"] = "111111111111111";
    jRoot["IMSI"] = "111111111111111";
    jRoot["Ethernet MAC"] = "00:11:22:33:44:55";
    devFamily = Json::writeString(jsonWriterBuilder, jRoot);
}

std::string DeviceInfoServicePlugin::getVersion()
{
    return devVersion;
}

uint8_t DeviceInfoServicePlugin::getDevFamily()
{
    return 255;
}

std::string DeviceInfoServicePlugin::getDevIdentifiers()
{
    return devFamily;
}

std::string DeviceInfoServicePlugin::getDevNetIdentifiers()
{
    Json::Value jRoot;

    jRoot["Current PLMN"] = "00000";
    jRoot["Short Network Name"] = "ShortNetworkName";
    jRoot["Long Network Name"] = "LongNetworkName";
    return Json::writeString(jsonWriterBuilder, jRoot);
}

uint8_t DeviceInfoServicePlugin::getDevState()
{
    return 3;
}

std::string DeviceInfoServicePlugin::getDevError()
{
    return "";
}

std::string DeviceInfoServicePlugin::getSimApn()
{
    Json::Value jRoot;
    Json::Value jProf;

    jProf["APN"] = "MyAPN1";
    jProf["IP Conn"] = "IPv4v6";
    jRoot["Prof1"] = jProf;

    jProf.clear(); // clear all object member
    jProf["APN"] = "MyAPN2";
    jProf["IP Conn"] = "IPv4";
    jRoot["Prof2"] = jProf;

    jRoot["SIM Status"] = "SIM OK";
    jRoot["SIM ICCID"] = "12345678901234567890";
    jRoot["SIM MSISDN"] = "1234567890123";
    return Json::writeString(jsonWriterBuilder, jRoot);
}

std::string DeviceInfoServicePlugin::getConnectivity()
{
    Json::Value jRoot;

    jRoot["Ethernet Link Status"] = "up";
    jRoot["Ethernet Link Speed"] = 1000;
    return Json::writeString(jsonWriterBuilder, jRoot);
}

std::string DeviceInfoServicePlugin::getIpAddresses()
{
    Json::Value jRoot;

    jRoot["LAN IP"] = "192.168.3.1";
    jRoot["LAN Port Status"] = "down";
    jRoot["WAN IPv4"] = "";
    jRoot["WAN IPv6"] = "";
    return Json::writeString(jsonWriterBuilder, jRoot);
}

std::string DeviceInfoServicePlugin::getTr069Status()
{
    Json::Value jRoot;

    jRoot["Last Connect"] = "2019-08-24 14:40:22";
    return Json::writeString(jsonWriterBuilder, jRoot);
}

std::string DeviceInfoServicePlugin::getSupportedBands()
{
    Json::Value jRoot;

    jRoot["LTE"] = "";
    jRoot["NR5GNSA"] = "";
    jRoot["NR5GSA"] = "";
    return Json::writeString(jsonWriterBuilder, jRoot);
}

std::string DeviceInfoServicePlugin::getSelectedBands()
{
    Json::Value jRoot;

    jRoot["LTE"] = "";
    jRoot["NR5GNSA"] = "";
    jRoot["NR5GSA"] = "";
    jRoot["Non-Persist"] = "1";
    return Json::writeString(jsonWriterBuilder, jRoot);
}

void DeviceInfoServicePlugin::setSelectedBands(const std::string &setVal)
{
}

std::string DeviceInfoServicePlugin::getGPSMagneticCharacteristic()
{
    Json::Value jRoot;

    jRoot["Antenna Azimuth"] = "358";
    jRoot["Antenna Downtilt"] = "-59";
    jRoot["MagneticStatus"] = "1";
    jRoot["Height"] = "449.086426";
    jRoot["HorizontalUncertainty"] = "3.535534";
    jRoot["Latitude"] = "43.898032";
    jRoot["Longitude"] = "-80.126179";
    jRoot["VerticalUncertainty"] = "2.500000";
    return Json::writeString(jsonWriterBuilder, jRoot);

}

std::string DeviceInfoServicePlugin::getBatteryCharacteristic()
{
    Json::Value jRoot;

    jRoot["Battery Level"] = "41";
    jRoot["Battery Voltage"] = "4.015515327453613";
    return Json::writeString(jsonWriterBuilder, jRoot);

}

std::string DeviceInfoServicePlugin::getCellLockSaBand()
{
    Json::Value jRoot;

    jRoot.append(77); // append integer to array.
    jRoot.append(78);
    return Json::writeString(jsonWriterBuilder, jRoot);

}

std::string DeviceInfoServicePlugin::getCellLockLte()
{
    return "";
}

bool DeviceInfoServicePlugin::setCellLockLte(const std::string &setVal)
{
    return true;
}

std::string DeviceInfoServicePlugin::getCellLockNr5g()
{
    return "";
}

bool DeviceInfoServicePlugin::setCellLockNr5g(const std::string &setVal)
{
    return true;
}

/*** Class constructor definition ***/
DeviceInfoServicePlugin::DeviceInfoServicePlugin(DBusObject &obj)
{
    INIT_PLUGIN();

    initServicePlugin();

    obj.gattServiceBegin("deviceInformationService", DEVICE_INFORMATION_UUID)
        .gattCharacteristicBegin("deviceVersionCharacteristic", DEVICE_VERSION_UUID, {"encrypt-read"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getVersion(), true);
            }, false)  /* Do not need authentication */
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Device Version";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("deviceFamilyCharacteristic", DEVICE_FAMILY_UUID, {"encrypt-read"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getDevFamily(), true);
            }, false) /* Do not need authentication */
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Device Family";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("deviceIdentifiersCharacteristic", DEVICE_IDENTIFIERS_UUID, {"encrypt-read"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getDevIdentifiers(), true);
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Device Identifiers";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("deviceNetworkIdentifiersCharacteristic", DEVICE_NETWORK_IDENTIFIERS_UUID, {"encrypt-read", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getDevNetIdentifiers(), true);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getDevNetIdentifiers());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Device Network Identifiers";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("stateCharacteristic", STATE_UUID, {"encrypt-read", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getDevState(), true);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getDevState());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Device State";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("errorCharacteristic", ERROR_UUID, {"encrypt-read", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getDevError(), true);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getDevError());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Error Message";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("simApnCharacteristic", SIM_APN_UUID, {"encrypt-read", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getSimApn(), true);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getSimApn());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "SIM and APN Status";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("connectivityCharacteristic", CONNECTIVITY_UUID, {"encrypt-read"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getConnectivity(), true);
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Connectivity";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("ipAddressesCharacteristic", IP_ADDRESSES_UUID, {"encrypt-read", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getIpAddresses(), true);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getIpAddresses());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "IP Addresses";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("tr069StatusCharacteristic", TR069_STATUS_UUID, {"encrypt-read", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getTr069Status(), true);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getTr069Status());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "TR-069 Status";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("supportedBandsCharacteristic", SUPPORTED_BANDS_UUID, {"encrypt-read", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getSupportedBands(), true);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getSupportedBands());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Supported Bands";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("bandLockCharacteristic", BANDLOCK_UUID, {"encrypt-read", "notify", "encrypt-write"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getSelectedBands(), true);
            })
            .onWriteValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                GVariant *pAyBuffer = g_variant_get_child_value(pParameters, 0);
                PLUGIN->setSelectedBands(Utils::stringFromGVariantByteArray(pAyBuffer));
                self.methodReturnVariant(pInvocation, NULL);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getSelectedBands());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Band lock bitfields";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("gpsMagneticCharacteristic", GPS_MAGNETIC_DATA_UUID, {"encrypt-read", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getGPSMagneticCharacteristic(), true);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getGPSMagneticCharacteristic());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "GPS, Magnetic Data";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("batteryCharacteristic", BATTERY_DATA_UUID, {"encrypt-read", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getBatteryCharacteristic(), true);
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getBatteryCharacteristic());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Battery Data";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("cellLockBandCharacteristic", CELL_LOCK_SA_BAND_UUID, {"encrypt-read"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getCellLockSaBand(), true);
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Supported NR5G SA band for cell locking";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("cellLockLteCharacteristic", CELL_LOCK_LTE_UUID, {"encrypt-read", "encrypt-write", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getCellLockLte(), true);
            })
            .onWriteValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                GVariant *pAyBuffer = g_variant_get_child_value(pParameters, 0);
                if (PLUGIN->setCellLockLte(Utils::stringFromGVariantByteArray(pAyBuffer))) {
                    self.methodReturnVariant(pInvocation, NULL);
                } else {
                    g_dbus_method_invocation_return_error(pInvocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Write Error");
                }
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getCellLockLte());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Cell locking on LTE";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

        .gattCharacteristicBegin("cellLockNr5gCharacteristic", CELL_LOCK_NR5G_UUID, {"encrypt-read", "encrypt-write", "notify"})
            .onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnValue(pInvocation, PLUGIN->getCellLockNr5g(), true);
            })
            .onWriteValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                GVariant *pAyBuffer = g_variant_get_child_value(pParameters, 0);
                if (PLUGIN->setCellLockNr5g(Utils::stringFromGVariantByteArray(pAyBuffer))) {
                    self.methodReturnVariant(pInvocation, NULL);
                } else {
                    g_dbus_method_invocation_return_error(pInvocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Write Error");
                }
            })
            .onUpdatedValue(CHARACTERISTIC_UPDATED_VALUE_CALLBACK_LAMBDA
            {
                self.sendChangeNotificationValue(pConnection, PLUGIN->getCellLockNr5g());
                return true;
            })
            .onStartNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .onStopNotify(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
            {
                self.methodReturnVariant(pInvocation, NULL); // should free pInvocation
            })
            .gattDescriptorBegin("description", "2901", {"encrypt-read"})
                .onReadValue(DESCRIPTOR_METHOD_CALLBACK_LAMBDA
                {
                    const char *pDescription = "Cell locking on NR5G";
                    self.methodReturnValue(pInvocation, pDescription, true);
                })
            .gattDescriptorEnd()
        .gattCharacteristicEnd()

    .gattServiceEnd();
}
}; // namespace ggk

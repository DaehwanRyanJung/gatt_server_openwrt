#ifndef DEVICEINFOSERVICEPLUGIN_H_13584805122023
#define DEVICEINFOSERVICEPLUGIN_H_13584805122023

/*
 * GATT Device Information service plugin
 */

#include "NtcServicePluginBase.h"

namespace ggk {

class DeviceInfoServicePlugin : public NtcServicePluginBase
{
  public:
    DeviceInfoServicePlugin(DBusObject &obj);

  private:
    /** type members **/
    enum class enumLateConfType: uint8_t { RDB, CERT, MBN, EFS, };

    /** data members **/
    std::string devVersion;
    std::string devFamily;

    /** function members **/
    void initServicePlugin();
    std::string getVersion();
    uint8_t getDevFamily();
    std::string getDevIdentifiers();
    std::string getDevNetIdentifiers();
    uint8_t getDevState();
    std::string getDevError();
    std::string getSimApn();
    std::string getConnectivity();
    std::string getIpAddresses();
    std::string getTr069Status();
    std::string getSupportedBands();
    std::string getSelectedBands();
    void setSelectedBands(const std::string &);
    std::string getGPSMagneticCharacteristic();
    std::string getBatteryCharacteristic();
    std::string getCellLockSaBand();
    std::string getCellLockLte();
    bool setCellLockLte(const std::string &);
    std::string getCellLockNr5g();
    bool setCellLockNr5g(const std::string &);
    std::string getConfigIds();
    std::string getLateConfigVerInfo(enumLateConfType);

    /* /1* Example of UCI scuscription *1/ */
    /* DEFINE_UCI_SUBSCRIBE_TOKEN(exampleTok1); */
};

}; // namespace ggk

#endif // DEVICEINFOSERVICEPLUGIN_H_13584805122023

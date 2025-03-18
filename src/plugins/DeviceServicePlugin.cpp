/*
 * GATT Device service
 */
#include "DeviceServicePlugin.h"

namespace ggk {

DEFINE_PLUGIN(DeviceServicePlugin)

/*** Class constructor definition ***/
DeviceServicePlugin::DeviceServicePlugin(DBusObject &obj)
{
    INIT_PLUGIN();

	// Service: Device Information (0x180A)
	//
	// See: https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.device_information.xml
	obj.gattServiceBegin("device", "180A")

		// Characteristic: Manufacturer Name String (0x2A29)
		//
		// See: https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.manufacturer_name_string.xml
		.gattCharacteristicBegin("mfgr_name", "2A29", {"encrypt-read"})

			// Standard characteristic "ReadValue" method call
			.onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
			{
				self.methodReturnValue(pInvocation, "##Manufacturer", true);
			}, false) /* Do not need authentication */

		.gattCharacteristicEnd()

		// Characteristic: Model Number String (0x2A24)
		//
		// See: https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.model_number_string.xml
		.gattCharacteristicBegin("model_num", "2A24", {"encrypt-read"})

			// Standard characteristic "ReadValue" method call
			.onReadValue(CHARACTERISTIC_METHOD_CALLBACK_LAMBDA
			{
				self.methodReturnValue(pInvocation, "##ModelNumber", true);
			}, false) /* Do not need authentication */

		.gattCharacteristicEnd()

	.gattServiceEnd();
}

}; // namespace ggk

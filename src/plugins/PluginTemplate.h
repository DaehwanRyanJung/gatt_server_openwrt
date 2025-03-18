#ifndef ${TODO::PluginTemplate}_H_$(date '+%H%M%S%d%m%Y')
#define ${TODO::PluginTemplate}_H_$(date '+%H%M%S%d%m%Y')

/*
 * GATT ${TODO::PluginTemplate} service plugin
 */

#include "NtcServicePluginBase.h"

namespace ggk {

class ${TODO::PluginTemplate} : public NtcServicePluginBase
{
  public:
    ${TODO::PluginTemplate}(DBusObject &obj);

  private:
    /** type members **/

    /** data members **/

    /** function members **/
};

}; // namespace ggk

#endif // ${TODO::PluginTemplate}_H_$(date '+%H%M%S%d%m%Y')

/*
 * GATT main service plugin
 */

#include "NtcServicePluginBase.h"

namespace ggk {

Json::StreamWriterBuilder NtcServicePluginBase::jsonWriterBuilder;

Json::CharReaderBuilder   NtcServicePluginBase::jsonReaderBuilder;

uci::UciHandle NtcServicePluginBase::uciHdl;

}; // namespace ggk

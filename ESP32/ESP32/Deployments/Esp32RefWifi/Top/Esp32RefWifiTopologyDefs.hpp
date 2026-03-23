#ifndef ESP32REFWIFI_TOPOLOGY_DEFS_HPP
#define ESP32REFWIFI_TOPOLOGY_DEFS_HPP

#include "ESP32/Subtopologies/ESP32CdhCore/PingEntries.hpp"
#include "ESP32/Subtopologies/ESP32ComCcsds/PingEntries.hpp"
#include "ESP32/Subtopologies/ESP32CdhCore/SubtopologyTopologyDefs.hpp"
#include "ESP32/Subtopologies/ESP32ComCcsds/SubtopologyTopologyDefs.hpp"
#include "ESP32/Subtopologies/ESP32ComCcsds/Ports_ComBufferQueueEnumAc.hpp"
#include "ESP32/Subtopologies/ESP32ComCcsds/Ports_ComPacketQueueEnumAc.hpp"

namespace PingEntries {
namespace Esp32RefWifi_rateGroup1Comp {
enum { WARN = 3, FATAL = 5 };
}
namespace Esp32RefWifi_rateGroup2Comp {
enum { WARN = 3, FATAL = 5 };
}
namespace Esp32RefWifi_rateGroup3Comp {
enum { WARN = 3, FATAL = 5 };
}
}  // namespace PingEntries

namespace Esp32RefWifi {

enum class WifiMode : unsigned char {
    SOFT_AP = 0,
    STATION = 1,
};

struct TopologyState {
    const char* remoteIp = nullptr;
    unsigned short remotePort = 0;
    unsigned short localPort = 0;
    unsigned int ledPin = 13;
    bool ledActiveHigh = true;
    WifiMode wifiMode = WifiMode::SOFT_AP;
    const char* wifiSsid = nullptr;
    const char* wifiPassword = nullptr;
    unsigned char wifiChannel = 1;
    unsigned char wifiMaxConnections = 1;
    unsigned int wifiRxBufferSize = 1024;
    ESP32CdhCore::SubtopologyState cdhCore;
    ESP32ComCcsds::SubtopologyState comCcsds;
};

namespace PingEntries = ::PingEntries;

}  // namespace Esp32RefWifi

#endif

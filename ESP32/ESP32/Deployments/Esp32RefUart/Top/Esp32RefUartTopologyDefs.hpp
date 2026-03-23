#ifndef ESP32REFUART_TOPOLOGY_DEFS_HPP
#define ESP32REFUART_TOPOLOGY_DEFS_HPP

#include "ESP32/Subtopologies/ESP32CdhCore/PingEntries.hpp"
#include "ESP32/Subtopologies/ESP32ComCcsds/PingEntries.hpp"
#include "ESP32/Subtopologies/ESP32CdhCore/SubtopologyTopologyDefs.hpp"
#include "ESP32/Subtopologies/ESP32ComCcsds/SubtopologyTopologyDefs.hpp"
#include "ESP32/Subtopologies/ESP32ComCcsds/Ports_ComBufferQueueEnumAc.hpp"
#include "ESP32/Subtopologies/ESP32ComCcsds/Ports_ComPacketQueueEnumAc.hpp"

namespace PingEntries {
namespace Esp32RefUart_rateGroup1Comp {
enum { WARN = 3, FATAL = 5 };
}
namespace Esp32RefUart_rateGroup2Comp {
enum { WARN = 3, FATAL = 5 };
}
namespace Esp32RefUart_rateGroup3Comp {
enum { WARN = 3, FATAL = 5 };
}
}  // namespace PingEntries

namespace Esp32RefUart {

struct TopologyState {
    unsigned int uartNum = 0;
    unsigned int uartBaud = 0;
    int uartTxPin = -1;
    int uartRxPin = -1;
    unsigned int ledPin = 0;
    bool ledActiveHigh = false;
    ESP32CdhCore::SubtopologyState cdhCore;
    ESP32ComCcsds::SubtopologyState comCcsds;
};

namespace PingEntries = ::PingEntries;

}  // namespace Esp32RefUart

#endif

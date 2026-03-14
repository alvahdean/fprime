#ifndef ESP32REFUART_TOPOLOGY_HPP
#define ESP32REFUART_TOPOLOGY_HPP

#include <Fw/Time/TimeInterval.hpp>
#include <ESP32/Deployments/Esp32RefUart/Top/Esp32RefUartTopologyDefs.hpp>

namespace Esp32RefUart {

void setupTopology(const TopologyState& state);
void teardownTopology(const TopologyState& state);
void startRateGroups(const Fw::TimeInterval& interval);
void stopRateGroups();

}  // namespace Esp32RefUart

#endif

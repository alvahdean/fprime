#ifndef ESP32REFWIFI_TOPOLOGY_HPP
#define ESP32REFWIFI_TOPOLOGY_HPP

#include <Fw/Time/TimeInterval.hpp>
#include <ESP32/Deployments/Esp32RefWifi/Top/Esp32RefWifiTopologyDefs.hpp>

namespace Esp32RefWifi {

void setupTopology(const TopologyState& state);
void teardownTopology(const TopologyState& state);
bool waitForGroundConnection(const Fw::TimeInterval& pollInterval, const Fw::TimeInterval& timeout);
void startRateGroups(const Fw::TimeInterval& interval);
void stopRateGroups();

}  // namespace Esp32RefWifi

#endif

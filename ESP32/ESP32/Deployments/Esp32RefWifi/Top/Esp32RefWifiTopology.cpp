#include <ESP32/Deployments/Esp32RefWifi/Top/Esp32RefWifiTopologyAc.hpp>

#include <Fw/Types/Assert.hpp>
#include <Svc/ActiveRateGroup/ActiveRateGroup.hpp>
#include <Svc/RateGroupDriver/RateGroupDriver.hpp>

using namespace Esp32RefWifi;

Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{1, 0}}};
U32 rateGroup1Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

namespace {
void startDeploymentTasks() {
    ESP32CdhCore::cmdDisp.start(
        static_cast<FwTaskPriorityType>(Priorities::ESP32CdhCore_cmdDisp),
        static_cast<Os::Task::ParamType>(StackSizes::ESP32CdhCore_cmdDisp),
        Os::Task::TASK_DEFAULT,
        static_cast<Os::Task::ParamType>(TaskIds::ESP32CdhCore_cmdDisp));

    ESP32CdhCore::events.start(
        static_cast<FwTaskPriorityType>(Priorities::ESP32CdhCore_events),
        static_cast<Os::Task::ParamType>(StackSizes::ESP32CdhCore_events),
        Os::Task::TASK_DEFAULT,
        static_cast<Os::Task::ParamType>(TaskIds::ESP32CdhCore_events));

    ESP32CdhCore::tlmSend.start(
        static_cast<FwTaskPriorityType>(Priorities::ESP32CdhCore_tlmSend),
        static_cast<Os::Task::ParamType>(StackSizes::ESP32CdhCore_tlmSend),
        Os::Task::TASK_DEFAULT,
        static_cast<Os::Task::ParamType>(TaskIds::ESP32CdhCore_tlmSend));

    ESP32ComCcsds::comQueue.start(
        static_cast<FwTaskPriorityType>(Priorities::ESP32ComCcsds_comQueue),
        static_cast<Os::Task::ParamType>(StackSizes::ESP32ComCcsds_comQueue),
        Os::Task::TASK_DEFAULT,
        static_cast<Os::Task::ParamType>(TaskIds::ESP32ComCcsds_comQueue));

    rateGroup1Comp.start(
        static_cast<FwTaskPriorityType>(Priorities::Esp32RefWifi_rateGroup1Comp),
        static_cast<Os::Task::ParamType>(StackSizes::Esp32RefWifi_rateGroup1Comp),
        Os::Task::TASK_DEFAULT,
        static_cast<Os::Task::ParamType>(TaskIds::Esp32RefWifi_rateGroup1Comp));
}

void configureTopology(const TopologyState& state) {
    rateGroupDriverComp.configure(rateGroupDivisorsSet);
    rateGroup1Comp.configure(rateGroup1Context, FW_NUM_ARRAY_ELEMENTS(rateGroup1Context));

    const bool configured =
        wifiDriver.configure(state.remoteIp, state.remotePort, state.localPort, state.wifiRxBufferSize);
    FW_ASSERT(configured);
}
}  // namespace

namespace Esp32RefWifi {

void setupTopology(const TopologyState& state) {
    initComponents(state);
    setBaseIds();
    connectComponents();
    regCommands();
    configComponents(state);
    configureTopology(state);
    loadParameters();
    startDeploymentTasks();
}

void teardownTopology(const TopologyState& state) {
    stopTasks(state);
    freeThreads(state);
    tearDownComponents(state);
}

void startRateGroups(const Fw::TimeInterval& interval) {
    freeRtosTimer.startTimer(interval);
}

void stopRateGroups() {
    freeRtosTimer.quit();
}

}  // namespace Esp32RefWifi

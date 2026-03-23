#include <ESP32/Deployments/Esp32RefWifi/Top/Esp32RefWifiTopologyAc.hpp>

#include <cstdio>

#include <Fw/Types/Assert.hpp>
#include <Svc/ActiveRateGroup/ActiveRateGroup.hpp>
#include <Svc/RateGroupDriver/RateGroupDriver.hpp>
#include <Os/Task.hpp>

using namespace Esp32RefWifi;

Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{1, 0}, {50, 0}}};
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

    const bool ledConfigured = ledGpioDriver.configure(state.ledPin, state.ledActiveHigh, Fw::Logic::LOW);
    FW_ASSERT(ledConfigured);

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

bool waitForGroundConnection(const Fw::TimeInterval& pollInterval, const Fw::TimeInterval& timeout) {
    const U64 timeoutUsec = static_cast<U64>(timeout.getSeconds()) * 1000000ULL + timeout.getUSeconds();
    U64 elapsedUsec = 0;

    while (!wifiDriver.pollTransportReady()) {
        if ((timeoutUsec > 0U) && (elapsedUsec >= timeoutUsec)) {
            printf("Esp32RefWifi: ground TCP wait timed out after %llu ms\n",
                   static_cast<unsigned long long>(elapsedUsec / 1000ULL));
            fflush(stdout);
            return false;
        }
        const Os::Task::Status status = Os::Task::delay(pollInterval);
        FW_ASSERT(status == Os::Task::Status::OP_OK, static_cast<FwAssertArgType>(status));
        elapsedUsec += static_cast<U64>(pollInterval.getSeconds()) * 1000000ULL + pollInterval.getUSeconds();
    }
    printf("Esp32RefWifi: ground TCP became ready after %llu ms\n",
           static_cast<unsigned long long>(elapsedUsec / 1000ULL));
    fflush(stdout);
    return true;
}

void startRateGroups(const Fw::TimeInterval& interval) {
    freeRtosTimer.startTimer(interval);
}

void stopRateGroups() {
    freeRtosTimer.quit();
}

}  // namespace Esp32RefWifi

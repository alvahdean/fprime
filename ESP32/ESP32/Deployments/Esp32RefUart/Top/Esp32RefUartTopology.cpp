#include <ESP32/Deployments/Esp32RefUart/Top/Esp32RefUartTopologyAc.hpp>

#include <Fw/Types/Assert.hpp>
#include <Svc/ActiveRateGroup/ActiveRateGroup.hpp>
#include <Svc/RateGroupDriver/RateGroupDriver.hpp>

using namespace Esp32RefUart;

Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{1, 0}, {2, 0}, {4, 0}}};
U32 rateGroup1Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
U32 rateGroup2Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
U32 rateGroup3Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

namespace {
void configureTopology(const TopologyState& state) {
    rateGroupDriverComp.configure(rateGroupDivisorsSet);
    rateGroup1Comp.configure(rateGroup1Context, FW_NUM_ARRAY_ELEMENTS(rateGroup1Context));
    rateGroup2Comp.configure(rateGroup2Context, FW_NUM_ARRAY_ELEMENTS(rateGroup2Context));
    rateGroup3Comp.configure(rateGroup3Context, FW_NUM_ARRAY_ELEMENTS(rateGroup3Context));

    const bool ledConfigured = ledGpioDriver.configure(state.ledPin, state.ledActiveHigh, Fw::Logic::LOW);
    FW_ASSERT(ledConfigured);

    (void)comDriver.configure(state.uartNum, state.uartBaud);
}
}  // namespace

namespace Esp32RefUart {

void setupTopology(const TopologyState& state) {
    initComponents(state);
    setBaseIds();
    connectComponents();
    regCommands();
    configComponents(state);
    configureTopology(state);
    loadParameters();
    startTasks(state);
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

}  // namespace Esp32RefUart

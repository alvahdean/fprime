// ======================================================================
// \title  STM32MinimalTopology.cpp
// \brief  Topology lifecycle for minimal STM32 UART deployment
// ======================================================================

#include <STM32/Deployments/MinimalUart/Top/STM32MinimalTopologyAc.hpp>

#include <cstdio>

#include <Fw/Types/Assert.hpp>
#include <Os/RawTime.hpp>
#include <Os/Task.hpp>

using namespace STM32Minimal;

namespace {
Svc::RateGroupDriver::DividerSet s_rateGroupDivisorsSet{{{1, 0}, {1, 0}, {1, 0}}};
U32 s_rateGroup1Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

void configureTopology(const TopologyState& state) {
    rateGroupDriverComp.configure(s_rateGroupDivisorsSet);
    rateGroup1Comp.configure(s_rateGroup1Context, FW_NUM_ARRAY_ELEMENTS(s_rateGroup1Context));

    uartDriver.setDmaHooks(state.dmaHooks.context, state.dmaHooks.startRx, state.dmaHooks.startTx);
}
}  // namespace

namespace STM32Minimal {

void setupTopology(const TopologyState& state) {
    initComponents(state);
    setBaseIds();
    connectComponents();
    regCommands();
    configComponents(state);

    configureTopology(state);
    loadParameters();
    startTasks(state);

    const bool uartStarted = uartDriver.start();
    if (!uartStarted) {
        (void)std::printf("[WARNING] STM32 uartDriver.start() failed. Install DMA hooks in TopologyState.\n");
    }
}

void runMainLoop(const TopologyState& state) {
    const U32 cyclePeriodUs = (state.cyclePeriodUs == 0U) ? 1000U : state.cyclePeriodUs;
    const U32 maxCycles = state.maxCycles;

    U32 cycleCount = 0;
    while (true) {
        Os::RawTime cycleStart;
        const Os::RawTime::Status status = cycleStart.now();
        FW_ASSERT(status == Os::RawTime::Status::OP_OK, static_cast<FwAssertArgType>(status));

        Svc::InputCyclePort* cycleIn = rateGroupDriverComp.get_CycleIn_InputPort(0);
        FW_ASSERT(cycleIn != nullptr);
        cycleIn->invoke(cycleStart);

        cycleCount++;
        if ((maxCycles != 0U) && (cycleCount >= maxCycles)) {
            break;
        }
        (void)Os::Task::delay(Fw::TimeInterval(0, cyclePeriodUs));
    }
}

void teardownTopology(const TopologyState& state) {
    stopTasks(state);
    freeThreads(state);
    tearDownComponents(state);
}

}  // namespace STM32Minimal

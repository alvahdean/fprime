// ======================================================================
// \title  Main.cpp
// \brief  Main program for minimal STM32 UART deployment
// ======================================================================

#include <STM32/Deployments/MinimalUart/Top/STM32MinimalTopology.hpp>

#include <Fw/Types/Assert.hpp>
#include <Fw/Time/TimeInterval.hpp>
#include <Os/Os.hpp>
#include <Os/RawTime.hpp>
#include <Os/Task.hpp>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    Os::init();

    STM32Minimal::TopologyState state{};
    state.cyclePeriodUs = 1000;  // 1 kHz scheduler tick

    STM32Minimal::setupTopology(state);
    STM32Minimal::runMainLoop(state);
    STM32Minimal::teardownTopology(state);
    return 0;
}

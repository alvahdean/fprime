// ======================================================================
// \title  STM32MinimalTopologyDefs.hpp
// \brief  Definitions required by the topology autocoder
// ======================================================================
#ifndef STM32_DEPLOYMENTS_MINIMALUART_STM32MINIMALTOPOLOGYDEFS_HPP
#define STM32_DEPLOYMENTS_MINIMALUART_STM32MINIMALTOPOLOGYDEFS_HPP

#include <Fw/FPrimeBasicTypes.hpp>
#include <STM32/Deployments/MinimalUart/Top/FppConstantsAc.hpp>
#include <STM32/Drv/STM32UartDriver/STM32UartDriver.hpp>

#include <STM32/Subtopologies/STM32CdhCore/PingEntries.hpp>
#include <STM32/Subtopologies/STM32CdhCore/SubtopologyTopologyDefs.hpp>
#include <STM32/Subtopologies/STM32ComFprime/PingEntries.hpp>
#include <STM32/Subtopologies/STM32ComFprime/SubtopologyTopologyDefs.hpp>
#include <STM32/Subtopologies/STM32ComFprime/Ports_ComBufferQueueEnumAc.hpp>
#include <STM32/Subtopologies/STM32ComFprime/Ports_ComPacketQueueEnumAc.hpp>

namespace PingEntries {
namespace STM32Minimal_rateGroup1Comp {
enum { WARN = 3, FATAL = 5 };
}
}  // namespace PingEntries

namespace STM32Minimal {

using ::STM32ComFprime::Ports_ComBufferQueue;
using ::STM32ComFprime::Ports_ComPacketQueue;

struct UartDmaHooks {
    void* context;
    Drv::STM32UartDriver::StartRxDmaFn startRx;
    Drv::STM32UartDriver::StartTxDmaFn startTx;
};

struct TopologyState {
    U32 cyclePeriodUs;
    U32 maxCycles;
    UartDmaHooks dmaHooks;
    STM32CdhCore::SubtopologyState cdhCore;
    STM32ComFprime::SubtopologyState comFprime;
};

namespace PingEntries = ::PingEntries;
}  // namespace STM32Minimal

#endif

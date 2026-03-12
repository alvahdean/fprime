// ======================================================================
// \title  FreeRTOS/Os/DefaultConsole.cpp
// \brief  Sets default Os::Console implementation to FreeRTOS
// ======================================================================

#include "Os/Console.hpp"
#include "Os/Delegate.hpp"
#include "FreeRTOS/Os/Console.hpp"

namespace Os {

ConsoleInterface* ConsoleInterface::getDelegate(ConsoleHandleStorage& aligned_new_memory, const ConsoleInterface* to_copy) {
    return Os::Delegate::makeDelegate<ConsoleInterface, Os::FreeRTOS::Console::FreeRtosConsole>(aligned_new_memory,
                                                                                                   to_copy);
}

}  // namespace Os

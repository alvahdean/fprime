// ======================================================================
// \title  FreeRTOS/Os/Console.cpp
// \brief  FreeRTOS implementation for Os::Console
// ======================================================================

#include "FreeRTOS/Os/Console.hpp"

#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

namespace Os {
namespace FreeRTOS {
namespace Console {

void FreeRtosConsole::writeMessage(const CHAR* message, FwSizeType size) {
    if (message == nullptr) {
        return;
    }
    const Os::FreeRTOSSupport::ConsoleWriteFn writer = Os::FreeRTOSSupport::getConsoleWriter();
    if (writer != nullptr) {
        writer(message, size);
    }
}

ConsoleHandle* FreeRtosConsole::getHandle() {
    return &this->m_handle;
}

}  // namespace Console
}  // namespace FreeRTOS
}  // namespace Os

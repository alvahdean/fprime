// ======================================================================
// \title  FreeRTOS/Os/Console.hpp
// \brief  FreeRTOS implementation for Os::Console
// ======================================================================
#ifndef OS_FREERTOS_CONSOLE_HPP
#define OS_FREERTOS_CONSOLE_HPP

#include <Os/Console.hpp>

namespace Os {
namespace FreeRTOS {
namespace Console {

struct FreeRtosConsoleHandle : public ConsoleHandle {};

class FreeRtosConsole : public ConsoleInterface {
  public:
    FreeRtosConsole() = default;
    FreeRtosConsole(const FreeRtosConsole& other) = default;
    FreeRtosConsole& operator=(const FreeRtosConsole& other) = default;
    ~FreeRtosConsole() override = default;

    void writeMessage(const CHAR* message, FwSizeType size) override;
    ConsoleHandle* getHandle() override;

  private:
    FreeRtosConsoleHandle m_handle;
};

}  // namespace Console
}  // namespace FreeRTOS
}  // namespace Os

#endif

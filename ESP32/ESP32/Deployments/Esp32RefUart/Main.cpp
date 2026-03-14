#include <Os/Os.hpp>
#include <Fw/Time/TimeInterval.hpp>
#include <Fw/Types/Assert.hpp>
#include <ESP32/Deployments/Esp32RefUart/Top/Esp32RefUartTopology.hpp>

#if defined(TGT_OS_TYPE_ESP32)
extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
}
#endif

static void run_esp32_ref_uart() {
    Os::init();

    Esp32RefUart::TopologyState state;
    state.uartNum = 0;
    state.uartBaud = 115200;

    Esp32RefUart::setupTopology(state);
    Esp32RefUart::startRateGroups(Fw::TimeInterval(0, 100000));
}

#if defined(TGT_OS_TYPE_ESP32)
static void run_esp32_ref_uart_task(void* argument) {
    static_cast<void>(argument);
    run_esp32_ref_uart();
    vTaskDelete(nullptr);
}

extern "C" void app_main() {
    static constexpr U32 STARTUP_TASK_STACK_BYTES = 16U * 1024U;
    const BaseType_t status = xTaskCreatePinnedToCore(run_esp32_ref_uart_task,
                                                       "fprime_startup",
                                                       STARTUP_TASK_STACK_BYTES,
                                                       nullptr,
                                                       static_cast<UBaseType_t>(tskIDLE_PRIORITY + 4U),
                                                       nullptr,
                                                       tskNO_AFFINITY);
    FW_ASSERT(status == pdPASS, static_cast<FwAssertArgType>(status));
}
#else
int main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);
    run_esp32_ref_uart();
    return 0;
}
#endif

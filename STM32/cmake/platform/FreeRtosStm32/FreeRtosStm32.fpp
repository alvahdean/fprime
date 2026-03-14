# ==========================================================================
# FreeRtosStm32.fpp
#
# Platform-local assumptions for STM32 + FreeRTOS.
#
# NOTE: This file is intentionally additive and does not override framework
# defaults by filename. Deployment projects may layer additional config modules
# later in CMake detection order for board-specific tuning.
# ==========================================================================

@ Assumed FreeRTOS tick rate for timeout/tick conversion reasoning.
@ Keep aligned with configTICK_RATE_HZ in the deployment's FreeRTOSConfig.h.
constant STM32_FREERTOS_TICK_RATE_HZ_ASSUMPTION = 1000

@ Conservative initial string size hint for embedded targets.
@ Projects should override their concrete FpConfig values as needed.
constant STM32_FREERTOS_STRING_SIZE_HINT = 128

@ Conservative initial serialized buffer size hint for embedded targets.
constant STM32_FREERTOS_SERIAL_BUFFER_SIZE_HINT = 256

@ Placeholder for assert/log routing policy (UART/SWO/RTT) to be defined
@ by the deployment integrating STM32 HAL + startup code.
constant STM32_FREERTOS_LOG_ASSERT_ROUTING_PLACEHOLDER = 0

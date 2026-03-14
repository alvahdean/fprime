// =========================================================================
// \title  system_stm32f4xx.c
// \brief  Minimal STM32F405 system initialization scaffold
// =========================================================================

#include <stdint.h>

// Default to the internal HSI clock until board-specific clock tree setup is added.
uint32_t SystemCoreClock = 16000000UL;

void SystemInit(void) {
    // Board-specific clock, FPU, cache, and peripheral setup should be added here.
    SystemCoreClock = 16000000UL;
}

void SystemCoreClockUpdate(void) {
    // Keep in sync with the clock tree once board-specific init is implemented.
    SystemCoreClock = 16000000UL;
}

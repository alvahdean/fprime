# FreeRTOS F' Library

This library provides a generic FreeRTOS OSAL implementation and FreeRTOS platform/toolchain files without modifying F' core packages.

## Layout
- `library.cmake`: library manifest for F' module discovery
- `FreeRTOS/Os`: FreeRTOS OSAL implementation modules and tests
- `cmake/toolchain/freertos-generic.cmake`: generic FreeRTOS toolchain
- `cmake/platform/FreeRTOS.cmake`: FreeRTOS platform configuration

## Usage
1. Add this library path to `FPRIME_LIBRARY_LOCATIONS` in your project.
2. Run `fprime-util generate freertos-generic`.
3. Build with `fprime-util build`.

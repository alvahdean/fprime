# STM32MinimalUart Deployment

This deployment is a minimal F' application for STM32 that wires:

- `CdhCore` for command/event/telemetry services
- `ComFprime` for F' framing used by GDS
- `Drv::STM32UartDriver` as the byte-stream transport

## Purpose

Use this deployment to validate STM32 UART transport with F' GDS over serial.

## Build

```bash
source ~/src/fprime/.venv/bin/activate
cd ~/src/fprime/STM32/STM32/Deployments/MinimalUart
fprime-util generate arm-gcc-stm32
fprime-util build
```

This deployment resolves both the `STM32` library and the shared `FreeRTOS`
OSAL library through its local `settings.ini`, and that same settings file sets
`default_toolchain: arm-gcc-stm32` so plain `fprime-util build` uses the STM32
build cache from this directory.

Equivalent explicit commands are:

```bash
fprime-util build arm-gcc-stm32
fprime-util build --target STM32MinimalUart
```

Build artifacts are emitted at:

- `build-minimaluart-image/bin/arm-gcc-stm32/STM32MinimalUart.elf`
- `build-minimaluart-image/bin/arm-gcc-stm32/STM32MinimalUart.bin`
- `build-minimaluart-image/bin/arm-gcc-stm32/STM32MinimalUart.hex`

## Flash Helpers

If your host has `st-flash` installed:

```bash
cmake --build build-minimaluart-image --target STM32MinimalUart_flash_stlink
```

If your host has `openocd` installed:

```bash
cmake --build build-minimaluart-image --target STM32MinimalUart_flash_openocd
```

The default flash address is `0x08000000`.

## PlatformIO Flashing

PlatformIO can also be used as a flashing wrapper without taking over the F'
build:

```bash
source ~/src/fprime/.venv/bin/activate
cd ~/src/fprime/STM32/STM32/Deployments/MinimalUart
fprime-util build --build-cache build-minimaluart-image --target STM32MinimalUart
pio run -e minimaluart-stm32f405 -t checkfprime
pio run -e minimaluart-stm32f405 -t uploadfprime
```

This PlatformIO environment assumes:

- OpenOCD/ST-Link for `uploadfprime`
- USB DFU bootloader for `uploadfprime_dfu`
- the prebuilt F' artifact at `build-minimaluart-image/bin/arm-gcc-stm32/STM32MinimalUart.elf`

For a Feather-style USB-only flow:

```bash
pio run -e minimaluart-stm32f405 -t uploadfprime_dfu
```

The DFU helper currently assumes:

- touch/reset port: `/dev/ttyACM0`
- DFU VID:PID: `0x0483:0xDF11`
- flash start address: `0x08000000`

## Runtime Hookup Requirement

`Drv::STM32UartDriver` requires DMA-start hooks for RX/TX to be passed in
`STM32Minimal::TopologyState::dmaHooks` before `setupTopology`.

By default, `Main.cpp` leaves hooks unset, so `uartDriver.start()` will fail
and log a warning. This is intentional for portability of the deployment
skeleton.

## Current Status

The deployment now configures and builds successfully with the STM32 toolchain,
and it uses the shared FreeRTOS OSAL implementations selected by the
`FreeRtosStm32` platform configuration.

What it now provides is a flashable `.elf/.bin/.hex` image and optional host
flash targets. What it does not yet provide is full board bring-up.

## Remaining STM32 MVP Work

MVP should emit a flashable image for an STM32 board.

1. Add board, HAL, and FreeRTOS port integration.
   Link CMSIS, STM32 HAL, and the Cortex-M4 FreeRTOS kernel/port sources.
1. Wire real UART DMA hooks.
   Fill `STM32Minimal::TopologyState::dmaHooks` in `Main.cpp` and connect the ISR callbacks to `STM32UartDriver`.
1. Start the scheduler correctly.
   Ensure the FreeRTOS scheduler startup path matches the selected STM32 port.
1. Retarget low-level I/O if desired.
   `startup/syscalls_stm32.c` currently provides minimal bare-metal stubs only.

Prompt: Wire the real STM32F405 HAL, FreeRTOS port, and UART DMA callbacks into the existing deployment skeleton.
 

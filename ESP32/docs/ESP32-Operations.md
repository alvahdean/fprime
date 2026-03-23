# ESP32

ESP32 platform library for F'.

## Scope
- `esp32-idf` toolchain support
- `ESP32FreeRTOS` platform support
- shared FreeRTOS OSAL implementations from the `FreeRTOS` library
- ESP32-owned UART and Wi-Fi transport drivers
- ESP32-local subtopologies and deployment tuning
- example reference deployments for UART and Wi-Fi bring-up

## Library Structure
- `ESP32/library.cmake`
- `ESP32/cmake`
- `ESP32/config/ESP32FreeRTOS`
- `ESP32/ESP32/Drv`
- `ESP32/ESP32/Svc`
- `ESP32/ESP32/Subtopologies`
- `ESP32/ESP32/Deployments`
- `ESP32/scripts`

Script details are documented in [ESP32-Helper-Scripts.md](/home/dfuqua/src/fprime/ESP32/docs/ESP32-Helper-Scripts.md).

## Prerequisites
Use a Linux host for the current bring-up flow.

1. Python 3.9+ with a working F' virtual environment
2. CMake 3.26+
3. Ninja
4. ESP-IDF 5.5.x with ESP32 target tools installed
5. USB serial access to the board device, typically via `dialout` or `uucp`

## Environment Setup
```bash
export FPRIME_REPO_ROOT="${HOME}/src/fprime.worktrees/esp32-support"
source "${HOME}/src/fprime/.venv/bin/activate"
source "${HOME}/.espressif/tools/activate_idf_v5.5.3.sh"
```

Optional helper:
```bash
source "${FPRIME_REPO_ROOT}/ESP32/scripts/build-env.sh"
```

`build-env.sh` should be sourced when you want the ESP32 build environment to persist in the current shell.

## Library Discovery
Add both `ESP32` and `FreeRTOS` to `settings.ini` when consuming this library outside this repo:

```ini
[fprime]
library_locations: /absolute/path/to/ESP32:/absolute/path/to/FreeRTOS
```

## Shared FreeRTOS Usage
ESP32 now uses the shared FreeRTOS OSAL library instead of a private ESP32-specific copy.

Validated shared implementations in the ESP32 build:
- `Os_Task_FreeRTOS`
- `Os_Mutex_FreeRTOS`
- `Os_Queue_FreeRTOS`
- `Os_RawTime_FreeRTOS`

The shared FreeRTOS library was extended to support ESP-IDF's critical-section API without reintroducing ESP32-local OSAL forks.

## Generate And Build
The ESP-IDF wrapper setup runs automatically during CMake configure. Normal `fprime-util generate/build` is the supported flow.

Optional flash image packaging is now available directly in the CMake build:
- `esp32_flash_image` packages the flashable app image and writes `idf-wrapper/build/fprime_flash_artifacts.json`
- for the reference `Esp32RefUart` and `Esp32RefWifi` deployments, flash image packaging is attached as a post-build step on the deployment executable, so `fprime-util build` also packages the flash image automatically when the deployment target is rebuilt

### UART deployment
```bash
cd "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefUart"
fprime-util generate esp32-idf
fprime-util build
```

`Esp32RefUart` hardware settings live in
`ESP32/ESP32/Deployments/Esp32RefUart/config/Esp32RefUartDeploymentCfg.hpp`.
After changing that header, rerun `fprime-util generate esp32-idf --force`.
Deployment-specific notes live in [ESP32-Deployment-Esp32RefUart.md](/home/dfuqua/src/fprime/ESP32/docs/ESP32-Deployment-Esp32RefUart.md).

### Wi-Fi deployment
```bash
cd "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefWifi"
fprime-util generate esp32-idf
fprime-util build
```

Deployment-specific notes live in [ESP32-Deployment-Esp32RefWifi.md](/home/dfuqua/src/fprime/ESP32/docs/ESP32-Deployment-Esp32RefWifi.md).

To package manually after a successful build:

```bash
cmake --build build-fprime-automatic-esp32-idf --target esp32_flash_image
```

## Flash Image Packaging
Package the F' ELF into an ESP32 flash image:

```bash
"${FPRIME_REPO_ROOT}/ESP32/scripts/build_flash_image.sh" \
  "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefUart"

"${FPRIME_REPO_ROOT}/ESP32/scripts/build_flash_image.sh" \
  "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefWifi"
```

Generated outputs:
- `build-fprime-automatic-esp32-idf/bin/esp32-idf/<Deployment>.bin`
- `idf-wrapper/build/fprime_flash_artifacts.json`

## Flashing
Flash with the provided helper:

```bash
"${FPRIME_REPO_ROOT}/ESP32/scripts/flash.sh" \
  "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefUart" \
  --port /dev/ttyUSB0 --baud 460800
```

The same flow works for `Esp32RefWifi` by changing the deployment path.

For `Esp32RefWifi`, `flash.sh` will also reprovision the `fprimecfg` Wi-Fi partition after flashing when a local `wifi.config` file exists and is newer than the last provision stamp.

Start the monitor automatically after a successful flash:

```bash
"${FPRIME_REPO_ROOT}/ESP32/scripts/flash.sh" \
  "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefUart" \
  --port /dev/ttyUSB0 --baud 460800 --monitor
```

## Monitoring
Monitor the serial console with the provided helper:

```bash
"${FPRIME_REPO_ROOT}/ESP32/scripts/monitor.sh" \
  "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefUart" \
  --port /dev/ttyUSB0
```

Optional flags:
- `--baud <rate>` to override the deployment monitor baud
- `--timestamps` to add timestamps to monitor output
- `--no-reset` to avoid resetting the target when the monitor starts

## Deployment Status
### `Esp32RefUart`
- `fprime-util generate esp32-idf` verified
- `fprime-util build` verified
- flash image packaging verified
- flash to Huzzah32 verified
- intended as the primary GDS-oriented bring-up deployment
- currently the more complete reference deployment
- supports command, event, and telemetry flow over the UART transport
- includes the shared `Svc.LedController` component with the ESP32 GPIO backend
- suitable for exercising the LED control commands and normal GDS-visible event/telemetry behavior

### `Esp32RefWifi`
- `fprime-util generate esp32-idf` verified
- `fprime-util build` verified
- flash image packaging verified
- hardware flashing verified
- supports both SoftAP and station-mode bring-up selected by provisioned `fprimecfg` NVS data
- currently configured as a command-only deployment for reliability on the target
- Wi-Fi transport is used for simple command bring-up and basic ground connectivity checks
- event and telemetry downlink are intentionally disabled in the current topology
- includes the shared `Svc.LedController` component with the ESP32 GPIO backend
- supports simple command testing such as `CMD_NO_OP`, `CMD_NO_OP_STRING`, and LED `GET_STATE`/`SET_STATE`/`TOGGLE`
- additional service enablement over Wi-Fi is currently limited by ESP32 memory and task-stack constraints
- Wi-Fi AP client connect/disconnect and STA connect/disconnect events are still visible on the local serial console

## ESP32 Memory Tuning
ESP32-specific memory reductions stay local to the library.

Local tuning lives in:
- `ESP32/ESP32/Subtopologies/ESP32CdhCore/ESP32CdhCoreConfig`
- `ESP32/ESP32/Subtopologies/ESP32ComCcsds/ESP32ComCcsdsConfig`
- `ESP32/ESP32/Deployments/Esp32RefUart/config/TlmChanImplCfg.hpp`
- `ESP32/ESP32/Deployments/Esp32RefWifi/config/TlmChanImplCfg.hpp`

This keeps shared `Svc/Subtopologies` defaults unchanged for non-ESP32 builds.

## Remaining Work
1. Re-stabilize the Wi-Fi deployment for full GDS traffic beyond simple command handling.
2. Re-enable selected Wi-Fi services incrementally once memory and task-stack budgets are characterized.
3. Add ESP32-focused unit or hardware smoke tests when the deployment shape settles.

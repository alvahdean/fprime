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
source "${FPRIME_REPO_ROOT}/ESP32/scripts/activate-env"
```

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

### UART deployment
```bash
cd "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefUart"
fprime-util generate esp32-idf
fprime-util build
```

`Esp32RefUart` hardware settings live in
`ESP32/ESP32/Deployments/Esp32RefUart/config/Esp32RefUartDeploymentCfg.hpp`.
After changing that header, rerun `fprime-util generate esp32-idf --force`.

### Wi-Fi deployment
```bash
cd "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefWifi"
fprime-util generate esp32-idf
fprime-util build
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

### `Esp32RefWifi`
- `fprime-util generate esp32-idf` verified
- `fprime-util build` verified
- flash image packaging verified
- hardware flashing verified
- supports both SoftAP and station-mode bring-up selected by provisioned `fprimecfg` NVS data
- UDP socket driver runs on top of the selected Wi-Fi mode
- Wi-Fi AP client connect/disconnect and STA connect/disconnect events are logged to the console

## ESP32 Memory Tuning
ESP32-specific memory reductions stay local to the library.

Local tuning lives in:
- `ESP32/ESP32/Subtopologies/ESP32CdhCore/ESP32CdhCoreConfig`
- `ESP32/ESP32/Subtopologies/ESP32ComCcsds/ESP32ComCcsdsConfig`
- `ESP32/ESP32/Deployments/Esp32RefUart/config/TlmChanImplCfg.hpp`
- `ESP32/ESP32/Deployments/Esp32RefWifi/config/TlmChanImplCfg.hpp`

This keeps shared `Svc/Subtopologies` defaults unchanged for non-ESP32 builds.

## Remaining Work
1. Connect the Wi-Fi deployment to a real GDS endpoint and validate packet flow in both SoftAP and station mode.
2. Validate the NVS-provisioned Wi-Fi flow against GDS once the post-DHCP runtime abort is fixed.
3. Add ESP32-focused unit or hardware smoke tests when the deployment shape settles.

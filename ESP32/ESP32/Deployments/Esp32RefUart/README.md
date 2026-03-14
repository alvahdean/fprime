# Esp32RefUart

Minimal ESP32 UART deployment for the `ESP32` F' library.

## What It Does
- builds as a normal F' deployment with `fprime-util`
- uses the shared `FreeRTOS` OSAL library
- communicates through `Drv::Esp32UartDriver`
- packages into a flashable ESP32 image

## Validated Commands
```bash
export FPRIME_REPO_ROOT="${HOME}/src/fprime.worktrees/esp32-support"
source "${HOME}/src/fprime/.venv/bin/activate"
source "${HOME}/.espressif/tools/activate_idf_v5.5.3.sh"

cd "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefUart"
fprime-util generate esp32-idf
fprime-util build

"${FPRIME_REPO_ROOT}/ESP32/scripts/build_flash_image.sh" .
"${FPRIME_REPO_ROOT}/ESP32/scripts/flash.sh" . --port /dev/ttyUSB0 --baud 460800
```

## Current Defaults
- UART number: `0`
- baud rate: `115200`

These defaults are set in `Main.cpp`.

## Current Status
- generate/build verified
- flash packaging verified
- hardware flashing verified
- intended as the main ESP32 bring-up path for eventual GDS communication

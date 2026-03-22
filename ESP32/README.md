# ESP32 Library

Formal F' library for ESP32 support.

This library owns:
- ESP32 platform and toolchain integration in `ESP32/cmake`
- ESP32-specific configuration in `ESP32/config`
- ESP32 drivers, services, subtopologies, and example deployments in `ESP32/ESP32`
- helper scripts for ESP-IDF wrapper generation, image packaging, flashing, and monitoring in `ESP32/scripts`

This library also consumes the shared FreeRTOS OSAL library from `FreeRTOS` instead of carrying a private FreeRTOS fork.

## Layout
- `ESP32/library.cmake`
- `ESP32/cmake/toolchain/esp32-idf.cmake`
- `ESP32/config/ESP32FreeRTOS`
- `ESP32/ESP32/Drv`
- `ESP32/ESP32/Svc`
- `ESP32/ESP32/Subtopologies`
- `ESP32/ESP32/Deployments`
- `ESP32/docs/README.md`

## Included Deployments
- `ESP32/ESP32/Deployments/Esp32RefUart`
  UART-based reference deployment for GDS-oriented bring-up.
- `ESP32/ESP32/Deployments/Esp32RefWifi`
  Wi-Fi transport deployment with a simple `Main.cpp` switch between SoftAP and station mode.

## Validated Flow
- `fprime-util generate esp32-idf`
- `fprime-util build`
- `ESP32/scripts/build_flash_image.sh <deployment-dir>`
- `ESP32/scripts/flash.sh <deployment-dir> --port /dev/ttyUSB0 [--baud 460800] [--monitor]`
- `ESP32/scripts/monitor.sh <deployment-dir> --port /dev/ttyUSB0 [--baud 115200]`
- `ESP32/scripts/provision_wifi_config.sh <deployment-dir> --port /dev/ttyUSB0 ...`

## Notes
- The ESP32 library expects the `FreeRTOS` library to be available through `library_locations`.
- The ESP-IDF wrapper setup is run automatically during `fprime-util generate esp32-idf`; no separate manual SDK configuration step is required.
- ESP32-specific memory reductions are localized to ESP32-owned subtopologies and deployment config overrides, not shared F' framework configs.
- `Esp32RefWifi` now reserves a dedicated `fprimecfg` NVS partition so Wi-Fi mode and credentials are provisioned from the host over USB instead of being compiled into the deployment.

See `ESP32/docs/README.md` for setup, build, packaging, flashing, and deployment details.

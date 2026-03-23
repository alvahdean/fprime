# ESP32 Scripts

Helper scripts for configuring the ESP32 build environment, packaging flash images, flashing hardware, and monitoring deployment output.

## Scripts
- [build-env.sh](/home/dfuqua/src/fprime/ESP32/scripts/build-env.sh)
  - Sources the expected Python and ESP-IDF environment for ESP32 builds.
  - Use `source build-env.sh` so the environment changes persist in the current shell.
- [build_flash_image.sh](/home/dfuqua/src/fprime/ESP32/scripts/build_flash_image.sh)
  - Packages a built deployment ELF into an ESP32 flashable application image.
  - Writes `idf-wrapper/build/fprime_flash_artifacts.json` for the flash and monitor helpers.
- [flash.sh](/home/dfuqua/src/fprime/ESP32/scripts/flash.sh)
  - Flashes a deployment to a target board using the packaged flash manifest.
  - If `<deployment-dir>/wifi.config` exists and is newer than the last provision stamp, reprovisions the Wi-Fi `fprimecfg` partition after flashing.
  - Can optionally start the serial monitor after a successful flash.
- [monitor.sh](/home/dfuqua/src/fprime/ESP32/scripts/monitor.sh)
  - Starts the ESP-IDF serial monitor for a deployment.
  - Uses the current directory as the deployment if no deployment path is provided.
- [prepare_idf_wrapper.sh](/home/dfuqua/src/fprime/ESP32/scripts/prepare_idf_wrapper.sh)
  - Prepares the ESP-IDF wrapper metadata consumed during CMake configure.
  - Normally invoked automatically by the ESP32 CMake flow.
- [provision_wifi_config.sh](/home/dfuqua/src/fprime/ESP32/scripts/provision_wifi_config.sh)
  - Provisions or clears Wi-Fi configuration in the ESP32 `fprimecfg` NVS partition for the Wi-Fi deployment.
- [redeploy.sh](/home/dfuqua/src/fprime/ESP32/scripts/redeploy.sh)
  - Convenience helper for regenerating, rebuilding, flashing, and monitoring a deployment from its directory.
  - Assumes the current working directory is an ESP32 deployment directory.

## Notes
- The preferred build flow is still `fprime-util generate` and `fprime-util build`.
- `esp32_flash_image` is the CMake target that wraps `build_flash_image.sh`, and the UART/Wi-Fi reference deployments also invoke packaging automatically as a deployment post-build step.
- `flash.sh` and `monitor.sh` expect wrapper build metadata and the flash manifest to exist.

# Esp32RefWifi

Minimal ESP32 Wi-Fi deployment for the `ESP32` F' library.

## What It Does
- builds as a normal F' deployment with `fprime-util`
- uses the shared `FreeRTOS` OSAL library
- communicates through `Drv::Esp32WifiDriver`
- supports both ESP32 SoftAP mode and station mode
- uses a TCP client transport to the configured ground endpoint
- publishes basic system memory telemetry every 5 seconds
- logs Wi-Fi connect/disconnect events to the serial console through the F' console writer
- packages into a flashable ESP32 image

## Validated Commands
```bash
export FPRIME_REPO_ROOT="${HOME}/src/fprime.worktrees/esp32-support"
source "${HOME}/src/fprime/venv/bin/activate"
source "${HOME}/.espressif/tools/activate_idf_v5.5.3.sh"

cd "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefWifi"
fprime-util generate esp32-idf
fprime-util build

"${FPRIME_REPO_ROOT}/ESP32/scripts/build_flash_image.sh" .
"${FPRIME_REPO_ROOT}/ESP32/scripts/flash.sh" . --port /dev/ttyUSB0 --baud 460800
"${FPRIME_REPO_ROOT}/ESP32/scripts/flash.sh" . --port /dev/ttyUSB0 --baud 460800 --monitor
"${FPRIME_REPO_ROOT}/ESP32/scripts/monitor.sh" . --port /dev/ttyUSB0
```

## Startup Behavior
This deployment no longer has compiled Wi-Fi defaults.

`Main.cpp` only reads Wi-Fi mode and network settings from the `fprimecfg` NVS partition.
If `fprimecfg` is empty or incomplete, the deployment will assert and Wi-Fi will not start.

## Current Status
- generate/build verified
- flash packaging verified
- hardware flashing verified
- SoftAP and station bring-up are both part of deployment startup
- TCP socket transport runs on top of the selected Wi-Fi mode
- Wi-Fi AP client connect/disconnect and STA connect/disconnect events are logged to the console
- custom ESP-IDF partitioning is used for this deployment
- a dedicated `fprimecfg` NVS partition is reserved for future deployment-owned Wi-Fi configuration
- Wi-Fi mode and credentials can now be provisioned from the host into `fprimecfg` over USB
- basic memory telemetry (`MEMORY_TOTAL`, `MEMORY_USED`) is emitted every 5 seconds
- Validated packet flow to a live GDS endpoint in both SoftAP and station mode

## USB Wi-Fi Provisioning
The deployment reads Wi-Fi configuration only from the `fprimecfg` NVS partition on boot.

The provisioning helper reads defaults from a local `wifi.config` file in this deployment directory when present, then applies command-line overrides on top.
`wifi.config` should remain untracked. Use `wifi.config.example` as the format reference.

Provision from local `wifi.config`:

```bash
export FPRIME_REPO_ROOT="<YOUR REPO CHECKOUT DIR>"
source "${FPRIME_REPO_ROOT}/ESP32/scripts/activate-env"

cd "ESP32/ESP32/Deployments/Esp32RefWifi"
"${FPRIME_REPO_ROOT}/ESP32/scripts/provision_wifi_config.sh" . \
  --port /dev/ttyUSB0
```

Provision with command-line overrides:

```bash
export FPRIME_REPO_ROOT="<YOUR REPO CHECKOUT DIR>"
source "${FPRIME_REPO_ROOT}/ESP32/scripts/activate-env"

cd "${FPRIME_REPO_ROOT}/ESP32/ESP32/Deployments/Esp32RefWifi"
"${FPRIME_REPO_ROOT}/ESP32/scripts/provision_wifi_config.sh" . \
  --port /dev/ttyUSB0 \
  --mode sta \
  --sta-ssid "Athens House Main" \
  --sta-pass "Athens20" \
  --sta-remote-ip 192.168.1.10
```

Clear provisioned config:

```bash
"${FPRIME_REPO_ROOT}/ESP32/scripts/provision_wifi_config.sh" . \
  --port /dev/ttyUSB0 \
  --clear
```

After `--clear`, the deployment will not bring up Wi-Fi again until configuration is reprovisioned.

Supported provisioned keys:
- `mode`
- `ap_ssid`, `ap_pass`, `ap_remote_ip`, `ap_remote_port`, `ap_local_port`, `ap_channel`, `ap_max_conn`
- `sta_ssid`, `sta_pass`, `sta_remote_ip`, `sta_remote_port`, `sta_local_port`

## Future work
- Add a live USB/GDS-based config update path if runtime updates are needed later
- Expand TCP interoperability testing
- Provide drivers for
  - GPIO
  - SPI
  - I2C
  - BLE
  - ADC
  - I2S
  - DAC
  - Hall Effect Sensor
  - Onboard LED

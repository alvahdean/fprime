#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'USAGE'
Usage: provision_wifi_config.sh <deployment-dir> [options]

Generate and flash the `fprimecfg` NVS partition for an ESP32 deployment.
This writes Wi-Fi mode/credential overrides that the deployment reads on boot.
The script reads defaults from `<deployment-dir>/wifi.config` when present,
then applies command-line overrides on top.

Required:
  <deployment-dir>              Deployment root (e.g. .../Deployments/Esp32RefWifi)

Optional transport:
  --port <device>               Serial device in WSL (/dev/ttyUSB* or /dev/ttyACM*)
  --baud <value>                Flash baud rate (default: 460800)

Optional config values:
  --mode <ap|sta>
  --ap-ssid <value>
  --ap-pass <value>
  --ap-remote-ip <value>
  --ap-remote-port <value>
  --ap-local-port <value>
  --ap-channel <value>
  --ap-max-conn <value>
  --sta-ssid <value>
  --sta-pass <value>
  --sta-remote-ip <value>
  --sta-remote-port <value>
  --sta-local-port <value>

Maintenance:
  --clear                       Erase the `fprimecfg` partition so compiled defaults are used again

Examples:
  provision_wifi_config.sh . --port /dev/ttyUSB0

  provision_wifi_config.sh . --port /dev/ttyUSB0 --clear
USAGE
}

if [[ $# -lt 1 ]]; then
    usage
    exit 1
fi

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

DEPLOY_DIR="$(realpath "$1")"
shift

CONFIG_FILE="${DEPLOY_DIR}/wifi.config"

PORT=""
BAUD="460800"
CLEAR_CONFIG=0

MODE=""
AP_SSID=""
AP_PASS=""
AP_REMOTE_IP=""
AP_REMOTE_PORT=""
AP_LOCAL_PORT=""
AP_CHANNEL=""
AP_MAX_CONN=""
STA_SSID=""
STA_PASS=""
STA_REMOTE_IP=""
STA_REMOTE_PORT=""
STA_LOCAL_PORT=""

if [[ -f "${CONFIG_FILE}" ]]; then
    # shellcheck disable=SC1090
    source "${CONFIG_FILE}"
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        --port) PORT="$2"; shift 2 ;;
        --baud) BAUD="$2"; shift 2 ;;
        --mode) MODE="$2"; shift 2 ;;
        --ap-ssid) AP_SSID="$2"; shift 2 ;;
        --ap-pass) AP_PASS="$2"; shift 2 ;;
        --ap-remote-ip) AP_REMOTE_IP="$2"; shift 2 ;;
        --ap-remote-port) AP_REMOTE_PORT="$2"; shift 2 ;;
        --ap-local-port) AP_LOCAL_PORT="$2"; shift 2 ;;
        --ap-channel) AP_CHANNEL="$2"; shift 2 ;;
        --ap-max-conn) AP_MAX_CONN="$2"; shift 2 ;;
        --sta-ssid) STA_SSID="$2"; shift 2 ;;
        --sta-pass) STA_PASS="$2"; shift 2 ;;
        --sta-remote-ip) STA_REMOTE_IP="$2"; shift 2 ;;
        --sta-remote-port) STA_REMOTE_PORT="$2"; shift 2 ;;
        --sta-local-port) STA_LOCAL_PORT="$2"; shift 2 ;;
        --clear) CLEAR_CONFIG=1; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    esac
done

if [[ ! -d "${DEPLOY_DIR}" ]]; then
    echo "Deployment directory does not exist: ${DEPLOY_DIR}" >&2
    exit 1
fi

if [[ -z "${IDF_PATH:-}" || ! -f "${IDF_PATH}/tools/idf.py" ]]; then
    echo "IDF_PATH is not set correctly. Source ESP-IDF environment first." >&2
    exit 1
fi

ESPTOOL="${IDF_PATH}/components/esptool_py/esptool/esptool.py"
NVS_GEN="${IDF_PATH}/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py"

if [[ ! -f "${ESPTOOL}" ]]; then
    echo "esptool.py not found at: ${ESPTOOL}" >&2
    exit 1
fi

if [[ ! -f "${NVS_GEN}" ]]; then
    echo "nvs_partition_gen.py not found at: ${NVS_GEN}" >&2
    exit 1
fi

if [[ -n "${IDF_PYTHON_ENV_PATH:-}" && -x "${IDF_PYTHON_ENV_PATH}/bin/python" ]]; then
    IDF_PYTHON_BIN="${IDF_PYTHON_ENV_PATH}/bin/python"
else
    IDF_PYTHON_BIN="python3"
fi

PARTITION_TABLE_CSV="${DEPLOY_DIR}/idf-wrapper/partitions_fprime.csv"
if [[ ! -f "${PARTITION_TABLE_CSV}" ]]; then
    echo "Expected deployment partition table not found: ${PARTITION_TABLE_CSV}" >&2
    exit 1
fi

if [[ -z "${PORT}" ]]; then
    shopt -s nullglob
    CANDIDATES=(/dev/ttyUSB* /dev/ttyACM*)
    shopt -u nullglob

    if [[ ${#CANDIDATES[@]} -eq 1 ]]; then
        PORT="${CANDIDATES[0]}"
    elif [[ ${#CANDIDATES[@]} -gt 1 ]]; then
        echo "Multiple serial devices found. Re-run with --port:" >&2
        printf '  %s\n' "${CANDIDATES[@]}" >&2
        exit 1
    else
        echo "No /dev/ttyUSB* or /dev/ttyACM* device found in WSL." >&2
        exit 1
    fi
fi

if [[ ! -e "${PORT}" ]]; then
    echo "Port does not exist: ${PORT}" >&2
    exit 1
fi

if [[ -n "${MODE}" ]]; then
    case "${MODE}" in
        ap|sta) ;;
        *) echo "--mode must be 'ap' or 'sta'" >&2; exit 1 ;;
    esac
fi

if [[ ${CLEAR_CONFIG} -eq 0 && -z "${MODE}${AP_SSID}${AP_PASS}${AP_REMOTE_IP}${AP_REMOTE_PORT}${AP_LOCAL_PORT}${AP_CHANNEL}${AP_MAX_CONN}${STA_SSID}${STA_PASS}${STA_REMOTE_IP}${STA_REMOTE_PORT}${STA_LOCAL_PORT}" ]]; then
    echo "No config values were provided. Use --clear or pass one or more Wi-Fi settings." >&2
    exit 1
fi

if [[ ${CLEAR_CONFIG} -eq 0 ]]; then
    if [[ -z "${MODE}" ]]; then
        echo "Wi-Fi config must define MODE in wifi.config or on the command line." >&2
        exit 1
    fi

    if [[ "${MODE}" == "ap" ]]; then
        [[ -n "${AP_SSID}" ]] || { echo "AP mode requires AP_SSID" >&2; exit 1; }
        [[ -n "${AP_REMOTE_IP}" ]] || { echo "AP mode requires AP_REMOTE_IP" >&2; exit 1; }
        [[ -n "${AP_REMOTE_PORT}" ]] || { echo "AP mode requires AP_REMOTE_PORT" >&2; exit 1; }
        [[ -n "${AP_LOCAL_PORT}" ]] || { echo "AP mode requires AP_LOCAL_PORT" >&2; exit 1; }
        [[ -n "${AP_CHANNEL}" ]] || { echo "AP mode requires AP_CHANNEL" >&2; exit 1; }
        [[ -n "${AP_MAX_CONN}" ]] || { echo "AP mode requires AP_MAX_CONN" >&2; exit 1; }
    else
        [[ -n "${STA_SSID}" ]] || { echo "STA mode requires STA_SSID" >&2; exit 1; }
        [[ -n "${STA_REMOTE_IP}" ]] || { echo "STA mode requires STA_REMOTE_IP" >&2; exit 1; }
        [[ -n "${STA_REMOTE_PORT}" ]] || { echo "STA mode requires STA_REMOTE_PORT" >&2; exit 1; }
        [[ -n "${STA_LOCAL_PORT}" ]] || { echo "STA mode requires STA_LOCAL_PORT" >&2; exit 1; }
    fi
fi

read -r PARTITION_OFFSET PARTITION_SIZE < <(
    "${IDF_PYTHON_BIN}" - "${PARTITION_TABLE_CSV}" <<'PY'
import csv
import sys

with open(sys.argv[1], newline="", encoding="utf-8") as stream:
    for row in csv.reader(stream):
        if not row or row[0].startswith("#"):
            continue
        if row[0].strip() != "fprimecfg":
            continue
        offset = int(row[3], 0)
        size = int(row[4], 0)
        print(hex(offset), str(size))
        break
    else:
        raise SystemExit("fprimecfg partition not found")
PY
)

if [[ ${CLEAR_CONFIG} -eq 1 ]]; then
    echo "Erasing fprimecfg partition at ${PARTITION_OFFSET} (${PARTITION_SIZE} bytes)..."
    "${IDF_PYTHON_BIN}" "${ESPTOOL}" \
        --chip esp32 \
        --port "${PORT}" \
        --baud "${BAUD}" \
        --before default_reset \
        --after hard_reset \
        erase_region "${PARTITION_OFFSET}" "${PARTITION_SIZE}"
    echo "Wi-Fi config cleared. The deployment will refuse to start Wi-Fi until new config is provisioned."
    exit 0
fi

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

export MODE AP_SSID AP_PASS AP_REMOTE_IP AP_REMOTE_PORT AP_LOCAL_PORT AP_CHANNEL AP_MAX_CONN
export STA_SSID STA_PASS STA_REMOTE_IP STA_REMOTE_PORT STA_LOCAL_PORT

CSV_PATH="${TMP_DIR}/wifi_config.csv"
BIN_PATH="${TMP_DIR}/wifi_config.bin"

"${IDF_PYTHON_BIN}" - "${CSV_PATH}" <<'PY'
import csv
import os
import sys

mode_map = {"ap": 0, "sta": 1}
rows = [["key", "type", "encoding", "value"], ["wifi", "namespace", "", ""]]

def maybe_add(name: str, encoding: str, value: str):
    if value != "":
        rows.append([name, "data", encoding, value])

mode = os.environ.get("MODE", "")
if mode:
    rows.append(["mode", "data", "u8", str(mode_map[mode])])

maybe_add("ap_ssid", "string", os.environ.get("AP_SSID", ""))
maybe_add("ap_pass", "string", os.environ.get("AP_PASS", ""))
maybe_add("ap_remote_ip", "string", os.environ.get("AP_REMOTE_IP", ""))
maybe_add("ap_remote_port", "u16", os.environ.get("AP_REMOTE_PORT", ""))
maybe_add("ap_local_port", "u16", os.environ.get("AP_LOCAL_PORT", ""))
maybe_add("ap_channel", "u8", os.environ.get("AP_CHANNEL", ""))
maybe_add("ap_max_conn", "u8", os.environ.get("AP_MAX_CONN", ""))
maybe_add("sta_ssid", "string", os.environ.get("STA_SSID", ""))
maybe_add("sta_pass", "string", os.environ.get("STA_PASS", ""))
maybe_add("sta_remote_ip", "string", os.environ.get("STA_REMOTE_IP", ""))
maybe_add("sta_remote_port", "u16", os.environ.get("STA_REMOTE_PORT", ""))
maybe_add("sta_local_port", "u16", os.environ.get("STA_LOCAL_PORT", ""))

with open(sys.argv[1], "w", newline="", encoding="utf-8") as stream:
    writer = csv.writer(stream)
    writer.writerows(rows)
PY

"${IDF_PYTHON_BIN}" "${NVS_GEN}" generate "${CSV_PATH}" "${BIN_PATH}" "${PARTITION_SIZE}"

echo "Flashing Wi-Fi config to ${PORT} at ${BAUD}..."
"${IDF_PYTHON_BIN}" "${ESPTOOL}" \
    --chip esp32 \
    --port "${PORT}" \
    --baud "${BAUD}" \
    --before default_reset \
    --after hard_reset \
    write_flash "${PARTITION_OFFSET}" "${BIN_PATH}"

echo "Wi-Fi config written to fprimecfg. Reboot complete."

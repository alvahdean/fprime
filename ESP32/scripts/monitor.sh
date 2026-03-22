#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat >&2 <<'EOF'
Usage: monitor.sh <deployment-dir> --port <serial-port> [--baud <rate>] [--timestamps] [--no-reset]
Example:
  monitor.sh $HOME/src/fprime/ESP32/ESP32/Deployments/Esp32RefUart --port /dev/ttyUSB0

Notes:
  Exit the ESP-IDF monitor with Ctrl-]
EOF
}

if [[ $# -lt 1 ]]; then
    usage
    exit 1
fi

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    usage
    exit 0
fi

DEPLOY_DIR="$(realpath "$1")"
shift

PORT=""
BAUD=""
TIMESTAMPS="false"
NO_RESET="false"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --port)
            if [[ $# -lt 2 ]]; then
                echo "Missing value for --port" >&2
                exit 1
            fi
            PORT="$2"
            shift 2
            ;;
        --baud)
            if [[ $# -lt 2 ]]; then
                echo "Missing value for --baud" >&2
                exit 1
            fi
            BAUD="$2"
            shift 2
            ;;
        --timestamps)
            TIMESTAMPS="true"
            shift
            ;;
        --no-reset)
            NO_RESET="true"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage
            exit 1
            ;;
    esac
done

if [[ -z "${PORT}" ]]; then
    echo "--port is required" >&2
    usage
    exit 1
fi

if [[ ! -d "${DEPLOY_DIR}" ]]; then
    echo "Deployment directory does not exist: ${DEPLOY_DIR}" >&2
    exit 1
fi

MANIFEST_PATH="${DEPLOY_DIR}/idf-wrapper/build/fprime_flash_artifacts.json"
PROJECT_DESCRIPTION_JSON="${DEPLOY_DIR}/idf-wrapper/build/project_description.json"

if [[ ! -f "${MANIFEST_PATH}" ]]; then
    echo "Missing flash manifest: ${MANIFEST_PATH}" >&2
    echo "Run ESP32/scripts/build_flash_image.sh ${DEPLOY_DIR} first." >&2
    exit 1
fi

if [[ ! -f "${PROJECT_DESCRIPTION_JSON}" ]]; then
    echo "Missing wrapper project description: ${PROJECT_DESCRIPTION_JSON}" >&2
    exit 1
fi

readarray -t MONITOR_INFO < <(
    python3 - "${MANIFEST_PATH}" "${PROJECT_DESCRIPTION_JSON}" <<'PY'
import json
import pathlib
import sys

manifest = json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8"))
project = json.loads(pathlib.Path(sys.argv[2]).read_text(encoding="utf-8"))

fprime_elf = pathlib.Path(manifest["fprime_elf"]).resolve()
bootloader_elf = pathlib.Path(project["bootloader_elf"]).resolve()

if not fprime_elf.is_file():
    raise SystemExit(f"Missing deployment ELF: {fprime_elf}")
if not bootloader_elf.is_file():
    raise SystemExit(f"Missing bootloader ELF: {bootloader_elf}")

print(fprime_elf)
print(bootloader_elf)
print(project.get("monitor_baud", "115200"))
print(project.get("monitor_toolprefix", "xtensa-esp32-elf-"))
print(project.get("target", "esp32"))
print(project.get("rev", "0"))
print(project.get("idf_path", ""))
PY
)

if [[ ${#MONITOR_INFO[@]} -lt 7 ]]; then
    echo "Monitor metadata is incomplete for ${DEPLOY_DIR}" >&2
    exit 1
fi

FPRIME_ELF="${MONITOR_INFO[0]}"
BOOTLOADER_ELF="${MONITOR_INFO[1]}"
DEFAULT_BAUD="${MONITOR_INFO[2]}"
TOOLCHAIN_PREFIX="${MONITOR_INFO[3]}"
TARGET_NAME="${MONITOR_INFO[4]}"
TARGET_REV="${MONITOR_INFO[5]}"
IDF_PATH_FROM_BUILD="${MONITOR_INFO[6]}"

if [[ -z "${BAUD}" ]]; then
    BAUD="${DEFAULT_BAUD}"
fi

if [[ -n "${IDF_PYTHON_ENV_PATH:-}" && -x "${IDF_PYTHON_ENV_PATH}/bin/idf-monitor" ]]; then
    IDF_MONITOR="${IDF_PYTHON_ENV_PATH}/bin/idf-monitor"
elif [[ -x "${HOME}/.espressif/tools/python/v5.5.3/venv/bin/idf-monitor" ]]; then
    IDF_MONITOR="${HOME}/.espressif/tools/python/v5.5.3/venv/bin/idf-monitor"
else
    IDF_MONITOR="idf-monitor"
fi

if [[ -n "${IDF_PATH:-}" ]]; then
    export IDF_PATH
elif [[ -n "${IDF_PATH_FROM_BUILD}" ]]; then
    export IDF_PATH="${IDF_PATH_FROM_BUILD}"
fi

CMD=(
    "${IDF_MONITOR}"
    --port "${PORT}"
    --baud "${BAUD}"
    --toolchain-prefix "${TOOLCHAIN_PREFIX}"
    --target "${TARGET_NAME}"
    --revision "${TARGET_REV}"
)

if [[ "${NO_RESET}" == "true" ]]; then
    CMD+=(--no-reset)
fi

if [[ "${TIMESTAMPS}" == "true" ]]; then
    CMD+=(--timestamps)
fi

CMD+=(
    "${FPRIME_ELF}"
    "${BOOTLOADER_ELF}"
)

echo "Starting monitor for ${DEPLOY_DIR} on ${PORT} at ${BAUD} baud"
echo "Exit monitor with Ctrl-]"
exec "${CMD[@]}"

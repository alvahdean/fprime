#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat >&2 <<'EOF'
Usage: flash.sh <deployment-dir> --port <serial-port> [--baud <rate>] [--monitor]
Example:
  flash.sh $HOME/src/fprime/ESP32/ESP32/Deployments/Esp32RefUart --port /dev/ttyUSB0 --baud 460800 --monitor
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
BAUD="460800"
START_MONITOR="false"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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
        --monitor)
            START_MONITOR="true"
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
if [[ ! -f "${MANIFEST_PATH}" ]]; then
    echo "Missing flash manifest: ${MANIFEST_PATH}" >&2
    echo "Run ESP32/scripts/build_flash_image.sh ${DEPLOY_DIR} first." >&2
    exit 1
fi

readarray -t FLASH_INFO < <(
    python3 - "${MANIFEST_PATH}" <<'PY'
import json
import pathlib
import sys

manifest = json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8"))

extra = manifest.get("extra_esptool_args", {})
write_flash_args = manifest.get("write_flash_args", [])
flash_files = manifest.get("flash_files", {})

print(extra.get("chip", "esp32"))
print(extra.get("before", "default_reset"))
print(extra.get("after", "hard_reset"))
print("true" if extra.get("stub", True) else "false")

for value in write_flash_args:
    print(f"ARG\t{value}")

for offset, path in sorted(flash_files.items(), key=lambda item: int(item[0], 0)):
    artifact = pathlib.Path(path)
    if not artifact.is_file():
        raise SystemExit(f"Missing flash artifact for {offset}: {artifact}")
    print(f"FILE\t{offset}\t{artifact}")
PY
)

if [[ ${#FLASH_INFO[@]} -lt 4 ]]; then
    echo "Flash manifest is incomplete: ${MANIFEST_PATH}" >&2
    exit 1
fi

CHIP="${FLASH_INFO[0]}"
BEFORE="${FLASH_INFO[1]}"
AFTER="${FLASH_INFO[2]}"
USE_STUB="${FLASH_INFO[3]}"

WRITE_FLASH_ARGS=()
FLASH_FILES=()

for entry in "${FLASH_INFO[@]:4}"; do
    if [[ "${entry}" == ARG$'\t'* ]]; then
        WRITE_FLASH_ARGS+=("${entry#ARG$'\t'}")
    elif [[ "${entry}" == FILE$'\t'* ]]; then
        payload="${entry#FILE$'\t'}"
        offset="${payload%%$'\t'*}"
        file_path="${payload#*$'\t'}"
        FLASH_FILES+=("${offset}" "${file_path}")
    fi
done

if [[ ${#FLASH_FILES[@]} -eq 0 ]]; then
    echo "No flash artifacts were found in ${MANIFEST_PATH}" >&2
    exit 1
fi

if [[ -n "${IDF_PYTHON_ENV_PATH:-}" && -x "${IDF_PYTHON_ENV_PATH}/bin/python" ]]; then
    IDF_PYTHON_BIN="${IDF_PYTHON_ENV_PATH}/bin/python"
elif [[ -x "${HOME}/.espressif/tools/python/v5.5.3/venv/bin/python" ]]; then
    IDF_PYTHON_BIN="${HOME}/.espressif/tools/python/v5.5.3/venv/bin/python"
else
    IDF_PYTHON_BIN="python3"
fi

if [[ -n "${IDF_PATH:-}" ]]; then
    EFFECTIVE_IDF_PATH="${IDF_PATH}"
else
    EFFECTIVE_IDF_PATH="$(python3 - "${DEPLOY_DIR}/idf-wrapper/build/project_description.json" <<'PY'
import json
import pathlib
import sys

project_description = pathlib.Path(sys.argv[1])
if not project_description.is_file():
    raise SystemExit("Missing project_description.json; run the wrapper build first.")
print(json.loads(project_description.read_text(encoding="utf-8")).get("idf_path", ""))
PY
)"
fi

ESPTOOL_PY="${EFFECTIVE_IDF_PATH}/components/esptool_py/esptool/esptool.py"
if [[ ! -f "${ESPTOOL_PY}" ]]; then
    echo "Unable to find esptool.py at: ${ESPTOOL_PY}" >&2
    exit 1
fi

CMD=(
    "${IDF_PYTHON_BIN}" "${ESPTOOL_PY}"
    --chip "${CHIP}"
    -b "${BAUD}"
    --before "${BEFORE}"
    --after "${AFTER}"
    -p "${PORT}"
)

if [[ "${USE_STUB}" != "true" ]]; then
    CMD+=(--no-stub)
fi

CMD+=(write_flash)
CMD+=("${WRITE_FLASH_ARGS[@]}")
CMD+=("${FLASH_FILES[@]}")

echo "Flashing ${DEPLOY_DIR} on ${PORT} at ${BAUD} baud"
"${CMD[@]}"

WIFI_CONFIG_PATH="${DEPLOY_DIR}/wifi.config"
WIFI_PROVISION_SCRIPT="${SCRIPT_DIR}/provision_wifi_config.sh"
WIFI_PROVISION_STAMP="${DEPLOY_DIR}/idf-wrapper/build/fprimecfg_provisioned.stamp"

if [[ -f "${WIFI_CONFIG_PATH}" && -x "${WIFI_PROVISION_SCRIPT}" ]]; then
    SHOULD_PROVISION="false"
    if [[ ! -f "${WIFI_PROVISION_STAMP}" ]]; then
        SHOULD_PROVISION="true"
    elif [[ "${WIFI_CONFIG_PATH}" -nt "${WIFI_PROVISION_STAMP}" ]]; then
        SHOULD_PROVISION="true"
    fi

    if [[ "${SHOULD_PROVISION}" == "true" ]]; then
        echo "Detected updated Wi-Fi config at ${WIFI_CONFIG_PATH}; provisioning fprimecfg partition"
        "${WIFI_PROVISION_SCRIPT}" "${DEPLOY_DIR}" --port "${PORT}" --baud "${BAUD}"
        touch -r "${WIFI_CONFIG_PATH}" "${WIFI_PROVISION_STAMP}"
    fi
fi

if [[ "${START_MONITOR}" == "true" ]]; then
    MONITOR_SCRIPT="${SCRIPT_DIR}/monitor.sh"
    if [[ ! -x "${MONITOR_SCRIPT}" ]]; then
        echo "Flash completed, but monitor helper is missing or not executable: ${MONITOR_SCRIPT}" >&2
        exit 1
    fi

    echo "Flash complete. Starting monitor on ${PORT}"
    exec "${MONITOR_SCRIPT}" "${DEPLOY_DIR}" --port "${PORT}"
fi

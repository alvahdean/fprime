#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <deployment-dir>" >&2
    echo "Example: $0 \$HOME/src/fprime/ESP32/ESP32/Deployments/Esp32RefUart" >&2
    exit 1
fi

DEPLOY_DIR="$(realpath "$1")"
DEPLOY_NAME="$(basename "${DEPLOY_DIR}")"
IDF_WRAPPER_BUILD_DIR="${DEPLOY_DIR}/idf-wrapper/build"
FLASHER_ARGS_JSON="${IDF_WRAPPER_BUILD_DIR}/flasher_args.json"
PROJECT_DESCRIPTION_JSON="${IDF_WRAPPER_BUILD_DIR}/project_description.json"
FPRIME_FLASH_MANIFEST="${IDF_WRAPPER_BUILD_DIR}/fprime_flash_artifacts.json"

if [[ ! -d "${DEPLOY_DIR}" ]]; then
    echo "Deployment directory does not exist: ${DEPLOY_DIR}" >&2
    exit 1
fi

if [[ ! -d "${IDF_WRAPPER_BUILD_DIR}" ]]; then
    echo "Missing IDF wrapper build directory: ${IDF_WRAPPER_BUILD_DIR}" >&2
    echo "Run 'fprime-util generate esp32-idf' and 'fprime-util build' first." >&2
    exit 1
fi

if [[ ! -f "${FLASHER_ARGS_JSON}" ]]; then
    echo "Missing flasher args file: ${FLASHER_ARGS_JSON}" >&2
    echo "Run the ESP-IDF wrapper build first." >&2
    exit 1
fi

if [[ ! -f "${PROJECT_DESCRIPTION_JSON}" ]]; then
    echo "Missing project description: ${PROJECT_DESCRIPTION_JSON}" >&2
    exit 1
fi

readarray -t BUILD_INFO < <(
    python3 - "${DEPLOY_DIR}" "${DEPLOY_NAME}" <<'PY'
import pathlib
import sys

deploy_dir = pathlib.Path(sys.argv[1])
deploy_name = sys.argv[2]

candidates = sorted(
    path for path in deploy_dir.glob("build*-esp32-idf")
    if path.is_dir() and (path / "bin" / "esp32-idf" / deploy_name).is_file()
)

if not candidates:
    raise SystemExit(
        "Unable to locate an ESP32 F' build directory containing "
        f"bin/esp32-idf/{deploy_name}"
    )

build_dir = candidates[0].resolve()
elf_path = (build_dir / "bin" / "esp32-idf" / deploy_name).resolve()
bin_path = elf_path.with_suffix(".bin")

print(build_dir)
print(elf_path)
print(bin_path)
PY
)

FPRIME_BUILD_DIR="${BUILD_INFO[0]}"
FPRIME_ELF="${BUILD_INFO[1]}"
FPRIME_APP_BIN="${BUILD_INFO[2]}"

if [[ ! -f "${FPRIME_ELF}" ]]; then
    echo "Missing F' deployment ELF: ${FPRIME_ELF}" >&2
    exit 1
fi

readarray -t IDF_INFO < <(
    python3 - "${PROJECT_DESCRIPTION_JSON}" "${FLASHER_ARGS_JSON}" <<'PY'
import json
import pathlib
import sys

project_description = json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8"))
flasher_args = json.loads(pathlib.Path(sys.argv[2]).read_text(encoding="utf-8"))

idf_path = project_description.get("idf_path", "")
app = flasher_args.get("app", {})
extra = flasher_args.get("extra_esptool_args", {})

print(idf_path)
print(app.get("offset", ""))
print(extra.get("chip", "esp32"))
PY
)

IDF_PATH_FROM_BUILD="${IDF_INFO[0]}"
APP_OFFSET="${IDF_INFO[1]}"
IDF_TARGET="${IDF_INFO[2]}"

if [[ -n "${IDF_PATH:-}" ]]; then
    EFFECTIVE_IDF_PATH="${IDF_PATH}"
elif [[ -n "${IDF_PATH_FROM_BUILD}" ]]; then
    EFFECTIVE_IDF_PATH="${IDF_PATH_FROM_BUILD}"
else
    echo "IDF_PATH is not set and could not be inferred from wrapper metadata." >&2
    exit 1
fi

if [[ ! -d "${EFFECTIVE_IDF_PATH}" ]]; then
    echo "Resolved IDF_PATH does not exist: ${EFFECTIVE_IDF_PATH}" >&2
    exit 1
fi

if [[ -n "${IDF_PYTHON_ENV_PATH:-}" && -x "${IDF_PYTHON_ENV_PATH}/bin/python" ]]; then
    IDF_PYTHON_BIN="${IDF_PYTHON_ENV_PATH}/bin/python"
elif [[ -x "${HOME}/.espressif/tools/python/v5.5.3/venv/bin/python" ]]; then
    IDF_PYTHON_BIN="${HOME}/.espressif/tools/python/v5.5.3/venv/bin/python"
else
    IDF_PYTHON_BIN="python3"
fi

ESPTOOL_PY="${EFFECTIVE_IDF_PATH}/components/esptool_py/esptool/esptool.py"

if [[ ! -f "${ESPTOOL_PY}" ]]; then
    echo "Unable to find esptool.py at: ${ESPTOOL_PY}" >&2
    exit 1
fi

readarray -t ELF2IMAGE_CMD < <(
    python3 - "${IDF_WRAPPER_BUILD_DIR}/build.ninja" "${DEPLOY_NAME}" "${FPRIME_APP_BIN}" "${FPRIME_ELF}" <<'PY'
import pathlib
import shlex
import sys

build_ninja = pathlib.Path(sys.argv[1])
deploy_name = sys.argv[2]
output_bin = sys.argv[3]
input_elf = sys.argv[4]

command_line = None
for raw_line in build_ninja.read_text(encoding="utf-8").splitlines():
    stripped = raw_line.strip()
    if "esptool.py --chip " not in stripped or " elf2image " not in stripped:
        continue
    if "IdfConfig.bin" not in stripped:
        continue
    command_line = stripped.split("COMMAND = ", 1)[-1]
    break

if command_line is None:
    raise SystemExit(f"Unable to find the elf2image command in {build_ninja}")

segments = [segment.strip() for segment in command_line.split("&&")]
elf2image_segment = next((segment for segment in segments if "esptool.py" in segment and " elf2image " in segment), None)
if elf2image_segment is None:
    raise SystemExit(f"Unable to isolate elf2image segment from command: {command_line}")

tokens = shlex.split(elf2image_segment)
try:
    output_index = tokens.index("-o") + 1
except ValueError as error:
    raise SystemExit(f"Missing -o in elf2image command: {elf2image_segment}") from error

tokens[output_index] = output_bin
tokens[-1] = input_elf

for token in tokens:
    print(token)
PY
)

if [[ ${#ELF2IMAGE_CMD[@]} -eq 0 ]]; then
    echo "Unable to derive the IDF elf2image command from wrapper build output." >&2
    exit 1
fi

mkdir -p "$(dirname "${FPRIME_APP_BIN}")"

"${ELF2IMAGE_CMD[@]}"

python3 - "${FLASHER_ARGS_JSON}" "${IDF_WRAPPER_BUILD_DIR}" "${FPRIME_APP_BIN}" "${FPRIME_FLASH_MANIFEST}" "${FPRIME_BUILD_DIR}" "${FPRIME_ELF}" "${DEPLOY_NAME}" <<'PY'
import json
import pathlib
import sys
from datetime import datetime, timezone

flasher_args_path = pathlib.Path(sys.argv[1]).resolve()
idf_wrapper_build_dir = pathlib.Path(sys.argv[2]).resolve()
fprime_app_bin = pathlib.Path(sys.argv[3]).resolve()
manifest_path = pathlib.Path(sys.argv[4]).resolve()
fprime_build_dir = pathlib.Path(sys.argv[5]).resolve()
fprime_elf = pathlib.Path(sys.argv[6]).resolve()
deployment = sys.argv[7]

flasher_args = json.loads(flasher_args_path.read_text(encoding="utf-8"))

def resolve_artifact(relative_path: str) -> str:
    return str((idf_wrapper_build_dir / relative_path).resolve())

manifest = {
    "version": 1,
    "generated_at_utc": datetime.now(timezone.utc).isoformat(),
    "deployment": deployment,
    "fprime_build_dir": str(fprime_build_dir),
    "fprime_elf": str(fprime_elf),
    "fprime_app_bin": str(fprime_app_bin),
    "write_flash_args": flasher_args.get("write_flash_args", []),
    "flash_settings": flasher_args.get("flash_settings", {}),
    "extra_esptool_args": flasher_args.get("extra_esptool_args", {}),
    "flash_files": {},
}

for offset, artifact in flasher_args.get("flash_files", {}).items():
    manifest["flash_files"][offset] = str(fprime_app_bin) if offset == flasher_args.get("app", {}).get("offset") else resolve_artifact(artifact)

manifest["bootloader"] = {
    **flasher_args.get("bootloader", {}),
    "file": resolve_artifact(flasher_args.get("bootloader", {}).get("file", "")),
}
manifest["partition-table"] = {
    **flasher_args.get("partition-table", {}),
    "file": resolve_artifact(flasher_args.get("partition-table", {}).get("file", "")),
}
manifest["app"] = {
    **flasher_args.get("app", {}),
    "file": str(fprime_app_bin),
}

manifest_path.write_text(json.dumps(manifest, indent=4) + "\n", encoding="utf-8")
PY

echo "Packaged flash image:"
echo "  ELF:      ${FPRIME_ELF}"
echo "  App bin:  ${FPRIME_APP_BIN}"
echo "  Offset:   ${APP_OFFSET}"
echo "  Manifest: ${FPRIME_FLASH_MANIFEST}"

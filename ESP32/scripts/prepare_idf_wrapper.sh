#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <deployment-dir> [jobs]"
    echo "Example: $0 \$HOME/src/fprime/ESP32/ESP32/Deployments/Esp32RefUart 8"
    exit 1
fi

DEPLOY_DIR="$(realpath "$1")"
JOBS="${2:-8}"
IDF_WRAPPER_DIR="${DEPLOY_DIR}/idf-wrapper"
IDF_WRAPPER_BUILD_DIR="${IDF_WRAPPER_DIR}/build"
SDKCONFIG_INCLUDE_DIR="${IDF_WRAPPER_BUILD_DIR}/config"
IDF_INCLUDE_DIRS_FILE="${IDF_WRAPPER_BUILD_DIR}/fprime_idf_include_dirs.cmake"
IDF_LINK_INFO_FILE="${IDF_WRAPPER_BUILD_DIR}/fprime_idf_link_info.cmake"
IDF_LINK_PROFILE="uart"

if [[ "${DEPLOY_DIR}" == *"Esp32RefWifi"* ]]; then
    IDF_LINK_PROFILE="wifi"
fi

if [[ ! -d "${DEPLOY_DIR}" ]]; then
    echo "Deployment directory does not exist: ${DEPLOY_DIR}"
    exit 1
fi

if [[ ! -d "${IDF_WRAPPER_DIR}" ]]; then
    echo "Missing IDF wrapper directory: ${IDF_WRAPPER_DIR}"
    exit 1
fi

if [[ -z "${IDF_PATH:-}" || ! -f "${IDF_PATH}/tools/idf.py" ]]; then
    echo "IDF_PATH is not set correctly. Source ESP-IDF environment first."
    exit 1
fi

if [[ -n "${IDF_PYTHON_ENV_PATH:-}" && -x "${IDF_PYTHON_ENV_PATH}/bin/python" ]]; then
    IDF_PYTHON_BIN="${IDF_PYTHON_ENV_PATH}/bin/python"
else
    IDF_PYTHON_BIN="python3"
fi

if IDF_EXPORT_SCRIPT="$("${IDF_PYTHON_BIN}" "${IDF_PATH}/tools/idf_tools.py" export 2>/dev/null)"; then
    eval "${IDF_EXPORT_SCRIPT}"
fi

if ! command -v xtensa-esp32-elf-gcc >/dev/null 2>&1; then
    if [[ -n "${IDF_TOOLS_PATH:-}" ]]; then
        XTENSA_GCC="$(find "${IDF_TOOLS_PATH}/tools/xtensa-esp-elf" -type f -name xtensa-esp32-elf-gcc 2>/dev/null | head -n 1 || true)"
    else
        XTENSA_GCC="$(find "${HOME}/.espressif/tools" -type f -name xtensa-esp32-elf-gcc 2>/dev/null | head -n 1 || true)"
    fi
    if [[ -n "${XTENSA_GCC:-}" ]]; then
        export PATH="$(dirname "${XTENSA_GCC}"):${PATH}"
    fi
fi

if ! command -v xtensa-esp32-elf-gcc >/dev/null 2>&1; then
    echo "xtensa-esp32-elf-gcc was not found on PATH after IDF export."
    exit 1
fi

export CC=xtensa-esp32-elf-gcc
export CXX=xtensa-esp32-elf-g++
export ASM=xtensa-esp32-elf-gcc

"${IDF_PYTHON_BIN}" "${IDF_PATH}/tools/idf.py" -C "${IDF_WRAPPER_DIR}" -B "${IDF_WRAPPER_BUILD_DIR}" set-target esp32
"${IDF_PYTHON_BIN}" "${IDF_PATH}/tools/idf.py" -C "${IDF_WRAPPER_DIR}" -B "${IDF_WRAPPER_BUILD_DIR}" reconfigure
"${IDF_PYTHON_BIN}" "${IDF_PATH}/tools/idf.py" -C "${IDF_WRAPPER_DIR}" -B "${IDF_WRAPPER_BUILD_DIR}" build

python3 - "${IDF_WRAPPER_BUILD_DIR}/compile_commands.json" "${IDF_INCLUDE_DIRS_FILE}" <<'PY'
import json
import shlex
import sys
from pathlib import Path

compile_commands = Path(sys.argv[1])
output = Path(sys.argv[2])

if not compile_commands.exists():
    raise SystemExit(f"Missing compile commands: {compile_commands}")

commands = json.loads(compile_commands.read_text(encoding="utf-8"))
include_dirs = []
seen = set()
for entry in commands:
    command = entry.get("command")
    if not command:
        continue
    tokens = shlex.split(command)
    index = 0
    while index < len(tokens):
        token = tokens[index]
        if token == "-I" and (index + 1) < len(tokens):
            value = tokens[index + 1]
            index += 1
        elif token.startswith("-I"):
            value = token[2:]
        else:
            index += 1
            continue
        if "/linux/" in value:
            index += 1
            continue
        if value and value not in seen:
            seen.add(value)
            include_dirs.append(value)
        index += 1

with output.open("w", encoding="utf-8") as stream:
    stream.write("set(ESP32_IDF_EXTRA_INCLUDE_DIRS\n")
    for include_dir in include_dirs:
        stream.write(f'    "{include_dir}"\n')
    stream.write(")\n")
PY

python3 - "${IDF_WRAPPER_BUILD_DIR}" "${IDF_LINK_INFO_FILE}" "${IDF_LINK_PROFILE}" <<'PY'
import json
import shlex
import sys
from pathlib import Path

build_dir = Path(sys.argv[1])
output = Path(sys.argv[2])
link_profile = sys.argv[3]

base_components = {
    "xtensa",
    "esp_driver_gpio",
    "esp_pm",
    "esp_app_format",
    "esp_bootloader_format",
    "app_update",
    "esp_partition",
    "efuse",
    "bootloader_support",
    "esp_mm",
    "spi_flash",
    "esp_system",
    "esp_common",
    "esp_rom",
    "hal",
    "log",
    "heap",
    "soc",
    "esp_security",
    "esp_hw_support",
    "freertos",
    "newlib",
    "pthread",
    "cxx",
    "esp_timer",
    "esp_driver_gptimer",
    "esp_ringbuf",
    "esp_driver_spi",
    "esp_psram",
    "esp_driver_uart",
    "app_trace",
    "esp_event",
    "esp_vfs_console",
    "vfs",
    "main",
}
wifi_components = {
    "lwip",
    "esp_netif",
    "wpa_supplicant",
    "esp_coex",
    "esp_wifi",
    "mbedtls",
    "esp-tls",
    "nvs_flash",
    "esp_phy",
}
allowed_components = set(base_components)
if link_profile == "wifi":
    allowed_components.update(wifi_components)

project_description = build_dir / "project_description.json"
if not project_description.exists():
    raise SystemExit(f"Missing project description: {project_description}")

app_elf = json.loads(project_description.read_text(encoding="utf-8")).get("app_elf")
if not app_elf:
    raise SystemExit("project_description.json does not define app_elf")

ninja_file = build_dir / "build.ninja"
if not ninja_file.exists():
    raise SystemExit(f"Missing build.ninja: {ninja_file}")

link_vars = {}
capture = False
for raw_line in ninja_file.read_text(encoding="utf-8").splitlines():
    if raw_line.startswith(f"build {app_elf}:"):
        capture = True
        continue
    if not capture:
        continue
    if not raw_line.startswith("  "):
        break
    line = raw_line.strip()
    if " = " not in line:
        continue
    key, value = line.split(" = ", 1)
    if key in {"LINK_FLAGS", "LINK_LIBRARIES", "LINK_PATH"}:
        link_vars[key] = value

if "LINK_LIBRARIES" not in link_vars:
    raise SystemExit(f"Unable to locate LINK_LIBRARIES for {app_elf} in {ninja_file}")

link_directories = []
seen_directories = set()
for token in shlex.split(link_vars.get("LINK_PATH", "")):
    if not token.startswith("-L"):
        continue
    directory = token[2:]
    if not directory:
        continue
    directory_path = Path(directory)
    if not directory_path.is_absolute():
        directory_path = (build_dir / directory_path).resolve()
    else:
        directory_path = directory_path.resolve()
    directory_string = str(directory_path)
    if directory_string in seen_directories:
        continue
    seen_directories.add(directory_string)
    link_directories.append(directory_string)

link_libraries = []
seen_libraries = set()
link_options = []
seen_options = set()

def component_from_token(token: str):
    token_path = Path(token)
    parts = token_path.parts
    if "components" in parts:
        index = parts.index("components")
        if (index + 1) < len(parts):
            return parts[index + 1]
    if "esp-idf" in parts:
        index = parts.index("esp-idf")
        if (index + 2) < len(parts) and parts[index + 1] == "components":
            return parts[index + 2]
        if (index + 1) < len(parts):
            return parts[index + 1]
    return None

tokens = shlex.split(link_vars["LINK_LIBRARIES"])
index = 0
while index < len(tokens):
    token = tokens[index]
    if token == "-u" and (index + 1) < len(tokens):
        option = f"-u{tokens[index + 1]}"
        index += 2
        if option not in seen_options:
            seen_options.add(option)
            link_options.append(option)
        continue

    if token.endswith(".a"):
        library_path = Path(token)
        if not library_path.is_absolute():
            library_path = (build_dir / library_path).resolve()
        else:
            library_path = library_path.resolve()
        token_out = str(library_path)
        component = component_from_token(token_out)
        if component and component not in allowed_components:
            index += 1
            continue
        if token_out not in seen_libraries:
            seen_libraries.add(token_out)
            link_libraries.append(token_out)
    elif token.startswith("-l"):
        if token in {"-lphy", "-lrtc"} and "esp_phy" not in allowed_components:
            index += 1
            continue
        if token not in seen_libraries:
            seen_libraries.add(token)
            link_libraries.append(token)
    elif token.startswith("-Wl,--undefined=") or token.startswith("-Wl,--wrap=") or token.startswith("-u"):
        if token not in seen_options:
            seen_options.add(token)
            link_options.append(token)

    index += 1

search_paths = [Path(path) for path in link_directories]
search_paths.append(build_dir)

def resolve_linker_script(path_text: str) -> str:
    script_path = Path(path_text)
    if script_path.is_absolute():
        return str(script_path.resolve())
    for base in search_paths:
        candidate = (base / script_path).resolve()
        if candidate.exists():
            return str(candidate)
    return path_text

link_flag_tokens = shlex.split(link_vars.get("LINK_FLAGS", ""))
index = 0
while index < len(link_flag_tokens):
    token = link_flag_tokens[index]
    if token == "-T" and (index + 1) < len(link_flag_tokens):
        token = f"-T{resolve_linker_script(link_flag_tokens[index + 1])}"
        index += 2
    else:
        if token.startswith("-T") and len(token) > 2:
            token = f"-T{resolve_linker_script(token[2:])}"
        index += 1
    if token not in seen_options:
        seen_options.add(token)
        link_options.append(token)

with output.open("w", encoding="utf-8") as stream:
    stream.write("set(ESP32_IDF_EXTRA_LINK_DIRECTORIES\n")
    for directory in link_directories:
        stream.write(f'    "{directory}"\n')
    stream.write(")\n\n")

    stream.write("set(ESP32_IDF_EXTRA_LINK_OPTIONS\n")
    for option in link_options:
        stream.write(f'    "{option}"\n')
    stream.write(")\n\n")

    stream.write("set(ESP32_IDF_EXTRA_LINK_LIBRARIES\n")
    for library in link_libraries:
        stream.write(f'    "{library}"\n')
    stream.write(")\n")
PY

echo "Prepared ESP-IDF wrapper metadata for ${DEPLOY_DIR}"

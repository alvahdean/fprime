#!/usr/bin/bash 

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ -z "$FPRIME_REPO_ROOT" ]; then
    if ! FPRIME_REPO_ROOT="$(git -C "${SCRIPT_DIR}" rev-parse --show-toplevel 2>/dev/null)"; then
        echo "No git repository found, please set FPRIME_REPO_ROOT manually" >&2
        exit 1
    fi
fi

echo "FPRIME_REPO_ROOT=${FPRIME_REPO_ROOT}"

source ${FPRIME_REPO_ROOT}/venv/bin/activate
source ${HOME}/.espressif/tools/activate_idf_v5.5.3.sh
eval "$(python "$IDF_PATH/tools/idf_tools.py" export 2>/dev/null)"; 

# This reasserts the F' python env as primary because the ESP-IDF overwrites it.
export PATH="${FPRIME_REPO_ROOT}/venv/bin:$PATH" 

# This adds the helper script directory to the PATH for convenience 
export PATH="$PATH:${FPRIME_REPO_ROOT}/ESP32/scripts"; 

#!/usr/bin/bash
set -euo pipefail

fprime-util generate -f
fprime-util build
flash.sh . --port /dev/ttyUSB0
clear
monitor.sh . --port /dev/ttyUSB0

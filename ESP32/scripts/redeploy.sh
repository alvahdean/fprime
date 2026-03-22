#!/usr/bin/bash
set -euo pipefail

fprime-util generate -f  
fprime-util build
build_flash_image.sh .
flash.sh . --port /dev/ttyUSB0
clear
monitor.sh . --port /dev/ttyUSB0
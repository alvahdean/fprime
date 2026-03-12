# FreeRTOS OSAL Getting Started

## 1) Scope
This guide covers:
- Building the baseline, platform-agnostic FreeRTOS OSAL under `FreeRTOS/Os`.
- Running the FreeRTOS OSAL unit tests in a generic host setup.
- The required steps to move from generic support to a specific hardware platform (ESP32, STM32, and similar targets).

## 2) Prerequisites
- A working F´ developer environment (Python + `fprime-util`, CMake, C/C++ compiler).
- FreeRTOS headers for target builds (`FreeRTOS.h`, `task.h`, `semphr.h`).
- A toolchain file that sets `CMAKE_SYSTEM_NAME` to `Generic` and selects `FPRIME_PLATFORM=FreeRTOS`.

Example minimal toolchain file (`cmake/toolchain/freertos-generic.cmake`):

```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(FPRIME_PLATFORM FreeRTOS)

find_program(CMAKE_C_COMPILER NAMES cc gcc clang REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES c++ g++ clang++ REQUIRED)
```

## 3) Build (Generic FreeRTOS)
From the repository root:

```bash
fprime-util generate freertos-generic -f --ut -DOS_FREERTOS_ENABLE_COMMON_UTS=ON
```

Notes:
- `freertos-generic` is the toolchain filename without `.cmake`.
- `OS_FREERTOS_ENABLE_COMMON_UTS=ON` enables Task/Mutex/Condition/RawTime/Queue common-rule suites in addition to smoke tests.

## 4) Run Tests (Generic FreeRTOS)
Run all unit tests for the generated unit-test cache:

```bash
fprime-util check freertos-generic --all
```

Run only FreeRTOS-focused tests (from the generated build cache):

```bash
ctest -R "^FreeRTOS" --output-on-failure
```

Expected FreeRTOS test executables:
- `FreeRTOSSupportTest` (always)
- `FreeRTOSTaskTest` (when `OS_FREERTOS_ENABLE_COMMON_UTS=ON`)
- `FreeRTOSMutexTest` (when `OS_FREERTOS_ENABLE_COMMON_UTS=ON`)
- `FreeRTOSConditionTest` (when `OS_FREERTOS_ENABLE_COMMON_UTS=ON`)
- `FreeRTOSRawTimeTest` (when `OS_FREERTOS_ENABLE_COMMON_UTS=ON`)
- `FreeRTOSQueueTest` (when `OS_FREERTOS_ENABLE_COMMON_UTS=ON`)

## 5) Enabling a Specific Platform (ESP32, STM32, etc.)
Use this checklist to move from the generic baseline to a production board port.

### A) Create a platform toolchain
- Add `cmake/toolchain/<platform>.cmake` for the board/compiler.
- Set `CMAKE_SYSTEM_NAME` to `Generic`.
- Set `FPRIME_PLATFORM` to a board platform name (for example, `ESP32`, `STM32H7`).
- Configure cross-compilers, sysroot, linker options, and any SDK environment setup.

### B) Add a board platform file
- Add `cmake/platform/<platform>.cmake`.
- Register a config module with `register_fprime_config(...)`.
- Choose FreeRTOS OSAL implementations:
  - `Os_File_FreeRTOS`
  - `Os_Console_FreeRTOS`
  - `Os_Task_FreeRTOS`
  - `Os_Mutex_FreeRTOS`
  - `Os_Queue_FreeRTOS`
  - `Os_RawTime_FreeRTOS`
  - `Os_Cpu_FreeRTOS`
  - `Os_Memory_FreeRTOS`
  - `Fw_StringFormat_snprintf`
- Set compile definitions (for example `-DTGT_OS_TYPE_FREERTOS`) and board-specific flags.
- Keep `FPRIME_HAS_SOCKETS` aligned with board networking support.

### C) Provide platform adapters/backends
- Implement and register a `VfsAdapter` for your filesystem stack (FatFs/littlefs/vendor FS).
- Register a console writer bound to board UART/RTT/ITM logging.
- Optionally register `registerRawTimeEpochHook(...)` if wall-clock time is available (RTC, GNSS, SNTP).
- Optionally register `registerStaticTaskAllocator(...)` when using static FreeRTOS task allocation.

### D) Register adapters during startup
Register adapters before any components use OSAL services:

```cpp
#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

void platformOsalInit() {
    static MyVfsAdapter vfs;
    Os::FreeRTOSSupport::registerVfsAdapter(&vfs);
    Os::FreeRTOSSupport::registerConsoleWriter(&boardConsoleWrite);
    Os::FreeRTOSSupport::registerRawTimeEpochHook(&boardEpochHook);         // optional
    Os::FreeRTOSSupport::registerStaticTaskAllocator(&boardTaskAllocator);  // optional
}
```

### E) Validate in stages
- Stage 1: run host/generic FreeRTOS unit tests in CI.
- Stage 2: boot on hardware with minimal topology and verify tasking, console, and timing behavior.
- Stage 3: enable filesystem and full mission services, then run mission-level integration tests.

## 6) Common Bring-Up Issues
- `NOT_SUPPORTED` from file/directory/filesystem APIs: `VfsAdapter` not registered (or incomplete).
- Missing console output: no console writer registered.
- `Task::start` failures with static allocation enabled: static allocator hook not registered.
- `RawTime` shows uptime only: no epoch hook registered (expected behavior).

# FreeRTOS OSAL Integration Guide

## 1) Select the platform
Use `FPRIME_PLATFORM=FreeRTOS` in your build configuration so `cmake/platform/FreeRTOS.cmake` is selected.

## 2) Supply FreeRTOS kernel headers
The toolchain/platform project is expected to provide:
- `FreeRTOS.h`
- `task.h`
- `semphr.h`

## 3) Register platform adapters during bring-up
Register adapters before OSAL-dependent components run.

```cpp
#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

void setupOsAdapters() {
    static MyVfsAdapter vfs;
    Os::FreeRTOSSupport::registerVfsAdapter(&vfs);

    Os::FreeRTOSSupport::registerConsoleWriter(&myConsoleWrite);
    Os::FreeRTOSSupport::registerRawTimeEpochHook(&myEpochHook);          // optional
    Os::FreeRTOSSupport::registerStaticTaskAllocator(&myTaskAllocator);   // optional
}
```

## 4) Adapter expectations
`VfsAdapter` must implement file, directory, and filesystem operations used by:
- `Os::File`
- `Os::Directory`
- `Os::FileSystem`

## 5) Default behavior without adapters
- `Os::File`, `Os::Directory`, and `Os::FileSystem` return `NOT_SUPPORTED` for backend operations.
- `Os::Console` drops writes.
- `Os::RawTime` remains monotonic uptime (no wall-clock translation).
- Static task fallback path is disabled unless a static allocator is registered.

## 6) Notes on task stack units
`Os::Task` passes stack size in bytes. The FreeRTOS implementation converts bytes to stack words (`StackType_t` units) with rounding-up.

## 7) Unit test scope on FreeRTOS
Use FreeRTOS-focused tests by default:

- Generate a UT cache with common shared OS rule suites disabled:
  - `fprime-util generate freertos-generic --ut -DOS_FREERTOS_ENABLE_COMMON_UTS=OFF -f`
- Run FreeRTOS module tests:
  - `fprime-util check --build-cache /home/dfuqua/src/fprime/build-fprime-automatic-freertos-generic-ut --path /home/dfuqua/src/fprime/FreeRTOS/FreeRTOS/Os --recursive`

This baseline intentionally does not require all host-oriented or Posix-coupled unit tests to pass under FreeRTOS.

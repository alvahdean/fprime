# FreeRTOS Baseline OSAL Implementation Plan

## Summary
- Create a full, platform-agnostic FreeRTOS OSAL baseline under `FreeRTOS/Os/` covering all Os interfaces: File, FileSystem, Directory, Console, Task, Mutex, ConditionVariable, Queue, RawTime, Cpu, and Memory.
- Add core build integration so this implementation is selectable via a new `cmake/platform/FreeRTOS.cmake`.
- Keep platform-specific dependencies out of core by introducing explicit registration-based adapter contracts for filesystem, console output, optional wall-clock epoch conversion, and static-task memory provisioning.
- Deliver all FreeRTOS docs under `FreeRTOS/Os/docs/`, including this plan as `FreeRTOS-Implementation-Plan.md`.

## Implementation Changes

### 1) Build and packaging integration
- Add `FreeRTOS/Os/CMakeLists.txt` with `restrict_platforms(FreeRTOS)` and `register_os_implementation(...)` for:
  - `"File;FileSystem;Directory"` as `FreeRTOS`
  - `Console`, `Task`, `"Mutex;ConditionVariable"`, `Queue`, `RawTime`, `Cpu`, and `Memory` as `FreeRTOS`
- Add `add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/FreeRTOS")` to `Os/CMakeLists.txt`.
- Add `cmake/platform/FreeRTOS.cmake` with a `register_fprime_config` that selects:
  - `Os_File_FreeRTOS`, `Os_Console_FreeRTOS`, `Os_Task_FreeRTOS`, `Os_Mutex_FreeRTOS`, `Os_Queue_FreeRTOS`, `Os_RawTime_FreeRTOS`, `Os_Cpu_FreeRTOS`, `Os_Memory_FreeRTOS`, `Fw_StringFormat_snprintf`
- In `cmake/platform/FreeRTOS.cmake`, set:
  - `FPRIME_HAS_SOCKETS OFF`
  - `-DTGT_OS_TYPE_FREERTOS`
- Reuse existing platform type aliases (same type model as current unix platform types) unless overridden by project config.

### 2) New FreeRTOS adapter contracts (public additions)
- Add a FreeRTOS support header in `FreeRTOS/Os/` defining registration APIs:
- `void registerVfsAdapter(VfsAdapter* adapter);`
- `void registerConsoleWriter(ConsoleWriteFn writer);`
- `void registerRawTimeEpochHook(RawTimeEpochHook hook);`
- `void registerStaticTaskAllocator(StaticTaskAllocator allocator);`
- `VfsAdapter` is a pure virtual interface that covers file, directory, and filesystem operations used by `Os::File`, `Os::Directory`, and `Os::FileSystem`.
- Default behavior when adapters/hooks are not registered:
  - VFS-backed calls return `NOT_SUPPORTED`
  - Console write is no-op
  - RawTime uses monotonic uptime only
  - Static task allocation path is unavailable

### 3) Os module behavior in `FreeRTOS/Os/`
- `Task`:
  - `start()` tries dynamic task creation first when supported; falls back to static creation via registered `StaticTaskAllocator` when available.
  - Map default priority/stack sentinels to FreeRTOS defaults.
  - `join()` implemented via completion semaphore/event signaled on task exit.
  - `suspend()/resume()/delay()` map to FreeRTOS APIs.
- `Mutex`:
  - Implement with FreeRTOS mutex semaphore.
  - Track owner task to return `ERROR_DEADLOCK` on self-lock and guard invalid release.
- `ConditionVariable`:
  - Implement using waiter counting + semaphore signaling + FreeRTOS critical sections.
  - Preserve required wait semantics with mutex release/block/reacquire flow.
- `Queue` (hybrid, priority-preserving):
  - Implement FreeRTOS-backed blocking with counting semaphores for slots/items.
  - Preserve F´ queue priority semantics via software max-heap ordering over stored messages.
  - Use memory allocator registry for queue storage buffers.
- `RawTime`:
  - Base time source on monotonic FreeRTOS ticks.
  - Convert to sec/usec with overflow-safe tick extension.
  - Apply optional epoch hook for wall-clock translation when registered.
  - Keep serialization format compatible with existing RawTime convention.
- `Console`:
  - Forward writes to registered console writer callback.
- `File`, `FileSystem`, `Directory`:
  - Fully implement interfaces by forwarding to registered `VfsAdapter`.
  - Include deterministic status mapping from adapter results to Os enums.
  - No direct dependency on FatFs/littlefs in core.
- `Cpu`:
  - `getCount`: return `configNUM_CORES` when available, else `1`.
  - `getTicks`: use FreeRTOS runtime stats APIs when available; otherwise return `ERROR`.
- `Memory`:
  - Use FreeRTOS heap stats APIs when present (`xPortGetFreeHeapSize`/related).
  - Return `ERROR` if required metrics are unavailable at compile time.

### 4) Defaults and unsupported behavior
- APIs are task-context only unless explicitly documented otherwise.
- ISR-safe variants are out of scope for baseline.
- Timeout-capable extensions are out of scope; blocking follows existing Os API semantics.
- If a capability is unavailable by configuration, return existing Os error statuses instead of asserting (except hard contract violations already asserted in core patterns).

## Public API / Interface Additions
- New registration APIs in `FreeRTOS/Os`:
  - `registerVfsAdapter`
  - `registerConsoleWriter`
  - `registerRawTimeEpochHook`
  - `registerStaticTaskAllocator`
- New adapter type(s):
  - `VfsAdapter` for file/directory/filesystem backend binding.
- No changes to existing public Os interface signatures in `Os/*.hpp`.

## Test Plan
- Add FreeRTOS implementation tests under `FreeRTOS/Os/test` using a host FreeRTOS shim layer for CI:
  - Run shared common rule suites for Task, Mutex, Condition, RawTime, and Queue.
  - Add smoke/status tests for File/FileSystem/Directory via mock `VfsAdapter`.
  - Add Cpu/Memory behavior tests for “stats available” and “stats unavailable” compile paths.
- Add CMake selection test to verify `FPRIME_PLATFORM=FreeRTOS` chooses `Os_*_FreeRTOS` implementations.
- Run regression build/tests for existing Linux/Darwin/Stub paths to confirm no selection regressions.
- Acceptance criteria:
  - FreeRTOS platform config links without unresolved `Os_*` implementations.
  - Shared OSAL common tests pass for implemented semantics.
  - Missing adapters/hooks fail gracefully via defined status codes, not undefined behavior.

## Documentation Deliverables
- `FreeRTOS/Os/docs/FreeRTOS-Implementation-Plan.md` (this plan).
- `FreeRTOS/Os/docs/Integration-Guide.md`:
  - How to enable `FPRIME_PLATFORM=FreeRTOS`
  - How to register VFS/console/time/task alloc adapters
  - Which APIs return `NOT_SUPPORTED` without backend registration
- `FreeRTOS/Os/docs/Behavior-Notes.md`:
  - Blocking model, priority model, join model, non-ISR-safe scope, and config dependencies.

## Assumptions
- FreeRTOS headers and kernel configuration are provided by the toolchain/platform project.
- Platform-agnostic means no direct FatFs/littlefs dependency in fprime core.
- Queue priority behavior must be preserved, hence the hybrid design.
- Full OSAL coverage is required in this baseline, with backend registration where platform dependencies are unavoidable.

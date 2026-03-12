# FreeRTOS OSAL Behavior Notes

## Blocking model
- `Task::join()` blocks on a completion semaphore signaled by the task wrapper.
- `Queue` uses counting semaphores for producer/consumer blocking (`slots` and `items`).
- `ConditionVariable` uses waiter accounting + pending signal accounting to preserve wait semantics.

## Priority model
- Queue priority is preserved in software via `Os::Generic::Types::MaxHeap`.
- Blocking is handled by FreeRTOS semaphores; dequeue order is priority-driven by the software heap.

## RawTime model
- `RawTime::now()` is monotonic and derived from FreeRTOS ticks.
- Tick wrap is extended using a rollover accumulator.
- An optional epoch hook can translate monotonic time to wall-clock time.

## Scope and constraints
- Implementation is task-context oriented; ISR-safe variants are not provided in this baseline.
- Timeout-capable API extensions are out of scope for this baseline.
- If backend/platform capability is unavailable, methods return existing Os status errors (e.g. `NOT_SUPPORTED`, `ERROR`, etc.) rather than asserting.

## Platform dependencies
- FreeRTOS kernel headers and configuration macros are expected from the platform/toolchain.
- Filesystem backends are supplied externally via `VfsAdapter` registration.

// ======================================================================
// \title  FreeRTOS/Os/FreeRTOSSupport.cpp
// \brief  Adapter registration API implementation
// ======================================================================

#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

#include <atomic>

namespace Os {
namespace FreeRTOSSupport {

namespace {
std::atomic<VfsAdapter*> s_vfs_adapter{nullptr};
std::atomic<ConsoleWriteFn> s_console_writer{nullptr};
std::atomic<RawTimeEpochHook> s_rawtime_epoch_hook{nullptr};
std::atomic<StaticTaskAllocator> s_static_task_allocator{nullptr};
}  // namespace

void registerVfsAdapter(VfsAdapter* adapter) {
    s_vfs_adapter.store(adapter, std::memory_order_release);
}

void registerConsoleWriter(ConsoleWriteFn writer) {
    s_console_writer.store(writer, std::memory_order_release);
}

void registerRawTimeEpochHook(RawTimeEpochHook hook) {
    s_rawtime_epoch_hook.store(hook, std::memory_order_release);
}

void registerStaticTaskAllocator(StaticTaskAllocator allocator) {
    s_static_task_allocator.store(allocator, std::memory_order_release);
}

VfsAdapter* getVfsAdapter() {
    return s_vfs_adapter.load(std::memory_order_acquire);
}

ConsoleWriteFn getConsoleWriter() {
    return s_console_writer.load(std::memory_order_acquire);
}

RawTimeEpochHook getRawTimeEpochHook() {
    return s_rawtime_epoch_hook.load(std::memory_order_acquire);
}

StaticTaskAllocator getStaticTaskAllocator() {
    return s_static_task_allocator.load(std::memory_order_acquire);
}

Os::File::Status toFileStatus(const VfsStatus status) {
    switch (status) {
        case VfsStatus::OP_OK:
            return Os::File::Status::OP_OK;
        case VfsStatus::DOESNT_EXIST:
            return Os::File::Status::DOESNT_EXIST;
        case VfsStatus::NO_SPACE:
            return Os::File::Status::NO_SPACE;
        case VfsStatus::NO_PERMISSION:
            return Os::File::Status::NO_PERMISSION;
        case VfsStatus::BAD_SIZE:
            return Os::File::Status::BAD_SIZE;
        case VfsStatus::NOT_OPENED:
            return Os::File::Status::NOT_OPENED;
        case VfsStatus::FILE_EXISTS:
            return Os::File::Status::FILE_EXISTS;
        case VfsStatus::NOT_SUPPORTED:
            return Os::File::Status::NOT_SUPPORTED;
        case VfsStatus::INVALID_MODE:
            return Os::File::Status::INVALID_MODE;
        case VfsStatus::INVALID_ARGUMENT:
            return Os::File::Status::INVALID_ARGUMENT;
        case VfsStatus::NO_MORE_RESOURCES:
            return Os::File::Status::NO_MORE_RESOURCES;
        case VfsStatus::OTHER_ERROR:
        default:
            return Os::File::Status::OTHER_ERROR;
    }
}

Os::Directory::Status toDirectoryStatus(const VfsStatus status) {
    switch (status) {
        case VfsStatus::OP_OK:
            return Os::Directory::Status::OP_OK;
        case VfsStatus::DOESNT_EXIST:
            return Os::Directory::Status::DOESNT_EXIST;
        case VfsStatus::NO_PERMISSION:
            return Os::Directory::Status::NO_PERMISSION;
        case VfsStatus::NOT_OPENED:
            return Os::Directory::Status::NOT_OPENED;
        case VfsStatus::NOT_DIR:
            return Os::Directory::Status::NOT_DIR;
        case VfsStatus::NO_MORE_FILES:
            return Os::Directory::Status::NO_MORE_FILES;
        case VfsStatus::FILE_LIMIT:
            return Os::Directory::Status::FILE_LIMIT;
        case VfsStatus::ALREADY_EXISTS:
            return Os::Directory::Status::ALREADY_EXISTS;
        case VfsStatus::NOT_SUPPORTED:
            return Os::Directory::Status::NOT_SUPPORTED;
        case VfsStatus::OTHER_ERROR:
        case VfsStatus::BAD_SIZE:
        case VfsStatus::INVALID_ARGUMENT:
        default:
            return Os::Directory::Status::OTHER_ERROR;
    }
}

Os::FileSystem::Status toFileSystemStatus(const VfsStatus status) {
    switch (status) {
        case VfsStatus::OP_OK:
            return Os::FileSystem::Status::OP_OK;
        case VfsStatus::ALREADY_EXISTS:
            return Os::FileSystem::Status::ALREADY_EXISTS;
        case VfsStatus::NO_SPACE:
            return Os::FileSystem::Status::NO_SPACE;
        case VfsStatus::NO_PERMISSION:
            return Os::FileSystem::Status::NO_PERMISSION;
        case VfsStatus::NOT_DIR:
            return Os::FileSystem::Status::NOT_DIR;
        case VfsStatus::IS_DIR:
            return Os::FileSystem::Status::IS_DIR;
        case VfsStatus::NOT_EMPTY:
            return Os::FileSystem::Status::NOT_EMPTY;
        case VfsStatus::INVALID_PATH:
            return Os::FileSystem::Status::INVALID_PATH;
        case VfsStatus::DOESNT_EXIST:
            return Os::FileSystem::Status::DOESNT_EXIST;
        case VfsStatus::FILE_LIMIT:
            return Os::FileSystem::Status::FILE_LIMIT;
        case VfsStatus::BUSY:
            return Os::FileSystem::Status::BUSY;
        case VfsStatus::NO_MORE_FILES:
            return Os::FileSystem::Status::NO_MORE_FILES;
        case VfsStatus::BUFFER_TOO_SMALL:
            return Os::FileSystem::Status::BUFFER_TOO_SMALL;
        case VfsStatus::EXDEV_ERROR:
            return Os::FileSystem::Status::EXDEV_ERROR;
        case VfsStatus::OVERFLOW_ERROR:
            return Os::FileSystem::Status::OVERFLOW_ERROR;
        case VfsStatus::NOT_SUPPORTED:
            return Os::FileSystem::Status::NOT_SUPPORTED;
        case VfsStatus::OTHER_ERROR:
        case VfsStatus::INVALID_ARGUMENT:
        case VfsStatus::BAD_SIZE:
        case VfsStatus::NOT_OPENED:
        case VfsStatus::FILE_EXISTS:
        case VfsStatus::INVALID_MODE:
        case VfsStatus::NO_MORE_RESOURCES:
        case VfsStatus::NOT_EXIST:
        case VfsStatus::OTHER_PATH_TYPE:
        default:
            return Os::FileSystem::Status::OTHER_ERROR;
    }
}

}  // namespace FreeRTOSSupport
}  // namespace Os

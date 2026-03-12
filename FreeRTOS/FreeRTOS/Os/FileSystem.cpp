// ======================================================================
// \title  FreeRTOS/Os/FileSystem.cpp
// \brief  FreeRTOS implementation for Os::FileSystem
// ======================================================================

#include "FreeRTOS/Os/FileSystem.hpp"

#include "Fw/Types/Assert.hpp"
#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

namespace Os {
namespace FreeRTOS {
namespace FileSystem {

FreeRtosFileSystem::Status FreeRtosFileSystem::_removeDirectory(const char* path) {
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileSystemStatus(adapter->filesystemRemoveDirectory(path));
}

FreeRtosFileSystem::Status FreeRtosFileSystem::_removeFile(const char* path) {
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileSystemStatus(adapter->filesystemRemoveFile(path));
}

FreeRtosFileSystem::Status FreeRtosFileSystem::_rename(const char* source_path, const char* destination_path) {
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileSystemStatus(adapter->filesystemRename(source_path, destination_path));
}

FreeRtosFileSystem::Status FreeRtosFileSystem::_getFreeSpace(const char* path,
                                                             FwSizeType& total_bytes,
                                                             FwSizeType& free_bytes) {
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileSystemStatus(adapter->filesystemGetFreeSpace(path, total_bytes, free_bytes));
}

FreeRtosFileSystem::Status FreeRtosFileSystem::_getPathType(const char* path, PathType& path_type) {
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileSystemStatus(adapter->filesystemGetPathType(path, path_type));
}

FreeRtosFileSystem::Status FreeRtosFileSystem::_getWorkingDirectory(char* path, FwSizeType buffer_size) {
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileSystemStatus(adapter->filesystemGetWorkingDirectory(path, buffer_size));
}

FreeRtosFileSystem::Status FreeRtosFileSystem::_changeWorkingDirectory(const char* path) {
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileSystemStatus(adapter->filesystemChangeWorkingDirectory(path));
}

FileSystemHandle* FreeRtosFileSystem::getHandle() {
    return &this->m_handle;
}

}  // namespace FileSystem
}  // namespace FreeRTOS
}  // namespace Os

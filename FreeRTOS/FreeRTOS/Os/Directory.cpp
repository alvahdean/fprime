// ======================================================================
// \title  FreeRTOS/Os/Directory.cpp
// \brief  FreeRTOS implementation for Os::Directory
// ======================================================================

#include "FreeRTOS/Os/Directory.hpp"

#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

namespace Os {
namespace FreeRTOS {
namespace Directory {

FreeRtosDirectory::FreeRtosDirectory() : m_backend_handle(nullptr) {}

FreeRtosDirectory::~FreeRtosDirectory() {
    this->close();
}

DirectoryHandle* FreeRtosDirectory::getHandle() {
    return reinterpret_cast<DirectoryHandle*>(this->m_backend_handle);
}

FreeRtosDirectory::Status FreeRtosDirectory::open(const char* path, OpenMode mode) {
    if (this->m_backend_handle != nullptr) {
        this->close();
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    void* handle = nullptr;
    const Os::FreeRTOSSupport::VfsStatus status = adapter->directoryOpen(path, mode, handle);
    if (status == Os::FreeRTOSSupport::VfsStatus::OP_OK) {
        this->m_backend_handle = handle;
    }
    return Os::FreeRTOSSupport::toDirectoryStatus(status);
}

FreeRtosDirectory::Status FreeRtosDirectory::rewind() {
    if (this->m_backend_handle == nullptr) {
        return Status::NOT_OPENED;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toDirectoryStatus(adapter->directoryRewind(this->m_backend_handle));
}

FreeRtosDirectory::Status FreeRtosDirectory::read(char* file_name_buffer, FwSizeType buffer_size) {
    if (this->m_backend_handle == nullptr) {
        return Status::NOT_OPENED;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toDirectoryStatus(
        adapter->directoryRead(this->m_backend_handle, file_name_buffer, buffer_size));
}

void FreeRtosDirectory::close() {
    if (this->m_backend_handle == nullptr) {
        return;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter != nullptr) {
        adapter->directoryClose(this->m_backend_handle);
    }
    this->m_backend_handle = nullptr;
}

}  // namespace Directory
}  // namespace FreeRTOS
}  // namespace Os

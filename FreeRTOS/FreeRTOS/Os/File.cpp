// ======================================================================
// \title  FreeRTOS/Os/File.cpp
// \brief  FreeRTOS implementation for Os::File
// ======================================================================

#include "FreeRTOS/Os/File.hpp"

#include <Fw/Types/Assert.hpp>
#include <new>

#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

namespace Os {
namespace FreeRTOS {
namespace File {

FreeRtosFile::FreeRtosFile() : m_state(nullptr) {}

FreeRtosFile::FreeRtosFile(const FreeRtosFile& other) : m_state(other.m_state) {
    if (this->m_state != nullptr) {
        this->m_state->m_ref_count += 1;
    }
}

FreeRtosFile& FreeRtosFile::operator=(const FreeRtosFile& other) {
    if (this != &other) {
        this->release();
        this->m_state = other.m_state;
        if (this->m_state != nullptr) {
            this->m_state->m_ref_count += 1;
        }
    }
    return *this;
}

FreeRtosFile::~FreeRtosFile() {
    this->release();
}

void FreeRtosFile::release() {
    if (this->m_state == nullptr) {
        return;
    }
    FW_ASSERT(this->m_state->m_ref_count > 0);
    this->m_state->m_ref_count -= 1;
    if (this->m_state->m_ref_count == 0) {
        Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
        if (adapter != nullptr) {
            adapter->fileClose(this->m_state->m_backend_handle);
        }
        delete this->m_state;
    }
    this->m_state = nullptr;
}

FreeRtosFile::Status FreeRtosFile::open(const char* path, Mode mode, OverwriteType overwrite) {
    if (this->m_state != nullptr) {
        return Status::INVALID_MODE;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }

    void* backend_handle = nullptr;
    const Os::FreeRTOSSupport::VfsStatus status = adapter->fileOpen(path, mode, overwrite, backend_handle);
    if (status != Os::FreeRTOSSupport::VfsStatus::OP_OK) {
        return Os::FreeRTOSSupport::toFileStatus(status);
    }

    SharedState* state = new (std::nothrow) SharedState{backend_handle, 1};
    if (state == nullptr) {
        adapter->fileClose(backend_handle);
        return Status::NO_MORE_RESOURCES;
    }
    this->m_state = state;
    return Status::OP_OK;
}

void FreeRtosFile::close() {
    this->release();
}

FreeRtosFile::Status FreeRtosFile::size(FwSizeType& size_result) {
    if (this->m_state == nullptr) {
        return Status::NOT_OPENED;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileStatus(adapter->fileSize(this->m_state->m_backend_handle, size_result));
}

FreeRtosFile::Status FreeRtosFile::position(FwSizeType& position_result) {
    if (this->m_state == nullptr) {
        return Status::NOT_OPENED;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileStatus(adapter->filePosition(this->m_state->m_backend_handle, position_result));
}

FreeRtosFile::Status FreeRtosFile::preallocate(FwSizeType offset, FwSizeType length) {
    if (this->m_state == nullptr) {
        return Status::NOT_OPENED;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileStatus(
        adapter->filePreallocate(this->m_state->m_backend_handle, offset, length));
}

FreeRtosFile::Status FreeRtosFile::seek(FwSignedSizeType offset, SeekType seek_type) {
    if (this->m_state == nullptr) {
        return Status::NOT_OPENED;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileStatus(adapter->fileSeek(this->m_state->m_backend_handle, offset, seek_type));
}

FreeRtosFile::Status FreeRtosFile::flush() {
    if (this->m_state == nullptr) {
        return Status::NOT_OPENED;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileStatus(adapter->fileFlush(this->m_state->m_backend_handle));
}

FreeRtosFile::Status FreeRtosFile::read(U8* buffer, FwSizeType& size, WaitType wait) {
    if (this->m_state == nullptr) {
        size = 0;
        return Status::NOT_OPENED;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        size = 0;
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileStatus(adapter->fileRead(this->m_state->m_backend_handle, buffer, size, wait));
}

FreeRtosFile::Status FreeRtosFile::write(const U8* buffer, FwSizeType& size, WaitType wait) {
    if (this->m_state == nullptr) {
        size = 0;
        return Status::NOT_OPENED;
    }
    Os::FreeRTOSSupport::VfsAdapter* adapter = Os::FreeRTOSSupport::getVfsAdapter();
    if (adapter == nullptr) {
        size = 0;
        return Status::NOT_SUPPORTED;
    }
    return Os::FreeRTOSSupport::toFileStatus(
        adapter->fileWrite(this->m_state->m_backend_handle, buffer, size, wait));
}

FileHandle* FreeRtosFile::getHandle() {
    return reinterpret_cast<FileHandle*>(this->m_state == nullptr ? nullptr : this->m_state->m_backend_handle);
}

}  // namespace File
}  // namespace FreeRTOS
}  // namespace Os

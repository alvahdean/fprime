// ======================================================================
// \title  FreeRTOS/Os/FreeRTOSSupport.hpp
// \brief  Adapter registration API for platform bindings
// ======================================================================
#ifndef OS_FREERTOS_SUPPORT_HPP
#define OS_FREERTOS_SUPPORT_HPP

#include <Fw/FPrimeBasicTypes.hpp>
#include <Os/Directory.hpp>
#include <Os/File.hpp>
#include <Os/FileSystem.hpp>
#include <Os/Task.hpp>

#include "FreeRTOS/Os/FreeRTOSApi.hpp"

namespace Os {
namespace FreeRTOSSupport {

enum class VfsStatus {
    OP_OK,
    DOESNT_EXIST,
    NO_SPACE,
    NO_PERMISSION,
    BAD_SIZE,
    NOT_OPENED,
    FILE_EXISTS,
    NOT_SUPPORTED,
    INVALID_MODE,
    INVALID_ARGUMENT,
    NO_MORE_RESOURCES,
    OTHER_ERROR,
    ALREADY_EXISTS,
    NOT_DIR,
    IS_DIR,
    NOT_EMPTY,
    INVALID_PATH,
    FILE_LIMIT,
    BUSY,
    NO_MORE_FILES,
    BUFFER_TOO_SMALL,
    EXDEV_ERROR,
    OVERFLOW_ERROR,
    NOT_EXIST,
    OTHER_PATH_TYPE,
};

class VfsAdapter {
  public:
    virtual ~VfsAdapter() = default;

    virtual VfsStatus fileOpen(const char* path,
                               Os::File::Mode mode,
                               Os::File::OverwriteType overwrite,
                               void*& out_handle) = 0;
    virtual void fileClose(void* file_handle) = 0;
    virtual VfsStatus fileDuplicate(void* source_file_handle, void*& out_duplicate_handle) = 0;
    virtual VfsStatus fileSize(void* file_handle, FwSizeType& out_size) = 0;
    virtual VfsStatus filePosition(void* file_handle, FwSizeType& out_position) = 0;
    virtual VfsStatus filePreallocate(void* file_handle, FwSizeType offset, FwSizeType length) = 0;
    virtual VfsStatus fileSeek(void* file_handle, FwSignedSizeType offset, Os::File::SeekType seek_type) = 0;
    virtual VfsStatus fileFlush(void* file_handle) = 0;
    virtual VfsStatus fileRead(void* file_handle,
                               U8* buffer,
                               FwSizeType& in_out_size,
                               Os::File::WaitType wait_type) = 0;
    virtual VfsStatus fileWrite(void* file_handle,
                                const U8* buffer,
                                FwSizeType& in_out_size,
                                Os::File::WaitType wait_type) = 0;

    virtual VfsStatus directoryOpen(const char* path, Os::Directory::OpenMode mode, void*& out_handle) = 0;
    virtual VfsStatus directoryRewind(void* directory_handle) = 0;
    virtual VfsStatus directoryRead(void* directory_handle, char* file_name_buffer, FwSizeType buffer_size) = 0;
    virtual void directoryClose(void* directory_handle) = 0;

    virtual VfsStatus filesystemRemoveDirectory(const char* path) = 0;
    virtual VfsStatus filesystemRemoveFile(const char* path) = 0;
    virtual VfsStatus filesystemRename(const char* source_path, const char* destination_path) = 0;
    virtual VfsStatus filesystemGetFreeSpace(const char* path,
                                             FwSizeType& out_total_bytes,
                                             FwSizeType& out_free_bytes) = 0;
    virtual VfsStatus filesystemGetPathType(const char* path, Os::FileSystem::PathType& out_path_type) = 0;
    virtual VfsStatus filesystemGetWorkingDirectory(char* path_buffer, FwSizeType buffer_size) = 0;
    virtual VfsStatus filesystemChangeWorkingDirectory(const char* path) = 0;
};

using ConsoleWriteFn = void (*)(const CHAR* message, FwSizeType size);
using RawTimeEpochHook =
    bool (*)(U32 monotonic_seconds, U32 monotonic_useconds, U32& out_epoch_seconds, U32& out_epoch_useconds);
using StaticTaskAllocator =
    bool (*)(const Os::Task::Arguments& arguments,
             StaticTask_t*& out_task_buffer,
             StackType_t*& out_stack_buffer,
             U32& out_stack_depth_in_words);

void registerVfsAdapter(VfsAdapter* adapter);
void registerConsoleWriter(ConsoleWriteFn writer);
void registerRawTimeEpochHook(RawTimeEpochHook hook);
void registerStaticTaskAllocator(StaticTaskAllocator allocator);

VfsAdapter* getVfsAdapter();
ConsoleWriteFn getConsoleWriter();
RawTimeEpochHook getRawTimeEpochHook();
StaticTaskAllocator getStaticTaskAllocator();

Os::File::Status toFileStatus(VfsStatus status);
Os::Directory::Status toDirectoryStatus(VfsStatus status);
Os::FileSystem::Status toFileSystemStatus(VfsStatus status);

}  // namespace FreeRTOSSupport
}  // namespace Os

#endif

// ======================================================================
// \title  FreeRTOS/Os/FileSystem.hpp
// \brief  FreeRTOS implementation for Os::FileSystem
// ======================================================================
#ifndef OS_FREERTOS_FILESYSTEM_HPP
#define OS_FREERTOS_FILESYSTEM_HPP

#include <Os/FileSystem.hpp>

namespace Os {
namespace FreeRTOS {
namespace FileSystem {

struct FreeRtosFileSystemHandle : public FileSystemHandle {};

class FreeRtosFileSystem final : public FileSystemInterface {
  public:
    FreeRtosFileSystem() = default;
    ~FreeRtosFileSystem() override = default;

    Status _removeDirectory(const char* path) override;
    Status _removeFile(const char* path) override;
    Status _rename(const char* source_path, const char* destination_path) override;
    Status _getFreeSpace(const char* path, FwSizeType& total_bytes, FwSizeType& free_bytes) override;
    Status _getPathType(const char* path, PathType& path_type) override;
    Status _getWorkingDirectory(char* path, FwSizeType buffer_size) override;
    Status _changeWorkingDirectory(const char* path) override;
    FileSystemHandle* getHandle() override;

  private:
    FreeRtosFileSystemHandle m_handle;
};

}  // namespace FileSystem
}  // namespace FreeRTOS
}  // namespace Os

#endif

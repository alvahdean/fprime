// ======================================================================
// \title  FreeRTOS/Os/Directory.hpp
// \brief  FreeRTOS implementation for Os::Directory
// ======================================================================
#ifndef OS_FREERTOS_DIRECTORY_HPP
#define OS_FREERTOS_DIRECTORY_HPP

#include <Os/Directory.hpp>

namespace Os {
namespace FreeRTOS {
namespace Directory {

class FreeRtosDirectory final : public DirectoryInterface {
  public:
    FreeRtosDirectory();
    ~FreeRtosDirectory() override;

    DirectoryHandle* getHandle() override;
    Status open(const char* path, OpenMode mode) override;
    Status rewind() override;
    Status read(char* file_name_buffer, FwSizeType buffer_size) override;
    void close() override;

  private:
    void* m_backend_handle;
};

}  // namespace Directory
}  // namespace FreeRTOS
}  // namespace Os

#endif

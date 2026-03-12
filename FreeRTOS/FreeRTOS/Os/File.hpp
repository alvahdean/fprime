// ======================================================================
// \title  FreeRTOS/Os/File.hpp
// \brief  FreeRTOS implementation for Os::File
// ======================================================================
#ifndef OS_FREERTOS_FILE_HPP
#define OS_FREERTOS_FILE_HPP

#include <Os/File.hpp>

namespace Os {
namespace FreeRTOS {
namespace File {

class FreeRtosFile final : public FileInterface {
  public:
    FreeRtosFile();
    FreeRtosFile(const FreeRtosFile& other);
    FreeRtosFile& operator=(const FreeRtosFile& other);
    ~FreeRtosFile() override;

    Status open(const char* path, Mode mode, OverwriteType overwrite) override;
    void close() override;
    Status size(FwSizeType& size_result) override;
    Status position(FwSizeType& position_result) override;
    Status preallocate(FwSizeType offset, FwSizeType length) override;
    Status seek(FwSignedSizeType offset, SeekType seek_type) override;
    Status flush() override;
    Status read(U8* buffer, FwSizeType& size, WaitType wait) override;
    Status write(const U8* buffer, FwSizeType& size, WaitType wait) override;
    FileHandle* getHandle() override;

  private:
    struct SharedState {
        void* m_backend_handle;
        U32 m_ref_count;
    };

    void release();

  private:
    SharedState* m_state;
};

}  // namespace File
}  // namespace FreeRTOS
}  // namespace Os

#endif

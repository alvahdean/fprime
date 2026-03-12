#include <gtest/gtest.h>
#include <cstring>

#include "Fw/Types/String.hpp"
#include "Os/Console.hpp"
#include "Os/Directory.hpp"
#include "Os/File.hpp"
#include "Os/FileSystem.hpp"
#include "FreeRTOS/Os/FreeRTOSSupport.hpp"

namespace {

U32 s_console_calls = 0;
void console_writer(const CHAR* message, FwSizeType size) {
    (void)message;
    (void)size;
    s_console_calls += 1;
}

class MockVfsAdapter final : public Os::FreeRTOSSupport::VfsAdapter {
  public:
    Os::FreeRTOSSupport::VfsStatus fileOpen(const char* path,
                                            Os::File::Mode mode,
                                            Os::File::OverwriteType overwrite,
                                            void*& out_handle) override {
        (void)path;
        (void)mode;
        (void)overwrite;
        out_handle = &this->m_token;
        this->m_open = true;
        this->m_position = 0;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    void fileClose(void* file_handle) override {
        (void)file_handle;
        this->m_open = false;
    }

    Os::FreeRTOSSupport::VfsStatus fileDuplicate(void* source_file_handle, void*& out_duplicate_handle) override {
        out_duplicate_handle = source_file_handle;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus fileSize(void* file_handle, FwSizeType& out_size) override {
        (void)file_handle;
        out_size = this->m_size;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus filePosition(void* file_handle, FwSizeType& out_position) override {
        (void)file_handle;
        out_position = this->m_position;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus filePreallocate(void* file_handle, FwSizeType offset, FwSizeType length) override {
        (void)file_handle;
        (void)offset;
        (void)length;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus fileSeek(void* file_handle,
                                            FwSignedSizeType offset,
                                            Os::File::SeekType seek_type) override {
        (void)file_handle;
        if (seek_type == Os::File::SeekType::ABSOLUTE) {
            this->m_position = static_cast<FwSizeType>(offset);
        } else {
            this->m_position += static_cast<FwSizeType>(offset);
        }
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus fileFlush(void* file_handle) override {
        (void)file_handle;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus fileRead(void* file_handle,
                                            U8* buffer,
                                            FwSizeType& in_out_size,
                                            Os::File::WaitType wait_type) override {
        (void)file_handle;
        (void)wait_type;
        for (FwSizeType i = 0; i < in_out_size; i++) {
            buffer[i] = static_cast<U8>('A');
        }
        this->m_position += in_out_size;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus fileWrite(void* file_handle,
                                             const U8* buffer,
                                             FwSizeType& in_out_size,
                                             Os::File::WaitType wait_type) override {
        (void)file_handle;
        (void)buffer;
        (void)wait_type;
        this->m_position += in_out_size;
        if (this->m_position > this->m_size) {
            this->m_size = this->m_position;
        }
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus directoryOpen(const char* path,
                                                 Os::Directory::OpenMode mode,
                                                 void*& out_handle) override {
        (void)path;
        (void)mode;
        out_handle = &this->m_token;
        this->m_dir_reads = 0;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus directoryRewind(void* directory_handle) override {
        (void)directory_handle;
        this->m_dir_reads = 0;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus directoryRead(void* directory_handle,
                                                 char* file_name_buffer,
                                                 FwSizeType buffer_size) override {
        (void)directory_handle;
        if (this->m_dir_reads > 0) {
            return Os::FreeRTOSSupport::VfsStatus::NO_MORE_FILES;
        }
        if (buffer_size < 6) {
            return Os::FreeRTOSSupport::VfsStatus::BUFFER_TOO_SMALL;
        }
        this->m_dir_reads += 1;
        (void)strncpy(file_name_buffer, "hello", static_cast<size_t>(buffer_size));
        file_name_buffer[buffer_size - 1] = '\0';
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    void directoryClose(void* directory_handle) override {
        (void)directory_handle;
    }

    Os::FreeRTOSSupport::VfsStatus filesystemRemoveDirectory(const char* path) override {
        (void)path;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus filesystemRemoveFile(const char* path) override {
        (void)path;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus filesystemRename(const char* source_path, const char* destination_path) override {
        (void)source_path;
        (void)destination_path;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus filesystemGetFreeSpace(const char* path,
                                                          FwSizeType& out_total_bytes,
                                                          FwSizeType& out_free_bytes) override {
        (void)path;
        out_total_bytes = 1024;
        out_free_bytes = 512;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus filesystemGetPathType(const char* path,
                                                         Os::FileSystem::PathType& out_path_type) override {
        (void)path;
        out_path_type = Os::FileSystem::PathType::FILE;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus filesystemGetWorkingDirectory(char* path_buffer, FwSizeType buffer_size) override {
        if (buffer_size < 2) {
            return Os::FreeRTOSSupport::VfsStatus::BUFFER_TOO_SMALL;
        }
        (void)strncpy(path_buffer, "/", static_cast<size_t>(buffer_size));
        path_buffer[buffer_size - 1] = '\0';
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

    Os::FreeRTOSSupport::VfsStatus filesystemChangeWorkingDirectory(const char* path) override {
        (void)path;
        return Os::FreeRTOSSupport::VfsStatus::OP_OK;
    }

  private:
    U32 m_token = 0;
    bool m_open = false;
    FwSizeType m_size = 0;
    FwSizeType m_position = 0;
    U32 m_dir_reads = 0;
};

}  // namespace

TEST(FreeRTOSSupport, DefaultsReturnNotSupported) {
    Os::FreeRTOSSupport::registerVfsAdapter(nullptr);

    Os::File file;
    ASSERT_EQ(file.open("/tmp/nope", Os::File::Mode::OPEN_READ), Os::File::Status::NOT_SUPPORTED);
}

TEST(FreeRTOSSupport, ConsoleWriterRegistration) {
    s_console_calls = 0;
    Os::FreeRTOSSupport::registerConsoleWriter(console_writer);

    Os::Console console;
    console.writeMessage("abc", 3);
    ASSERT_EQ(s_console_calls, 1U);

    Os::FreeRTOSSupport::registerConsoleWriter(nullptr);
}

TEST(FreeRTOSSupport, VfsAdapterRoundTrip) {
    MockVfsAdapter adapter;
    Os::FreeRTOSSupport::registerVfsAdapter(&adapter);

    Os::File file;
    ASSERT_EQ(file.open("/mock.bin", Os::File::Mode::OPEN_CREATE), Os::File::Status::OP_OK);

    U8 write_data[4] = {1, 2, 3, 4};
    FwSizeType write_size = sizeof(write_data);
    ASSERT_EQ(file.write(write_data, write_size), Os::File::Status::OP_OK);
    ASSERT_EQ(write_size, sizeof(write_data));

    FwSizeType size = 0;
    ASSERT_EQ(file.size(size), Os::File::Status::OP_OK);
    ASSERT_EQ(size, sizeof(write_data));

    file.close();
    Os::FreeRTOSSupport::registerVfsAdapter(nullptr);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

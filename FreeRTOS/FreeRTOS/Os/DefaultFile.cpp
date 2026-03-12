// ======================================================================
// \title  FreeRTOS/Os/DefaultFile.cpp
// \brief  Sets default File/FileSystem/Directory implementation to FreeRTOS
// ======================================================================

#include "Os/Delegate.hpp"
#include "Os/Directory.hpp"
#include "Os/File.hpp"
#include "Os/FileSystem.hpp"
#include "FreeRTOS/Os/Directory.hpp"
#include "FreeRTOS/Os/File.hpp"
#include "FreeRTOS/Os/FileSystem.hpp"

namespace Os {

FileInterface* FileInterface::getDelegate(FileHandleStorage& aligned_new_memory, const FileInterface* to_copy) {
    return Os::Delegate::makeDelegate<FileInterface, Os::FreeRTOS::File::FreeRtosFile>(aligned_new_memory, to_copy);
}

FileSystemInterface* FileSystemInterface::getDelegate(FileSystemHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<FileSystemInterface, Os::FreeRTOS::FileSystem::FreeRtosFileSystem>(
        aligned_new_memory);
}

DirectoryInterface* DirectoryInterface::getDelegate(DirectoryHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<DirectoryInterface, Os::FreeRTOS::Directory::FreeRtosDirectory>(
        aligned_new_memory);
}

}  // namespace Os

#ifndef STORAGEMANAGER_HPP
#define STORAGEMANAGER_HPP

#include <cstddef>   // for size_t
#include <cstdint>   // for uint8_t

class StorageManager
{
public:
    virtual ~StorageManager() = default;

    [[nodiscard]] virtual bool begin() = 0;

    [[nodiscard]] virtual bool createEmptyFile(const char* path) = 0;

    // Escribe al final / sobrescribe
    [[nodiscard]] virtual bool appendBytesToFile(const char* path, const uint8_t* data, size_t length) = 0;

    [[nodiscard]] virtual bool overwriteBytesToFile(const char* path, const uint8_t* data, size_t length) = 0;

    [[nodiscard]] virtual size_t readFileBytes(const char* path, uint8_t* outBuffer, size_t maxLen) = 0;

    [[nodiscard]] virtual size_t readFileRegionBytes(const char* path, size_t offset, uint8_t* outBuffer, size_t len) =
    0;

    [[nodiscard]] virtual bool writeFileBytes(const char* path, const uint8_t* data, size_t length) = 0;

    [[nodiscard]] virtual bool truncateFileFromOffset(const char* path, size_t offset) = 0;

    [[nodiscard]] virtual bool clearFile(const char* path) = 0;

    [[nodiscard]] virtual bool deleteFile(const char* path) = 0;

    [[nodiscard]] virtual bool fileExists(const char* path) = 0;

    [[nodiscard]] virtual bool renameFile(const char* oldPath, const char* newPath) = 0;

    [[nodiscard]] virtual bool createDirectory(const char* str) = 0;

    [[nodiscard]] virtual size_t fileSize(const char* str) = 0;
};


#endif //STORAGEMANAGER_HPP

#ifndef SD_STORAGE_MANAGER_H
#define SD_STORAGE_MANAGER_H

#ifdef PLATFORM_ARDUINO

#include "StorageManager/StorageManager.hpp"
#include "ClassName.h"
#include <SdFat.h>

#define SDCARD_SS_PIN 4
#define SPI_SPEED SD_SCK_MHZ(2)  // Ajusta según tu placa

class SDStorageManager final : public StorageManager
{
    CLASS_NAME(SDStorageManager)

public:
    ~SDStorageManager() override = default;

    explicit SDStorageManager(uint8_t chipSelectPin = SDCARD_SS_PIN);

    [[nodiscard]] bool begin() override;

    [[nodiscard]] bool createEmptyFile(const char* path) override;

    [[nodiscard]] bool appendBytesToFile(const char* path, const uint8_t* data, size_t length) override;

    [[nodiscard]] bool overwriteBytesToFile(const char* path, const uint8_t* data, size_t length) override;

    [[nodiscard]] bool clearFile(const char* path) override;

    [[nodiscard]] bool writeFileBytes(const char* path, const uint8_t* data, size_t length) override;

    [[nodiscard]] size_t readFileBytes(const char* path, uint8_t* outBuffer, size_t maxLen) override;

    [[nodiscard]] size_t readFileRegionBytes(const char* path, size_t offset, uint8_t* outBuffer, size_t len) override;

    [[nodiscard]] bool deleteFile(const char* path) override;

    [[nodiscard]] bool truncateFileFromOffset(const char* path, size_t offset) override;

    [[nodiscard]] bool fileExists(const char* path) override;

    [[nodiscard]] bool renameFile(const char* oldPath, const char* newPath) override;

    [[nodiscard]] size_t fileSize(const char* str) override;

    [[nodiscard]] bool createDirectory(const char* str) override;

    [[nodiscard]] bool existsDirectory(const char* path);

    [[nodiscard]] bool clearDirectory(const char *path);

    [[nodiscard]] bool deleteDirectory(const char* path);

private:
    uint8_t chipSelectPin;
};

#endif  // PLATFORM_ARDUINO
#endif  // SD_STORAGE_MANAGER_H

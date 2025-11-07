#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#ifdef PLATFORM_ARDUINO

#include "StorageManager/StorageManager.hpp"
#include <cstddef>

#include "ClassName.h"


#define SDCARD_SS_PIN 4

/**
 * @brief Implementación de StorageManager usando la tarjeta SD de Arduino.
 */
class SDStorageManager final : public StorageManager
{
    CLASS_NAME(SDStorageManager)

public:
    explicit SDStorageManager(uint8_t chipSelectPin = SDCARD_SS_PIN);

    bool begin() override;

    bool appendToFile(const char* path, const char* content) override;
    bool overwriteFile(const char* path, const char* content) override;


    size_t readFile(const char* path, char* outBuffer, size_t maxLen) override;

    bool deleteFile(const char* path) override;

    bool writeFileBytes(const char* path, const uint8_t* data, size_t length) override;
    size_t readFileBytes(const char* path, uint8_t* outBuffer, size_t maxLen) override;

private:
    uint8_t chipSelectPin;
};

#endif // ARDUINO

#endif // SD_MANAGER_H

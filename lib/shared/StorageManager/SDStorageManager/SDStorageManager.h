#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#ifdef ARDUINO

#include "StorageManager/StorageManager.hpp"

#define SDCARD_SS_PIN 4

class SDStorageManager : public StorageManager{
public:
    explicit SDStorageManager(uint8_t chipSelectPin = SDCARD_SS_PIN);
    bool begin() override;
    bool appendToFile(const char* path, const std::string& content) override;
    bool overwriteFile(const char* path, const std::string& content) override;
    std::string readFile(const char* path) override;
    bool deleteFile(const char* path) override;

    bool writeFileBytes(const char* path, const uint8_t* data, size_t length) override;
    bool writeFileBytes(const char* path, const std::vector<uint8_t>& data) override;

    std::vector<uint8_t> readFileBytes(const char* path) override;

private:
    uint8_t chipSelectPin;
};

#endif // ARDUINO

#endif // SD_MANAGER_H

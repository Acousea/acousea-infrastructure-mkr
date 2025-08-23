#ifndef HDDSTORAGEMANAGER_HPP
#define HDDSTORAGEMANAGER_HPP

#include "StorageManager/StorageManager.hpp"


class HDDStorageManager final : public StorageManager {
public:
    HDDStorageManager() = default;

    bool writeFileBytes(const char* path, const uint8_t* data, size_t length) override;
    std::vector<uint8_t> readFileBytes(const char* path) override;

    bool begin() override; // No-op en host
    bool appendToFile(const char* path, const std::string& content) override;
    bool overwriteFile(const char* path, const std::string& content) override;
    std::string readFile(const char* path) override;
    bool deleteFile(const char* path) override;

};


#endif //HDDSTORAGEMANAGER_HPP

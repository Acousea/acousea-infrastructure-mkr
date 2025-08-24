#ifndef STORAGEMANAGER_HPP
#define STORAGEMANAGER_HPP

#include <string>
#include <vector>
#include <cstdint>

class StorageManager {
public:
    virtual ~StorageManager() = default;

    virtual bool begin() = 0;

    // Escribe al final / sobrescribe
    virtual bool appendToFile(const char* path, const std::string& content) = 0;
    virtual bool overwriteFile(const char* path, const std::string& content) = 0;

    // Lee archivo completo
    virtual std::string readFile(const char* path) = 0;

    // Borra archivo
    virtual bool deleteFile(const char* path) = 0;

    // En StorageManager.hpp
    virtual bool writeFileBytes(const char* path, const uint8_t* data, size_t length) = 0;

    virtual bool writeFileBytes(const char* path, const std::vector<uint8_t>& data) = 0;

    virtual std::vector<uint8_t> readFileBytes(const char* path) = 0;
};


#endif //STORAGEMANAGER_HPP

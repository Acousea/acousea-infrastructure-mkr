#ifndef ACOUSEA_INFRASTRUCTURE_MKR_INMEMORY_STORAGE_MANAGER_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_INMEMORY_STORAGE_MANAGER_HPP

#include "StorageManager/StorageManager.hpp"
#include <unordered_map>
#include <cstring>
#include <vector>
#include <mutex>

/**
 * @brief Implementación en memoria de StorageManager para tests.
 *        Simula un sistema de archivos simple usando un mapa clave->contenido binario.
 */
class InMemoryStorageManager : public StorageManager
{
private:
    std::unordered_map<std::string, std::vector<uint8_t>> files;
    mutable std::mutex mtx;

public:
    bool begin() override { return true; }

    // ----------------------------------------------------
    // Escritura de texto
    // ----------------------------------------------------
    bool appendToFile(const char* path, const char* content) override
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!path || !content) return false;

        const size_t len = std::strlen(content);
        auto& fileData = files[path];
        fileData.insert(fileData.end(), content, content + len);
        return true;
    }

    bool overwriteFile(const char* path, const char* content) override
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!path || !content) return false;

        const size_t len = std::strlen(content);
        files[path] = std::vector<uint8_t>(content, content + len);
        return true;
    }

    // ----------------------------------------------------
    // Lectura y borrado de texto
    // ----------------------------------------------------
    size_t readFile(const char* path, char* outBuffer, size_t maxLen) override
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!path || !outBuffer || maxLen == 0) return 0;

        auto it = files.find(path);
        if (it == files.end()) return 0;

        const auto& data = it->second;
        size_t len = data.size() < (maxLen - 1) ? data.size() : (maxLen - 1);

        std::memcpy(outBuffer, data.data(), len);
        outBuffer[len] = '\0';  // Null-terminate
        return len;
    }

    bool deleteFile(const char* path) override
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!path) return false;
        return files.erase(path) > 0;
    }

    // ----------------------------------------------------
    // Escritura binaria
    // ----------------------------------------------------
    bool writeFileBytes(const char* path, const uint8_t* data, size_t length) override
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!path || (!data && length > 0)) return false;

        files[path] = std::vector<uint8_t>(data, data + length);
        return true;
    }


    // ----------------------------------------------------
    // Lectura binaria
    // ----------------------------------------------------
    size_t readFileBytes(const char* path, uint8_t* outBuffer, size_t maxLen) override
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!path || !outBuffer || maxLen == 0) return 0;

        auto it = files.find(path);
        if (it == files.end()) return 0;

        const auto& data = it->second;
        const size_t len = (data.size() < maxLen) ? data.size() : maxLen;

        std::memcpy(outBuffer, data.data(), len);
        return len;
    }

    // ----------------------------------------------------
    // Utilidades extra para tests
    // ----------------------------------------------------
    bool exists(const char* path) const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return path && files.find(path) != files.end();
    }


    size_t fileCount() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return files.size();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mtx);
        files.clear();
    }
};

#endif // ACOUSEA_INFRASTRUCTURE_MKR_INMEMORY_STORAGE_MANAGER_HPP

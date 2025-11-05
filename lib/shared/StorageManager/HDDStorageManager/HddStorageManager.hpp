#ifndef HDDSTORAGEMANAGER_HPP
#define HDDSTORAGEMANAGER_HPP

#ifdef PLATFORM_NATIVE

#include "StorageManager/StorageManager.hpp"


#include <cstddef>
#include <cstdint>

/**
 * @brief Implementación de StorageManager que opera sobre el sistema de archivos del host.
 *        Compatible con la nueva interfaz libre de STL en la API pública.
 */
class HDDStorageManager final : public StorageManager
{
public:
    HDDStorageManager() = default;

    bool begin() override;

    bool appendToFile(const char* path, const char* content) override;
    bool overwriteFile(const char* path, const char* content) override;
    size_t readFile(const char* path, char* outBuffer, size_t maxLen) override;
    bool deleteFile(const char* path) override;

    bool writeFileBytes(const char* path, const uint8_t* data, size_t length) override;
    size_t readFileBytes(const char* path, uint8_t* outBuffer, size_t maxLen) override;
};


#endif // PLATFORM_NATIVE

#endif //HDDSTORAGEMANAGER_HPP

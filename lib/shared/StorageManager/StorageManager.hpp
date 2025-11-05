#ifndef STORAGEMANAGER_HPP
#define STORAGEMANAGER_HPP

#include <cstddef>   // for size_t
#include <cstdint>   // for uint8_t

class StorageManager
{
public:
    virtual ~StorageManager() = default;

    virtual bool begin() = 0;

    // Escribe al final / sobrescribe
    virtual bool appendToFile(const char* path, const char* content) = 0;
    virtual bool overwriteFile(const char* path, const char* content) = 0;

    // Lee un archivo completo como texto (NULL-terminated). // Devuelve la longitud leída (sin incluir '\0').
    virtual size_t readFile(const char* path, char* outBuffer, size_t maxLen) = 0;

    // Borra un archivo
    virtual bool deleteFile(const char* path) = 0;

    // En StorageManager.hpp
    virtual bool writeFileBytes(const char* path, const uint8_t* data, size_t length) = 0;

    virtual size_t readFileBytes(const char* path, uint8_t* outBuffer, size_t maxLen) = 0;
};


#endif //STORAGEMANAGER_HPP

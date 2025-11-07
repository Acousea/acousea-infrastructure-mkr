#ifdef PLATFORM_ARDUINO

#include "SDStorageManager.h"
#include "SD.h"
#include "ErrorHandler/ErrorHandler.h"
#include "Logger/Logger.h"

SDStorageManager::SDStorageManager(uint8_t chipSelectPin) : chipSelectPin(chipSelectPin)
{
}

bool SDStorageManager::begin()
{
    if (!SD.begin(chipSelectPin))
    {
        ERROR_HANDLE_CLASS("SDStorageManager::begin() -> Failed to initialize SD card");
        return false;
    }
    Serial.println("SDStorageManager::begin() -> SD card initialized.");
    return true;
}

bool SDStorageManager::appendToFile(const char* path, const char* content)
{
    File file = SD.open(path, FILE_WRITE);
    if (!file)
    {
        LOG_CLASS_ERROR("SDStorageManager::appendToFile() -> Error opening file");
        return false;
    }
    file.print(content);
    file.close();
    return true;
}


bool SDStorageManager::overwriteFile(const char* path, const char* content)
{
    File file = SD.open(path, (O_READ | O_WRITE | O_CREAT | O_TRUNC));
    if (!file)
    {
        LOG_CLASS_ERROR("SDStorageManager::overwriteFile() -> Error opening file");
        return false;
    }
    file.print(content);
    file.close();
    return true;
}

size_t SDStorageManager::readFile(const char* path, char* outBuffer, size_t maxLen)
{
    File file = SD.open(path, FILE_READ);
    if (!file)
    {
        LOG_CLASS_ERROR("SDStorageManager::readFile() -> Error opening file");
        return 0;
    }

    size_t i = 0;
    while (file.available() && i < (maxLen - 1))
    {
        outBuffer[i++] = static_cast<char>(file.read());
    }
    outBuffer[i] = '\0';
    file.close();
    return i;
}


bool SDStorageManager::deleteFile(const char* path)
{
    if (!SD.exists(path))
    {
        // Serial.println("SDStorageManager::deleteFile() -> File: " + String(path) + " not found.");
        LOG_CLASS_ERROR("SDStorageManager::deleteFile() -> File: %s not found.", path);
        return false;
    }
    if (!SD.remove(path))
    {
        LOG_CLASS_ERROR("SDStorageManager::deleteFile() -> Error deleting file: %s", path);
        return false;
    }
    return true;
}

// Escribe (sobrescribe) un archivo binario completo
bool SDStorageManager::writeFileBytes(const char* path, const uint8_t* data, size_t length)
{
    File file = SD.open(path, (O_READ | O_WRITE | O_CREAT | O_TRUNC));
    if (!file)
    {
        LOG_CLASS_ERROR("SDStorageManager::writeFileBytes() -> Error opening file for writing");
        return false;
    }

    size_t written = 0;
    // Escribimos en bloques por si 'length' es grande (opcional, pero seguro)
    while (written < length)
    {
        constexpr size_t CHUNK_SIZE = 512;
        const size_t toWrite = (length - written) < CHUNK_SIZE ? (length - written) : CHUNK_SIZE;
        const size_t w = file.write(data + written, toWrite);

        if (w != toWrite)
        {
            LOG_CLASS_ERROR("SDStorageManager::writeFileBytes() -> Partial/failed write");
            file.close();
            return false;
        }

        written += w;
    }

    file.flush();
    file.close();
    return true;
}

size_t SDStorageManager::readFileBytes(const char* path, uint8_t* outBuffer, size_t maxLen)
{
    File file = SD.open(path, FILE_READ);
    if (!file)
    {
        LOG_CLASS_ERROR("SDStorageManager::readFileBytes() -> Error opening file");
        return 0;
    }

    size_t readTotal = 0;
    while (file.available() && readTotal < maxLen)
    {
        const int c = file.read();
        if (c < 0) break;
        outBuffer[readTotal++] = static_cast<uint8_t>(c);
    }

    file.close();
    return readTotal;
}


#endif // PLATFORM_ARDUINO

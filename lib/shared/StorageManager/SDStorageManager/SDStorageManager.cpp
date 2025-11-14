#ifdef PLATFORM_ARDUINO

#include "SDStorageManager.h"
#include "ErrorHandler/ErrorHandler.h"
#include "Logger/Logger.h"
#include <wait/WaitFor.hpp>

#define SD_CONFIG SdSpiConfig(chipSelectPin, SHARED_SPI, SPI_SPEED)

// Selecciona tipo FAT (3 = FAT + exFAT automático)
#define SD_FAT_TYPE 1

#if SD_FAT_TYPE == 0
SdFat sd;
typedef File File_t;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
typedef File32 File_t;
#elif SD_FAT_TYPE == 2
SdExFat sd;
typedef ExFile File_t;
#elif SD_FAT_TYPE == 3
SdFs sd;
typedef FsFile File_t;
#else
#error "Invalid SD_FAT_TYPE"
#endif

// -----------------------------------------------------
#define STABILIZATION_DELAY_MS 100

SDStorageManager::SDStorageManager(uint8_t chipSelectPin)
    : chipSelectPin(chipSelectPin)
{
}

bool SDStorageManager::begin()
{
    if (!sd.begin(SD_CONFIG))
    {
        sd.initErrorHalt(&Serial);
        ERROR_HANDLE_CLASS("SDStorageManager::begin() -> SD init failed");
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS); // Wait a bit for the SD card to settle
    Serial.println("SDStorageManager::begin() -> SD initialized successfully.");
    return true;
}

bool SDStorageManager::createEmptyFile(const char* path)
{
    File_t file;
    if (!file.open(path, O_WRONLY | O_CREAT | O_TRUNC))
    {
        LOG_CLASS_ERROR("createEmptyFile() -> Cannot create file: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS); 
    file.close();
    return true;
}


bool SDStorageManager::appendBytesToFile(const char* path, const uint8_t* data, size_t length)
{
    if (!data || length == 0) return false;

    File_t file;
    if (!file.open(path, O_WRONLY | O_CREAT | O_APPEND))
    {
        LOG_CLASS_ERROR("appendBytesToFile() -> Cannot open file: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS); 
    const size_t written = file.write(data, length);
    waitFor(STABILIZATION_DELAY_MS); 
    file.close();
    return (written == length);
}

bool SDStorageManager::overwriteBytesToFile(const char* path, const uint8_t* data, size_t length)
{
    if (!data) return false;

    File_t file;
    if (!file.open(path, O_WRONLY | O_CREAT | O_TRUNC))
    {
        LOG_CLASS_ERROR("overwriteBytesToFile() -> Cannot open file: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS); 
    const size_t written = file.write(data, length);
    waitFor(STABILIZATION_DELAY_MS); 
    file.close();
    return (written == length);
}


bool SDStorageManager::clearFile(const char* path)
{
    File_t file;
    if (!file.open(path, O_WRONLY | O_TRUNC))
    {
        LOG_CLASS_ERROR("clearFile() -> Cannot open file: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS);
    file.close();
    return true;
}



// -----------------------------------------------------

bool SDStorageManager::deleteFile(const char* path)
{
    if (!sd.exists(path))
    {
        LOG_CLASS_ERROR("deleteFile() -> File not found: %s", path);
        return false;
    }

    waitFor(STABILIZATION_DELAY_MS); 

    if (!sd.remove(path))
    {
        LOG_CLASS_ERROR("deleteFile() -> Failed to remove file: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS); 

    return true;
}

// -----------------------------------------------------

bool SDStorageManager::writeFileBytes(const char* path, const uint8_t* data, size_t length)
{
    File_t file;
    if (!file.open(path, O_WRONLY | O_CREAT | O_TRUNC))
    {
        LOG_CLASS_ERROR("writeFileBytes() -> Cannot open file: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS); 

    size_t written = file.write(data, length);
    waitFor(STABILIZATION_DELAY_MS); 
    file.close();

    return (written == length);
}

// -----------------------------------------------------

size_t SDStorageManager::readFileBytes(const char* path, uint8_t* outBuffer, size_t maxLen)
{
    File_t file;
    if (!file.open(path, O_RDONLY))
    {
        LOG_CLASS_ERROR("readFileBytes() -> Cannot open file: %s", path);
        return 0;
    }
    waitFor(STABILIZATION_DELAY_MS); 
    const size_t readTotal = file.read(outBuffer, maxLen);
    waitFor(STABILIZATION_DELAY_MS); 

    file.close();
    return readTotal;
}

// -----------------------------------------------------
// Lectura parcial por offset
// -----------------------------------------------------

size_t SDStorageManager::readFileRegionBytes(const char* path, size_t offset, uint8_t* outBuffer, size_t len)
{
    File_t file;
    if (!file.open(path, O_RDONLY))
    {
        LOG_CLASS_ERROR("readFileRegion() -> Cannot open file: %s", path);
        return 0;
    }
    waitFor(STABILIZATION_DELAY_MS); 

    file.seekSet(offset);
    waitFor(STABILIZATION_DELAY_MS); 

    const size_t readBytes = file.read(outBuffer, len);
    waitFor(STABILIZATION_DELAY_MS); 

    file.close();

    return readBytes;
}

// -----------------------------------------------------
// Truncar archivo desde offset
// -----------------------------------------------------

bool SDStorageManager::truncateFileFromOffset(const char* path, size_t offset)
{
    File_t file;
    if (!file.open(path, O_RDWR))
    {
        LOG_CLASS_ERROR("truncateFileFromOffset() -> Cannot open file: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS); 

    const size_t total = file.fileSize();
    if (offset >= total)
    {
        // Si offset >= tamaño, simplemente vaciamos
        file.truncate(0);
        file.close();
        return true;
    }

    const size_t remaining = total - offset;
    constexpr size_t CHUNK_SIZE = 512;
    uint8_t buffer[CHUNK_SIZE];
    size_t readPos = offset;
    size_t writePos = 0;

    while (readPos < total)
    {
        file.seekSet(readPos);
        size_t n = file.read(buffer, CHUNK_SIZE);
        if (!n)
            break;

        file.seekSet(writePos);
        file.write(buffer, n);

        readPos += n;
        writePos += n;
    }

    file.truncate(writePos);
    file.close();
    return true;
}

bool SDStorageManager::fileExists(const char* path)
{
    // Helper para comprobar existencia
    const bool exists =  sd.exists(path);
    waitFor(STABILIZATION_DELAY_MS); 
    return exists;
}

bool SDStorageManager::renameFile(const char* oldPath, const char* newPath)
{
    File_t file;
    if (!file.open(oldPath, O_RDWR)) {
        LOG_CLASS_ERROR("renameFile() -> Cannot open: %s", oldPath);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS); 

    const char* baseName = strrchr(newPath, '/');
    baseName = baseName ? baseName + 1 : newPath;

    // rename new path (no directory)
    if (!file.rename(baseName)) {
        LOG_CLASS_ERROR("renameFile() -> Cannot rename %s to %s", oldPath, newPath);
        file.close();
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS); 

    file.close();
    return true;
}


size_t SDStorageManager::fileSize(const char* str)
{
    File_t file;
    if (!file.open(str, O_RDONLY))
    {
        LOG_CLASS_ERROR("fileSize() -> Cannot open file: %s", str);
        return 0;
    }
    waitFor(STABILIZATION_DELAY_MS); 
    
    const size_t size = file.fileSize();
    waitFor(STABILIZATION_DELAY_MS); 
    
    file.close();
    return size;
}

bool SDStorageManager::createDirectory(const char* str)
{
    if (sd.exists(str))
    {
        return true;
    }

    
    if (!sd.mkdir(str))
    {
        LOG_CLASS_ERROR("createDirectory() -> Cannot create directory: %s", str);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS);
    
    return true;
}

bool SDStorageManager::existsDirectory(const char* path)
{
    const bool exists =  sd.exists(path);
    waitFor(STABILIZATION_DELAY_MS);
    return exists;

}

bool SDStorageManager::clearDirectory(const char* path)
{
    // Verificar si el directorio existe
    if (!sd.exists(path))
    {
        LOG_CLASS_ERROR("clearDirectory() -> Directory does not exist: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS);

    // Abrir el directorio
    File_t dir;
    if (!dir.open(path, O_RDONLY))
    {
        LOG_CLASS_ERROR("clearDirectory() -> Cannot open directory: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS);

    File_t entry;
    char name[64];

    // Recorrer todos los elementos dentro del directorio
    while (entry.openNext(&dir, O_RDONLY))
    {
        entry.getName(name, sizeof(name));

        // Construir ruta absoluta del elemento interno
        char fullPath[128];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, name);

        if (entry.isDir())
        {
            entry.close();

            // Primero vaciar el subdirectorio
            if (!clearDirectory(fullPath))
            {
                dir.close();
                return false;
            }

            // Luego eliminarlo
            if (!deleteDirectory(fullPath))
            {
                dir.close();
                return false;
            }
        }
        else
        {
            // Es un archivo normal → eliminarlo
            entry.close();
            if (!sd.remove(fullPath))
            {
                LOG_CLASS_ERROR("clearDirectory() -> Cannot delete file: %s", fullPath);
                dir.close();
                return false;
            }
        }
    }

    dir.close();
    return true;
}

    

bool SDStorageManager::deleteDirectory(const char* path)
{
    if (!sd.exists(path))
    {
        LOG_CLASS_ERROR("deleteDirectory() -> Directory does not exist: %s", path);
        return false;
    }

    waitFor(STABILIZATION_DELAY_MS);

    if (!sd.rmdir(path))
    {
        LOG_CLASS_ERROR("deleteDirectory() -> Cannot remove directory: %s", path);
        return false;
    }
    waitFor(STABILIZATION_DELAY_MS);

    return true;
}



#endif // PLATFORM_ARDUINO

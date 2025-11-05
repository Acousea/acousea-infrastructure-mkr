#ifdef PLATFORM_NATIVE

#include "HddStorageManager.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

bool HDDStorageManager::begin(){
    return true;
}

bool HDDStorageManager::writeFileBytes(const char* path, const uint8_t* data, size_t length) {
    try {
        // Crear directorios padre si hiciera falta
        const fs::path p(path);
        if (p.has_parent_path()) {
            std::error_code ec;
            fs::create_directories(p.parent_path(), ec); // best-effort
        }

        std::ofstream ofs(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!ofs) {
            std::cerr << "HDDStorageManager::writeFileBytes() -> Cannot open: " << path << "\n";
            return false;
        }

        if (length > 0 && data != nullptr) {
            ofs.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(length));
            if (!ofs) {
                std::cerr << "HDDStorageManager::writeFileBytes() -> Write failed for: " << path << "\n";
                return false;
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "HDDStorageManager::writeFileBytes() -> Exception: " << e.what() << "\n";
        return false;
    }
}

// --------------------------------------------------------------
// Lee un archivo binario dentro de un buffer
// --------------------------------------------------------------
size_t HDDStorageManager::readFileBytes(const char* path, uint8_t* outBuffer, size_t maxLen) {
    if (!path || !outBuffer || maxLen == 0) return 0;

    try {
        std::ifstream ifs(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (!ifs) {
            std::cerr << "HDDStorageManager::readFileBytes() -> Cannot open: " << path << "\n";
            return 0;
        }

        const std::ifstream::pos_type endPos = ifs.tellg();
        if (endPos <= 0) return 0;

        const size_t fileSize = static_cast<size_t>(endPos);
        const size_t toRead = (fileSize < maxLen) ? fileSize : maxLen;

        ifs.seekg(0, std::ios::beg);
        ifs.read(reinterpret_cast<char*>(outBuffer), static_cast<std::streamsize>(toRead));

        if (!ifs && !ifs.eof()) {
            std::cerr << "HDDStorageManager::readFileBytes() -> Read failed for: " << path << "\n";
            return 0;
        }

        return toRead;
    } catch (const std::exception& e) {
        std::cerr << "HDDStorageManager::readFileBytes() -> Exception: " << e.what() << "\n";
        return 0;
    }
}


// --------------------------------------------------------------
// Añade texto al final de un archivo
// --------------------------------------------------------------
bool HDDStorageManager::appendToFile(const char* path, const char* content) {
    if (!path || !content) return false;

    std::ofstream ofs(path, std::ios::out | std::ios::app | std::ios::binary);
    if (!ofs) {
        std::cerr << "HDDStorageManager::appendToFile() -> Cannot open: " << path << "\n";
        return false;
    }

    ofs << content;
    return true;
}


// --------------------------------------------------------------
// Sobrescribe completamente un archivo de texto
// --------------------------------------------------------------
bool HDDStorageManager::overwriteFile(const char* path, const char* content) {
    if (!path || !content) return false;

    std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!ofs) {
        std::cerr << "HDDStorageManager::overwriteFile() -> Cannot open: " << path << "\n";
        return false;
    }

    ofs << content;
    return true;
}
// --------------------------------------------------------------
// Lee un archivo de texto completo
// --------------------------------------------------------------
size_t HDDStorageManager::readFile(const char* path, char* outBuffer, size_t maxLen) {
    if (!path || !outBuffer || maxLen == 0) return 0;

    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) {
        std::cerr << "HDDStorageManager::readFile() -> Cannot open: " << path << "\n";
        return 0;
    }

    size_t count = 0;
    while (ifs.good() && count < maxLen - 1) {
        char c;
        ifs.get(c);
        if (!ifs.good()) break;
        outBuffer[count++] = c;
    }
    outBuffer[count] = '\0';
    return count;
}

bool HDDStorageManager::deleteFile(const char* path){
    try{
        if (fs::exists(path) && fs::is_regular_file(path)){
            return fs::remove(path);
        }
        // Si no existe o no es regular, devuelve false
        return false;
    }
    catch (const std::exception& e){
        std::cerr << "HDDStorageManager::deleteFile() -> Error: " << e.what() << "\n";
        return false;
    }
}

#endif // ARDUINO
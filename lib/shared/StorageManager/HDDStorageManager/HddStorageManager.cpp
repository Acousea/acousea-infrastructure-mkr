#ifndef ARDUINO

#include "HddStorageManager.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

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

bool HDDStorageManager::writeFileBytes(const char* path, const std::vector<uint8_t>& data)
{
    return writeFileBytes(path, data.data(), data.size());
}

std::vector<uint8_t> HDDStorageManager::readFileBytes(const char* path) {
    try {
        std::ifstream ifs(path, std::ios::in | std::ios::binary | std::ios::ate);
        if (!ifs) {
            std::cerr << "HDDStorageManager::readFileBytes() -> Cannot open: " << path << "\n";
            return {};
        }

        const std::ifstream::pos_type endPos = ifs.tellg();
        if (endPos <= 0) {
            return {};
        }

        std::vector<uint8_t> buffer(static_cast<size_t>(endPos));
        ifs.seekg(0, std::ios::beg);

        if (!buffer.empty()) {
            ifs.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
            if (!ifs && !ifs.eof()) {
                std::cerr << "HDDStorageManager::readFileBytes() -> Read failed for: " << path << "\n";
                return {};
            }
        }
        return buffer;
    } catch (const std::exception& e) {
        std::cerr << "HDDStorageManager::readFileBytes() -> Exception: " << e.what() << "\n";
        return {};
    }
}


bool HDDStorageManager::begin(){
    return true;
}

bool HDDStorageManager::appendToFile(const char* path, const std::string& content){
    std::ofstream ofs(path, std::ios::out | std::ios::app | std::ios::binary);
    if (!ofs){
        std::cerr << "HDDStorageManager::appendToFile() -> Cannot open: " << path << "\n";
        return false;
    }
    ofs << content;
    return true;
}

bool HDDStorageManager::overwriteFile(const char* path, const std::string& content){
    // std::ios::trunc para truncar
    std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!ofs){
        std::cerr << "HDDStorageManager::overwriteFile() -> Cannot open: " << path << "\n";
        return false;
    }
    ofs << content;
    return true;
}

std::string HDDStorageManager::readFile(const char* path){
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs){
        std::cerr << "HDDStorageManager::readFile() -> Cannot open: " << path << "\n";
        return {};
    }
    std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return data;
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
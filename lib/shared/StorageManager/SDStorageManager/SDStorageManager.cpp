#ifdef ARDUINO

#include "SDStorageManager.h"

SDStorageManager::SDStorageManager(uint8_t chipSelectPin) : chipSelectPin(chipSelectPin) {}

bool SDStorageManager::begin() {
    if (!SD.begin(chipSelectPin)) {
        Serial.println("SDStorageManager::begin() -> Failed to initialize SD card");
        return false;
    }
    Serial.println("SDStorageManager::begin() -> SD card initialized.");
    return true;
}

bool SDStorageManager::appendToFile(const char* path, const std::string& content) {
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("SDStorageManager::appendToFile() -> Error opening file for writing");
        return false;
    }

    String contentStr(content.c_str());
    file.print(contentStr);
    file.close();
    return true;
}

bool SDStorageManager::overwriteFile(const char* path, const std::string& content) {
    File file = SD.open(path, (O_READ | O_WRITE | O_CREAT | O_TRUNC));
    if (!file) {
        Serial.println("SDStorageManager::overwriteFile() -> Error opening file for writing");
        return false;
    }


    String contentStr(content.c_str());
    file.print(contentStr);
    file.close();
    return true;
}

std::string SDStorageManager::readFile(const char* path) {
    File file = SD.open(path, FILE_READ);
    if (!file) {
        Serial.println("SDStorageManager::readFile() -> Error opening file for reading");
        return "";
    }

    std::string  content;
    while (file.available()) {
        content += (char)file.read();
    }

    file.close();
    return content;
}


bool SDStorageManager::deleteFile(const char* path) {
    if (SD.exists(path)) {
        SD.remove(path);
        Serial.print("SDStorageManager::deleteFile() -> File: " + String(path) + " deleted.");
        return true;
    } else {
        Serial.print("SDStorageManager::deleteFile() -> File: " + String(path) + " not found.");
        return false;
    }
}

// --- SDStorageManager.cpp (implementaciones nuevas) ---
#include <vector>

// Escribe (sobrescribe) un archivo binario completo
bool SDStorageManager::writeFileBytes(const char* path, const uint8_t* data, size_t length) {
    File file = SD.open(path, (O_READ | O_WRITE | O_CREAT | O_TRUNC));
    if (!file) {
        Serial.println("SDStorageManager::writeFileBytes() -> Error opening file for writing");
        return false;
    }

    size_t written = 0;
    // Escribimos en bloques por si 'length' es grande (opcional, pero seguro)
    const size_t CHUNK = 512;
    while (written < length) {
        size_t toWrite = (length - written) < CHUNK ? (length - written) : CHUNK;
        size_t w = file.write(data + written, toWrite);
        if (w != toWrite) {
            Serial.println("SDStorageManager::writeFileBytes() -> Partial/failed write");
            file.close();
            return false;
        }
        written += w;
    }

    file.flush();
    file.close();
    return true;
}

bool SDStorageManager::writeFileBytes(const char* path, const std::vector<uint8_t>& data)
{
    return writeFileBytes(path, data.data(), data.size());
}

// Lee todo el archivo como vector<uint8_t>
std::vector<uint8_t> SDStorageManager::readFileBytes(const char* path) {
    File file = SD.open(path, FILE_READ);
    if (!file) {
        Serial.println("SDStorageManager::readFileBytes() -> Error opening file for reading");
        return {};
    }

    // Intentamos reservar según tamaño reportado por la librería SD
    std::vector<uint8_t> buffer;
    const size_t fsize = file.size(); // Puede no estar disponible en todas las plataformas
    if (fsize > 0) {
        buffer.resize(fsize);
        size_t readTotal = 0;
        while (readTotal < fsize && file.available()) {
            int c = file.read();
            if (c < 0) break;
            buffer[readTotal++] = static_cast<uint8_t>(c);
        }
        // En caso de tamaños no fiables, ajustamos al número realmente leído
        buffer.resize(readTotal);
    } else {
        // Fallback: tamaño desconocido -> leemos por bloques
        const size_t CHUNK = 512;
        buffer.reserve(1024);
        while (file.available()) {
            uint8_t tmp[CHUNK];
            int r = file.read(tmp, CHUNK);
            if (r <= 0) break;
            buffer.insert(buffer.end(), tmp, tmp + r);
        }
    }

    file.close();
    return buffer;
}

#endif // ARDUINO
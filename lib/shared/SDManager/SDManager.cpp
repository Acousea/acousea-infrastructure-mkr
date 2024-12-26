#include "SDManager.h"


SDManager::SDManager(uint8_t chipSelectPin) : chipSelectPin(chipSelectPin) {}

bool SDManager::begin() {
    if (!SD.begin(chipSelectPin)) {
        Serial.println("SDManager::begin() -> Failed to initialize SD card");
        return false;
    }
    Serial.println("SDManager::begin() -> SD card initialized.");
    return true;
}

bool SDManager::appendToFile(const char* path, const String& content) {
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("SDManager::appendToFile() -> Error opening file for writing");
        return false;
    }

    file.print(content);
    file.close();
    return true;
}

bool SDManager::overwriteFile(const char* path, const String& content) {
    File file = SD.open(path, (O_READ | O_WRITE | O_CREAT | O_TRUNC));
    if (!file) {
        Serial.println("SDManager::overwriteFile() -> Error opening file for writing");
        return false;
    }

    file.print(content);
    file.close();
    return true;
}

String SDManager::readFile(const char* path) {
    File file = SD.open(path, FILE_READ);
    if (!file) {
        Serial.println("SDManager::readFile() -> Error opening file for reading");
        return "";
    }

    String content;
    while (file.available()) {
        content += (char)file.read();
    }

    file.close();
    return content;
}

void SDManager::listFiles(File dir, int numTabs) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) break;

        for (uint8_t i = 0; i < numTabs; i++) Serial.print('\t');
        Serial.print(entry.name());
        if (entry.isDirectory()) {
            Serial.println("/");
            listFiles(entry, numTabs + 1);
        } else {
            Serial.print("\t\t"); Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

bool SDManager::deleteFile(const char* path) {
    if (SD.exists(path)) {
        SD.remove(path);
        Serial.print("SDManager::deleteFile() -> File: " + String(path) + " deleted.");
        return true;
    } else {
        Serial.print("SDManager::deleteFile() -> File: " + String(path) + " not found.");
        return false;
    }
}

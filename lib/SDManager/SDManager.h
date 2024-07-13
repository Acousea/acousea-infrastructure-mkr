#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <SD.h>
#include <Arduino.h>

class SDManager {
public:
    SDManager(uint8_t chipSelectPin);
    bool begin();
    bool appendToFile(const char* path, const String& content);
    bool overwriteFile(const char* path, const String& content);
    String readFile(const char* path);
    void listFiles(File dir, int numTabs);
    bool deleteFile(const char* path);

private:
    uint8_t chipSelectPin;
};

#endif // SD_MANAGER_H

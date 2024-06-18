#ifndef _SD_LOGGER_H_
#define _SD_LOGGER_H_

#include <Arduino.h>
// #include "../../include/config.h" -> CAN NOT INCLUDE "Include" FOLDER files in LIBRARY
// #include <SPI.h>
// #include <Wire.h>
// #include <SD.h>
// #include <LoRa.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
// #include <Arduino_MKRGPS.h>
// #include <Arduino_PMIC.h>
// #include <RTCZero.h>
// #include <IridiumSBD.h> 

#include <SD.h>
#include "RTCManager.h"
#include <config.h>

// Structure for SDLogger config
typedef struct SDLoggerConfig {
    uint8_t sdCardPin;    
    char logFilename[14];
    char vemcoFilename[14];
    char commsFilename[14];
};

#include <SD.h>
#include "RTCManager.h"

class SDLogger {
public:
    SDLogger(const SDLoggerConfig& config, RTCManager& rtcManager)
    : _config(config), _rtcManager(rtcManager) {}

    bool init() {
        if (!SD.begin(_config.sdCardPin)) {
            #if _DEBUG_THROUGH_SERIAL_
                debugPort.print("ERROR: Card failed, or not present!\n");
            #endif
            return false;
        }

        logDebugString("Card initialized\n");
        return true;
    }

    void setFileNames() {
        char debug_str[100];
        char dateStr[9];

        File dataFile = SD.open("start.log", FILE_WRITE);
        _rtcManager.getDateStr(dateStr, sizeof(dateStr));

        if (strncmp(dateStr, _config.logFilename, 8)) {
            snprintf(_config.logFilename, sizeof(_config.logFilename), "%8s.log", dateStr);
            snprintf(debug_str, sizeof(debug_str),"Debug log filename: %s", _config.logFilename);
            dataFile.println(debug_str);
            #if _DEBUG_THROUGH_SERIAL_
                debugPort.print("Debug log filename: ");
                debugPort.println(_config.logFilename);
            #endif
        }
        if (strncmp(dateStr, _config.vemcoFilename, 8)) {
            snprintf(_config.vemcoFilename, sizeof(_config.vemcoFilename), "%8s.vrc", dateStr);
            snprintf(debug_str, sizeof(debug_str),"Vemco log filename: %s", _config.vemcoFilename);
            dataFile.println(debug_str);
            #if _DEBUG_THROUGH_SERIAL_
                debugPort.print("Vemco log filename: ");
                debugPort.println(_config.vemcoFilename);
            #endif
        }
        if (strncmp(dateStr, _config.commsFilename, 8)) {
            snprintf(_config.commsFilename, sizeof(_config.commsFilename), "%8s.com", dateStr);
            snprintf(debug_str, sizeof(debug_str),"Comms log filename: %s", _config.commsFilename);
            dataFile.println(debug_str);
            #if _DEBUG_THROUGH_SERIAL_
                debugPort.print("Comms log filename: ");
                debugPort.println(_config.commsFilename);
            #endif
        }

        dataFile.close();
    }

    void logDebugString(const char *str) {
        #if _DEBUG_THROUGH_SERIAL_
            debugPort.print(str);
        #endif

        char prefix[50];
        char dateTime[32];
        _rtcManager.getDateTime(dateTime, sizeof(dateTime));

        snprintf(prefix, sizeof(prefix), "%s: ", dateTime);
        File dataFile = SD.open(_config.logFilename, FILE_WRITE);

        dataFile.write((uint8_t *)prefix, strlen(prefix));
        dataFile.print(str);
        dataFile.close();
    }

    void logCommsPacket(const char *str, size_t numBytes, uint8_t mode, uint8_t channel) {
        char prefix[50];
        char dateTime[32];
        _rtcManager.getDateTime(dateTime, sizeof(dateTime));

        snprintf(prefix, sizeof(prefix), "%s: %s %hhu ", dateTime,
                 (mode == 0) ? ": NONE" : ((mode == OUT) ? ": SENT" : ": RECV"),
                 channel);
        File dataFile = SD.open(_config.commsFilename, FILE_WRITE);

        dataFile.write((uint8_t *)prefix, strlen(prefix));
        dataFile.write((uint8_t *)str, numBytes);
        dataFile.println();
        dataFile.close();
    }

    void logCommsBinPacket(const uint8_t *buffer, size_t numBytes, uint8_t mode, uint8_t channel) {
        char prefix[50];
        char dateTime[32];
        _rtcManager.getDateTime(dateTime, sizeof(dateTime));

        snprintf(prefix, sizeof(prefix), "%s: %s %hhu ", dateTime,
                 (mode == 0) ? ": NONE" : ((mode == OUT) ? ": SENT" : ": RECV"),
                 channel);
        File dataFile = SD.open(_config.commsFilename, FILE_WRITE);
        dataFile.write((uint8_t *)prefix, strlen(prefix));

        for (size_t i = 0; i < numBytes; i++) {
            char aux[4];
            snprintf(aux, sizeof(aux), "%02X ", buffer[i]);
            dataFile.write((uint8_t *)aux, 3);
        }
        dataFile.println();
        dataFile.close();
    }

    void logVr2cPacket(const char *str, size_t numBytes) {
        char prefix[50];
        char dateTime[32];
        _rtcManager.getDateTime(dateTime, sizeof(dateTime));
        snprintf(prefix, sizeof(prefix), "%s: ", dateTime);
        
        File dataFile = SD.open(_config.vemcoFilename, FILE_WRITE);
        dataFile.write((uint8_t *)prefix, strlen(prefix));
        dataFile.write((uint8_t *)str, numBytes);
        dataFile.println();
        dataFile.close();
    }

private:
    const SDLoggerConfig& _config;
    RTCManager& _rtcManager;
};

#endif
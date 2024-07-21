#include "ReportingPeriodManager.h"
#include <Arduino.h>


ReportingPeriodManager::ReportingPeriodManager(SDManager& sdManager, const char* filePath) 
    : sdManager(sdManager), filePath(filePath) {
    
    OperationModePeriod launchingMode = {
        .name = "Launching",
        .sbd_reporting_period_default = 65535, // 
        .sbd_reporting_period_custom = 0,
        .lora_reporting_period_default = 3,
        .lora_reporting_period_custom = 0
    };

    OperationModePeriod driftingMode = {
        .name = "Working",
        .sbd_reporting_period_default = 1,
        .sbd_reporting_period_custom = 0,
        .lora_reporting_period_default = 0, // By default, LoRa is disabled
        .lora_reporting_period_custom = 0
    };

    OperationModePeriod recoveringMode = {
        .name = "Recovering",
        .sbd_reporting_period_default = 65535, // Just for testing
        .sbd_reporting_period_custom = 0,
        .lora_reporting_period_default = 2,
        .lora_reporting_period_custom = 0
    };

    modes[0] = launchingMode;
    modes[1] = driftingMode;
    modes[2] = recoveringMode;
}

bool ReportingPeriodManager::begin() {  
    if (!loadData()) {
        Serial.println("ReportingPeriodManager::begin() -> Failed to load data, using default values.");
        if (!saveData()) {
            Serial.println("ReportingPeriodManager::begin() -> Error saving initial data.");
            return false;
        }
    }

    printData();
    SerialUSB.println("ReportingPeriodManager initialized.");
    return true;
}


bool ReportingPeriodManager::isNewConfigAvailable() {
    return newConfigAvailable;
}


// Reset method to overwrite the data with the values specified in the constructor
bool ReportingPeriodManager::reset() {
    if (!saveData()) {
        Serial.println("ReportingPeriodManager::reset() -> Error saving initial data.");
        return false;
    }
    return true;
}
    


bool ReportingPeriodManager::loadData() {
    String content = sdManager.readFile(filePath);
    if (content == "") return false;

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, content);
    if (error) {
        Serial.println("ReportingPeriodManager::loadData() -> Error deserializing JSON");
        return false;
    }

    for (int i = 0; i < 3; i++) {
        modes[i].sbd_reporting_period_custom = doc[modes[i].name]["sbd_reporting_period_custom"];
        modes[i].lora_reporting_period_custom = doc[modes[i].name]["lora_reporting_period_custom"];
    }

    return true;
}

bool ReportingPeriodManager::saveData() {
    StaticJsonDocument<512> doc;
    for (int i = 0; i < 3; i++) {
        doc[modes[i].name]["sbd_reporting_period_custom"] = modes[i].sbd_reporting_period_custom;
        doc[modes[i].name]["lora_reporting_period_custom"] = modes[i].lora_reporting_period_custom;
    }

    String content;
    serializeJson(doc, content);

    return sdManager.overwriteFile(filePath, content);
}

void ReportingPeriodManager::printData() {
    for (int i = 0; i < 3; i++) {
        Serial.print("Mode: "); Serial.println(modes[i].name);
        Serial.print("  SBD Default: "); Serial.println(modes[i].sbd_reporting_period_default);
        Serial.print("  SBD Custom: "); Serial.println(modes[i].sbd_reporting_period_custom);
        Serial.print("  LoRa Default: "); Serial.println(modes[i].lora_reporting_period_default);
        Serial.print("  LoRa Custom: "); Serial.println(modes[i].lora_reporting_period_custom);
    }
}

void ReportingPeriodManager::updateCustomValues(const char* mode, int sbd_custom, int lora_custom) {
    for (int i = 0; i < 3; i++) {
        if (strcmp(modes[i].name, mode) == 0) {
            modes[i].sbd_reporting_period_custom = sbd_custom;
            modes[i].lora_reporting_period_custom = lora_custom;
            if (!saveData()) {
                Serial.println("ReportingPeriodManager::updateCustomValues() -> Error saving updated data.");
            }
            if (!loadData()) {
                Serial.println("ReportingPeriodManager::updateCustomValues() -> Error loading updated data.");
            }
            newConfigAvailable = true;
            return;
        }
    }
    Serial.println("ReportingPeriodManager::updateCustomValues() -> Operation mode not found");
}


ReportingPeriods ReportingPeriodManager::getReportingPeriods(const char* mode) {
    for (int i = 0; i < 3; i++) {
        if (strcmp(modes[i].name, mode) == 0) {
            uint16_t sbd_period = modes[i].sbd_reporting_period_custom != 0 ? modes[i].sbd_reporting_period_custom : modes[i].sbd_reporting_period_default;
            uint16_t lora_period = modes[i].lora_reporting_period_custom != 0 ? modes[i].lora_reporting_period_custom : modes[i].lora_reporting_period_default;
            newConfigAvailable = false;
            return ReportingPeriods(sbd_period, lora_period);
        }
    }
    Serial.println("ReportingPeriodManager::getReportingPeriods() -> Operation mode not found");
    return ReportingPeriods();
}
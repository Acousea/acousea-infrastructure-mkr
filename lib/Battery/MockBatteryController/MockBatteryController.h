#ifndef MOCKBATTERYCONTROLLER_H
#define MOCKBATTERYCONTROLLER_H


#include "IBatteryController.h"
#include "Logger/Logger.h"
#include <string>

class MockBatteryController final : public IBatteryController{
private:
    uint8_t mockPercentage;
    acousea_BatteryStatus mockStatus;

public:
    explicit MockBatteryController(const uint8_t initialPercentage = 100, const acousea_BatteryStatus initialStatus = acousea_BatteryStatus_BATTERY_STATUS_FULL)
        : mockPercentage(initialPercentage), mockStatus(initialStatus){
    }

    bool init() override{
        Logger::logInfo("MockBatteryController: Initializing battery controller...");
        return true; // Always returns true in the mock implementation
    }

    uint8_t voltageSOC_rounded() override{
        return mockPercentage;
    }

    acousea_BatteryStatus status() override{
        return mockStatus;
    }

    // Setters for mock data
    void setMockPercentage(const uint8_t percentage){
        mockPercentage = percentage;
    }

    void setMockStatus(const acousea_BatteryStatus status){
        mockStatus = status;
    }
};


#endif //MOCKBATTERYCONTROLLER_H

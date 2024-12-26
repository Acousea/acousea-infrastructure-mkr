#ifndef MOCKBATTERYCONTROLLER_H
#define MOCKBATTERYCONTROLLER_H


#include "IBatteryController.h"
#include "Logger/Logger.h"
#include <string>

class MockBatteryController : public IBatteryController {
private:
    uint8_t mockPercentage;
    uint8_t mockStatus;

public:
    MockBatteryController(uint8_t initialPercentage = 100, uint8_t initialStatus = 0)
        : mockPercentage(initialPercentage), mockStatus(initialStatus) {
    }

    bool init() override {
        Logger::logInfo("MockBatteryController: Initializing battery controller...");
        return true; // Always returns true in the mock implementation
    }

    uint8_t percentage() override {
        Logger::logInfo("MockBatteryController: Returning battery percentage: " + std::to_string(mockPercentage));
        return mockPercentage;
    }

    uint8_t status() override {
        Logger::logInfo("MockBatteryController: Returning battery status: " + std::to_string(mockStatus));
        return mockStatus;
    }

    // Setters for mock data
    void setMockPercentage(uint8_t percentage) {
        mockPercentage = percentage;
    }

    void setMockStatus(uint8_t status) {
        mockStatus = status;
    }
};


#endif //MOCKBATTERYCONTROLLER_H

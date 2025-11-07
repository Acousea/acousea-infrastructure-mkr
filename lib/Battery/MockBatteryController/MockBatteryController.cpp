//
// Created by Admin on 26/12/2024.
//

#include "MockBatteryController.h"

MockBatteryController::MockBatteryController(const uint8_t initialPercentage, const acousea_BatteryStatus initialStatus): mockPercentage(initialPercentage), mockStatus(initialStatus)
{
}

bool MockBatteryController::init()
{
    LOG_CLASS_INFO("Initializing battery controller...");
    return true; // Always returns true in the mock implementation
}

bool MockBatteryController::sync()
{
    return true;
}

uint8_t MockBatteryController::voltageSOC_rounded()
{
    return mockPercentage;
}

acousea_BatteryStatus MockBatteryController::status()
{
    return mockStatus;
}

void MockBatteryController::setMockPercentage(const uint8_t percentage)
{
    mockPercentage = percentage;
}

void MockBatteryController::setMockStatus(const acousea_BatteryStatus status)
{
    mockStatus = status;
}

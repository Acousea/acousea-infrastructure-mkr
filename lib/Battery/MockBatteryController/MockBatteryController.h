#ifndef MOCKBATTERYCONTROLLER_H
#define MOCKBATTERYCONTROLLER_H


#include "IBatteryController.h"
#include "Logger/Logger.h"
#include "ClassName.h"

class MockBatteryController final : public IBatteryController{
    CLASS_NAME(MockBatteryController)

private:
    uint8_t mockPercentage;
    acousea_BatteryStatus mockStatus;

public:
    explicit MockBatteryController(const uint8_t initialPercentage = 100, const acousea_BatteryStatus initialStatus = acousea_BatteryStatus_BATTERY_STATUS_FULL);

    bool init() override;

    bool sync() override;

    uint8_t voltageSOC_rounded() override;

    acousea_BatteryStatus status() override;

    // Setters for mock data
    void setMockPercentage(const uint8_t percentage);

    void setMockStatus(const acousea_BatteryStatus status);
};


#endif //MOCKBATTERYCONTROLLER_H

#ifndef REPORTING_PERIOD_MANAGER_H
#define REPORTING_PERIOD_MANAGER_H

#include <ArduinoJson.h>
#include "SDManager.h"

struct ReportingPeriods {
    uint16_t sbd_reporting_period;
    uint16_t lora_reporting_period;

    ReportingPeriods(uint16_t sbd = 0, uint16_t lora = 0)
        : sbd_reporting_period(sbd), lora_reporting_period(lora) {}
};

class ReportingPeriodManager {
public:
    struct OperationModePeriod {
        const char* name;
        uint16_t sbd_reporting_period_default;
        uint16_t sbd_reporting_period_custom;
        uint16_t lora_reporting_period_default;
        uint16_t lora_reporting_period_custom;
    };

    ReportingPeriodManager(SDManager& sdManager, const char* filePath);
    bool begin();
    bool isNewConfigAvailable();
    bool reset();
    void printData();
    void updateCustomValues(const char* mode, int sbd_custom, int lora_custom);
    ReportingPeriods getReportingPeriods(const char* mode);

private:
    bool loadData();
    bool saveData();

    SDManager& sdManager;
    const char* filePath;
    OperationModePeriod modes[3];
    bool newConfigAvailable;
};

#endif // REPORTING_PERIOD_MANAGER_H

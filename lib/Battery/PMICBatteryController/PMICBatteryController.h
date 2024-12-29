#ifndef PMIC_MANAGER_H
#define PMIC_MANAGER_H


#include "Arduino_PMIC.h"
#include "IBatteryController.h"
#include <ErrorHandler/ErrorHandler.h>
#include <Logger/Logger.h>

class PMICBatteryController : public IBatteryController {
public:
    bool init() override;

    uint8_t percentage() override;

    uint8_t status() override;

    void printStatus() const;

private:
    bool error = false;

    static constexpr float INPUT_CURRENT_LIMIT = 2.0;
    static constexpr float INPUT_VOLTAGE_LIMIT = 3.88;
    static constexpr float MIN_SYSTEM_VOLTAGE = 3.5;
    static constexpr float CHARGE_VOLTAGE = 4.2;
    static constexpr float CHARGE_CURRENT = 0.375;

    void applySettings();

    [[nodiscard]] const char *getChargeStatusMessage(uint8_t chargeStatus) const;
};

#endif // PMIC_MANAGER_H

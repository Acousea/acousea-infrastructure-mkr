#ifndef ACOUSEA_MKR1310_NODES_BATTERYMODULE_H
#define ACOUSEA_MKR1310_NODES_BATTERYMODULE_H

#include "Payload/Modules/SerializableModule.h"

class BatteryModule : public SerializableModule {
public:
    static BatteryModule from(uint8_t batteryPercentage, uint8_t batteryStatus);

    static BatteryModule from(const std::vector<uint8_t> &value);

    [[nodiscard]] uint8_t getBatteryPercentage() const;

    [[nodiscard]] uint8_t getBatteryStatus() const;

private:
    explicit BatteryModule(const std::vector<uint8_t> &value);

    BatteryModule(uint8_t batteryPercentage, uint8_t batteryStatus);

private:
    uint8_t batteryPercentage; // Almacena el porcentaje de batería
    uint8_t batteryStatus; // Almacena el estado de la batería
};

#endif // ACOUSEA_MKR1310_NODES_BATTERYMODULE_H

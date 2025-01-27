#ifndef ACOUSEA_MKR1310_NODES_MOSFETCONTROLLER_HPP
#define ACOUSEA_MKR1310_NODES_MOSFETCONTROLLER_HPP

#include <Arduino.h>
#include "Logger/Logger.h"
#include "../PiPowerControlPins.hpp"

class MosfetController {
public:
    explicit MosfetController(int mosfetControlPin = MOSFET_CONTROL_PIN);

    void switchOnMOSFET() const;

    void switchOffMOSFET() const;

    [[nodiscard]] bool isMOSFETOn() const;

private:
    const int mosfetControlPin;
};

#endif //ACOUSEA_MKR1310_NODES_MOSFETCONTROLLER_HPP

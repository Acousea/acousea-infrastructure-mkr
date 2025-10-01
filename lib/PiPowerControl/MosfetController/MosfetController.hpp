#ifndef ACOUSEA_MKR1310_NODES_MOSFETCONTROLLER_HPP
#define ACOUSEA_MKR1310_NODES_MOSFETCONTROLLER_HPP

#ifdef ARDUINO

#include <Arduino.h>
#include "ClassName.h"
#include "Logger/Logger.h"
#include "../PiPowerControlPins.hpp"

/**
 * @class MosfetController
 * @brief Controls the MOSFET switch used to power external devices.
 *
 * The MOSFET is wired so that:
 *  - LOW drives the MOSFET "on" (output-A active, LED shows BLUE).
 *  - HIGH drives the MOSFET "off" (output-B active, LED shows RED).
 *
 * This controller allows turning the MOSFET on/off and querying its state.
 * Default pin is defined in PiPowerControlPins.hpp (MOSFET_CONTROL_PIN).
 */

class MosfetController{
    CLASS_NAME(MosfetController)

public:
    explicit MosfetController(int mosfetControlPin = MOSFET_CONTROL_PIN);

    void switchOnMOSFET() const;

    void switchOffMOSFET() const;

    [[nodiscard]] bool isMOSFETOn() const;

private:
    const int mosfetControlPin;
};

#endif // ARDUINO

#endif //ACOUSEA_MKR1310_NODES_MOSFETCONTROLLER_HPP

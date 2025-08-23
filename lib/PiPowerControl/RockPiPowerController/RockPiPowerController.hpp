#ifndef ACOUSEA_MKR1310_NODES_ROCKPIPOWERCONTROLLER_HPP
#define ACOUSEA_MKR1310_NODES_ROCKPIPOWERCONTROLLER_HPP

#ifdef ARDUINO
#include <Arduino.h>
#include "../MosfetController/MosfetController.hpp"
#include "Logger/Logger.h"
#include "ArduinoLowPower.h"
#include "../PiPowerControlPins.hpp"

class RockPiPowerController {
public:
    RockPiPowerController(
            MosfetController &mosfetController,
            int rockPiShutdownPin = ROCK_PI_SHUTDOWN_PIN,
            int rockPiMonitorPin = ROCK_PI_MONITOR_PIN
    );

    void commandShutdown();

    void commandStartup();

    bool isRockPiUp();

private:
    const int rockPiShutdownPin;
    const int rockPiMonitorPin;
    MosfetController &mosfetController;
};

#endif // ARDUINO


#endif //ACOUSEA_MKR1310_NODES_ROCKPIPOWERCONTROLLER_HPP

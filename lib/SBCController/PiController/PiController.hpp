#ifndef ACOUSEA_ROCKPIPOWERCONTROLLER_HPP
#define ACOUSEA_ROCKPIPOWERCONTROLLER_HPP

#ifdef ARDUINO
#include <Arduino.h>
#include "MosfetController/MosfetController.hpp"
#include "Logger/Logger.h"
#include "ArduinoLowPower.h"
#include "ClassName.h"
#include "PiPowerControlPins.hpp"

/**
 * @class PiController
 * @brief Manages power control and startup/shutdown sequence of the Rock Pi.
 *
 * Uses a MOSFET to control main power and a dedicated shutdown pin to request
 * a graceful shutdown. The monitor pin indicates Rock Pi power state.
 * Provides methods to:
 *  - Send shutdown signal and cut MOSFET after a grace period.
 *  - Power cycle MOSFET to start the Rock Pi.
 *  - Query current power status.
 */

class PiController
{
public:
    CLASS_NAME(RockPiPowerController)
    explicit PiController(
        int mosfetControlPin = MOSFET_CONTROL_PIN,
        int rockPiShutdownPin = ROCK_PI_SHUTDOWN_PIN,
        int rockPiMonitorPin = ROCK_PI_MONITOR_PIN
    );

    void commandShutdown() const;

    void commandStartup() const;
    void forceRestart() const;

    [[nodiscard]] bool isRockPiUp() const;

private:
    const int rockPiShutdownPin;
    const int rockPiMonitorPin;
    MosfetController mosfetController;

    enum : unsigned long
    {
        RECEIVE_SHUTDOWN_SIGNAL_PERIOD = 30000,
        STARTUP_TIMEOUT = 60000,
        POLL_INTERVAL = 500,
        FULL_SHUTDOWN_GRACE_PERIOD = 60000
    };
};

#endif // ARDUINO


#endif //ACOUSEA_ROCKPIPOWERCONTROLLER_HPP

#ifdef ARDUINO
#include "RockPiPowerController.hpp"

#include "WaitFor.hpp"

RockPiPowerController::RockPiPowerController(const int mosfetControlPin, const int rockPiShutdownPin,
                                             const int rockPiMonitorPin) :
    rockPiShutdownPin(rockPiShutdownPin), rockPiMonitorPin(rockPiMonitorPin), mosfetController(mosfetControlPin)
{
    pinMode(rockPiMonitorPin, INPUT_PULLDOWN);
    pinMode(rockPiShutdownPin, OUTPUT);
}

bool RockPiPowerController::isRockPiUp() const
{
    constexpr int READ_COUNT = 5;
    constexpr int DELAY_MS = 250;

    int highCount = 0;
    for (int i = 0; i < READ_COUNT; ++i)
    {
        // Rock Pi está activa si el pin está en HIGH (3.3V, same pin that activates TracoPower)
        if (digitalRead(rockPiMonitorPin) == HIGH) highCount++;
        delay(DELAY_MS); // DO NOT CHANGE BY WAITFOR (leads to infinite recursion)
    }

    return highCount == READ_COUNT;
}

void RockPiPowerController::commandStartup() const
{
    if (isRockPiUp())
    {
        Logger::logInfo(getClassNameString() + "::commandStartup() -> Rock Pi already on.");
        mosfetController.switchOnMOSFET();
        return;
    }

    Logger::logInfo(getClassNameString() + "::commandStartup() -> Power cycling MOSFET to start Rock Pi...");
    mosfetController.switchOffMOSFET();
    waitFor(2000); // asegurar corte total
    mosfetController.switchOnMOSFET();

    // Esperar a que arranque
    waitForOrUntil(STARTUP_TIMEOUT, [this] { return isRockPiUp(); });

    if (!isRockPiUp())
    {
        Logger::logError(getClassNameString() + "::commandStartup() -> Timeout: Rock Pi did not start.");
        return;
    }
    Logger::logInfo(getClassNameString() + "::commandStartup() -> Rock Pi is on.");
}

void RockPiPowerController::commandShutdown() const
{
    if (!isRockPiUp())
    {
        Logger::logInfo(getClassNameString() + "::commandShutdown() -> Rock Pi already off.");
        return;
    }

    Logger::logInfo(getClassNameString() + "commandShutdown() -> Sending shutdown signal to Rock Pi...");
    digitalWrite(rockPiShutdownPin, HIGH);

    // Esperar a que se apague
    waitForOrUntil(RECEIVE_SHUTDOWN_SIGNAL_PERIOD, [this] { return !isRockPiUp(); });

    if (isRockPiUp())
    {
        Logger::logError(
            getClassNameString() + "commandShutdown() -> Timeout: Rock Pi did not receive shut down signal.");
        return;
    }
    Logger::logInfo(
        getClassNameString() +
        "commandShutdown() -> Rock Pi has received shut down signal. Waiting 60 sec period before cutting MOSFET...");

    digitalWrite(rockPiShutdownPin, LOW);
    waitFor(FULL_SHUTDOWN_GRACE_PERIOD, 10000, [&](unsigned long elapsed)
    {
        Logger::logInfo(
            getClassNameString() + "commandShutdown() -> Grace period... " + std::to_string(elapsed / 1000) +
            "s elapsed"
        );
    });
    if (isRockPiUp())
    {
        Logger::logError(
            getClassNameString() + "commandShutdown() -> Timeout: Rock Pi did not shut down in grace period.");
        return;
    }

    Logger::logInfo(getClassNameString() + "commandShutdown() -> Rockpi is down. Cutting MOSFET power...");
    mosfetController.switchOffMOSFET();
}

void RockPiPowerController::forceRestart() const
{
    Logger::logWarning(getClassNameString() + "::forceRestart() -> Forcing power cycle of Rock Pi...");
    mosfetController.switchOffMOSFET();
    waitFor(2000); // asegurar corte total
    mosfetController.switchOnMOSFET();
}

#endif // ARDUINO

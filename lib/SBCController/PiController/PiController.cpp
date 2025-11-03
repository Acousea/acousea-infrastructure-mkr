#ifdef ARDUINO
#include "PiController.hpp"
#include "wait/WaitFor.hpp"

PiController::PiController(const int mosfetControlPin, const int rockPiShutdownPin,
                           const int rockPiMonitorPin) :
    rockPiShutdownPin(rockPiShutdownPin), rockPiMonitorPin(rockPiMonitorPin), mosfetController(mosfetControlPin)
{
    pinMode(rockPiMonitorPin, INPUT_PULLDOWN);
    pinMode(rockPiShutdownPin, OUTPUT);
}

bool PiController::isRockPiUp() const
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

void PiController::commandStartup() const
{
    if (isRockPiUp())
    {
        LOG_CLASS_INFO("::commandStartup() -> Rock Pi already on.");
        mosfetController.switchOnMOSFET();
        return;
    }

    LOG_CLASS_INFO("::commandStartup() -> Power cycling MOSFET to start Rock Pi...");
    mosfetController.switchOffMOSFET();
    waitFor(2000); // asegurar corte total
    mosfetController.switchOnMOSFET();

    // Esperar a que arranque
    waitForOrUntil(STARTUP_TIMEOUT, [this] { return isRockPiUp(); });

    if (!isRockPiUp())
    {
        LOG_CLASS_ERROR("::commandStartup() -> Timeout: Rock Pi did not start.");
        return;
    }
    LOG_CLASS_INFO("::commandStartup() -> Rock Pi is on.");
}

void PiController::commandShutdown() const
{
    if (!isRockPiUp())
    {
        LOG_CLASS_INFO("::commandShutdown() -> Rock Pi already off.");
        return;
    }

    LOG_CLASS_INFO("commandShutdown() -> Sending shutdown signal to Rock Pi...");
    digitalWrite(rockPiShutdownPin, HIGH);

    // Esperar a que se apague
    waitForOrUntil(RECEIVE_SHUTDOWN_SIGNAL_PERIOD, [this] { return !isRockPiUp(); });

    if (isRockPiUp())
    {
        LOG_CLASS_ERROR("commandShutdown() -> Timeout: Rock Pi did not receive shut down signal.");
        return;
    }
    LOG_CLASS_INFO(
        "commandShutdown() -> Rock Pi has received shut down signal. Waiting 60 sec period before cutting MOSFET...");

    digitalWrite(rockPiShutdownPin, LOW);
    waitFor(FULL_SHUTDOWN_GRACE_PERIOD, 10000, [&](unsigned long elapsed)
    {
        LOG_CLASS_INFO("commandShutdown() -> Grace period... %lus elapsed", elapsed / 1000);
    });
    if (isRockPiUp())
    {
        LOG_CLASS_ERROR("commandShutdown() -> Timeout: Rock Pi did not shut down in grace period.");
        return;
    }

    LOG_CLASS_INFO("commandShutdown() -> Rockpi is down. Cutting MOSFET power...");
    mosfetController.switchOffMOSFET();
}

void PiController::forceRestart() const
{
    LOG_CLASS_WARNING("::forceRestart() -> Forcing power cycle of Rock Pi...");
    mosfetController.switchOffMOSFET();
    waitFor(2000); // asegurar corte total
    mosfetController.switchOnMOSFET();
}

#endif // ARDUINO

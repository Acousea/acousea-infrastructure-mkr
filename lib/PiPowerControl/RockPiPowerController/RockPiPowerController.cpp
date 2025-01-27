#include "RockPiPowerController.hpp"

RockPiPowerController::RockPiPowerController(
        MosfetController &mosfetController, int rockPiShutdownPin, int rockPiMonitorPin)
        : rockPiShutdownPin(rockPiShutdownPin), rockPiMonitorPin(rockPiMonitorPin), mosfetController(mosfetController) {
    pinMode(rockPiMonitorPin, INPUT_PULLUP);
    pinMode(rockPiShutdownPin, INPUT_PULLDOWN);
}

bool RockPiPowerController::isRockPiUp() {
    return digitalRead(rockPiMonitorPin) == LOW; // Rock Pi está activa si el pin está en LOW
}

void RockPiPowerController::commandShutdown() {
    Logger::logInfo("Sending shutdown signal to Rock Pi...");
    pinMode(rockPiShutdownPin, OUTPUT);
    digitalWrite(rockPiShutdownPin, HIGH);

    delay(5000); // Mantener la señal durante 5 segundos

    pinMode(rockPiShutdownPin, INPUT_PULLDOWN);
    Logger::logInfo("Shutdown signal completed. Rock Pi should be off.");
    mosfetController.switchOffMOSFET(); // Cortar la alimentación después del apagado
}

void RockPiPowerController::commandStartup() {
    Logger::logInfo("Sending startup signal to Rock Pi...");

    mosfetController.switchOnMOSFET(); // Encender la alimentación
    delay(5000); // Esperar 5 segundos para que el Rock Pi se inicie
    Logger::logInfo("Startup signal completed. Rock Pi should be on.");
}

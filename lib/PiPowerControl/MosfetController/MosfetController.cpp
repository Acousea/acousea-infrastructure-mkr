#include "MosfetController.hpp"

MosfetController::MosfetController(int mosfetControlPin)
        : mosfetControlPin(mosfetControlPin) {
    pinMode(mosfetControlPin, OUTPUT);
    digitalWrite(mosfetControlPin, LOW); // Apagar inicialmente
}

void MosfetController::switchOnMOSFET() const {
    digitalWrite(mosfetControlPin, HIGH);
    Logger::logInfo("MOSFET turned on.");
}

void MosfetController::switchOffMOSFET() const {
    digitalWrite(mosfetControlPin, LOW);
    Logger::logInfo("MOSFET turned off.");
}

bool MosfetController::isMOSFETOn() const {
    return digitalRead(mosfetControlPin) == HIGH;
}

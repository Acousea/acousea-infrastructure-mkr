#ifdef ARDUINO

#include "MosfetController.hpp"
MosfetController::MosfetController(const int mosfetControlPin)
        : mosfetControlPin(mosfetControlPin) {
    pinMode(mosfetControlPin, OUTPUT);
    digitalWrite(mosfetControlPin, LOW); // Apagar inicialmente
}

void MosfetController::switchOnMOSFET() const {
    digitalWrite(mosfetControlPin, LOW); // LED turns BLUE
    Logger::logInfo(getClassNameString() + "MOSFET turned on.");
}

void MosfetController::switchOffMOSFET() const {
    digitalWrite(mosfetControlPin, HIGH); // LED turns RED
    Logger::logInfo(getClassNameString() + "MOSFET turned off.");
}

bool MosfetController::isMOSFETOn() const {
    return digitalRead(mosfetControlPin) == LOW; // LOW means MOSFET is "on" (on output-A <BLUE>, off output-B<RED>)
}

#endif // ARDUINO
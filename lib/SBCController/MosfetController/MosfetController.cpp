#ifdef ARDUINO

#include "MosfetController.hpp"

MosfetController::MosfetController(const int mosfetControlPin)
    : mosfetControlPin(mosfetControlPin)
{
    pinMode(mosfetControlPin, OUTPUT);
    digitalWrite(mosfetControlPin, LOW); // Apagar inicialmente (LED turns BLUE)
}

void MosfetController::switchOnMOSFET() const
{
    digitalWrite(mosfetControlPin, HIGH); // LED turns RED
    LOG_CLASS_INFO("MOSFET turned on.");
}

void MosfetController::switchOffMOSFET() const
{
    digitalWrite(mosfetControlPin, LOW); // LED turns BLUE
    LOG_CLASS_INFO("MOSFET turned off.");
}

bool MosfetController::isMOSFETOn() const
{
    return digitalRead(mosfetControlPin) == HIGH; // LOW means MOSFET is "on" (on output-A <BLUE>, off output-B<RED>)
}

#endif // ARDUINO

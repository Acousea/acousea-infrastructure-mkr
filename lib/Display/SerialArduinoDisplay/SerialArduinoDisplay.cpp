#ifdef ARDUINO
#include "SerialArduinoDisplay.h"

#ifndef SERIAL_USE_ANSI
#define SERIAL_USE_ANSI 1
#endif

inline void serialPrintAnsiPrefix(Stream* serial, const IDisplay::Color c) {
#if SERIAL_USE_ANSI
    serial->print(IDisplay::ColorUtils::getAnsiCode(c));
#endif
}
SerialArduinoDisplay::SerialArduinoDisplay(Stream* serial)
    : serialPort(serial) {}


void SerialArduinoDisplay::print(const uint8_t *data, size_t length) {
    // serialPrintAnsiPrefix(serialPort, activeColor);
    for (size_t i = 0; i < length; i++) {
        serialPort->print(data[i], HEX);
        serialPort->print(" ");
    }
    // serialPrintAnsiPrefix(serialPort, Color::RESET);
    serialPort->println();
    serialPort->flush();
}


void SerialArduinoDisplay::print(const char *message) {
    // serialPrintAnsiPrefix(serialPort, activeColor);
    serialPort->print(message);
    // serialPrintAnsiPrefix(serialPort, Color::RESET);
    serialPort->println();
    serialPort->flush();
}

void SerialArduinoDisplay::clear() {
    // serialPrintAnsiPrefix(serialPort, Color::RESET);
    serialPort->println("SerialGenericDisplay cleared");
    serialPort->flush();
}

#endif // ARDUINO

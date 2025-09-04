#ifdef ARDUINO
#include "SerialUSBDisplay.h"

// Activate ANSI escape codes for formatting if not already defined
#ifndef SERIALUSB_USE_ANSI
#define SERIALUSB_USE_ANSI 1
#endif

inline void serialUsbPrintAnsiPrefix(const IDisplay::Color c) {
#if SERIALUSB_USE_ANSI
    SerialUSB.print(IDisplay::ColorUtils::getAnsiCode(c));
#endif
}
inline void serialUsbPrintAnsiReset() {
#if SERIALUSB_USE_ANSI
    SerialUSB.print(IDisplay::ColorUtils::getAnsiReset());
#endif
}

void SerialUSBDisplay::init(int baudRate) {
    SerialUSB.begin(baudRate);
    while (!SerialUSB) {
        delay(10);
    }
    SerialUSB.println("SerialUSBDisplay initialized");
}

void SerialUSBDisplay::print(const uint8_t *data, size_t length) {
    serialUsbPrintAnsiPrefix(activeColor);
    for (size_t i = 0; i < length; i++) {
        SerialUSB.print(data[i], HEX);
        SerialUSB.print(" ");
    }
    serialUsbPrintAnsiReset();
    SerialUSB.println();
}

void SerialUSBDisplay::print(const std::string &message) {
    serialUsbPrintAnsiPrefix(activeColor);
    SerialUSB.println(message.c_str());
    serialUsbPrintAnsiReset();
}

void SerialUSBDisplay::print(const std::vector<uint8_t> &data) {
    serialUsbPrintAnsiPrefix(activeColor);
    for (size_t i = 0; i < data.size(); i++) {
        SerialUSB.print(data[i], HEX);
        SerialUSB.print(" ");
    }

    SerialUSB.println();
}

void SerialUSBDisplay::print(const char *message) {
    serialUsbPrintAnsiPrefix(activeColor);
    SerialUSB.println(message);
    serialUsbPrintAnsiReset();
}

void SerialUSBDisplay::clear() {
    serialUsbPrintAnsiReset();
    SerialUSB.println("SerialUSBDisplay cleared");
}

#endif // ARDUINO
#include "SerialUSBDisplay.h"

void SerialUSBDisplay::init(int baudRate) {
    SerialUSB.begin(baudRate);
    while (!SerialUSB) {
        delay(10);
    }
    SerialUSB.println("SerialUSBDisplay initialized");
}

void SerialUSBDisplay::print(const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        SerialUSB.print(data[i], HEX);
        SerialUSB.print(" ");
    }
    SerialUSB.println();
}

void SerialUSBDisplay::print(const std::string &message) {
    SerialUSB.println(message.c_str());
}

void SerialUSBDisplay::print(const std::vector<uint8_t> &data) {
    for (size_t i = 0; i < data.size(); i++) {
        SerialUSB.print(data[i], HEX);
        SerialUSB.print(" ");
    }
    SerialUSB.println();
}

void SerialUSBDisplay::print(const String &message) {
    SerialUSB.println(message);
}

void SerialUSBDisplay::print(const char *message) {
    SerialUSB.println(message);
}

void SerialUSBDisplay::clear() {
    SerialUSB.println("SerialUSBDisplay cleared");
}

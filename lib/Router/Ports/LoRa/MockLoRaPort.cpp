#include "MockLoRaPort.h"

MockLoRaPort::MockLoRaPort() : IPort(PortType::LoraPort) {}

void MockLoRaPort::init() {
    // Print through serial monitor
    SerialUSB.println("MockLoRaPort: Initializing LoRa port");
}

void MockLoRaPort::send(const std::vector<uint8_t> &data) {
    // Print packet through serial monitor for debugging
    SerialUSB.print("MockLoRaPort: Sending packet: ");
    SerialUSB.println("Data: ");
    for (auto &byte : data) {
        SerialUSB.print(byte, HEX);
        SerialUSB.print(" ");
    }

}

bool MockLoRaPort::available() {
    return false;
}

std::vector<std::vector<uint8_t>> MockLoRaPort::read() {
    return std::vector<std::vector<uint8_t>>();


}

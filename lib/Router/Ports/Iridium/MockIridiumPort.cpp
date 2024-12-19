#include "MockIridiumPort.h"

MockIridiumPort::MockIridiumPort() : IPort(PortType::SBDPort) {}

void MockIridiumPort::init() {
    SerialUSB.println("MockIridiumPort: Initializing Iridium port");
}

void MockIridiumPort::send(const std::vector<uint8_t> &data) {
    SerialUSB.print("MockIridiumPort: Sending packet: ");
    SerialUSB.println("Data: ");
    for (auto &byte : data) {
        SerialUSB.print(byte, HEX);
        SerialUSB.print(" ");
    }
}

bool MockIridiumPort::available() {
    return false;
}

std::vector<std::vector<uint8_t>> MockIridiumPort::read() {
    return std::vector<std::vector<uint8_t>>();
}

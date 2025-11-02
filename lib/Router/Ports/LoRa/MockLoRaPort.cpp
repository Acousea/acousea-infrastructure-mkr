#include "MockLoRaPort.h"

#include <Logger/Logger.h>

MockLoRaPort::MockLoRaPort() : IPort(PortType::LoraPort) {}

void MockLoRaPort::init() {
    // Print through serial monitor
    Logger::logInfo("MockLoRaPort: Initializing LoRa port...");
}

bool MockLoRaPort::send(const std::vector<uint8_t>& data) {
    // Print packet through serial monitor for debugging
    Logger::logInfo("MockLoRaPort: Sending packet... " + Logger::vectorToHexString(data));
    return true;
}

bool MockLoRaPort::available() {
    return false;
}

std::vector<std::vector<uint8_t>> MockLoRaPort::read() {
    return std::vector<std::vector<uint8_t>>();


}

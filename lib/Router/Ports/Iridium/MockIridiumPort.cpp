#include "MockIridiumPort.h"

#include <Logger/Logger.h>

MockIridiumPort::MockIridiumPort() : IPort(PortType::SBDPort) {
}

void MockIridiumPort::init() {
    Logger::logInfo("MockIridiumPort: Iridium modem initialized");
}

bool MockIridiumPort::send(const std::vector<uint8_t>& data) {
    Logger::logInfo("MockIridiumPort: Sending packet... " + Logger::vectorToHexString(data));
    return true;
}

bool MockIridiumPort::available() {
    return false;
}

std::vector<std::vector<uint8_t> > MockIridiumPort::read() {
    return std::vector<std::vector<uint8_t> >();
}

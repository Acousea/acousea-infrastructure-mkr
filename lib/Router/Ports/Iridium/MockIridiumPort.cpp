#include "MockIridiumPort.h"

#include <Logger/Logger.h>

MockIridiumPort::MockIridiumPort() : IPort(PortType::SBDPort)
{
}

void MockIridiumPort::init()
{
    LOG_CLASS_INFO("MockIridiumPort: Iridium modem initialized");
}

bool MockIridiumPort::send(const std::vector<uint8_t>& data)
{
    LOG_CLASS_INFO("MockIridiumPort: Sending packet... %s",
                   Logger::vectorToHexString(data.data(), data.size()).c_str());
    return true;
}

bool MockIridiumPort::available()
{
    return false;
}

std::vector<std::vector<uint8_t>> MockIridiumPort::read()
{
    return std::vector<std::vector<uint8_t>>();
}

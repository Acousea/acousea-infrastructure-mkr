#include "MockIridiumPort.h"

#include <Logger/Logger.h>

MockIridiumPort::MockIridiumPort() : IPort(PortType::SBDPort)
{
}

void MockIridiumPort::init()
{
    LOG_CLASS_INFO("MockIridiumPort: Iridium modem initialized");
}

bool MockIridiumPort::send(const uint8_t* data, const size_t length)
{
    LOG_CLASS_INFO("MockIridiumPort: Sending packet... %s",
                   Logger::vectorToHexString(data, length).c_str());
    return true;
}

bool MockIridiumPort::available()
{
    return false;
}

uint16_t MockIridiumPort::readInto(uint8_t* buffer, const uint16_t maxSize)
{
    return 0;
}

bool MockIridiumPort::sync()
{
    return true;
}

#include "MockLoRaPort.h"

#include <Logger/Logger.h>

MockLoRaPort::MockLoRaPort() : IPort(PortType::LoraPort)
{
}

void MockLoRaPort::init()
{
    // Print through serial monitor
    LOG_CLASS_INFO("MockLoRaPort: Initializing LoRa port...");
}

bool MockLoRaPort::send(const std::vector<uint8_t>& data)
{
    // Print packet through serial monitor for debugging
    LOG_CLASS_INFO("MockLoRaPort: Sending packet... %s", Logger::vectorToHexString(data.data(), data.size()).c_str());
    return true;
}

bool MockLoRaPort::available()
{
    return false;
}

std::vector<std::vector<uint8_t>> MockLoRaPort::read()
{
    return std::vector<std::vector<uint8_t>>();
}

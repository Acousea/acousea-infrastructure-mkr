#ifdef PLATFORM_HAS_LORA

#include "MockLoRaPort.h"

#include <Logger/Logger.h>

MockLoRaPort::MockLoRaPort(FlashPacketQueue& packetQueue) : IPort(PortType::LoraPort, packetQueue)
{
}

void MockLoRaPort::init()
{
    // Print through serial monitor
    LOG_CLASS_INFO("MockLoRaPort: Initializing LoRa port...");
}

bool MockLoRaPort::send(const uint8_t* data, size_t length)
{
    // Print packet through serial monitor for debugging
    LOG_CLASS_INFO("MockLoRaPort: Sending packet... %s", Logger::vectorToHexString(data, length).c_str());
    return true;
}

bool MockLoRaPort::available()
{
    return false;
}


uint16_t MockLoRaPort::readInto(uint8_t* buffer, uint16_t maxSize)
{
    return 0;
}

bool MockLoRaPort::sync()
{
    return true;
}


#endif // PLATFORM_HAS_LORA || UNIT_TESTING

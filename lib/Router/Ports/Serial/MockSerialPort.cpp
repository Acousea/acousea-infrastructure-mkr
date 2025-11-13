#include "MockSerialPort.h"

MockSerialPort::MockSerialPort()
    : IPort(PortType::SerialPort)
{
}

void MockSerialPort::init()
{
    LOG_CLASS_INFO("MOCKSerialPort::init() -> Serial port initialized");
}


bool MockSerialPort::available()
{
    return false;
}

bool MockSerialPort::send(const uint8_t* data, const size_t length)
{
    LOG_CLASS_INFO("MOCKSerialPort::send() -> %s", Logger::vectorToHexString(data, length).c_str());
    return true;
}

uint16_t MockSerialPort::readInto(uint8_t* buffer, uint16_t maxSize)
{
    return 0;
}

bool MockSerialPort::sync()
{
    return true;
}

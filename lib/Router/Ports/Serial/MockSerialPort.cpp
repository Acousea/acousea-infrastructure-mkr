#include "MockSerialPort.h"
#include "Logger/Logger.h"

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


bool MockSerialPort::sync()
{
    return true;
}

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

bool MockSerialPort::send(const std::vector<uint8_t>& data)
{
    LOG_CLASS_INFO("MOCKSerialPort::send() -> %s", Logger::vectorToHexString(data.data(), data.size()).c_str());
    return true;
}

std::vector<std::vector<uint8_t>> MockSerialPort::read()
{
    return std::vector<std::vector<uint8_t>>();
}

#include "MockSerialPort.h"

MockSerialPort::MockSerialPort()
    : IPort(PortType::SerialPort){
}

void MockSerialPort::init(){
    Logger::logInfo("MOCKSerialPort::init() -> Serial port initialized");
}


bool MockSerialPort::available(){
    return false;
}

void MockSerialPort::send(const std::vector<uint8_t>& data){
    Logger::logInfo("MOCKSerialPort::send() -> " + Logger::vectorToHexString(data));
}

std::vector<std::vector<uint8_t>> MockSerialPort::read(){
    return std::vector<std::vector<uint8_t>>();
}

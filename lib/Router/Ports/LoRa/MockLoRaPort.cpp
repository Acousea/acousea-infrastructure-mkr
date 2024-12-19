#include "MockLoRaPort.h"

MockLoRaPort::MockLoRaPort() : IPort(PortType::LoraPort) {}

void MockLoRaPort::init() {
    // Print through serial monitor
    SerialUSB.println("MockLoRaPort: Initializing LoRa port");
}

void MockLoRaPort::send(const Packet &packet) {
    // Print packet through serial monitor for debugging
    SerialUSB.print("MockLoRaPort: Sending packet: ");
    SerialUSB.println("Packet: " + String(packet.encode().c_str()));
}

bool MockLoRaPort::available() {
    return false;
}

Result<Packet> MockLoRaPort::read() {
    uint8_t mock_data[4] = {0x20, 0x01, 0x02, 0x03};
    std::vector<uint8_t> buffer(mock_data, mock_data + sizeof(mock_data) / sizeof(mock_data[0]));
    Packet packet = Packet::fromBytes(buffer);
    return Result<Packet>::success(packet);
}

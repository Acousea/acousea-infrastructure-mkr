#include "MockIridiumPort.h"

MockIridiumPort::MockIridiumPort() : IPort(PortType::SBDPort) {}

void MockIridiumPort::init() {
    SerialUSB.println("MockIridiumPort: Initializing Iridium port");
}

void MockIridiumPort::send(const Packet &packet) {
    SerialUSB.print("MockIridiumPort: Sending packet: ");
    SerialUSB.println("Packet: " + String(packet.encode().c_str()));
}

bool MockIridiumPort::available() {
    return false;
}

Result<Packet> MockIridiumPort::read() {
    uint8_t mock_data[4] = {0x20, 0x01, 0x02, 0x03};
    std::vector<uint8_t> buffer(mock_data, mock_data + sizeof(mock_data) / sizeof(mock_data[0]));
    Packet packet = Packet::fromBytes(buffer);

    return Result<Packet>::success(packet);
}

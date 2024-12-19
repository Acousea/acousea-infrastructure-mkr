#include "SerialPort.h"

SerialPort::SerialPort(Uart *serialPort, int baudRate)
        : IPort(PortType::SerialPort), serialPort(serialPort), baudRate(baudRate) {}

void SerialPort::init() {
    serialPort->begin(baudRate);
    SerialUSB.println("SerialPort::init() -> Serial port initialized");
}

void SerialPort::send(const Packet &packet) {
    SerialUSB.println("SerialPort::send() -> Sending packet...");
    SerialUSB.println("Packet: " + String(packet.encode().c_str()));
    auto packetBytes = packet.toBytes();
    serialPort->write(packetBytes.data(), packetBytes.size());
}

bool SerialPort::available() {
    return serialPort->available() >= Packet::SYNC_BYTE;
}

Result<Packet> SerialPort::read() {
    std::vector<uint8_t> buffer;
    buffer.reserve(MAX_PACKET_BUFFER);

    while (serialPort->available()) {
        uint8_t byte = serialPort->read();
        buffer.push_back(byte);

        // Comprobamos si el buffer contiene al menos los bytes mínimos para un paquete
        if (buffer.size() < 7) { // SYNC_BYTE + OPCODE + RoutingChunk + PayloadLength + CRC
            continue;
        }
        Packet packet = Packet::fromBytes(buffer);
        SerialUSB.println("SerialPort::read() -> Packet received successfully");
        return Result<Packet>::success(packet);
    }

    SerialUSB.println("SerialPort::read() -> No complete packet received");
    return Result<Packet>::failure("Incomplete packet data");
}

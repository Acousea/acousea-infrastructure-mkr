#include "SerialPort.h"

SerialPort::SerialPort(Uart *serialPort, int baudRate)
        : IPort(PortType::SerialPort), serialPort(serialPort), baudRate(baudRate) {}

void SerialPort::init() {
    serialPort->begin(baudRate);
    SerialUSB.println("SerialPort::init() -> Serial port initialized");
}

void SerialPort::send(const std::vector<uint8_t> &data) {
    SerialUSB.println("SerialPort::send() -> Sending packet...");
    SerialUSB.print("Data: ");
    for (const auto &byte: data) {
        SerialUSB.print(byte, HEX);
        SerialUSB.print(" ");
    }
    serialPort->write(data.data(), data.size());
}

bool SerialPort::available() {
    return serialPort->available() > 0;
}

std::vector<std::vector<uint8_t>> SerialPort::read() {
    std::vector<uint8_t> buffer;
    buffer.reserve(MAX_RECEIVED_PACKET_SIZE);

    while (serialPort->available()) {
        uint8_t byte = serialPort->read();
        buffer.push_back(byte);

        // Comprobamos si el buffer contiene al menos los bytes mínimos para un paquete
        if (buffer.size() < 7) { // SYNC_BYTE + OPCODE + RoutingChunk + PayloadLength + CRC
            continue;
        }

        SerialUSB.println("SerialPort::read() -> Packet received successfully");

    }
    if (buffer.size() == 0) {
        return {};
    }
    return {buffer};

}

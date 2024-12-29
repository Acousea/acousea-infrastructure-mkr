#include "SerialPort.h"

#include <Packet.h>

SerialPort::SerialPort(Uart *serialPort, int baudRate)
    : IPort(PortType::SerialPort), serialPort(serialPort), baudRate(baudRate) {
}

void SerialPort::init() {
    serialPort->begin(baudRate);
    serialPort->setTimeout(10000); // Set a timeout of 5000ms
    serialPort->flush();

    Logger::logInfo("SerialPort::init() -> Serial port initialized");
}

void SerialPort::send(const std::vector<uint8_t> &data) {
    Logger::logInfo("SerialPort::send() -> Sending packet... " + Logger::vectorToHexString(data));
    serialPort->write(data.data(), data.size());
}

bool SerialPort::available() {
    return serialPort->available() > 0;
}

std::vector<std::vector<uint8_t> > SerialPort::read() {
    constexpr size_t HEADER_SIZE = 5; // SYNC_BYTE (1) + OP_CODE (1) + ROUTING_CHUNK (3)
    constexpr size_t CHECKSUM_SIZE = 2; // CRC (2 bytes)
    constexpr size_t MIN_PACKET_SIZE = HEADER_SIZE + 1 + CHECKSUM_SIZE; // Header + PayloadLength + CRC

    std::vector<std::vector<uint8_t> > packets;

    while (serialPort->available()) {
        // Step 1: Read the header
        std::vector<uint8_t> header(HEADER_SIZE);
        size_t bytesRead = serialPort->readBytes(header.data(), HEADER_SIZE);

        if (bytesRead != HEADER_SIZE || header[0] != Packet::SYNC_BYTE) {
            Logger::logError("SerialPort::read(): Invalid header or sync byte");
            continue; // Skip invalid header
        }

        // Step 2: Read payload length
        uint8_t payloadLength = 0;
        bytesRead = serialPort->readBytes(&payloadLength, sizeof(payloadLength));

        if (bytesRead != sizeof(payloadLength)) {
            Logger::logError("SerialPort::read(): Failed to read payload length");
            continue; // Skip incomplete payload length
        }

        // Step 3: Read the payload
        std::vector<uint8_t> payload(payloadLength);
        bytesRead = serialPort->readBytes(payload.data(), payloadLength);

        if (bytesRead != payloadLength) {
            Logger::logError("SerialPort::read(): Failed to read the entire payload -> "
                             "Bytes read: " + std::to_string(bytesRead) +
                             ", Expected: " + std::to_string(payloadLength));
            continue; // Skip incomplete payload
        }

        // Step 4: Read the checksum
        uint8_t checksumBytes[CHECKSUM_SIZE];
        bytesRead = serialPort->readBytes(checksumBytes, CHECKSUM_SIZE);
        if (bytesRead != CHECKSUM_SIZE) {
            Logger::logError(
                "SerialPort::read(): Failed to read the checksum -> Bytes read: " + std::to_string(bytesRead));
            continue;
        }

        const uint16_t checksum = (static_cast<uint16_t>(checksumBytes[0]) << 8) | checksumBytes[1];


        // Step 5: Assemble the complete packet
        std::vector<uint8_t> packet;
        packet.reserve(MIN_PACKET_SIZE + payloadLength);
        packet.insert(packet.end(), header.begin(), header.end());
        packet.push_back(payloadLength);
        packet.insert(packet.end(), payload.begin(), payload.end());
        packet.push_back(static_cast<uint8_t>(checksum >> 8));
        packet.push_back(static_cast<uint8_t>(checksum & 0xFF));

        // Add the complete packet to the list
        packets.push_back(packet);
    }

    return packets;
}

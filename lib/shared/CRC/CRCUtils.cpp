#include "CRCUtils.h"

#include <Logger/Logger.h>

uint16_t CRCUtils::calculate16BitCRC(const std::vector<uint8_t> &data) {
    uint16_t crc = INITIAL_VALUE;
    for (uint8_t byte: data) {
        crc ^= (static_cast<uint16_t>(byte) << 8);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ POLYNOMIAL;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc & 0xFFFF; // Asegura que el CRC esté en un rango de 16 bits
}

bool CRCUtils::verifyCRC(const std::vector<uint8_t> &buffer) {
    if (buffer.size() < 2) {
        return false;
    }

    Logger::logInfo("Verifying CRC for buffer: " + Logger::vectorToHexString(buffer));

    // Extraer el CRC del paquete (últimos dos bytes)
    const uint16_t receivedCRC = (static_cast<uint16_t>(buffer[buffer.size() - 2]) << 8) |
                                 static_cast<uint16_t>(buffer[buffer.size() - 1]);

    // Calcular el CRC del paquete excluyendo los últimos dos bytes
    const std::vector<uint8_t> data(buffer.begin(), buffer.end() - 2);
    const uint16_t calculatedCRC = calculate16BitCRC(data);

    Logger::logInfo("CRCUtils::verifyCRC() -> "
                    "Received CRC: " + std::to_string(receivedCRC) +
                    ", Calculated CRC: " + std::to_string(calculatedCRC)
    );

    return receivedCRC == calculatedCRC;
}

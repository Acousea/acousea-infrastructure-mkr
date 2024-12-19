#include "CRCUtils.h"

uint16_t CRCUtils::calculate16BitCRC(const std::vector<uint8_t> &data) {
    uint16_t crc = INITIAL_VALUE;
    for (uint8_t byte : data) {
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
        ErrorHandler::handleError("Data length must be at least 2 bytes for CRC verification.");
    }

    // Extraer el CRC del paquete (últimos dos bytes)
    uint16_t receivedCRC = (static_cast<uint16_t>(buffer[buffer.size() - 2]) << 8) |
                           static_cast<uint16_t>(buffer[buffer.size() - 1]);

    // Calcular el CRC del paquete excluyendo los últimos dos bytes
    std::vector<uint8_t> data(buffer.begin(), buffer.end() - 2);
    uint16_t calculatedCRC = calculate16BitCRC(data);

    return receivedCRC == calculatedCRC;
}

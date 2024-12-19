#ifndef ACOUSEA_MKR1310_NODES_CRCUTILS_H
#define ACOUSEA_MKR1310_NODES_CRCUTILS_H

#include <vector>
#include <cstdint>

class CRCUtils {
public:
    // Polinomio CRC-16-CCITT estándar
    static constexpr uint16_t POLYNOMIAL = 0x1021;
    static constexpr uint16_t INITIAL_VALUE = 0xFFFF;

    // Calcula el CRC-16 para un vector de datos
    static uint16_t calculate16BitCRC(const std::vector<uint8_t>& data);

    // Verifica el CRC de un paquete
    static bool verifyCRC(const std::vector<uint8_t>& buffer);
};

#endif //ACOUSEA_MKR1310_NODES_CRCUTILS_H

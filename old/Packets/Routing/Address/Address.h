#ifndef ACOUSEA_MKR1310_NODES_ADDRESS_H
#define ACOUSEA_MKR1310_NODES_ADDRESS_H


#include <cstdint>

#include "ErrorHandler/ErrorHandler.h"

class Address {
public:
    static const uint8_t BACKEND = 0x00; // 0b00000000
    static const uint8_t BROADCAST_ADDRESS = 0xFF; // 0b11111111
    explicit Address(uint8_t value);

    static size_t getSize();

    static Address fromValue(int value);

    static bool isReserved(int value);

    static bool isValidRange(int value);

    static Address backend();

    static Address broadcastAddress();

    bool operator==(const Address& other) const;

    bool operator!=(const Address& other) const;

    uint8_t getValue() const;

private:
    uint8_t value;

    static bool isValidUnsignedByte(int value);
};



#endif //ACOUSEA_MKR1310_NODES_ADDRESS_H

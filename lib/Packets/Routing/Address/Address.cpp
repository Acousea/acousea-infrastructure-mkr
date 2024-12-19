#include "Address.h"

Address::Address(uint8_t value) : value(value) {
    if (!isValidUnsignedByte(value)) {
        ErrorHandler::handleError("Address value is out of the valid range.");
    }
}

size_t Address::getSize() {
    return sizeof(uint8_t);
}

Address Address::fromValue(int value) {
    if (!isValidUnsignedByte(value)) {
        ErrorHandler::handleError("Address value is out of the valid range.");
    }
    return Address(static_cast<uint8_t>(value));
}

bool Address::isReserved(int value) {
    return value == 0x00 || value == 0xFF;
}

bool Address::isValidRange(int value) {
    // Example: Allowed range between 0x01 and 0xFE (excluding reserved)
    return value >= 0x01 && value <= 0xFE;
}

Address Address::backend() {
    return Address(BACKEND);
}

Address Address::broadcastAddress() {
    return Address(BROADCAST_ADDRESS);
}

std::string Address::toString() const {
    std::ostringstream oss;
    oss << "Address{value=0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value) << "}";
    return oss.str();
}

bool Address::operator==(const Address &other) const {
    return value == other.value;
}

bool Address::operator!=(const Address &other) const {
    return !(*this == other);
}

uint8_t Address::getValue() const {
    return value;
}

bool Address::isValidUnsignedByte(int value) {
    return value >= 0x00 && value <= 0xFF;
}

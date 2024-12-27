#include "SerializableModule.h"

SerializableModule& SerializableModule::operator=(SerializableModule&& other) noexcept {
    if (this != &other) {
        // Reasignar miembros `const` usando `const_cast`
        const_cast<uint8_t&>(TYPE) = std::move(other.TYPE);
        const_cast<std::vector<uint8_t>&>(VALUE) = std::move(other.VALUE);
    }
    return *this;
}

SerializableModule::SerializableModule(const SerializableModule &other) noexcept
    : TYPE(other.TYPE), VALUE(other.VALUE) {
}

SerializableModule& SerializableModule::operator=(const SerializableModule& other) noexcept {
    if (this != &other) {
        const_cast<uint8_t&>(TYPE) = other.TYPE;
        const_cast<std::vector<uint8_t>&>(VALUE) = other.VALUE;
    }
    return *this;
}

std::vector<uint8_t> SerializableModule::toBytes() const {
    std::vector<uint8_t> bytes;
    bytes.reserve(VALUE.size() + 2);
    bytes.push_back(TYPE);
    bytes.push_back(static_cast<uint8_t>(VALUE.size()));
    bytes.insert(bytes.end(), VALUE.begin(), VALUE.end());
    return bytes;
}

int SerializableModule::getFullLength() const {
    return static_cast<int>(VALUE.size()) + 2;
}

std::string SerializableModule::encode() const {
    std::string encodedString;

    // Convert and append the size (VALUE.size() + 1) to the string
    char sizeHex[5]; // Enough for 4 hex digits + null terminator
    snprintf(sizeHex, sizeof(sizeHex), "%x", static_cast<int>(VALUE.size() + 1));
    encodedString += sizeHex;

    // Convert and append TYPE to the string
    char typeHex[3]; // Enough for 2 hex digits + null terminator
    snprintf(typeHex, sizeof(typeHex), "%02x", static_cast<int>(TYPE));
    encodedString += typeHex;

    // Convert and append each byte in VALUE to the string
    for (const auto& b : VALUE) {
        char byteHex[3]; // Enough for 2 hex digits + null terminator
        snprintf(byteHex, sizeof(byteHex), "%02x", static_cast<unsigned char>(b));
        encodedString += byteHex;
    }

    return encodedString;
}
uint8_t SerializableModule::getType() const {
    return TYPE;
}

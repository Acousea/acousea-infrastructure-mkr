#include "SerializableModule.h"

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
    std::ostringstream ss;
    ss << std::hex << (VALUE.size() + 1);
    ss << std::hex << static_cast<int>(TYPE);
    for (const auto& b : VALUE) {
        ss << std::hex << static_cast<int>(b);
    }
    return ss.str();
}

uint8_t SerializableModule::getType() const {
    return TYPE;
}

#include "GetUpdatedNodeConfigurationPayload.h"

GetUpdatedNodeConfigurationPayload::GetUpdatedNodeConfigurationPayload(const std::vector<ModuleCode> &moduleCodes) : moduleCodes(
        moduleCodes) {}

uint16_t GetUpdatedNodeConfigurationPayload::getBytesSize() const {
    const auto size = static_cast<uint16_t>(moduleCodes.size() * sizeof(ModuleCode::TYPES));
    return size;
}

std::vector<uint8_t> GetUpdatedNodeConfigurationPayload::toBytes() const {
    std::vector<uint8_t> buffer;
    for (const auto &moduleCode: moduleCodes) {
        buffer.push_back(static_cast<uint8_t>(moduleCode.getValue()));
    }
    return buffer;
}

GetUpdatedNodeConfigurationPayload GetUpdatedNodeConfigurationPayload::fromBytes(const std::vector<uint8_t> &data) {
    std::vector<ModuleCode> moduleCodes;
    for (const auto &byte: data) {
        moduleCodes.push_back(ModuleCode::fromValue(byte));
    }
    return GetUpdatedNodeConfigurationPayload(moduleCodes);
}


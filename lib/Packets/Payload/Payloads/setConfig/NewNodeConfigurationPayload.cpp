#include "NewNodeConfigurationPayload.h"

NewNodeConfigurationPayload::NewNodeConfigurationPayload(const std::vector<SerializableModule> &tags) : modules(tags) {}

uint16_t NewNodeConfigurationPayload::getBytesSize() const {
    uint16_t size = 0;
    for (const auto &tag: modules) {
        size += tag.getFullLength();
    }
    return size;
}

std::vector<uint8_t> NewNodeConfigurationPayload::toBytes() const {
    std::vector<uint8_t> buffer;
    for (const auto &tag: modules) {
        const auto &tagBytes = tag.toBytes();
        buffer.insert(buffer.end(), tagBytes.begin(), tagBytes.end());
    }
    return buffer;
}

NewNodeConfigurationPayload NewNodeConfigurationPayload::fromBytes(const std::vector<uint8_t> &data) {
    std::vector<SerializableModule> modules = ModuleFactory::createModules(data);
    return NewNodeConfigurationPayload(modules);
}

const std::vector<SerializableModule> &NewNodeConfigurationPayload::getModules() const {
    return modules;
}

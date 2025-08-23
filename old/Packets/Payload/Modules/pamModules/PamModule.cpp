#include "PamModule.h"

PamModule::PamModule(ModuleCode::TYPES type, const std::vector<uint8_t> &value, const std::string &serialNumber,
                     const std::string &name) : SerializableModule(type, value), serialNumber(serialNumber), name(name) {}

const std::string &PamModule::getSerialNumber() const {
    return serialNumber;
}

const std::string &PamModule::getName() const {
    return name;
}

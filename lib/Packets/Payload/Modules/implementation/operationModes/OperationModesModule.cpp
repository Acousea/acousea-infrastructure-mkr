#include "OperationModesModule.h"

OperationModesModule OperationModesModule::from(const std::map<uint8_t, OperationMode> &operationModes) {
    return OperationModesModule(operationModes, 0);
}

OperationModesModule
OperationModesModule::from(const std::map<uint8_t, OperationMode> &operationModes, uint8_t activeModeIdx) {
    return OperationModesModule(operationModes, activeModeIdx);
}

OperationModesModule OperationModesModule::from(const std::vector<uint8_t> &value) {
    return OperationModesModule(value);
}

OperationModesModule OperationModesModule::fromJSON(const JsonArrayConst &doc) {
    std::map<uint8_t, OperationMode> operationModes;
    uint8_t activeModeIdx = 0;
    for (const auto &item: doc) {
        uint8_t id = item["id"];
        std::string name = item["name"];
        operationModes.emplace(id, OperationMode::create(id, name));
    }
    return OperationModesModule(operationModes, activeModeIdx);
}

JsonDocument OperationModesModule::toJson() const {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    for (const auto &mode: operationModes) {
        JsonObject obj = array.add<JsonObject>();
        OperationMode opMode = mode.second;
        obj["id"] = opMode.id;
        obj["name"] = opMode.name;
    }

    return doc;
}

uint8_t OperationModesModule::getActiveModeIdx() const {
    return activeModeIdx;
}

const std::map<uint8_t, OperationMode> &OperationModesModule::getOperationModes() const {
    return operationModes;
}

OperationModesModule::OperationModesModule(const std::vector<uint8_t> &value)
        : SerializableModule(ModuleCode::TYPES::OPERATION_MODES, value) {
    for (size_t i = 0; i < value.size() - 1; ++i) {
        uint8_t idx = value[i];
        operationModes.emplace(idx, OperationMode::create(idx, "currentOperationMode" + std::to_string(i)));
    }
    activeModeIdx = value[value.size() - 1];
}

OperationModesModule::OperationModesModule(const std::map<uint8_t, OperationMode> &operationModes,
                                           uint8_t activeModeIdx)
        : SerializableModule(ModuleCode::TYPES::OPERATION_MODES, serializeValues(operationModes, activeModeIdx)),
          operationModes(operationModes) {}

std::vector<uint8_t>
OperationModesModule::serializeValues(const std::map<uint8_t, OperationMode> &operationModes, uint8_t activeModeIdx) {
    std::vector<uint8_t> value;
    for (const auto &mode: operationModes) {
        value.push_back(mode.second.id);
    }
    value.push_back(activeModeIdx);
    return value;
}

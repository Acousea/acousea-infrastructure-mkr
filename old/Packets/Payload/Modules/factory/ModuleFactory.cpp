#include "ModuleFactory.h"



const std::map<ModuleCode::TYPES, ModuleFactory::ModuleCreator> ModuleFactory::moduleCreators = {
    {ModuleCode::TYPES::BATTERY, [](const std::vector<uint8_t>& data) {
        return std::make_unique<BatteryModule>(BatteryModule::from(data));
    }},
    {ModuleCode::TYPES::LOCATION, [](const std::vector<uint8_t>& data) {
        return std::make_unique<LocationModule>(LocationModule::from(data));
    }},
    {ModuleCode::TYPES::NETWORK, [](const std::vector<uint8_t>& data) {
        return std::make_unique<NetworkModule>(NetworkModule::from(data));
    }},
    {ModuleCode::TYPES::OPERATION_MODES, [](const std::vector<uint8_t>& data) {
        return std::make_unique<OperationModesModule>(OperationModesModule::from(data));
    }},
    {ModuleCode::TYPES::REAL_TIME_CLOCK, [](const std::vector<uint8_t>& data) {
        return std::make_unique<RTCModule>(RTCModule::from(data));
    }},
    {ModuleCode::TYPES::REPORTING, [](const std::vector<uint8_t>& data) {
        return std::make_unique<ReportingModule>(ReportingModule::from(data));
    }},
    {ModuleCode::TYPES::STORAGE, [](const std::vector<uint8_t>& data) {
        return std::make_unique<StorageModule>(StorageModule::from(data));
    }},
    {ModuleCode::TYPES::AMBIENT, [](const std::vector<uint8_t>& data) {
        return std::make_unique<AmbientModule>(AmbientModule::from(data));
    }},
    {ModuleCode::TYPES::ICLISTEN_STATUS, [](const std::vector<uint8_t>& data) {
        return std::make_unique<ICListenStatus>(ICListenStatus::fromBytes(data));
    }},
    {ModuleCode::TYPES::ICLISTEN_LOGGING_CONFIG, [](const std::vector<uint8_t>& data) {
        return std::make_unique<ICListenLoggingConfig>(ICListenLoggingConfig::fromBytes(data));
    }},
    {ModuleCode::TYPES::ICLISTEN_STREAMING_CONFIG, [](const std::vector<uint8_t>& data) {
        return std::make_unique<ICListenStreamingConfig>(ICListenStreamingConfig::fromBytes(data));
    }},
    {ModuleCode::TYPES::ICLISTEN_RECORDING_STATS, [](const std::vector<uint8_t>& data) {
        return std::make_unique<ICListenRecordingStats>(ICListenRecordingStats::fromBytes(data));
    }},
    {ModuleCode::TYPES::ICLISTEN_COMPLETE, [](const std::vector<uint8_t>& data) {
        return std::make_unique<ICListenHF>(ICListenHF::fromBytes(data));
    }}
};

std::unique_ptr<SerializableModule> ModuleFactory::createModule(const std::vector<uint8_t>& buffer) {
    if (buffer.empty()) {
        ErrorHandler::handleError("ModuleFactory: Buffer is empty.");
    }

    uint8_t typeCode = buffer[0];
    ModuleCode::TYPES moduleCode = ModuleCode::enumFromValue(typeCode);

    auto it = moduleCreators.find(moduleCode);
    if (it == moduleCreators.end()) {
        ErrorHandler::handleError("ModuleFactory: Unsupported module type: " + std::to_string(typeCode));
    }

    // Extract module data (excluding the first byte for type and the second byte for size)
    std::vector<uint8_t> moduleData(buffer.begin() + 2, buffer.end());
    return it->second(moduleData);
}

std::vector<std::unique_ptr<SerializableModule>> ModuleFactory::createModules(const std::vector<uint8_t>& buffer) {
    std::vector<std::unique_ptr<SerializableModule>> modules;
    size_t offset = 0;

    while (offset < buffer.size()) {
        if (offset + 1 > buffer.size()) {
            ErrorHandler::handleError("ModuleFactory: Invalid buffer size.");
        }

        uint8_t moduleSize = buffer[offset + 1];
        if (offset + moduleSize + 2 > buffer.size()) {
            ErrorHandler::handleError("ModuleFactory: Module size exceeds buffer length.");
        }

        // Extract the module from the buffer
        std::vector<uint8_t> moduleBuffer(buffer.begin() + offset, buffer.begin() + offset + moduleSize + 2);
        modules.push_back(createModule(moduleBuffer));
        offset += moduleSize + 2;
    }

    return modules;
}

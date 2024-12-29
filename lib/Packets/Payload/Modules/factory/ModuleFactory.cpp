#include "ModuleFactory.h"

#include "Payload/Modules/pamModules/ICListen/ICListenHF.h"
#include "Payload/Modules/pamModules/ICListen/implementation/logging/ICListenLoggingConfig.h"
#include "Payload/Modules/pamModules/ICListen/implementation/stats/ICListenRecordingStats.h"
#include "Payload/Modules/pamModules/ICListen/implementation/status/ICListenStatus.h"
#include "Payload/Modules/pamModules/ICListen/implementation/streaming/ICListenStreamingConfig.h"


// Define and initialize the map of module creators
const std::map<ModuleCode::TYPES, ModuleFactory::ModuleCreator> ModuleFactory::moduleCreators = {
        {ModuleCode::TYPES::BATTERY,         [](const std::vector<uint8_t> &data) {
            return BatteryModule::from(data);
        }},
        {ModuleCode::TYPES::LOCATION,        [](const std::vector<uint8_t> &data) {
            return LocationModule::from(data);
        }},
        {ModuleCode::TYPES::NETWORK,         [](const std::vector<uint8_t> &data) {
            return NetworkModule::from(data);
        }},
        {ModuleCode::TYPES::OPERATION_MODES, [](const std::vector<uint8_t> &data) {
            return OperationModesModule::from(data);
        }},
        {ModuleCode::TYPES::REAL_TIME_CLOCK, [](const std::vector<uint8_t> &data) {
            return RTCModule::from(data);
        }},
        {ModuleCode::TYPES::REPORTING,       [](const std::vector<uint8_t> &data) {
            return ReportingModule::from(data);
        }},
        {ModuleCode::TYPES::STORAGE,         [](const std::vector<uint8_t> &data) {
            return StorageModule::from(data);
        }},
        {ModuleCode::TYPES::AMBIENT,         [](const std::vector<uint8_t> &data) {
            return AmbientModule::from(data);
        }},
        {ModuleCode::TYPES::ICLISTEN_STATUS,         [](const std::vector<uint8_t> &data) {
            return ICListenStatus::fromBytes(data);
        }},
        {ModuleCode::TYPES::ICLISTEN_LOGGING_CONFIG,         [](const std::vector<uint8_t> &data) {
            return ICListenLoggingConfig::fromBytes(data);
        }},
        {ModuleCode::TYPES::ICLISTEN_STREAMING_CONFIG,         [](const std::vector<uint8_t> &data) {
            return ICListenStreamingConfig::fromBytes(data);
        }},
        {ModuleCode::TYPES::ICLISTEN_RECORDING_STATS,         [](const std::vector<uint8_t> &data) {
            return ICListenRecordingStats::fromBytes(data);
        }},
        {ModuleCode::TYPES::ICLISTEN_COMPLETE,         [](const std::vector<uint8_t> &data) {
            return ICListenHF::fromBytes(data);
        }}

};

SerializableModule ModuleFactory::createModule(const std::vector<uint8_t> &buffer) {
    if (buffer.empty()) {
        ErrorHandler::handleError("ModuleFactory: Buffer is empty.");
    }

    uint8_t typeCode = buffer[0];
    ModuleCode::TYPES moduleCode = ModuleCode::enumFromValue(typeCode);

    auto it = moduleCreators.find(moduleCode);
    if (it == moduleCreators.end()) {
        ErrorHandler::handleError("ModuleFactory: Unsupported module type: " + std::to_string(typeCode));
    }

    // Extract module data (excluding the first byte for type)
    std::vector<uint8_t> moduleData(buffer.begin() + 1, buffer.end());
    return it->second(moduleData);
}


std::vector<SerializableModule> ModuleFactory::createModules(const std::vector<uint8_t> &buffer) {
    std::vector<SerializableModule> modules;
    size_t offset = 0;

    while (offset < buffer.size()) {
        if (offset + 1 > buffer.size()) {
            ErrorHandler::handleError("ModuleFactory: Invalid buffer size.");
        }

        uint8_t moduleSize = buffer[offset + 1];
        if (offset + moduleSize + 1 > buffer.size()) {
            ErrorHandler::handleError("ModuleFactory: Module size exceeds buffer length.");
        }

        // Extract the module from the buffer
        std::vector<uint8_t> moduleBuffer(buffer.begin() + offset, buffer.begin() + offset + moduleSize + 2);
        modules.push_back(createModule(moduleBuffer));
        offset += moduleSize + 2;
    }

    return modules;
}

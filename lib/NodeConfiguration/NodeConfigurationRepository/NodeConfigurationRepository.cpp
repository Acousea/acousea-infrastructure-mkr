#include "NodeConfigurationRepository.h"

#include "Result/Result.h"

NodeConfigurationRepository::NodeConfigurationRepository(StorageManager& sdManager, const char* filePath)
    : storageManager(sdManager), configFilePath(filePath){
}

void NodeConfigurationRepository::init(){
    const std::string content = storageManager.readFile(configFilePath);

    if (content.empty()){
        Logger::logError(
            "NodeConfigurationRepository::begin() -> No configuration file found. Creating default configuration.");
        if (!saveConfiguration(makeDefault())){
            ErrorHandler::handleError("NodeConfigurationRepository::begin() -> Error saving default configuration.");
        }
    }
    Logger::logInfo("NodeConfigurationRepository initialized.");
}

void NodeConfigurationRepository::reset(){
    Logger::logInfo("NodeConfigurationRepository::reset() -> Resetting to default configuration.");
    if (!saveConfiguration(makeDefault())){
        ErrorHandler::handleError("NodeConfigurationRepository::reset() -> Error saving default configuration.");
    }
}


void NodeConfigurationRepository::printNodeConfiguration(acousea_NodeConfiguration configuration) const{
    Logger::logInfo("Node Configuration:");
    Logger::logInfo("Local Address: ");
    Logger::logInfo(std::to_string(configuration.localAddress));

    if (configuration.has_operationGraphModule){
        Logger::logInfo("Operation Graph Module:");
        for (int i = 0; i < configuration.operationGraphModule.graph_count; ++i){
            const auto& entry = configuration.operationGraphModule.graph[i];
            Logger::logInfo("  Key: ");
            Logger::logInfo(std::to_string(entry.key));
            if (entry.has_value){
                Logger::logInfo("    Target Mode: ");
                Logger::logInfo(std::to_string(entry.value.targetMode));
                Logger::logInfo("    Duration: ");
                Logger::logInfo(std::to_string(entry.value.duration));
            }
        }
    }

    if (configuration.has_loraModule){
        Logger::logInfo("LoRa Module:");
        for (int i = 0; i < configuration.loraModule.entries_count; ++i){
            const auto& entry = configuration.loraModule.entries[i];
            Logger::logInfo("  Mode ID: ");
            Logger::logInfo(std::to_string(entry.modeId));
            Logger::logInfo("  Period: ");
            Logger::logInfo(std::to_string(entry.period));
        }
    }

    if (configuration.has_iridiumModule){
        Logger::logInfo("Iridium Module:");
        for (int i = 0; i < configuration.iridiumModule.entries_count; ++i){
            const auto& entry = configuration.iridiumModule.entries[i];
            Logger::logInfo("  Mode ID: ");
            Logger::logInfo(std::to_string(entry.modeId));
            Logger::logInfo("  Period: ");
            Logger::logInfo(std::to_string(entry.period));
        }
    }
}


Result<std::vector<uint8_t>> NodeConfigurationRepository::encodeProto(const acousea_NodeConfiguration& m){
    pb_ostream_t s1 = PB_OSTREAM_SIZING;
    if (!pb_encode(&s1, acousea_NodeConfiguration_fields, &m))
        return Result<std::vector<uint8_t>>::failure(PB_GET_ERROR(&s1));

    std::vector<uint8_t> buf(s1.bytes_written);
    pb_ostream_t s2 = pb_ostream_from_buffer(buf.data(), buf.size());
    if (!pb_encode(&s2, acousea_NodeConfiguration_fields, &m))
        return Result<std::vector<uint8_t>>::failure(PB_GET_ERROR(&s2));

    return Result<std::vector<uint8_t>>::success(std::move(buf));
}

// ------------------------------------------------------------------
// Decodifica desde bytes a struct nanopb
// ------------------------------------------------------------------
Result<acousea_NodeConfiguration> NodeConfigurationRepository::decodeProto(const std::vector<uint8_t>& bytes){
    acousea_NodeConfiguration m = acousea_NodeConfiguration_init_default;

    pb_istream_t is = pb_istream_from_buffer(bytes.data(), bytes.size());
    if (!pb_decode(&is, acousea_NodeConfiguration_fields, &m)){
        return Result<acousea_NodeConfiguration>::failure(PB_GET_ERROR(&is));
    }

    return Result<acousea_NodeConfiguration>::success(m);
}


// ------------------------------------------------------------------
// Lee el fichero binario y devuelve la configuración (o default)
// ------------------------------------------------------------------
acousea_NodeConfiguration NodeConfigurationRepository::getNodeConfiguration() const{
    const std::vector<uint8_t> bytes = storageManager.readFileBytes(configFilePath);
    if (bytes.empty()){
        return makeDefault();
    }

    const auto dec = decodeProto(bytes);
    if (!dec.isSuccess()){
        // Opcional: log del error dec.getError()
        return makeDefault();
    }
    return dec.getValue();
}

bool NodeConfigurationRepository::saveConfiguration(const acousea_NodeConfiguration& cfg){
    auto enc = encodeProto(cfg);
    if (!enc.isSuccess()) return false;
    return storageManager.writeFileBytes(configFilePath, enc.getValue().data(), enc.getValue().size());
}


acousea_NodeConfiguration NodeConfigurationRepository::makeDefault(){
    acousea_NodeConfiguration m = acousea_NodeConfiguration_init_default;
    m.localAddress = 255;

    acousea_OperationModesGraphModule operationGraphModule = acousea_OperationModesGraphModule_init_default;
    operationGraphModule.graph_count = 1;
    operationGraphModule.graph[0] = acousea_OperationModesGraphModule_GraphEntry_init_default;
    operationGraphModule.graph[0].key = 0;
    operationGraphModule.graph[0].has_value = true;
    operationGraphModule.graph[0].value.targetMode = 0;
    operationGraphModule.graph[0].value.duration = 1;

    m.has_operationGraphModule = true;
    m.operationGraphModule = operationGraphModule;

    // ---------------- LoRa e Iridium con 15s en modo 0 ----------------
    acousea_LoRaReportingModule loraModule = acousea_LoRaReportingModule_init_default;
    loraModule.entries_count = 1;
    loraModule.entries[0] = acousea_ReportingPeriodEntry_init_default;
    loraModule.entries[0].modeId = 0;
    loraModule.entries[0].period = 15;

    m.has_loraModule = true;
    m.loraModule = loraModule;

    acousea_IridiumReportingModule iridiumModule = acousea_IridiumReportingModule_init_default;
    iridiumModule.entries_count = 1;
    iridiumModule.entries[0] = acousea_ReportingPeriodEntry_init_default;
    iridiumModule.entries[0].modeId = 0;
    iridiumModule.entries[0].period = 15;

    m.has_iridiumModule = true;
    m.iridiumModule = iridiumModule;

    return m;
}

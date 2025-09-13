#include "NodeConfigurationRepository.h"


NodeConfigurationRepository::NodeConfigurationRepository(StorageManager& sdManager)
    : storageManager(sdManager)
{
}

void NodeConfigurationRepository::init()
{
    const std::string content = storageManager.readFile(configFilePath);
    Logger::logInfo("NodeConfigurationRepository::init() -> Reading configuration from file " + std::string(configFilePath));

    if (content.empty())
    {
        Logger::logError(
            "NodeConfigurationRepository::init() -> No configuration file found. Creating default configuration.");
        if (!saveConfiguration(makeDefault()))
        {
            ErrorHandler::handleError("NodeConfigurationRepository::begin() -> Error saving default configuration.");
        }
    }
    Logger::logInfo("NodeConfigurationRepository initialized.");
}

void NodeConfigurationRepository::reset()
{
    Logger::logInfo("NodeConfigurationRepository::reset() -> Resetting to default configuration.");
    if (!saveConfiguration(makeDefault()))
    {
        ErrorHandler::handleError("NodeConfigurationRepository::reset() -> Error saving default configuration.");
    }
}


void NodeConfigurationRepository::printNodeConfiguration(const acousea_NodeConfiguration& cfg) const
{
    std::string line;
    line.reserve(256);
    line += "Node Configuration | LocalAddress=" + std::to_string(cfg.localAddress);

    if (cfg.has_operationModesModule)
    {
        line += " | OperationModes=[";
        for (int i = 0; i < cfg.operationModesModule.modes_count; ++i)
        {
            const auto& m = cfg.operationModesModule.modes[i];
            if (i) line += ", ";
            line += "{id=" + std::to_string(m.id) +
                    ", name=" + std::string(m.name) + "}";
        }
        line += "]";
        line += " | ActiveIdx=" + std::to_string(cfg.operationModesModule.activeOperationModeIdx);
    }
    else
    {
        line += " | OperationModes=<none>";
    }

    if (cfg.has_operationGraphModule)
    {
        line += " | OperationGraph=[";
        for (int i = 0; i < cfg.operationGraphModule.graph_count; ++i)
        {
            const auto& e = cfg.operationGraphModule.graph[i];
            if (i) line += ", ";
            line += "{key=" + std::to_string(e.key);
            if (e.has_value)
            {
                line += ", target=" + std::to_string(e.value.targetMode);
                line += ", duration=" + std::to_string(e.value.duration);
            }
            else
            {
                line += ", value=<none>";
            }
            line += "}";
        }
        line += "]";
    }
    else
    {
        line += " | OperationGraph=<none>";
    }

    if (cfg.has_loraModule)
    {
        line += " | LoRa=[";
        for (int i = 0; i < cfg.loraModule.entries_count; ++i)
        {
            const auto& e = cfg.loraModule.entries[i];
            if (i) line += ", ";
            line += "{mode=" + std::to_string(e.modeId)
                + ", period=" + std::to_string(e.period) + "}";
        }
        line += "]";
    }

    if (cfg.has_iridiumModule)
    {
        line += " | Iridium=[";
        for (int i = 0; i < cfg.iridiumModule.entries_count; ++i)
        {
            const auto& e = cfg.iridiumModule.entries[i];
            if (i) line += ", ";
            line += "{mode=" + std::to_string(e.modeId)
                + ", period=" + std::to_string(e.period) + "}";
        }
        line += "]";
    }

    Logger::logInfo(line);
}


Result<std::vector<uint8_t>> NodeConfigurationRepository::encodeProto(const acousea_NodeConfiguration& m)
{
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
Result<acousea_NodeConfiguration> NodeConfigurationRepository::decodeProto(const std::vector<uint8_t>& bytes)
{
    acousea_NodeConfiguration m = acousea_NodeConfiguration_init_default;

    pb_istream_t is = pb_istream_from_buffer(bytes.data(), bytes.size());
    if (!pb_decode(&is, acousea_NodeConfiguration_fields, &m))
    {
        return Result<acousea_NodeConfiguration>::failure(PB_GET_ERROR(&is));
    }

    return Result<acousea_NodeConfiguration>::success(m);
}


// ------------------------------------------------------------------
// Lee el fichero binario y devuelve la configuración (o default)
// ------------------------------------------------------------------
acousea_NodeConfiguration NodeConfigurationRepository::getNodeConfiguration() const
{
    const std::vector<uint8_t> bytes = storageManager.readFileBytes(configFilePath);
    if (bytes.empty())
    {
        return makeDefault();
    }

    const auto dec = decodeProto(bytes);
    if (!dec.isSuccess())
    {
        // Opcional: log del error dec.getError()
        return makeDefault();
    }
    return dec.getValueConst();
}

bool NodeConfigurationRepository::saveConfiguration(const acousea_NodeConfiguration& cfg)
{
    auto enc = encodeProto(cfg);
    if (!enc.isSuccess()) return false;
    return storageManager.writeFileBytes(configFilePath, enc.getValue().data(), enc.getValue().size());
}


acousea_NodeConfiguration NodeConfigurationRepository::makeDefault()
{
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

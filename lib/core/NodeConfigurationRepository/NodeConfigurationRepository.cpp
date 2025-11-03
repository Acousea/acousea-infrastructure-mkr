#include "NodeConfigurationRepository.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "Logger/Logger.h"
#include "ErrorHandler/ErrorHandler.h"


NodeConfigurationRepository::NodeConfigurationRepository(StorageManager& sdManager)
    : storageManager(sdManager)
{
}

void NodeConfigurationRepository::init()
{
    const std::string content = storageManager.readFile(configFilePath);
    Logger::logInfo(
        "NodeConfigurationRepository::init() -> Reading configuration from file " + std::string(configFilePath));

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
    line += getClassNameString() + "Node Configuration ### LocalAddress=" + std::to_string(cfg.localAddress);

    if (cfg.has_operationModesModule)
    {
        line += " ### OperationModes=[";
        for (int i = 0; i < cfg.operationModesModule.modes_count; ++i)
        {
            const auto& m = cfg.operationModesModule.modes[i];
            if (i) line += ", ";
            line += "{id=" + std::to_string(m.id) +
                ", name=" + std::string(m.name) +
                ", reportTypeId=" + std::to_string(m.reportTypeId) +
                ", transition={";
            if (m.has_transition)
            {
                line += "targetModeId=" + std::to_string(m.transition.targetModeId) +
                    ", duration=" + std::to_string(m.transition.duration);
            }
            else
            {
                line += "<none>";
            }
            line += "}";
        }
        line += "]";
        line += " | ActiveIdx=" + std::to_string(cfg.operationModesModule.activeModeId);
    }
    else
    {
        line += " ### OperationModes=<none>";
    }

    if (cfg.has_reportTypesModule)
    {
        line += " ### ReportTypes=[";
        for (int i = 0; i < cfg.reportTypesModule.reportTypes_count; ++i)
        {
            const auto& reportTypes = cfg.reportTypesModule.reportTypes[i];
            if (i) line += ", ";
            line += "{id=" + std::to_string(reportTypes.id) +
                ", name=" + std::string(reportTypes.name) +
                ", moduleCodes=[";
            for (int j = 0; j < reportTypes.includedModules_count; ++j)
            {
                if (j) line += ", ";
                line += std::to_string(reportTypes.includedModules[j]);
            }
            line += "]}";
        }
        line += "]";
    }
    else
    {
        line += " ### ReportTypes=<none>";
    }

    if (cfg.has_loraModule)
    {
        line += " ### LoRa=[";
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
        line += " ### Iridium=[";
        for (int i = 0; i < cfg.iridiumModule.entries_count; ++i)
        {
            const auto& e = cfg.iridiumModule.entries[i];
            if (i) line += ", ";
            line += "{mode=" + std::to_string(e.modeId)
                + ", period=" + std::to_string(e.period) + "}";
        }
        line += "]";
    }

    if (cfg.has_gsmMqttModule)
    {
        line += " ### GsmMqtt=[";
        for (int i = 0; i < cfg.gsmMqttModule.entries_count; ++i)
        {
            const auto& e = cfg.gsmMqttModule.entries[i];
            if (i) line += ", ";
            line += "{mode=" + std::to_string(e.modeId)
                + ", period=" + std::to_string(e.period) + "}";
        }
        line += "]";
        line += " | Broker=" + std::string(cfg.gsmMqttModule.broker)
            + ":" + std::to_string(cfg.gsmMqttModule.port)
            + " ClientId=" + std::string(cfg.gsmMqttModule.clientId);
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
    if (!enc.isSuccess())
    {
        Logger::logError(
            "NodeConfigurationRepository::saveConfiguration() -> Error encoding configuration: " + enc.getError());
        return false;
    }
    return storageManager.writeFileBytes(configFilePath, enc.getValue().data(), enc.getValue().size());
}


acousea_NodeConfiguration NodeConfigurationRepository::makeDefault()
{
    acousea_NodeConfiguration defaultNodeConfiguration = acousea_NodeConfiguration_init_default;
    defaultNodeConfiguration.localAddress = 255;

    acousea_ReportTypesModule reportTypesModule = acousea_ReportTypesModule_init_default;
    // Definimos un ReportType "BasicReport"
    acousea_ReportType basic = acousea_ReportType_init_default;
    basic.id = 1;
    strncpy(basic.name, "BasicRep", sizeof(basic.name));
    basic.includedModules_count = 3;
    basic.includedModules[0] = acousea_ModuleCode_BATTERY_MODULE;
    basic.includedModules[1] = acousea_ModuleCode_AMBIENT_MODULE;
    basic.includedModules[2] = acousea_ModuleCode_LOCATION_MODULE;

    // Meterlos en el módulo
    reportTypesModule.reportTypes_count = 1;
    reportTypesModule.reportTypes[0] = basic;

    defaultNodeConfiguration.has_reportTypesModule = true;
    defaultNodeConfiguration.reportTypesModule = reportTypesModule;


    // --- OperationModesModule ---
    acousea_OperationModesModule opModesModule = acousea_OperationModesModule_init_default;
    opModesModule.activeModeId = 1;

    // Ejemplo: modo "Normal"
    acousea_OperationMode defaultMode = acousea_OperationMode_init_default;
    defaultMode.id = 1;
    strncpy(defaultMode.name, "DEFAULT", sizeof(defaultMode.name));
    defaultMode.reportTypeId = 1; // Usa el ReportType "BasicReport"

    // Transición en bucle hacia sí mismo
    defaultMode.has_transition = true;
    defaultMode.transition = acousea_OperationModeTransition_init_default;
    defaultMode.transition.targetModeId = defaultMode.id;
    defaultMode.transition.duration = 0; // o un valor por defecto


    opModesModule.modes_count = 1;
    opModesModule.modes[0] = defaultMode;

    defaultNodeConfiguration.has_operationModesModule = true;
    defaultNodeConfiguration.operationModesModule = opModesModule;


    // ---------------- LoRa con 15s en modo 0 ----------------
#ifdef PLATFORM_HAS_LORA
    acousea_LoRaReportingModule loraModule = acousea_LoRaReportingModule_init_default;
    loraModule.entries_count = 1;
    loraModule.entries[0] = acousea_ReportingPeriodEntry_init_default;
    loraModule.entries[0].modeId = 0;
    loraModule.entries[0].period = 15;

    defaultNodeConfiguration.has_loraModule = true;
    defaultNodeConfiguration.loraModule = loraModule;
#endif

    // ---------------- Gsm MQTT sin entradas ----------------
#ifdef PLATFORM_HAS_GSM
    acousea_GsmMqttReportingModule gsmMqttModule = acousea_GsmMqttReportingModule_init_default;
    gsmMqttModule.entries_count = 0; // sin entradas
    // broker y clientId vacíos
    gsmMqttModule.port = 0;

    defaultNodeConfiguration.has_gsmMqttModule = false; // explícitamente no configurado
#endif


    // ---------------- Iridium con 15s en modo 0 ----------------
    acousea_IridiumReportingModule iridiumModule = acousea_IridiumReportingModule_init_default;
    iridiumModule.entries_count = 1;
    iridiumModule.entries[0] = acousea_ReportingPeriodEntry_init_default;
    iridiumModule.entries[0].modeId = 0;
    iridiumModule.entries[0].period = 15;

    defaultNodeConfiguration.has_iridiumModule = true;
    defaultNodeConfiguration.iridiumModule = iridiumModule;

    return defaultNodeConfiguration;
}

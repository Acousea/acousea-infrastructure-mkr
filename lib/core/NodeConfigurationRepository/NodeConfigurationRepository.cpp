#include "NodeConfigurationRepository.h"

#include <cstdio>

#include "SharedMemory/SharedMemory.hpp"
#include "Logger/Logger.h"
#include "ErrorHandler/ErrorHandler.h"
#include "ProtoUtils/ProtoUtils.hpp"

namespace pb
{
    using ProtoUtils::NodeConfiguration::encode;
    using ProtoUtils::NodeConfiguration::decodeInto;
}

NodeConfigurationRepository::NodeConfigurationRepository(StorageManager& sdManager)
    : storageManager(sdManager)
{
}

void NodeConfigurationRepository::init()
{
    LOG_CLASS_INFO("Initializing NodeConfigurationRepository");

    [[maybe_unused]] const auto& nodeConfig = getNodeConfiguration();

    LOG_CLASS_INFO("Successfully Initialized.");
}

void NodeConfigurationRepository::reset()
{
    LOG_CLASS_INFO("::reset() -> Resetting to default configuration.");
    if (!saveConfiguration(makeDefault()))
    {
        ERROR_HANDLE_CLASS("::reset() -> Error saving default configuration.");
    }
}


void NodeConfigurationRepository::printNodeConfiguration(const acousea_NodeConfiguration& cfg)
{
    // Usar el buffer temporal global de SharedMemory
    char* line = SharedMemory::tmpBuffer();
    constexpr size_t lineSize = SharedMemory::tmpBufferSize();

    SharedMemory::clearTmpBuffer();

    int len = snprintf(line, lineSize,
                       "Node Configuration ### LocalAddress=%lu",
                       cfg.localAddress);

    // --- Operation Modes ---
    if (cfg.has_operationModesModule)
    {
        len += snprintf(line + len, lineSize - len, " ### OperationModes=[");
        for (int i = 0; i < cfg.operationModesModule.modes_count; ++i)
        {
            const auto& m = cfg.operationModesModule.modes[i];
            if (i) len += snprintf(line + len, lineSize - len, ", ");
            len += snprintf(line + len, lineSize - len,
                            "{id=%lu, name=%s, reportTypeId=%lu, transition=",
                            m.id, m.name, m.reportTypeId);

            if (m.has_transition)
            {
                len += snprintf(line + len, lineSize - len,
                                "{targetModeId=%ld, duration=%ld}",
                                m.transition.targetModeId,
                                m.transition.duration);
            }
            else
            {
                len += snprintf(line + len, lineSize - len, "<none>");
            }
            len += snprintf(line + len, lineSize - len, "}");
        }
        len += snprintf(line + len, lineSize - len, "] | ActiveIdx=%ld",
                        cfg.operationModesModule.activeModeId);
    }
    else
    {
        len += snprintf(line + len, lineSize - len, " ### OperationModes=<none>");
    }

    // --- Report Types ---
    if (cfg.has_reportTypesModule)
    {
        len += snprintf(line + len, lineSize - len, " ### ReportTypes=[");
        for (int i = 0; i < cfg.reportTypesModule.reportTypes_count; ++i)
        {
            const auto& report = cfg.reportTypesModule.reportTypes[i];
            if (i) len += snprintf(line + len, lineSize - len, ", ");
            len += snprintf(line + len, lineSize - len,
                            "{id=%ld, name=%s, moduleCodes=[",
                            report.id, report.name);
            for (int j = 0; j < report.includedModules_count; ++j)
            {
                if (j) len += snprintf(line + len, lineSize - len, ", ");
                len += snprintf(line + len, lineSize - len, "%d", report.includedModules[j]);
            }
            len += snprintf(line + len, lineSize - len, "]}");
        }
        len += snprintf(line + len, lineSize - len, "]");
    }
    else
    {
        len += snprintf(line + len, lineSize - len, " ### ReportTypes=<none>");
    }

    // --- LoRa ---
    if (cfg.has_loraModule)
    {
        len += snprintf(line + len, lineSize - len, " ### LoRa=[");
        for (int i = 0; i < cfg.loraModule.entries_count; ++i)
        {
            const auto& e = cfg.loraModule.entries[i];
            if (i) len += snprintf(line + len, lineSize - len, ", ");
            len += snprintf(line + len, lineSize - len,
                            "{mode=%ld, period=%lu}", e.modeId, e.period);
        }
        len += snprintf(line + len, lineSize - len, "]");
    }
    else
    {
        len += snprintf(line + len, lineSize - len, " ### LoRa=<none>");
    }

    // --- Iridium ---
    if (cfg.has_iridiumModule)
    {
        len += snprintf(line + len, lineSize - len, " ### Iridium=[");
        for (int i = 0; i < cfg.iridiumModule.entries_count; ++i)
        {
            const auto& e = cfg.iridiumModule.entries[i];
            if (i) len += snprintf(line + len, lineSize - len, ", ");
            len += snprintf(line + len, lineSize - len,
                            "{mode=%ld, period=%lu}", e.modeId, e.period);
        }
        len += snprintf(line + len, lineSize - len, "]");
    }
    else
    {
        len += snprintf(line + len, lineSize - len, " ### Iridium=<none>");
    }

    // --- GSM-MQTT ---
    if (cfg.has_gsmMqttModule)
    {
        len += snprintf(line + len, lineSize - len, " ### GsmMqtt=[");
        for (int i = 0; i < cfg.gsmMqttModule.entries_count; ++i)
        {
            const auto& e = cfg.gsmMqttModule.entries[i];
            if (i) len += snprintf(line + len, lineSize - len, ", ");
            len += snprintf(line + len, lineSize - len,
                            "{mode=%lu, period=%lu}", e.modeId, e.period);
        }
        len += snprintf(line + len, lineSize - len, "]");
        len += snprintf(line + len, lineSize - len,
                        " | Broker=%s:%ld ClientId=%s",
                        cfg.gsmMqttModule.broker,
                        cfg.gsmMqttModule.port,
                        cfg.gsmMqttModule.clientId);
    }
    else
    {
        len += snprintf(line + len, lineSize - len, " ### GsmMqtt=<none>");
    }

    // --- Log final ---
    LOG_CLASS_INFO("%s", line);
}


// ------------------------------------------------------------------
// Lee el fichero binario y devuelve la configuración (o default)
// ------------------------------------------------------------------
acousea_NodeConfiguration& NodeConfigurationRepository::getNodeConfiguration() const
{
    if (SharedMemory::isNodeConfigurationValid())
    {
        return SharedMemory::nodeConfigurationRef(); // Is already loaded and valid
    }

    SharedMemory::resetNodeConfiguration();

    // Usamos el buffer temporal global para evitar uso de stack
    auto* tmpBuf = reinterpret_cast<uint8_t*>(SharedMemory::tmpBuffer());
    constexpr size_t tmpBufSize = SharedMemory::tmpBufferSize();

    const size_t bytesRead = storageManager.readFileBytes(configFilePath, tmpBuf, tmpBufSize);
    if (bytesRead == 0)
    {
        LOG_CLASS_ERROR("No configuration file <%s> found or empty. Setting default configuration.", configFilePath);
        const auto& defaultConfig = makeDefault();
        SharedMemory::setNodeConfiguration(defaultConfig);
        return SharedMemory::nodeConfigurationRef();
    }

    const Result<void> decodedResult = pb::decodeInto(tmpBuf, bytesRead, &SharedMemory::nodeConfigurationRef());
    if (!decodedResult.isSuccess())
    {
        LOG_CLASS_ERROR("::getNodeConfiguration() -> Error decoding configuration: %s", decodedResult.getError());
        const auto& defaultConfig = makeDefault();
        SharedMemory::setNodeConfiguration(defaultConfig);
        return SharedMemory::nodeConfigurationRef();
    }
    return SharedMemory::nodeConfigurationRef();
}

bool NodeConfigurationRepository::saveConfiguration(const acousea_NodeConfiguration& cfg)
{
    auto enc = pb::encode(cfg);
    if (!enc.isSuccess())
    {
        LOG_CLASS_ERROR("::saveConfiguration() -> Error encoding configuration: %s",
                        enc.getError());
        return false;
    }
    const bool writeOk = storageManager.writeFileBytes(configFilePath, enc.getValue().data(), enc.getValue().size());
    if (!writeOk)
    {
        LOG_CLASS_ERROR("::saveConfiguration() -> Error writing configuration to file %s",
                        configFilePath);
        return false;
    }
    SharedMemory::setNodeConfiguration(cfg);
    return true;
}


acousea_NodeConfiguration& NodeConfigurationRepository::makeDefault()
{
    // 1️⃣ Inicialización base directa (una sola vez)
    acousea_NodeConfiguration& cfg = SharedMemory::nodeConfigurationRef();

    // 2️⃣ Ajustes generales
    cfg.localAddress = 255;

    // ------------------------------------------------------------
    // REPORT TYPES MODULE
    // ------------------------------------------------------------
    cfg.has_reportTypesModule = true;
    acousea_ReportTypesModule& reportTypesModule = cfg.reportTypesModule;

    reportTypesModule.reportTypes_count = 1;
    acousea_ReportType& basic = reportTypesModule.reportTypes[0];
    basic = acousea_ReportType_init_default;
    basic.id = 1;
    strncpy(basic.name, "BasicRep", sizeof(basic.name) - 1);
    basic.includedModules_count = 3;
    basic.includedModules[0] = acousea_ModuleCode_BATTERY_MODULE;
    basic.includedModules[1] = acousea_ModuleCode_AMBIENT_MODULE;
    basic.includedModules[2] = acousea_ModuleCode_LOCATION_MODULE;

    // ------------------------------------------------------------
    // OPERATION MODES MODULE
    // ------------------------------------------------------------
    cfg.has_operationModesModule = true;
    acousea_OperationModesModule& opModesModule = cfg.operationModesModule;
    opModesModule = acousea_OperationModesModule_init_default;

    opModesModule.activeModeId = 1;
    opModesModule.modes_count = 1;

    acousea_OperationMode& defaultMode = opModesModule.modes[0];
    defaultMode = acousea_OperationMode_init_default;
    defaultMode.id = 1;
    strncpy(defaultMode.name, "DEFAULT", sizeof(defaultMode.name) - 1);
    defaultMode.reportTypeId = 1;
    defaultMode.has_transition = true;

    acousea_OperationModeTransition& trans = defaultMode.transition;
    trans = acousea_OperationModeTransition_init_default;
    trans.targetModeId = defaultMode.id;
    trans.duration = 0;

    // ------------------------------------------------------------
    // LORA MODULE
    // ------------------------------------------------------------
#ifdef PLATFORM_HAS_LORA
    cfg.has_loraModule = true;
    acousea_LoRaReportingModule& lora = cfg.loraModule;
    lora = acousea_LoRaReportingModule_init_default;

    lora.entries_count = 1;
    acousea_ReportingPeriodEntry& loraEntry = lora.entries[0];
    loraEntry = acousea_ReportingPeriodEntry_init_default;
    loraEntry.modeId = 0;
    loraEntry.period = 15;
#endif

    // ------------------------------------------------------------
    // GSM MQTT MODULE
    // ------------------------------------------------------------
#ifdef PLATFORM_HAS_GSM
    cfg.has_gsmMqttModule = false; // deshabilitado por defecto
    acousea_GsmMqttReportingModule& gsm = cfg.gsmMqttModule;
    gsm = acousea_GsmMqttReportingModule_init_default;
    gsm.entries_count = 0;
    gsm.port = 0;
#endif

    // ------------------------------------------------------------
    // IRIDIUM MODULE
    // ------------------------------------------------------------
    cfg.has_iridiumModule = true;
    acousea_IridiumReportingModule& iridium = cfg.iridiumModule;
    iridium = acousea_IridiumReportingModule_init_default;

    iridium.entries_count = 1;
    acousea_ReportingPeriodEntry& iridiumEntry = iridium.entries[0];
    iridiumEntry = acousea_ReportingPeriodEntry_init_default;
    iridiumEntry.modeId = 0;
    iridiumEntry.period = 15;

    // ------------------------------------------------------------
    // RETURN
    // ------------------------------------------------------------
    return cfg;
}

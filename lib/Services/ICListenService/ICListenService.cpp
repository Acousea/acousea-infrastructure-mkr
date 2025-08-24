#include "ICListenService.h"


ICListenService::ICListenService(Router& router, StorageManager* storageManager):
    requester(std::make_unique<Requester>(router)),
    cache(std::make_unique<Cache>(storageManager))

{

}

void ICListenService::init()
{
    // We start by loading the data in the cache from the storage manager if available
    const auto configBytes = cache->storage_manager()->readFileBytes(Cache::ICLISTEN_CONFIG_STORAGE_FILE);
    if (configBytes.empty())
    {
        Logger::logInfo(getClassNameString() + ": No ICListen configuration file found in storage");
        return;
    }

    acousea_ICListenHF hfConfig;
    if (auto decodingResult = decodeICListenHF(configBytes); !decodingResult.isSuccess())
    {
        Logger::logError(getClassNameString() + ": Failed to decode ICListen configuration from storage");
        return;
    }
    else
    {
        hfConfig = decodingResult.getValue();
        cache->storeICListenHFConfiguration(hfConfig);
        Logger::logInfo(getClassNameString() + ": Loaded ICListen configuration from storage");
    }
}

Result<std::vector<uint8_t>> ICListenService::encodeICListenHF(const acousea_ICListenHF& hfConfiguration)
{
    std::vector<uint8_t> buffer;
    pb_ostream_t stream = pb_ostream_from_buffer(nullptr, 0);

    // First, calculate the size needed
    if (!pb_encode(&stream, acousea_ICListenHF_fields, &hfConfiguration))
    {
        Logger::logError(std::string("Failed to calculate encode size: ") + PB_GET_ERROR(&stream));
        return Result<std::vector<uint8_t>>::failure(getClassNameString() + "Encoding size error");
    }

    buffer.resize(stream.bytes_written);
    stream = pb_ostream_from_buffer(buffer.data(), buffer.size());

    // Now, actually encode
    if (!pb_encode(&stream, acousea_ICListenHF_fields, &hfConfiguration))
    {
        Logger::logError(std::string("Failed to encode ICListenHF: ") + PB_GET_ERROR(&stream));
        return Result<std::vector<uint8_t>>::failure(getClassNameString() + "Encoding error");
    }

    return Result<std::vector<uint8_t>>::success(buffer);
}

Result<acousea_ICListenHF> ICListenService::decodeICListenHF(const std::vector<uint8_t>& buffer)
{
    // Inicializar con valores por defecto
    acousea_ICListenHF out_icListenConfig = acousea_ICListenHF_init_default;

    // Crear un input stream desde los bytes
    pb_istream_t stream = pb_istream_from_buffer(buffer.data(), buffer.size());

    // Decodificar
    if (!pb_decode(&stream, acousea_ICListenHF_fields, &out_icListenConfig))
    {
        Logger::logError(std::string("Failed to decode ICListenHF: ") + PB_GET_ERROR(&stream));
        return Result<acousea_ICListenHF>::failure(getClassNameString() + "Decoding error");
    }
    return Result<acousea_ICListenHF>::success(out_icListenConfig);
}


acousea_CommunicationPacket ICListenService::Requester::buildFetchICListenConfigPacket(acousea_ModuleCode code)
{
    acousea_CommunicationPacket packet = acousea_CommunicationPacket_init_default;

    // -------- Routing --------
    packet.has_routing = true;
    packet.routing = acousea_RoutingChunk_init_default;
    packet.routing.sender = static_cast<int32_t>(Router::broadcastAddress);
    packet.routing.receiver = 0; // backend
    packet.routing.ttl = 5;

    // -------- Payload --------
    packet.has_payload = true;
    packet.payload = acousea_PayloadWrapper_init_default;
    packet.payload.which_payload = acousea_PayloadWrapper_requestedConfiguration_tag;
    packet.payload.payload.requestedConfiguration = acousea_GetUpdatedNodeConfigurationPayload_init_default;
    packet.payload.payload.requestedConfiguration.requestedModules_count = 1;
    packet.payload.payload.requestedConfiguration.requestedModules[0] = code;

    return packet;
}


acousea_CommunicationPacket ICListenService::Requester::buildSendICListenConfigPacket(
    SendICListenConfigModuleCode code, SendICListenConfigModuleValue value)
{
    acousea_CommunicationPacket packet = acousea_CommunicationPacket_init_default;

    // -------- Routing --------
    packet.has_routing = true;
    packet.routing = acousea_RoutingChunk_init_default;
    packet.routing.sender = static_cast<int32_t>(Router::broadcastAddress);
    packet.routing.receiver = 0;
    packet.routing.ttl = 0;

    // -------- Payload --------
    packet.has_payload = true;
    packet.payload = acousea_PayloadWrapper_init_default;
    packet.payload.which_payload = acousea_PayloadWrapper_setConfiguration_tag;
    packet.payload.payload.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;
    packet.payload.payload.setConfiguration.modulesToChange_count = 1;
    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry moduleEntry =
        acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;
    moduleEntry.has_value = true;
    moduleEntry.key = code;
    moduleEntry.value.which_module = code;
    moduleEntry.value = acousea_ModuleWrapper_init_default;

    switch (code)
    {
    case acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
        moduleEntry.value.module.icListenLoggingConfig = std::get<acousea_ICListenLoggingConfig>(value);
        break;
    case acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
        moduleEntry.value.module.icListenStreamingConfig = std::get<acousea_ICListenStreamingConfig>(value);
        break;
    default:
        ErrorHandler::handleError(
            "Unsupported module code" + std::to_string(code) + " in ICListenService::buildSendICListenConfigPacket");
        break;
    }

    packet.payload.payload.setConfiguration.modulesToChange[0] = moduleEntry;

    return packet;
}


// Implementation of Requester
void ICListenService::Requester::fetchStatus() const
{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_STATUS);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::fetchLoggingConfig() const
{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::fetchStreamingConfig() const
{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::fetchRecordingStats() const
{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_RECORDING_STATS);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::fetchHFConfiguration() const
{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_HF);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::sendLoggingConfig(const acousea_ICListenLoggingConfig& ic_listen_logging_config) const
{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildSendICListenConfigPacket(
        SendICListenConfigModuleCode::ICLISTEN_HF_LOGGING_CONFIG, ic_listen_logging_config
    );

    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::sendStreamingConfig(
    const acousea_ICListenStreamingConfig& ic_listen_streaming_config) const
{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildSendICListenConfigPacket(
        SendICListenConfigModuleCode::ICLISTEN_HF_STREAMING_CONFIG, ic_listen_streaming_config
    );
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

// ======================================================================================================
// =========================================== CACHE ====================================================
// ======================================================================================================

ICListenService::Cache::Cache(StorageManager* storage_manager): storageManager(storage_manager)
{
}


Result<acousea_ICListenStatus> ICListenService::Cache::retrieveICListenStatus()
{
    return Result<acousea_ICListenStatus>::fromOptional(
        icListenStatus, "ICListenStatus not available in cache"
    );
}

Result<acousea_ICListenLoggingConfig> ICListenService::Cache::retrieveICListenLoggingConfig()
{
    return Result<acousea_ICListenLoggingConfig>::fromOptional(
        icListenLoggingConfig, "ICListenLoggingConfig not available in cache");
}

Result<acousea_ICListenStreamingConfig> ICListenService::Cache::retrieveICListenStreamingConfig()
{
    return Result<acousea_ICListenStreamingConfig>::fromOptional(
        icListenStreamingConfig, "ICListenStreamingConfig not available in cache");
}

Result<acousea_ICListenRecordingStats> ICListenService::Cache::retrieveICListenRecordingStats()
{
    return Result<acousea_ICListenRecordingStats>::fromOptional(
        icListenRecordingStats, "ICListenRecordingStats not available in cache");
}

Result<acousea_ICListenHF> ICListenService::Cache::retrieveICListenCompleteConfiguration()
{
    const auto status = retrieveICListenStatus();
    const auto loggingConfig = retrieveICListenLoggingConfig();
    const auto streamingConfig = retrieveICListenStreamingConfig();
    const auto recordingStats = retrieveICListenRecordingStats();

    if (status.isError() || loggingConfig.isError() || streamingConfig.isError() || recordingStats.isError())
    {
        return Result<acousea_ICListenHF>::failure("Failed to retrieve HF configuration");
    }

    acousea_ICListenHF hf = acousea_ICListenHF_init_default;
    hf.has_status = true;
    hf.status = status.getValue();
    hf.has_loggingConfig = true;
    hf.loggingConfig = loggingConfig.getValue();
    hf.has_streamingConfig = true;
    hf.streamingConfig = streamingConfig.getValue();
    hf.has_recordingStats = true;
    hf.recordingStats = recordingStats.getValue();


    return Result<acousea_ICListenHF>::success(hf);
}

void ICListenService::Cache::persistHFConfiguration()
{
    // Construimos un HF con lo que haya en memoria

    acousea_ICListenHF iclistenHfConfig = acousea_ICListenHF_init_default;

    if (icListenStatus)
    {
        iclistenHfConfig.has_status = true;
        iclistenHfConfig.status = *icListenStatus;
    } else
    {
        iclistenHfConfig.has_status = this->icListenStatus.has_value();
        iclistenHfConfig.status = this->icListenStatus.value_or(acousea_ICListenStatus(acousea_ICListenStatus_init_default));
    }
    if (icListenLoggingConfig)
    {
        iclistenHfConfig.has_loggingConfig = true;
        iclistenHfConfig.loggingConfig = *icListenLoggingConfig;
    } else
    {
        iclistenHfConfig.has_loggingConfig = this->icListenLoggingConfig.has_value();
        iclistenHfConfig.loggingConfig = this->icListenLoggingConfig.value_or(acousea_ICListenLoggingConfig(acousea_ICListenLoggingConfig_init_default));
    }
    if (icListenStreamingConfig)
    {
        iclistenHfConfig.has_streamingConfig = true;
        iclistenHfConfig.streamingConfig = *icListenStreamingConfig;
    } else
    {
        iclistenHfConfig.has_streamingConfig = this->icListenStreamingConfig.has_value();
        iclistenHfConfig.streamingConfig = this->icListenStreamingConfig.value_or(acousea_ICListenStreamingConfig(acousea_ICListenStreamingConfig_init_default));
    }
    if (icListenRecordingStats)
    {
        iclistenHfConfig.has_recordingStats = true;
        iclistenHfConfig.recordingStats = *icListenRecordingStats;
    }
    else
    {
        iclistenHfConfig.has_recordingStats = this->icListenRecordingStats.has_value();
        iclistenHfConfig.recordingStats = this->icListenRecordingStats.value_or(acousea_ICListenRecordingStats(acousea_ICListenRecordingStats_init_default));
    }

    // Serializar
    const auto encoded = ICListenService::encodeICListenHF(iclistenHfConfig);
    if (!encoded.isSuccess())
    {
        Logger::logError("Cache::persistHFConfiguration -> Failed to encode HF config");
        return;
    }

    // Guardar en storage
    if (!storageManager->writeFileBytes(Cache::ICLISTEN_CONFIG_STORAGE_FILE, encoded.getValue()))
    {
        Logger::logError("Cache::persistHFConfiguration -> Failed to save HF config in storage");
    }
}


void ICListenService::Cache::storeICListenStatus(const acousea_ICListenStatus& ic_listen_status)
{
    icListenStatus = ic_listen_status;
    persistHFConfiguration();
}

void ICListenService::Cache::storeICListenLoggingConfig(
    const acousea_ICListenLoggingConfig& ic_listen_logging_config)
{
    icListenLoggingConfig = ic_listen_logging_config;
    persistHFConfiguration();
}

void ICListenService::Cache::storeICListenStreamingConfig(
    const acousea_ICListenStreamingConfig& ic_listen_streaming_config)
{
    icListenStreamingConfig = ic_listen_streaming_config;
    persistHFConfiguration();
}

void ICListenService::Cache::storeICListenRecordingStats(
    const acousea_ICListenRecordingStats& ic_listen_recording_stats)
{
    icListenRecordingStats = ic_listen_recording_stats;
    persistHFConfiguration();
}

void ICListenService::Cache::storeICListenHFConfiguration(const acousea_ICListenHF& hfConfiguration)
{
    icListenStatus = hfConfiguration.status;
    icListenLoggingConfig = hfConfiguration.loggingConfig;
    icListenStreamingConfig = hfConfiguration.streamingConfig;
    icListenRecordingStats = hfConfiguration.recordingStats;
    persistHFConfiguration();
}

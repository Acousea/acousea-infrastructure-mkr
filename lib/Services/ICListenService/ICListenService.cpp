#include "ICListenService.h"


ICListenService::ICListenService(Router& router):
    router(router),
    cache(std::make_unique<Cache>())

{
}

void ICListenService::init() const
{
    // We start by requesting an updated configuration from the iclisten device
    Logger::logInfo(getClassNameString() + " Initializing and fetching ICListen configuration");
    fetchHFConfiguration();
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


acousea_CommunicationPacket ICListenService::buildFetchICListenConfigPacket(acousea_ModuleCode code)
{
    acousea_CommunicationPacket packet = acousea_CommunicationPacket_init_default;

    // -------- Routing --------
    packet.has_routing = true;
    packet.routing = acousea_RoutingChunk_init_default;
    packet.routing.sender = static_cast<int32_t>(Router::broadcastAddress);
    packet.routing.receiver = 0; // backend
    packet.routing.ttl = 5;

    // -------- Payload --------
    packet.which_body = acousea_CommunicationPacket_command_tag;
    packet.body = acousea_CommandBody_init_default;
    packet.body.command.which_command = acousea_CommandBody_requestedConfiguration_tag;
    packet.body.command.command.requestedConfiguration = acousea_GetUpdatedNodeConfigurationPayload_init_default;
    packet.body.command.command.requestedConfiguration.requestedModules_count = 1;
    packet.body.command.command.requestedConfiguration.requestedModules[0] = code;

    return packet;
}


acousea_CommunicationPacket ICListenService::buildSendICListenConfigPacket(
    SendICListenConfigModuleCode code, SendICListenConfigModuleValue iclistenConfigValue)
{
    acousea_CommunicationPacket packet = acousea_CommunicationPacket_init_default;

    // -------- Routing --------
    packet.has_routing = true;
    packet.routing = acousea_RoutingChunk_init_default;
    packet.routing.sender = static_cast<int32_t>(Router::broadcastAddress);
    packet.routing.receiver = 0;
    packet.routing.ttl = 0;

    // -------- Payload --------
    packet.which_body = acousea_CommunicationPacket_command_tag;
    packet.body = acousea_CommandBody_init_default;
    packet.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    packet.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;
    packet.body.command.command.setConfiguration.modules_count = 1;
    acousea_SetNodeConfigurationPayload_ModulesEntry moduleEntry =
        acousea_SetNodeConfigurationPayload_ModulesEntry_init_default;
    moduleEntry.has_value = true;
    moduleEntry.key = code;
    moduleEntry.value.which_module = code;
    moduleEntry.value = acousea_ModuleWrapper_init_default;

    switch (code)
    {
    case acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
        moduleEntry.value.module.icListenLoggingConfig = std::get<acousea_ICListenLoggingConfig>(iclistenConfigValue);
        break;
    case acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
        moduleEntry.value.module.icListenStreamingConfig = std::get<acousea_ICListenStreamingConfig>(
            iclistenConfigValue);
        break;
    default:
        ErrorHandler::handleError(
            "Unsupported module code" + std::to_string(code) + " in ICListenService::buildSendICListenConfigPacket");
        break;
    }
    packet.body.command.command.setConfiguration.modules[0] = moduleEntry;

    return packet;
}


// Implementation of Requester
void ICListenService::fetchStatus() const
{
    cache->invalidateStatus();

    acousea_CommunicationPacket packet = ICListenService::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_STATUS);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::fetchLoggingConfig() const
{
    cache->invalidateLogging();

    acousea_CommunicationPacket packet = ICListenService::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::fetchStreamingConfig() const
{
    cache->invalidateStreaming();

    acousea_CommunicationPacket packet = ICListenService::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::fetchRecordingStats() const
{
    cache->invalidateRecordingStats();

    acousea_CommunicationPacket packet = ICListenService::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_RECORDING_STATS);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::fetchHFConfiguration() const
{
    cache->invalidateAll();

    acousea_CommunicationPacket packet = ICListenService::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_HF);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::sendLoggingConfig(const acousea_ICListenLoggingConfig& ic_listen_logging_config) const
{
    const auto cached = cache->getICListenLoggingConfig(); // copia segura
    if (cached.hasValue() &&
        nanopb_equal(&cached.get(), &ic_listen_logging_config, acousea_ICListenLoggingConfig_fields))
    {
        Logger::logInfo("ICListenService::sendLoggingConfig -> No changes in logging config, not sending");
        return;
    }
    cache->invalidateLogging();

    acousea_CommunicationPacket packet = ICListenService::buildSendICListenConfigPacket(
        SendICListenConfigModuleCode::ICLISTEN_HF_LOGGING_CONFIG, ic_listen_logging_config
    );


    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::sendStreamingConfig(
    const acousea_ICListenStreamingConfig& ic_listen_streaming_config) const
{
    const auto cached = cache->getICListenStreamingConfig();
    if (cached.hasValue() &&
        nanopb_equal(&cached.get(), &ic_listen_streaming_config, acousea_ICListenStreamingConfig_fields))
    {
        Logger::logInfo("ICListenService::sendStreamingConfig -> No changes in streaming config, not sending");
        return;
    }
    cache->invalidateStreaming();
    acousea_CommunicationPacket packet = ICListenService::buildSendICListenConfigPacket(
        SendICListenConfigModuleCode::ICLISTEN_HF_STREAMING_CONFIG, ic_listen_streaming_config
    );
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

// ======================================================================================================
// =========================================== CACHE ====================================================
// ======================================================================================================

ICListenService::CachedValue<acousea_ICListenStatus> ICListenService::Cache::getICListenStatus()
{
    return icListenStatus;
}

ICListenService::CachedValue<acousea_ICListenLoggingConfig> ICListenService::Cache::getICListenLoggingConfig()
{
    return icListenLoggingConfig;
}

ICListenService::CachedValue<acousea_ICListenStreamingConfig> ICListenService::Cache::getICListenStreamingConfig()
{
    return icListenStreamingConfig;
}

ICListenService::CachedValue<acousea_ICListenRecordingStats> ICListenService::Cache::getICListenRecordingStats()
{
    return icListenRecordingStats;
}

ICListenService::CachedValue<acousea_ICListenHF> ICListenService::Cache::getICListenCompleteConfiguration()
{
    if (!icListenStatus.hasValue() ||
        !icListenLoggingConfig.hasValue() ||
        !icListenStreamingConfig.hasValue() ||
        !icListenRecordingStats.hasValue())
    {
        return CachedValue<acousea_ICListenHF>(std::nullopt, false);
    }

    acousea_ICListenHF hf = acousea_ICListenHF_init_default;
    hf.has_status = true;
    hf.status = icListenStatus.get();
    hf.has_loggingConfig = true;
    hf.loggingConfig = icListenLoggingConfig.get();
    hf.has_streamingConfig = true;
    hf.streamingConfig = icListenStreamingConfig.get();
    hf.has_recordingStats = true;
    hf.recordingStats = icListenRecordingStats.get();


    return CachedValue<acousea_ICListenHF>(hf,
                                           (icListenStatus.isFresh &&
                                               icListenLoggingConfig.isFresh &&
                                               icListenStreamingConfig.isFresh &&
                                               icListenRecordingStats.isFresh)
    );
}


void ICListenService::Cache::storeICListenStatus(const acousea_ICListenStatus& ic_listen_status)
{
    icListenStatus.store(ic_listen_status); // isFresh = true
}

void ICListenService::Cache::storeICListenLoggingConfig(
    const acousea_ICListenLoggingConfig& ic_listen_logging_config)
{
    icListenLoggingConfig.store(ic_listen_logging_config);
}

void ICListenService::Cache::storeICListenStreamingConfig(
    const acousea_ICListenStreamingConfig& ic_listen_streaming_config)
{
    icListenStreamingConfig.store(ic_listen_streaming_config);
}

void ICListenService::Cache::storeICListenRecordingStats(
    const acousea_ICListenRecordingStats& ic_listen_recording_stats)
{
    icListenRecordingStats.store(ic_listen_recording_stats);
}

void ICListenService::Cache::storeICListenHFConfiguration(const acousea_ICListenHF& hfConfig)
{
    if (hfConfig.has_status)
    {
        icListenStatus.store(hfConfig.status);
    }
    if (hfConfig.has_loggingConfig)
    {
        icListenLoggingConfig.store(hfConfig.loggingConfig);
    }
    if (hfConfig.has_streamingConfig)
    {
        icListenStreamingConfig.store(hfConfig.streamingConfig);
    }
    if (hfConfig.has_recordingStats)
    {
        icListenRecordingStats.store(hfConfig.recordingStats);
    }
}

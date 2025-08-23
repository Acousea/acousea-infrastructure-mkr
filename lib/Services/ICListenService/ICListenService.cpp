#include "ICListenService.h"


acousea_CommunicationPacket ICListenService::Requester::buildFetchICListenConfigPacket(acousea_ModuleCode code){
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

// Implementation of Requester
void ICListenService::Requester::fetchStatus() const{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_STATUS);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::fetchLoggingConfig() const{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::fetchStreamingConfig() const{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::fetchRecordingStats() const{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_RECORDING_STATS);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}

void ICListenService::Requester::fetchHFConfiguration() const{
    acousea_CommunicationPacket packet = ICListenService::Requester::buildFetchICListenConfigPacket(
        acousea_ModuleCode_ICLISTEN_HF);
    router.sendFrom(Router::broadcastAddress).sendSerial(packet);
}


Result<acousea_ICListenStatus> ICListenService::Cache::retrieveICListenStatus() const{
    return Result<acousea_ICListenStatus>::fromOptional(std::move(icListenStatus),
                                                        "ICListenStatus not available in cache");
}

Result<acousea_ICListenLoggingConfig> ICListenService::Cache::retrieveICListenLoggingConfig() const{
    return Result<acousea_ICListenLoggingConfig>::fromOptional(std::move(icListenLoggingConfig),
                                                               "ICListenLoggingConfig not available in cache");
}

Result<acousea_ICListenStreamingConfig> ICListenService::Cache::retrieveICListenStreamingConfig() const{
    return Result<acousea_ICListenStreamingConfig>::fromOptional(std::move(icListenStreamingConfig),
                                                                 "ICListenStreamingConfig not available in cache");
}

Result<acousea_ICListenRecordingStats> ICListenService::Cache::retrieveICListenRecordingStats() const{
    return Result<acousea_ICListenRecordingStats>::fromOptional(std::move(icListenRecordingStats),
                                                                "ICListenRecordingStats not available in cache");
}

Result<acousea_ICListenHF> ICListenService::Cache::getHFCompleteConfiguration() const{
    const auto status = retrieveICListenStatus();
    const auto loggingConfig = retrieveICListenLoggingConfig();
    const auto streamingConfig = retrieveICListenStreamingConfig();
    const auto recordingStats = retrieveICListenRecordingStats();

    if (status.isError() || loggingConfig.isError() || streamingConfig.isError() || recordingStats.isError()){
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

void ICListenService::Cache::storeICListenStatus(const acousea_ICListenStatus& ic_listen_status) const{
    icListenStatus = std::make_optional<acousea_ICListenStatus>(ic_listen_status);
}

void ICListenService::Cache::storeICListenLoggingConfig(
    const acousea_ICListenLoggingConfig& ic_listen_logging_config) const{
    icListenLoggingConfig = std::make_optional<acousea_ICListenLoggingConfig>(ic_listen_logging_config);
}

void ICListenService::Cache::storeICListenStreamingConfig(
    const acousea_ICListenStreamingConfig& ic_listen_streaming_config) const{
    icListenStreamingConfig = std::make_optional<acousea_ICListenStreamingConfig>(ic_listen_streaming_config);
}

void ICListenService::Cache::
storeICListenRecordingStats(const acousea_ICListenRecordingStats& ic_listen_recording_stats) const{
    icListenRecordingStats = std::make_optional<acousea_ICListenRecordingStats>(ic_listen_recording_stats);
}

void ICListenService::Cache::storeHFConfiguration(const acousea_ICListenHF& hfConfiguration) const{
    storeICListenStatus(hfConfiguration.status);
    storeICListenLoggingConfig(hfConfiguration.loggingConfig);
    storeICListenStreamingConfig(hfConfiguration.streamingConfig);
    storeICListenRecordingStats(hfConfiguration.recordingStats);
}

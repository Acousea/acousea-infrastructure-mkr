#include "ICListenService.h"


// Implementation of Requester
void ICListenService::Requester::fetchStatus() const {
    const auto packet = FetchICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        FetchICListenConfigurationPayload::Builder().includeAspect(ICListenAspect::Aspect::STATUS).
        build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

void ICListenService::Requester::fetchLoggingConfig() const {
    const auto packet = FetchICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        FetchICListenConfigurationPayload::Builder().includeAspect(ICListenAspect::Aspect::LOGGING).
        build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

void ICListenService::Requester::fetchStreamingConfig() const {
    const auto packet = FetchICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        FetchICListenConfigurationPayload::Builder().includeAspect(ICListenAspect::Aspect::STREAMING)
        .build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

void ICListenService::Requester::fetchRecordingStats() const {
    const auto packet = FetchICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        FetchICListenConfigurationPayload::Builder().includeAspect(ICListenAspect::Aspect::STATS).
        build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

void ICListenService::Requester::fetchHFConfiguration() const {
    const auto packet = FetchICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        FetchICListenConfigurationPayload::Builder().all().build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

void ICListenService::Requester::setICListenStatus(const ICListenStatus &ic_listen_status) const {
    const auto packet = SetICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        SetICListenConfigurationPayload::Builder().includeAspect(ICListenAspect::Aspect::STATUS,
                                                                 ic_listen_status).build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

void ICListenService::Requester::setICListenLoggingConfig(const ICListenLoggingConfig &ic_listen_logging_config) const {
    const auto packet = SetICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        SetICListenConfigurationPayload::Builder().includeAspect(ICListenAspect::Aspect::LOGGING,
                                                                 ic_listen_logging_config).build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

void ICListenService::Requester::setICListenStreamingConfig(const ICListenStreamingConfig &ic_listen_streaming_config) const {
    const auto packet = SetICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        SetICListenConfigurationPayload::Builder().includeAspect(ICListenAspect::Aspect::STREAMING,
                                                                 ic_listen_streaming_config).build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

void ICListenService::Requester::setICListenRecordingStats(const ICListenRecordingStats &ic_listen_recording_stats) const {
    const auto packet = SetICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        SetICListenConfigurationPayload::Builder().includeAspect(ICListenAspect::Aspect::STATS,
                                                                 ic_listen_recording_stats).build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

void ICListenService::Requester::setICListenHFConfiguration(const ICListenHF &ic_listen_hf) const {
    const auto packet = SetICListenConfigPacket(
        RoutingChunk::fromNodeToBackend(Address(255)),
        SetICListenConfigurationPayload::Builder()
        .includeAspect(ICListenAspect::Aspect::STATS, ic_listen_hf.recordingStats)
        .includeAspect(ICListenAspect::Aspect::STATUS, ic_listen_hf.status)
        .includeAspect(ICListenAspect::Aspect::LOGGING, ic_listen_hf.loggingConfig)
        .includeAspect(ICListenAspect::Aspect::STREAMING, ic_listen_hf.streamingConfig)
        .build()
    );
    router.sendFrom(Address(255)).sendSerial(packet);
}

Result<ICListenStatus> ICListenService::Cache::retrieveICListenStatus() const {
    return Result<ICListenStatus>::fromOptional(std::move(icListenStatus), "ICListenStatus not available in cache");
}

Result<ICListenLoggingConfig> ICListenService::Cache::retrieveICListenLoggingConfig() const {
    return Result<ICListenLoggingConfig>::fromOptional(std::move(icListenLoggingConfig),
                                                       "ICListenLoggingConfig not available in cache");
}

Result<ICListenStreamingConfig> ICListenService::Cache::retrieveICListenStreamingConfig() const {
    return Result<ICListenStreamingConfig>::fromOptional(std::move(icListenStreamingConfig),
                                                         "ICListenStreamingConfig not available in cache");
}

Result<ICListenRecordingStats> ICListenService::Cache::retrieveICListenRecordingStats() const {
    return Result<ICListenRecordingStats>::fromOptional(std::move(icListenRecordingStats),
                                                        "ICListenRecordingStats not available in cache");
}

Result<ICListenHF> ICListenService::Cache::getHFCompleteConfiguration() const {
    const auto status = retrieveICListenStatus();
    const auto loggingConfig = retrieveICListenLoggingConfig();
    const auto streamingConfig = retrieveICListenStreamingConfig();
    const auto recordingStats = retrieveICListenRecordingStats();

    if (status.isError() || loggingConfig.isError() || streamingConfig.isError() || recordingStats.isError()) {
        return Result<ICListenHF>::failure("Failed to retrieve HF configuration");
    }

    return Result<ICListenHF>::success(ICListenHF(status.getValue(), loggingConfig.getValue(),
                                                  streamingConfig.getValue(), recordingStats.getValue()));
}

void ICListenService::Cache::storeICListenStatus(const ICListenStatus &ic_listen_status) const {
    icListenStatus = std::make_optional<ICListenStatus>(ic_listen_status);
}

void ICListenService::Cache::storeICListenLoggingConfig(const ICListenLoggingConfig &ic_listen_logging_config) const {
    icListenLoggingConfig = std::make_optional<ICListenLoggingConfig>(ic_listen_logging_config);
}

void ICListenService::Cache::storeICListenStreamingConfig(
    const ICListenStreamingConfig &ic_listen_streaming_config) const {
    icListenStreamingConfig = std::make_optional<ICListenStreamingConfig>(ic_listen_streaming_config);
}

void ICListenService::Cache::
storeICListenRecordingStats(const ICListenRecordingStats &ic_listen_recording_stats) const {
    icListenRecordingStats = std::make_optional<ICListenRecordingStats>(ic_listen_recording_stats);
}

void ICListenService::Cache::storeHFConfiguration(const ICListenHF &hfConfiguration) const {
    storeICListenStatus(hfConfiguration.status);
    storeICListenLoggingConfig(hfConfiguration.loggingConfig);
    storeICListenStreamingConfig(hfConfiguration.streamingConfig);
    storeICListenRecordingStats(hfConfiguration.recordingStats);
}

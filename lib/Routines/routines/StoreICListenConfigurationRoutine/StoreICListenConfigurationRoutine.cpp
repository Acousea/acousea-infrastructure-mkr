#include "StoreICListenConfigurationRoutine.h"

StoreICListenConfigurationRoutine::StoreICListenConfigurationRoutine(ICListenService &icListenService)
    : IRoutine(getClassNameString()), icListenService(icListenService) {
}

Result<Packet> StoreICListenConfigurationRoutine::execute(const Packet &packet) {
    const SetICListenConfigurationPayload configurationPayload = packet.getPayloadAs<SetICListenConfigurationPayload>();
    Logger::logInfo(getClassNameString() + "Storing ICListen configuration ");

    const auto status = configurationPayload.getStatus();
    const auto loggingConfig = configurationPayload.getLoggingConfig();
    const auto streamingConfig = configurationPayload.getStreamingConfig();
    const auto recordingStats = configurationPayload.getRecordingStats();

    if (status.has_value()) {
        Logger::logInfo("Storing ICListen status: " + status.value().toString());
        icListenService.getCache()->storeICListenStatus(status.value());
    }
    if (loggingConfig.has_value()) {
        Logger::logInfo("Storing ICListen logging config: " + loggingConfig.value().toString());
        icListenService.getCache()->storeICListenLoggingConfig(loggingConfig.value());
    }
    if (streamingConfig.has_value()) {
        Logger::logInfo("Storing ICListen streaming config: " + streamingConfig.value().toString());
        icListenService.getCache()->storeICListenStreamingConfig(streamingConfig.value());
    }
    if (recordingStats.has_value()) {
        Logger::logInfo("Storing ICListen recording stats: " + recordingStats.value().toString());
        icListenService.getCache()->storeICListenRecordingStats(recordingStats.value());
    }
    return Result<Packet>::emptySuccess();
}

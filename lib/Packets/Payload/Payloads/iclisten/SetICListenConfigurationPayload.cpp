#include "SetICListenConfigurationPayload.h"

#include "Payload/Modules/factory/ModuleFactory.h"

// Constructor privado
SetICListenConfigurationPayload::SetICListenConfigurationPayload(
    const std::optional<ICListenStatus> &status,
    const std::optional<ICListenLoggingConfig> &loggingConfig,
    const std::optional<ICListenStreamingConfig> &streamingConfig,
    const std::optional<ICListenRecordingStats> &recordingStats
): status(status),
   loggingConfig(loggingConfig),
   streamingConfig(streamingConfig),
   recordingStats(recordingStats) {
}

SetICListenConfigurationPayload::Builder &SetICListenConfigurationPayload::Builder::includeAspect(
    ICListenAspect::Aspect aspect, const SerializableModule &module) {
    aspects |= static_cast<uint8_t>(aspect);
    addConfiguration(aspect, module);
    return *this;
}

SetICListenConfigurationPayload SetICListenConfigurationPayload::Builder::build() const {
    return SetICListenConfigurationPayload(status, loggingConfig, streamingConfig, recordingStats);
}

void SetICListenConfigurationPayload::Builder::addConfiguration(const ICListenAspect::Aspect aspect,
                                                                const SerializableModule &module) {
    switch (aspect) {
        case ICListenAspect::Aspect::STATUS:
            status = static_cast<const ICListenStatus &>(module);
            break;
        case ICListenAspect::Aspect::LOGGING:
            loggingConfig = static_cast<const ICListenLoggingConfig &>(module);
            break;
        case ICListenAspect::Aspect::STREAMING:
            streamingConfig = static_cast<const ICListenStreamingConfig &>(module);
            break;
        case ICListenAspect::Aspect::STATS:
            recordingStats = static_cast<const ICListenRecordingStats &>(module);
            break;
    }
}

// Tamaño en bytes del payload
uint16_t SetICListenConfigurationPayload::getBytesSize() const {
    uint16_t size = 0;
    if (status) size += status->toBytes().size();
    if (loggingConfig) size += loggingConfig->toBytes().size();
    if (streamingConfig) size += streamingConfig->toBytes().size();
    if (recordingStats) size += recordingStats->toBytes().size();
    return size;
}


std::vector<uint8_t> SetICListenConfigurationPayload::toBytes() const {
    std::vector<uint8_t> bytes;

    // Serializar cada configuración si está presente
    if (status) {
        auto statusBytes = status->toBytes();
        bytes.insert(bytes.end(), statusBytes.begin(), statusBytes.end());
    }
    if (loggingConfig) {
        auto loggingBytes = loggingConfig->toBytes();
        bytes.insert(bytes.end(), loggingBytes.begin(), loggingBytes.end());
    }
    if (streamingConfig) {
        auto streamingBytes = streamingConfig->toBytes();
        bytes.insert(bytes.end(), streamingBytes.begin(), streamingBytes.end());
    }
    if (recordingStats) {
        auto statsBytes = recordingStats->toBytes();
        bytes.insert(bytes.end(), statsBytes.begin(), statsBytes.end());
    }

    return bytes;
}


SetICListenConfigurationPayload SetICListenConfigurationPayload::fromBytes(const std::vector<uint8_t> &data) {
    std::vector<SerializableModule> modules = ModuleFactory::createModules(data);
    auto status = std::optional<ICListenStatus>();
    auto loggingConfig = std::optional<ICListenLoggingConfig>();
    auto streamingConfig = std::optional<ICListenStreamingConfig>();
    auto recordingStats = std::optional<ICListenRecordingStats>();
    for (const auto &module: modules) {
        switch (module.getType()) {
            case static_cast<uint8_t>(ModuleCode::TYPES::ICLISTEN_RECORDING_STATS):
                recordingStats = static_cast<const ICListenRecordingStats &>(module);
                break;
            case static_cast<uint8_t>(ModuleCode::TYPES::ICLISTEN_STATUS):
                status = static_cast<const ICListenStatus &>(module);
                break;
            case static_cast<uint8_t>(ModuleCode::TYPES::ICLISTEN_LOGGING_CONFIG):
                loggingConfig = static_cast<const ICListenLoggingConfig &>(module);
                break;
            case static_cast<uint8_t>(ModuleCode::TYPES::ICLISTEN_STREAMING_CONFIG):
                streamingConfig = static_cast<const ICListenStreamingConfig &>(module);
                break;
            default:
                ErrorHandler::handleError("SetICListenConfigurationPayload::fromBytes() -> "
                                          "Invalid ICListen aspect " + std::to_string(module.getType()));
        }
    }

    return SetICListenConfigurationPayload(status, loggingConfig, streamingConfig, recordingStats);
}

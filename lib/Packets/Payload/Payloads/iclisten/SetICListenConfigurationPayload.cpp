#include "SetICListenConfigurationPayload.h"

// Constructor privado
SetICListenConfigurationPayload::SetICListenConfigurationPayload(const std::bitset<8> &selectedAspects,
                                                                 const std::optional<ICListenStatus> &status,
                                                                 const std::optional<ICListenLoggingConfig> &
                                                                 loggingConfig,
                                                                 const std::optional<ICListenStreamingConfig> &
                                                                 streamingConfig,
                                                                 const std::optional<ICListenRecordingStats> &
                                                                 recordingStats)
    : selectedAspects(selectedAspects),
      status(status),
      loggingConfig(loggingConfig),
      streamingConfig(streamingConfig),
      recordingStats(recordingStats) {
}

SetICListenConfigurationPayload::Builder &SetICListenConfigurationPayload::Builder::includeAspect(
    Aspect aspect, const SerializableModule &module) {
    aspects.set(static_cast<uint8_t>(aspect));
    addConfiguration(aspect, module);
    return *this;
}

SetICListenConfigurationPayload SetICListenConfigurationPayload::Builder::build() const {
    return SetICListenConfigurationPayload(aspects, status, loggingConfig, streamingConfig, recordingStats);
}

void SetICListenConfigurationPayload::Builder::addConfiguration(const Aspect aspect, const SerializableModule &module) {
    switch (aspect) {
        case Aspect::STATUS:
            status = static_cast<const ICListenStatus &>(module);
            break;
        case Aspect::LOGGING:
            loggingConfig = static_cast<const ICListenLoggingConfig &>(module);
            break;
        case Aspect::STREAMING:
            streamingConfig = static_cast<const ICListenStreamingConfig &>(module);
            break;
        case Aspect::STATS:
            recordingStats = static_cast<const ICListenRecordingStats &>(module);
            break;
    }
}

// Tamaño en bytes del payload
uint16_t SetICListenConfigurationPayload::getBytesSize() const {
    uint16_t size = 1; // Un byte para los aspectos seleccionados
    if (status) size += status->toBytes().size();
    if (loggingConfig) size += loggingConfig->toBytes().size();
    if (streamingConfig) size += streamingConfig->toBytes().size();
    if (recordingStats) size += recordingStats->toBytes().size();
    return size;
}

// Serialización del payload a un vector de bytes
std::vector<uint8_t> SetICListenConfigurationPayload::toBytes() const {
    std::vector<uint8_t> bytes;

    // Agregar los aspectos seleccionados como primer byte
    bytes.push_back(static_cast<uint8_t>(selectedAspects.to_ulong()));

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

std::vector<SetICListenConfigurationPayload::Aspect> SetICListenConfigurationPayload::getAspectsAsEnums() const {
    std::vector<Aspect> aspectsList;
    for (size_t i = 0; i < selectedAspects.size(); ++i) {
        if (selectedAspects.test(i)) {
            aspectsList.push_back(static_cast<Aspect>(1 << i));
        }
    }
    return aspectsList;
}


SetICListenConfigurationPayload SetICListenConfigurationPayload::fromBytes(const std::vector<uint8_t> &data) {
    if (data.empty()) {
        ErrorHandler::handleError("Empty data received");
    }

    std::bitset<8> aspects(data[0]);
    size_t offset = 1;

    std::optional<ICListenStatus> status;
    std::optional<ICListenLoggingConfig> loggingConfig;
    std::optional<ICListenStreamingConfig> streamingConfig;
    std::optional<ICListenRecordingStats> recordingStats;

    if (aspects.test(static_cast<uint8_t>(Aspect::STATUS))) {
        status = ICListenStatus::fromBytes({data.begin() + offset, data.end()});
        offset += status->toBytes().size();
    }
    if (aspects.test(static_cast<uint8_t>(Aspect::LOGGING))) {
        loggingConfig = ICListenLoggingConfig::fromBytes({data.begin() + offset, data.end()});
        offset += loggingConfig->toBytes().size();
    }
    if (aspects.test(static_cast<uint8_t>(Aspect::STREAMING))) {
        streamingConfig = ICListenStreamingConfig::fromBytes({data.begin() + offset, data.end()});
        offset += streamingConfig->toBytes().size();
    }
    if (aspects.test(static_cast<uint8_t>(Aspect::STATS))) {
        recordingStats = ICListenRecordingStats::fromBytes({data.begin() + offset, data.end()});
        offset += recordingStats->toBytes().size();
    }

    return SetICListenConfigurationPayload(aspects, status, loggingConfig, streamingConfig, recordingStats);
}

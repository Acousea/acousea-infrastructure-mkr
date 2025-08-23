#include "ICListenHF.h"

ICListenHF::ICListenHF(const ICListenStatus &status, const ICListenLoggingConfig &loggingConfig,
                       const ICListenStreamingConfig &streamingConfig, const ICListenRecordingStats &recordingStats)
        : PamModule(ModuleCode::TYPES::ICLISTEN_COMPLETE,
        // Lambda para fusionar los vectores
                    ([](const std::vector<std::vector<uint8_t>> &byteVectors) {
                        std::vector<uint8_t> merged;
                        for (const auto &vec: byteVectors) {
                            merged.insert(merged.end(), vec.begin(), vec.end());
                        }
                        return merged;
                    })({status.toBytes(), loggingConfig.toBytes(), streamingConfig.toBytes(), recordingStats.toBytes()}),
                    "RB9-ETH",
                    "ICListenHF"),
          status(status),
          loggingConfig(loggingConfig),
          streamingConfig(streamingConfig),
          recordingStats(recordingStats) {}

ICListenHF ICListenHF::fromBytes(const std::vector<uint8_t> &data) {
    size_t offset = 0;

    auto extractModule = [&](auto &module, auto fromBytesFunction) {
        offset += 2; // Skip moduleCode and tagLength
        module = fromBytesFunction(std::vector<uint8_t>(data.begin() + offset, data.end()));
        offset += module.getFullLength();
    };

    auto status = ICListenStatus::createDefault();
    auto loggingConfig = ICListenLoggingConfig::createDefault();
    auto streamingConfig = ICListenStreamingConfig::createDefault();
    auto recordingStats = ICListenRecordingStats::createDefault();

    extractModule(status, ICListenStatus::fromBytes);
    extractModule(loggingConfig, ICListenLoggingConfig::fromBytes);
    extractModule(streamingConfig, ICListenStreamingConfig::fromBytes);
    extractModule(recordingStats, ICListenRecordingStats::fromBytes);

    return ICListenHF(status, loggingConfig, streamingConfig, recordingStats);
}


std::string ICListenHF::toString() const {
    return "ICListenHF { status: " + status.toString() +
           ", loggingConfig: " + loggingConfig.toString() +
           ", streamingConfig: " + streamingConfig.toString() +
           ", recordingStats: " + recordingStats.toString() + " }";
}

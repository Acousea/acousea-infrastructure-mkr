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
                    "MOCK_SERIAL_NUMBER",
                    "ICListenHF"),
          status(status),
          loggingConfig(loggingConfig),
          streamingConfig(streamingConfig),
          recordingStats(recordingStats) {}

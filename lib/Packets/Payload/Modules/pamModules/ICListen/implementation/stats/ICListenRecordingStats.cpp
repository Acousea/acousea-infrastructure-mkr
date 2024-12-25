#include "ICListenRecordingStats.h"


ICListenRecordingStats::ICListenRecordingStats(std::time_t epochTime, int numberOfClicks, int recordedMinutes,
                                               int numberOfFiles)
        : SerializableModule(ModuleCode::TYPES::ICLISTEN_RECORDING_STATS, serializeValues(epochTime, numberOfClicks, recordedMinutes, numberOfFiles)),
          epochTime(epochTime), numberOfClicks(numberOfClicks), recordedMinutes(recordedMinutes), numberOfFiles(numberOfFiles) {}

ICListenRecordingStats ICListenRecordingStats::createDefault() {
    return ICListenRecordingStats(std::time(nullptr), 0, 0, 0);
}

std::vector<uint8_t>
ICListenRecordingStats::serializeValues(std::time_t epochTime, int numberOfClicks, int recordedMinutes,
                                        int numberOfFiles) {
    std::vector<uint8_t> value;

    value.insert(value.end(), reinterpret_cast<const uint8_t*>(&epochTime), reinterpret_cast<const uint8_t*>(&epochTime) + sizeof(epochTime));
    value.push_back(static_cast<uint8_t>(numberOfClicks));
    value.push_back(static_cast<uint8_t>(recordedMinutes));
    value.push_back(static_cast<uint8_t>(numberOfFiles));

    return value;
}

ICListenRecordingStats ICListenRecordingStats::from(const std::vector<uint8_t> &data) {
    if (data.size() < 11) {
        ErrorHandler::handleError("Invalid data size for ICListenRecordingStats");
//        throw std::invalid_argument("Invalid data size for ICListenRecordingStats");
    }

    std::time_t epochTime = *reinterpret_cast<const std::time_t*>(&data[0]);
    int numberOfClicks = data[8];
    int recordedMinutes = data[9];
    int numberOfFiles = data[10];

    return {epochTime, numberOfClicks, recordedMinutes, numberOfFiles};
}

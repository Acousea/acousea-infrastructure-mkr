#ifndef ACOUSEA_MKR1310_NODES_ICLISTENRECORDINGSTATS_H
#define ACOUSEA_MKR1310_NODES_ICLISTENRECORDINGSTATS_H

#include "Payload/Modules/SerializableModule.h"

class ICListenRecordingStats : public SerializableModule {
public:
    ICListenRecordingStats(std::time_t epochTime, int numberOfClicks, int recordedMinutes, int numberOfFiles);

    static ICListenRecordingStats createDefault();

public:
    const std::time_t epochTime;
    const int numberOfClicks;
    const int recordedMinutes;
    const int numberOfFiles;

    static std::vector<uint8_t>
    serializeValues(std::time_t epochTime, int numberOfClicks, int recordedMinutes, int numberOfFiles);

    static ICListenRecordingStats from(const std::vector<uint8_t> &data);
};


#endif //ACOUSEA_MKR1310_NODES_ICLISTENRECORDINGSTATS_H

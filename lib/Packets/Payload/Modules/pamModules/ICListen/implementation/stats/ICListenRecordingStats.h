#ifndef ACOUSEA_MKR1310_NODES_ICLISTENRECORDINGSTATS_H
#define ACOUSEA_MKR1310_NODES_ICLISTENRECORDINGSTATS_H

#include "Payload/Modules/SerializableModule.h"
#include <ctime>
#include <vector>
#include <cstdint>

class ICListenRecordingStats : public SerializableModule {
public:
    // Constructor estándar
    ICListenRecordingStats(std::time_t epochTime, int numberOfClicks, int recordedMinutes, int numberOfFiles);

    // Constructor de movimiento
    ICListenRecordingStats(ICListenRecordingStats &&other) noexcept;

    // Operador de asignación por movimiento
    ICListenRecordingStats &operator=(ICListenRecordingStats &&other) noexcept;

    // Constructor y operador de copia eliminados
    ICListenRecordingStats(const ICListenRecordingStats &other) noexcept;

    ICListenRecordingStats &operator=(const ICListenRecordingStats &other) noexcept;

    // Métodos estáticos
    static ICListenRecordingStats createDefault();

    static ICListenRecordingStats fromBytes(const std::vector<uint8_t> &data);

private:
    static std::vector<uint8_t> serializeValues(std::time_t epochTime, int numberOfClicks, int recordedMinutes,
                                                int numberOfFiles);

public:
    const std::time_t epochTime;
    const int numberOfClicks;
    const int recordedMinutes;
    const int numberOfFiles;
};

#endif // ACOUSEA_MKR1310_NODES_ICLISTENRECORDINGSTATS_H

#include "ICListenRecordingStats.h"

// Constructor estándar
ICListenRecordingStats::ICListenRecordingStats(std::time_t epochTime, int numberOfClicks, int recordedMinutes,
                                               int numberOfFiles)
    : SerializableModule(ModuleCode::TYPES::ICLISTEN_RECORDING_STATS,
                         serializeValues(epochTime, numberOfClicks, recordedMinutes, numberOfFiles)),
      epochTime(epochTime), numberOfClicks(numberOfClicks), recordedMinutes(recordedMinutes),
      numberOfFiles(numberOfFiles) {
}

// Constructor de movimiento
ICListenRecordingStats::ICListenRecordingStats(ICListenRecordingStats &&other) noexcept
    : SerializableModule(std::move(other)),
      epochTime(other.epochTime), numberOfClicks(other.numberOfClicks),
      recordedMinutes(other.recordedMinutes), numberOfFiles(other.numberOfFiles) {
}

// Operador de asignación por movimiento
ICListenRecordingStats &ICListenRecordingStats::operator=(ICListenRecordingStats &&other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(std::move(other));
        const_cast<std::time_t &>(epochTime) = other.epochTime;
        const_cast<int &>(numberOfClicks) = other.numberOfClicks;
        const_cast<int &>(recordedMinutes) = other.recordedMinutes;
        const_cast<int &>(numberOfFiles) = other.numberOfFiles;
    }
    return *this;
}

// Copy constructor
ICListenRecordingStats::ICListenRecordingStats(const ICListenRecordingStats &other) noexcept
    : SerializableModule(other),
      epochTime(other.epochTime), numberOfClicks(other.numberOfClicks),
      recordedMinutes(other.recordedMinutes), numberOfFiles(other.numberOfFiles) {
}

// Copy assignment operator
ICListenRecordingStats &ICListenRecordingStats::operator=(const ICListenRecordingStats &other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(other);
        const_cast<std::time_t &>(epochTime) = other.epochTime;
        const_cast<int &>(numberOfClicks) = other.numberOfClicks;
        const_cast<int &>(recordedMinutes) = other.recordedMinutes;
        const_cast<int &>(numberOfFiles) = other.numberOfFiles;
    }
    return *this;
}

// Crear configuración predeterminada
ICListenRecordingStats ICListenRecordingStats::createDefault() {
    return ICListenRecordingStats(std::time(nullptr), 0, 0, 0);
}

// Serializar valores
std::vector<uint8_t>
ICListenRecordingStats::serializeValues(std::time_t epochTime, int numberOfClicks, int recordedMinutes,
                                        int numberOfFiles) {
    std::vector<uint8_t> value;

    // Serializar epochTime (8 bytes)
    const auto *epochTimeBytes = reinterpret_cast<const uint8_t *>(&epochTime);
    value.insert(value.end(), epochTimeBytes, epochTimeBytes + sizeof(epochTime));

    // Serializar campos de 1 byte
    value.push_back(static_cast<uint8_t>(numberOfClicks));
    value.push_back(static_cast<uint8_t>(recordedMinutes));
    value.push_back(static_cast<uint8_t>(numberOfFiles));

    return value;
}

// Deserializar desde un vector
ICListenRecordingStats ICListenRecordingStats::fromBytes(const std::vector<uint8_t> &data) {
    if (data.size() < 11) {
        // Tamaño mínimo requerido
        ErrorHandler::handleError("Invalid data size for ICListenRecordingStats");
    }

    // Deserializar epochTime (8 bytes)
    std::time_t epochTime = *reinterpret_cast<const std::time_t *>(&data[0]);

    // Deserializar campos de 1 byte
    int numberOfClicks = data[8];
    int recordedMinutes = data[9];
    int numberOfFiles = data[10];

    return ICListenRecordingStats(epochTime, numberOfClicks, recordedMinutes, numberOfFiles);
}

std::string ICListenRecordingStats::toString() const {
    return "ICListenRecordingStats {"
           "epochTime: " + std::to_string(epochTime) +
           ", numberOfClicks: " + std::to_string(numberOfClicks) +
           ", recordedMinutes: " + std::to_string(recordedMinutes) +
           ", numberOfFiles: " + std::to_string(numberOfFiles) +
           "}";
}

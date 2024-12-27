#include "ICListenStreamingConfig.h"

// Constructor estándar
ICListenStreamingConfig::ICListenStreamingConfig(bool recordWaveform, bool processWaveform, int waveformProcessingType,
                                                 int waveformInterval, int waveformDuration, bool recordFFT,
                                                 bool processFFT, int fftProcessingType, int fftInterval,
                                                 int fftDuration, std::time_t timestamp)
        : SerializableModule(ModuleCode::TYPES::ICLISTEN_STREAMING_CONFIG,
                             serializeValues(recordWaveform, processWaveform, waveformProcessingType,
                                             waveformInterval, waveformDuration, recordFFT, processFFT,
                                             fftProcessingType, fftInterval, fftDuration, timestamp)),
          recordWaveform(recordWaveform), processWaveform(processWaveform),
          waveformProcessingType(waveformProcessingType), waveformInterval(waveformInterval),
          waveformDuration(waveformDuration), recordFFT(recordFFT), processFFT(processFFT),
          fftProcessingType(fftProcessingType), fftInterval(fftInterval), fftDuration(fftDuration),
          timestamp(timestamp) {}

// Constructor de movimiento
ICListenStreamingConfig::ICListenStreamingConfig(ICListenStreamingConfig&& other) noexcept
        : SerializableModule(std::move(other)),
          recordWaveform(other.recordWaveform), processWaveform(other.processWaveform),
          waveformProcessingType(other.waveformProcessingType), waveformInterval(other.waveformInterval),
          waveformDuration(other.waveformDuration), recordFFT(other.recordFFT), processFFT(other.processFFT),
          fftProcessingType(other.fftProcessingType), fftInterval(other.fftInterval),
          fftDuration(other.fftDuration), timestamp(other.timestamp) {}

// Operador de asignación por movimiento
ICListenStreamingConfig& ICListenStreamingConfig::operator=(ICListenStreamingConfig&& other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(std::move(other));
        const_cast<bool&>(recordWaveform) = other.recordWaveform;
        const_cast<bool&>(processWaveform) = other.processWaveform;
        const_cast<int&>(waveformProcessingType) = other.waveformProcessingType;
        const_cast<int&>(waveformInterval) = other.waveformInterval;
        const_cast<int&>(waveformDuration) = other.waveformDuration;
        const_cast<bool&>(recordFFT) = other.recordFFT;
        const_cast<bool&>(processFFT) = other.processFFT;
        const_cast<int&>(fftProcessingType) = other.fftProcessingType;
        const_cast<int&>(fftInterval) = other.fftInterval;
        const_cast<int&>(fftDuration) = other.fftDuration;
        const_cast<std::time_t&>(timestamp) = other.timestamp;
    }
    return *this;
}

// Copy constructor
ICListenStreamingConfig::ICListenStreamingConfig(const ICListenStreamingConfig &other) noexcept
        : SerializableModule(other),
          recordWaveform(other.recordWaveform), processWaveform(other.processWaveform),
          waveformProcessingType(other.waveformProcessingType), waveformInterval(other.waveformInterval),
          waveformDuration(other.waveformDuration), recordFFT(other.recordFFT), processFFT(other.processFFT),
          fftProcessingType(other.fftProcessingType), fftInterval(other.fftInterval),
          fftDuration(other.fftDuration), timestamp(other.timestamp) {}

// Copy assignment operator
ICListenStreamingConfig& ICListenStreamingConfig::operator=(const ICListenStreamingConfig &other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(other);
        const_cast<bool&>(recordWaveform) = other.recordWaveform;
        const_cast<bool&>(processWaveform) = other.processWaveform;
        const_cast<int&>(waveformProcessingType) = other.waveformProcessingType;
        const_cast<int&>(waveformInterval) = other.waveformInterval;
        const_cast<int&>(waveformDuration) = other.waveformDuration;
        const_cast<bool&>(recordFFT) = other.recordFFT;
        const_cast<bool&>(processFFT) = other.processFFT;
        const_cast<int&>(fftProcessingType) = other.fftProcessingType;
        const_cast<int&>(fftInterval) = other.fftInterval;
        const_cast<int&>(fftDuration) = other.fftDuration;
        const_cast<std::time_t&>(timestamp) = other.timestamp;
    }
    return *this;
}


// Crear configuración predeterminada
ICListenStreamingConfig ICListenStreamingConfig::createDefault() {
    return ICListenStreamingConfig(false, false, 0, 0, 0, false, false, 0, 0, 0, std::time(nullptr));
}

// Serializar valores
std::vector<uint8_t>
ICListenStreamingConfig::serializeValues(bool recordWaveform, bool processWaveform, int waveformProcessingType,
                                         int waveformInterval, int waveformDuration, bool recordFFT, bool processFFT,
                                         int fftProcessingType, int fftInterval, int fftDuration,
                                         std::time_t timestamp) {
    std::vector<uint8_t> value;

    // Serializar campos de 1 byte
    value.push_back(static_cast<uint8_t>(recordWaveform));
    value.push_back(static_cast<uint8_t>(processWaveform));

    // Serializar waveformProcessingType (4 bytes)
    value.push_back(static_cast<uint8_t>(waveformProcessingType >> 24));
    value.push_back(static_cast<uint8_t>(waveformProcessingType >> 16));
    value.push_back(static_cast<uint8_t>(waveformProcessingType >> 8));
    value.push_back(static_cast<uint8_t>(waveformProcessingType & 0xFF));

    // Serializar waveformInterval y waveformDuration (4 bytes cada uno)
    value.push_back(static_cast<uint8_t>(waveformInterval >> 24));
    value.push_back(static_cast<uint8_t>(waveformInterval >> 16));
    value.push_back(static_cast<uint8_t>(waveformInterval >> 8));
    value.push_back(static_cast<uint8_t>(waveformInterval & 0xFF));

    value.push_back(static_cast<uint8_t>(waveformDuration >> 24));
    value.push_back(static_cast<uint8_t>(waveformDuration >> 16));
    value.push_back(static_cast<uint8_t>(waveformDuration >> 8));
    value.push_back(static_cast<uint8_t>(waveformDuration & 0xFF));

    // Serializar campos de 1 byte
    value.push_back(static_cast<uint8_t>(recordFFT));
    value.push_back(static_cast<uint8_t>(processFFT));

    // Serializar fftProcessingType, fftInterval, fftDuration (4 bytes cada uno)
    value.push_back(static_cast<uint8_t>(fftProcessingType >> 24));
    value.push_back(static_cast<uint8_t>(fftProcessingType >> 16));
    value.push_back(static_cast<uint8_t>(fftProcessingType >> 8));
    value.push_back(static_cast<uint8_t>(fftProcessingType & 0xFF));

    value.push_back(static_cast<uint8_t>(fftInterval >> 24));
    value.push_back(static_cast<uint8_t>(fftInterval >> 16));
    value.push_back(static_cast<uint8_t>(fftInterval >> 8));
    value.push_back(static_cast<uint8_t>(fftInterval & 0xFF));

    value.push_back(static_cast<uint8_t>(fftDuration >> 24));
    value.push_back(static_cast<uint8_t>(fftDuration >> 16));
    value.push_back(static_cast<uint8_t>(fftDuration >> 8));
    value.push_back(static_cast<uint8_t>(fftDuration & 0xFF));

    // Serializar timestamp (8 bytes)
    const auto* timestampBytes = reinterpret_cast<const uint8_t*>(&timestamp);
    value.insert(value.end(), timestampBytes, timestampBytes + sizeof(std::time_t));

    return value;
}

// Deserializar desde un vector
ICListenStreamingConfig ICListenStreamingConfig::fromBytes(const std::vector<uint8_t>& data) {
    if (data.size() < 28) { // Tamaño mínimo requerido
        ErrorHandler::handleError("Invalid data size for ICListenStreamingConfig");
    }

    bool recordWaveform = static_cast<bool>(data[0]);
    bool processWaveform = static_cast<bool>(data[1]);

    int waveformProcessingType = (data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5];
    int waveformInterval = (data[6] << 24) | (data[7] << 16) | (data[8] << 8) | data[9];
    int waveformDuration = (data[10] << 24) | (data[11] << 16) | (data[12] << 8) | data[13];

    bool recordFFT = static_cast<bool>(data[14]);
    bool processFFT = static_cast<bool>(data[15]);

    int fftProcessingType = (data[16] << 24) | (data[17] << 16) | (data[18] << 8) | data[19];
    int fftInterval = (data[20] << 24) | (data[21] << 16) | (data[22] << 8) | data[23];
    int fftDuration = (data[24] << 24) | (data[25] << 16) | (data[26] << 8) | data[27];

    std::time_t timestamp = *reinterpret_cast<const std::time_t*>(&data[28]);

    return ICListenStreamingConfig(recordWaveform, processWaveform, waveformProcessingType, waveformInterval,
                                   waveformDuration, recordFFT, processFFT, fftProcessingType, fftInterval,
                                   fftDuration, timestamp);
}

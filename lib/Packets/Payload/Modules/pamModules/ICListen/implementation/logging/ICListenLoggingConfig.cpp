#include "ICListenLoggingConfig.h"

// Constructor estándar
ICListenLoggingConfig::ICListenLoggingConfig(int gain, int waveformSampleRate, int waveformLoggingMode,
                                             int waveformLogLength, int bitDepth, int fftSampleRate,
                                             int fftProcessingType, int fftsAccumulated, int fftLoggingMode,
                                             int fftLogLength)
        : SerializableModule(ModuleCode::TYPES::ICLISTEN_LOGGING_CONFIG,
                             serializeValues(gain, waveformSampleRate, waveformLoggingMode,
                                             waveformLogLength, bitDepth, fftSampleRate, fftProcessingType,
                                             fftsAccumulated, fftLoggingMode, fftLogLength)),
          gain(gain), waveformSampleRate(waveformSampleRate), waveformLoggingMode(waveformLoggingMode),
          waveformLogLength(waveformLogLength), bitDepth(bitDepth), fftSampleRate(fftSampleRate),
          fftProcessingType(fftProcessingType), fftsAccumulated(fftsAccumulated), fftLoggingMode(fftLoggingMode),
          fftLogLength(fftLogLength) {}

// Constructor de movimiento
ICListenLoggingConfig::ICListenLoggingConfig(ICListenLoggingConfig&& other) noexcept
        : SerializableModule(std::move(other)),
          gain(other.gain), waveformSampleRate(other.waveformSampleRate),
          waveformLoggingMode(other.waveformLoggingMode), waveformLogLength(other.waveformLogLength),
          bitDepth(other.bitDepth), fftSampleRate(other.fftSampleRate),
          fftProcessingType(other.fftProcessingType), fftsAccumulated(other.fftsAccumulated),
          fftLoggingMode(other.fftLoggingMode), fftLogLength(other.fftLogLength) {}

// Operador de asignación por movimiento
ICListenLoggingConfig& ICListenLoggingConfig::operator=(ICListenLoggingConfig&& other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(std::move(other));
        const_cast<int&>(gain) = other.gain;
        const_cast<int&>(waveformSampleRate) = other.waveformSampleRate;
        const_cast<int&>(waveformLoggingMode) = other.waveformLoggingMode;
        const_cast<int&>(waveformLogLength) = other.waveformLogLength;
        const_cast<int&>(bitDepth) = other.bitDepth;
        const_cast<int&>(fftSampleRate) = other.fftSampleRate;
        const_cast<int&>(fftProcessingType) = other.fftProcessingType;
        const_cast<int&>(fftsAccumulated) = other.fftsAccumulated;
        const_cast<int&>(fftLoggingMode) = other.fftLoggingMode;
        const_cast<int&>(fftLogLength) = other.fftLogLength;
    }
    return *this;
}

// Copy constructor
ICListenLoggingConfig::ICListenLoggingConfig(const ICListenLoggingConfig &other) noexcept
        : SerializableModule(other),
          gain(other.gain), waveformSampleRate(other.waveformSampleRate),
          waveformLoggingMode(other.waveformLoggingMode), waveformLogLength(other.waveformLogLength),
          bitDepth(other.bitDepth), fftSampleRate(other.fftSampleRate),
          fftProcessingType(other.fftProcessingType), fftsAccumulated(other.fftsAccumulated),
          fftLoggingMode(other.fftLoggingMode), fftLogLength(other.fftLogLength) {}

// Copy assignment operator
ICListenLoggingConfig& ICListenLoggingConfig::operator=(const ICListenLoggingConfig &other) noexcept {
    if (this != &other) {
        SerializableModule::operator=(other);
        const_cast<int&>(gain) = other.gain;
        const_cast<int&>(waveformSampleRate) = other.waveformSampleRate;
        const_cast<int&>(waveformLoggingMode) = other.waveformLoggingMode;
        const_cast<int&>(waveformLogLength) = other.waveformLogLength;
        const_cast<int&>(bitDepth) = other.bitDepth;
        const_cast<int&>(fftSampleRate) = other.fftSampleRate;
        const_cast<int&>(fftProcessingType) = other.fftProcessingType;
        const_cast<int&>(fftsAccumulated) = other.fftsAccumulated;
        const_cast<int&>(fftLoggingMode) = other.fftLoggingMode;
        const_cast<int&>(fftLogLength) = other.fftLogLength;
    }
    return *this;
}

// Crear configuración predeterminada
ICListenLoggingConfig ICListenLoggingConfig::createDefault() {
    return ICListenLoggingConfig(0, 1000, 0, 0, 0, 1000, 0, 0, 0, 0);
}

// Serializar valores
std::vector<uint8_t>
ICListenLoggingConfig::serializeValues(int gain, int waveformSampleRate, int waveformLoggingMode,
                                       int waveformLogLength, int bitDepth, int fftSampleRate,
                                       int fftProcessingType, int fftsAccumulated, int fftLoggingMode,
                                       int fftLogLength) {
    std::vector<uint8_t> value;

    value.push_back(static_cast<uint8_t>(gain >> 8)); // High byte
    value.push_back(static_cast<uint8_t>(gain & 0xFF)); // Low byte

    value.push_back(static_cast<uint8_t>(waveformSampleRate >> 24));
    value.push_back(static_cast<uint8_t>(waveformSampleRate >> 16));
    value.push_back(static_cast<uint8_t>(waveformSampleRate >> 8));
    value.push_back(static_cast<uint8_t>(waveformSampleRate & 0xFF));

    value.push_back(static_cast<uint8_t>(waveformLoggingMode));
    value.push_back(static_cast<uint8_t>(waveformLogLength));
    value.push_back(static_cast<uint8_t>(bitDepth));

    value.push_back(static_cast<uint8_t>(fftSampleRate >> 24));
    value.push_back(static_cast<uint8_t>(fftSampleRate >> 16));
    value.push_back(static_cast<uint8_t>(fftSampleRate >> 8));
    value.push_back(static_cast<uint8_t>(fftSampleRate & 0xFF));

    value.push_back(static_cast<uint8_t>(fftProcessingType >> 8));
    value.push_back(static_cast<uint8_t>(fftProcessingType & 0xFF));

    value.push_back(static_cast<uint8_t>(fftsAccumulated >> 8));
    value.push_back(static_cast<uint8_t>(fftsAccumulated & 0xFF));

    value.push_back(static_cast<uint8_t>(fftLoggingMode));
    value.push_back(static_cast<uint8_t>(fftLogLength));

    return value;
}

// Deserializar desde un vector
ICListenLoggingConfig ICListenLoggingConfig::fromBytes(const std::vector<uint8_t>& data) {
    if (data.size() < 18) {
        ErrorHandler::handleError("Invalid data size for ICListenLoggingConfig");
    }

    int gain = (data[0] << 8) | data[1];
    int waveformSampleRate = (data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5];
    int waveformLoggingMode = data[6];
    int waveformLogLength = data[7];
    int bitDepth = data[8];
    int fftSampleRate = (data[9] << 24) | (data[10] << 16) | (data[11] << 8) | data[12];
    int fftProcessingType = (data[13] << 8) | data[14];
    int fftsAccumulated = (data[15] << 8) | data[16];
    int fftLoggingMode = data[17];
    int fftLogLength = data[18];

    return ICListenLoggingConfig(gain, waveformSampleRate, waveformLoggingMode, waveformLogLength,
                                 bitDepth, fftSampleRate, fftProcessingType, fftsAccumulated,
                                 fftLoggingMode, fftLogLength);
}

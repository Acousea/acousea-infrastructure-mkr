#include "ICListenLoggingConfig.h"


ICListenLoggingConfig::ICListenLoggingConfig(int gain, int waveformSampleRate, int waveformLoggingMode,
                                             int waveformLogLength, int bitDepth, int fftSampleRate,
                                             int fftProcessingType, int fftsAccumulated, int fftLoggingMode,
                                             int fftLogLength)
        : SerializableModule(ModuleCode::TYPES::ICLISTEN_LOGGING,
                             serializeValues(gain, waveformSampleRate, waveformLoggingMode,
                                             waveformLogLength, bitDepth, fftSampleRate, fftProcessingType,
                                             fftsAccumulated, fftLoggingMode, fftLogLength)),
          gain(gain), waveformSampleRate(waveformSampleRate), waveformLoggingMode(waveformLoggingMode),
          waveformLogLength(waveformLogLength), bitDepth(bitDepth), fftSampleRate(fftSampleRate),
          fftProcessingType(fftProcessingType), fftsAccumulated(fftsAccumulated), fftLoggingMode(fftLoggingMode),
          fftLogLength(fftLogLength) {}

ICListenLoggingConfig ICListenLoggingConfig::createDefault() {
    return ICListenLoggingConfig(0, 1000, 0, 0, 0, 1000, 0, 0, 0, 0);
}

std::vector<uint8_t>
ICListenLoggingConfig::serializeValues(uint16_t gain, int waveformSampleRate, uint8_t waveformLoggingMode,
                                       uint8_t waveformLogLength, uint8_t bitDepth, int fftSampleRate,
                                       uint16_t fftProcessingType, uint16_t fftsAccumulated, uint8_t fftLoggingMode,
                                       uint8_t fftLogLength) {
    std::vector<uint8_t> value;

    // Serializar gain (2 bytes)
    value.push_back(static_cast<uint8_t>(gain >> 8)); // High byte
    value.push_back(static_cast<uint8_t>(gain & 0xFF)); // Low byte

    // Serializar waveformSampleRate (4 bytes)
    value.push_back(static_cast<uint8_t>((waveformSampleRate >> 24) & 0xFF));
    value.push_back(static_cast<uint8_t>((waveformSampleRate >> 16) & 0xFF));
    value.push_back(static_cast<uint8_t>((waveformSampleRate >> 8) & 0xFF));
    value.push_back(static_cast<uint8_t>(waveformSampleRate & 0xFF));

    // Serializar campos de 1 byte
    value.push_back(waveformLoggingMode);
    value.push_back(waveformLogLength);
    value.push_back(bitDepth);

    // Serializar fftSampleRate (4 bytes)
    value.push_back(static_cast<uint8_t>((fftSampleRate >> 24) & 0xFF));
    value.push_back(static_cast<uint8_t>((fftSampleRate >> 16) & 0xFF));
    value.push_back(static_cast<uint8_t>((fftSampleRate >> 8) & 0xFF));
    value.push_back(static_cast<uint8_t>(fftSampleRate & 0xFF));

    // Serializar fftProcessingType (2 bytes)
    value.push_back(static_cast<uint8_t>(fftProcessingType >> 8)); // High byte
    value.push_back(static_cast<uint8_t>(fftProcessingType & 0xFF)); // Low byte

    // Serializar fftsAccumulated (2 bytes)
    value.push_back(static_cast<uint8_t>(fftsAccumulated >> 8)); // High byte
    value.push_back(static_cast<uint8_t>(fftsAccumulated & 0xFF)); // Low byte

    // Serializar campos de 1 byte
    value.push_back(fftLoggingMode);
    value.push_back(fftLogLength);

    return value;
}

ICListenLoggingConfig ICListenLoggingConfig::from(const std::vector<uint8_t> &data) {
    if (data.size() < 18) { // Tamaño mínimo requerido
        ErrorHandler::handleError("Invalid data size for ICListenLoggingConfig");
//        throw std::invalid_argument("Invalid data size for ICListenLoggingConfig");
    }

    // Deserializar gain (2 bytes)
    uint16_t gain = (data[0] << 8) | data[1];

    // Deserializar waveformSampleRate (4 bytes)
    int waveformSampleRate = (data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5];

    // Deserializar campos de 1 byte
    uint8_t waveformLoggingMode = data[6];
    uint8_t waveformLogLength = data[7];
    uint8_t bitDepth = data[8];

    // Deserializar fftSampleRate (4 bytes)
    int fftSampleRate = (data[9] << 24) | (data[10] << 16) | (data[11] << 8) | data[12];

    // Deserializar fftProcessingType (2 bytes)
    uint16_t fftProcessingType = (data[13] << 8) | data[14];

    // Deserializar fftsAccumulated (2 bytes)
    uint16_t fftsAccumulated = (data[15] << 8) | data[16];

    // Deserializar campos de 1 byte
    uint8_t fftLoggingMode = data[17];
    uint8_t fftLogLength = data[18];

    return ICListenLoggingConfig(gain, waveformSampleRate, waveformLoggingMode, waveformLogLength, bitDepth,
                                 fftSampleRate, fftProcessingType, fftsAccumulated, fftLoggingMode, fftLogLength);
}

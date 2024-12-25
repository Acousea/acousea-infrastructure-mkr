#ifndef ACOUSEA_MKR1310_NODES_ICLISTENLOGIGNGCONFIG_H
#define ACOUSEA_MKR1310_NODES_ICLISTENLOGIGNGCONFIG_H


#include "../../../../SerializableModule.h"

class ICListenLoggingConfig : public SerializableModule {
public:
    ICListenLoggingConfig(int gain, int waveformSampleRate, int waveformLoggingMode,
                          int waveformLogLength, int bitDepth, int fftSampleRate, int fftProcessingType,
                          int fftsAccumulated, int fftLoggingMode, int fftLogLength);

    static ICListenLoggingConfig createDefault();

    static ICListenLoggingConfig from(const std::vector<uint8_t> &data);

public:
    const int gain;
    const int waveformSampleRate;
    const int waveformLoggingMode;
    const int waveformLogLength;
    const int bitDepth;
    const int fftSampleRate;
    const int fftProcessingType;
    const int fftsAccumulated;
    const int fftLoggingMode;
    const int fftLogLength;

private:

    std::vector<uint8_t>
    serializeValues(uint16_t gain, int waveformSampleRate, uint8_t waveformLoggingMode, uint8_t waveformLogLength,
                    uint8_t bitDepth, int fftSampleRate, uint16_t fftProcessingType, uint16_t fftsAccumulated,
                    uint8_t fftLoggingMode, uint8_t fftLogLength);
};

#endif //ACOUSEA_MKR1310_NODES_ICLISTENLOGIGNGCONFIG_H

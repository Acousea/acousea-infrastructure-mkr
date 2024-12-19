#ifndef ACOUSEA_MKR1310_NODES_ICLISTENLOGIGNGCONFIG_H
#define ACOUSEA_MKR1310_NODES_ICLISTENLOGIGNGCONFIG_H


#include "Payload/Modules/SerializableModule.h"

class ICListenLoggingConfig : public SerializableModule {
public:
    ICListenLoggingConfig(int gain, int waveformSampleRate, int waveformLoggingMode,
                          int waveformLogLength, int bitDepth, int fftSampleRate, int fftProcessingType,
                          int fftsAccumulated, int fftLoggingMode, int fftLogLength);

    static ICListenLoggingConfig createDefault();

private:
    int gain;
    int waveformSampleRate;
    int waveformLoggingMode;
    int waveformLogLength;
    int bitDepth;
    int fftSampleRate;
    int fftProcessingType;
    int fftsAccumulated;
    int fftLoggingMode;
    int fftLogLength;

    static std::vector<uint8_t> serializeValues(int gain, int waveformSampleRate, int waveformLoggingMode,
                                                int waveformLogLength, int bitDepth, int fftSampleRate,
                                                int fftProcessingType,
                                                int fftsAccumulated, int fftLoggingMode, int fftLogLength) {
        std::vector<uint8_t> value;
        value.push_back(static_cast<uint8_t>(gain));
        value.push_back(static_cast<uint8_t>(waveformSampleRate));
        value.push_back(static_cast<uint8_t>(waveformLoggingMode));
        value.push_back(static_cast<uint8_t>(waveformLogLength));
        value.push_back(static_cast<uint8_t>(bitDepth));
        value.push_back(static_cast<uint8_t>(fftSampleRate));
        value.push_back(static_cast<uint8_t>(fftProcessingType));
        value.push_back(static_cast<uint8_t>(fftsAccumulated));
        value.push_back(static_cast<uint8_t>(fftLoggingMode));
        value.push_back(static_cast<uint8_t>(fftLogLength));
        return value;
    }
};

#endif //ACOUSEA_MKR1310_NODES_ICLISTENLOGIGNGCONFIG_H

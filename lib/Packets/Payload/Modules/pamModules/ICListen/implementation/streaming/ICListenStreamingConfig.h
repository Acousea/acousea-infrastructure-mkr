#ifndef ACOUSEA_MKR1310_NODES_ICLISTENSTREAMINGCONFIG_H
#define ACOUSEA_MKR1310_NODES_ICLISTENSTREAMINGCONFIG_H


#include "../../../../SerializableModule.h"

class ICListenStreamingConfig : public SerializableModule {
public:
    ICListenStreamingConfig(bool recordWaveform, bool processWaveform, int waveformProcessingType, int waveformInterval,
                            int waveformDuration, bool recordFFT, bool processFFT, int fftProcessingType,
                            int fftInterval, int fftDuration, std::time_t timestamp);

    static ICListenStreamingConfig createDefault();

public:
    const bool recordWaveform;
    const bool processWaveform;
    const int waveformProcessingType;
    const int waveformInterval;
    const int waveformDuration;
    const bool recordFFT;
    const bool processFFT;
    const int fftProcessingType;
    const int fftInterval;
    const int fftDuration;
    const std::time_t timestamp;

    static std::vector<uint8_t> serializeValues(bool recordWaveform, bool processWaveform, int waveformProcessingType,
                                                int waveformInterval, int waveformDuration, bool recordFFT,
                                                bool processFFT,
                                                int fftProcessingType, int fftInterval, int fftDuration,
                                                std::time_t timestamp);

    static ICListenStreamingConfig from(const std::vector<uint8_t> &data);
};


#endif //ACOUSEA_MKR1310_NODES_ICLISTENSTREAMINGCONFIG_H

#ifndef ACOUSEA_MKR1310_NODES_ICLISTENSTREAMINGCONFIG_H
#define ACOUSEA_MKR1310_NODES_ICLISTENSTREAMINGCONFIG_H

#include "Payload/Modules/SerializableModule.h"

class ICListenStreamingConfig : public SerializableModule {
public:
    ICListenStreamingConfig(bool recordWaveform, bool processWaveform, int waveformProcessingType, int waveformInterval,
                            int waveformDuration, bool recordFFT, bool processFFT, int fftProcessingType,
                            int fftInterval,
                            int fftDuration, std::time_t timestamp);

    static ICListenStreamingConfig createDefault();

private:
    bool recordWaveform;
    bool processWaveform;
    int waveformProcessingType;
    int waveformInterval;
    int waveformDuration;
    bool recordFFT;
    bool processFFT;
    int fftProcessingType;
    int fftInterval;
    int fftDuration;
    std::time_t timestamp;

    static std::vector<uint8_t> serializeValues(bool recordWaveform, bool processWaveform, int waveformProcessingType,
                                                int waveformInterval, int waveformDuration, bool recordFFT,
                                                bool processFFT,
                                                int fftProcessingType, int fftInterval, int fftDuration,
                                                std::time_t timestamp);
};


#endif //ACOUSEA_MKR1310_NODES_ICLISTENSTREAMINGCONFIG_H

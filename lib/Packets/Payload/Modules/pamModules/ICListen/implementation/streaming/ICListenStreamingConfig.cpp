#include "ICListenStreamingConfig.h"

ICListenStreamingConfig::ICListenStreamingConfig(bool recordWaveform, bool processWaveform, int waveformProcessingType,
                                                 int waveformInterval, int waveformDuration, bool recordFFT,
                                                 bool processFFT, int fftProcessingType, int fftInterval,
                                                 int fftDuration, std::time_t timestamp)
        : SerializableModule(ModuleCode::TYPES::ICLISTEN_STREAMING_CONFIG,
                             serializeValues(recordWaveform, processWaveform,
                                             waveformProcessingType, waveformInterval,
                                             waveformDuration, recordFFT, processFFT,
                                             fftProcessingType, fftInterval, fftDuration,
                                             timestamp)),
          recordWaveform(recordWaveform), processWaveform(processWaveform),
          waveformProcessingType(waveformProcessingType),
          waveformInterval(waveformInterval), waveformDuration(waveformDuration), recordFFT(recordFFT),
          processFFT(processFFT),
          fftProcessingType(fftProcessingType), fftInterval(fftInterval), fftDuration(fftDuration),
          timestamp(timestamp) {}

ICListenStreamingConfig ICListenStreamingConfig::createDefault() {
    return ICListenStreamingConfig(false, false, 0, 0, 0, false, false, 0, 0, 0, std::time(nullptr));
}

std::vector<uint8_t>
ICListenStreamingConfig::serializeValues(bool recordWaveform, bool processWaveform, int waveformProcessingType,
                                         int waveformInterval, int waveformDuration, bool recordFFT, bool processFFT,
                                         int fftProcessingType, int fftInterval, int fftDuration, std::time_t timestamp) {
    std::vector<uint8_t> value;

    value.push_back(recordWaveform ? 1 : 0);
    value.push_back(processWaveform ? 1 : 0);
    value.push_back(static_cast<uint8_t>(waveformProcessingType));
    value.push_back(static_cast<uint8_t>(waveformInterval));
    value.push_back(static_cast<uint8_t>(waveformDuration));
    value.push_back(recordFFT ? 1 : 0);
    value.push_back(processFFT ? 1 : 0);
    value.push_back(static_cast<uint8_t>(fftProcessingType));
    value.push_back(static_cast<uint8_t>(fftInterval));
    value.push_back(static_cast<uint8_t>(fftDuration));
    value.insert(value.end(), reinterpret_cast<const uint8_t *>(&timestamp),
                 reinterpret_cast<const uint8_t *>(&timestamp) + sizeof(timestamp));

    return value;
}

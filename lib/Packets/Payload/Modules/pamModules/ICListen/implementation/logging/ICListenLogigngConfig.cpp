#include "ICListenLogigngConfig.h"

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

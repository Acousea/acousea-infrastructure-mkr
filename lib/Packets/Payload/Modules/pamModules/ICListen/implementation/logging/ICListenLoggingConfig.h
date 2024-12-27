#ifndef ACOUSEA_MKR1310_NODES_ICLISTENLOGGINGCONFIG_H
#define ACOUSEA_MKR1310_NODES_ICLISTENLOGGINGCONFIG_H

#include "../../../../SerializableModule.h"

class ICListenLoggingConfig : public SerializableModule {
public:
    // Constructor estándar
    ICListenLoggingConfig(int gain, int waveformSampleRate, int waveformLoggingMode,
                          int waveformLogLength, int bitDepth, int fftSampleRate, int fftProcessingType,
                          int fftsAccumulated, int fftLoggingMode, int fftLogLength);

    // Constructor de movimiento
    ICListenLoggingConfig(ICListenLoggingConfig&& other) noexcept;

    // Operador de asignación por movimiento
    ICListenLoggingConfig& operator=(ICListenLoggingConfig&& other) noexcept;

    // Constructor y operador de copia eliminados
    ICListenLoggingConfig(const ICListenLoggingConfig& other) noexcept;
    ICListenLoggingConfig& operator=(const ICListenLoggingConfig& other) noexcept;

    // Métodos estáticos
    static ICListenLoggingConfig createDefault();
    static ICListenLoggingConfig fromBytes(const std::vector<uint8_t>& data);

private:
    static std::vector<uint8_t> serializeValues(int gain, int waveformSampleRate, int waveformLoggingMode,
                                                int waveformLogLength, int bitDepth, int fftSampleRate,
                                                int fftProcessingType, int fftsAccumulated, int fftLoggingMode,
                                                int fftLogLength);

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
};

#endif // ACOUSEA_MKR1310_NODES_ICLISTENLOGGINGCONFIG_H

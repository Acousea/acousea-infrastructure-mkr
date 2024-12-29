#ifndef ACOUSEA_MKR1310_NODES_ICLISTENSTREAMINGCONFIG_H
#define ACOUSEA_MKR1310_NODES_ICLISTENSTREAMINGCONFIG_H

#include "../../../../SerializableModule.h"
#include <ctime>
#include <vector>
#include <cstdint>

class ICListenStreamingConfig : public SerializableModule {
public:
    // Constructor estándar
    ICListenStreamingConfig(bool recordWaveform, bool processWaveform, int waveformProcessingType,
                            int waveformInterval, int waveformDuration, bool recordFFT, bool processFFT,
                            int fftProcessingType, int fftInterval, int fftDuration, std::time_t timestamp);

    // Constructor de movimiento
    ICListenStreamingConfig(ICListenStreamingConfig&& other) noexcept;

    // Operador de asignación por movimiento
    ICListenStreamingConfig& operator=(ICListenStreamingConfig&& other) noexcept;

    // Constructor y operador de copia eliminados
    ICListenStreamingConfig(const ICListenStreamingConfig&) noexcept;

    ICListenStreamingConfig& operator=(const ICListenStreamingConfig&) noexcept;

    // Métodos estáticos
    static ICListenStreamingConfig createDefault();
    static ICListenStreamingConfig fromBytes(const std::vector<uint8_t>& data);

    std::string toString() const;

private:
    static std::vector<uint8_t> serializeValues(bool recordWaveform, bool processWaveform, int waveformProcessingType,
                                                int waveformInterval, int waveformDuration, bool recordFFT,
                                                bool processFFT, int fftProcessingType, int fftInterval,
                                                int fftDuration, std::time_t timestamp);

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
};

#endif // ACOUSEA_MKR1310_NODES_ICLISTENSTREAMINGCONFIG_H

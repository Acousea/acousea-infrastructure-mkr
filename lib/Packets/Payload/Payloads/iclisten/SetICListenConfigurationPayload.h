#ifndef ACOUSEA_MKR1310_NODES_SET_ICLISTEN_CONFIGURATION_PAYLOAD_H
#define ACOUSEA_MKR1310_NODES_SET_ICLISTEN_CONFIGURATION_PAYLOAD_H

#include "Payload/Payload.h"
#include <bitset>
#include <vector>
#include <cstdint>
#include <optional>
#include "Payload/Modules/pamModules/ICListen/implementation/status/ICListenStatus.h"
#include "Payload/Modules/pamModules/ICListen/implementation/logging/ICListenLoggingConfig.h"
#include "Payload/Modules/pamModules/ICListen/implementation/streaming/ICListenStreamingConfig.h"
#include "Payload/Modules/pamModules/ICListen/implementation/stats/ICListenRecordingStats.h"

class SetICListenConfigurationPayload : public Payload {
public:
    // Reutilizamos el enum Aspect
    enum class Aspect : uint8_t {
        STATUS = 0x01,
        LOGGING = 0x02,
        STREAMING = 0x04,
        STATS = 0x08
    };

    // Builder para construir el payload
    class Builder {
    public:
        Builder &includeAspect(Aspect aspect, const SerializableModule &module);

        [[nodiscard]] SetICListenConfigurationPayload build() const;

    private:
        std::bitset<8> aspects; // Representa los aspectos seleccionados
        std::optional<ICListenStatus> status;
        std::optional<ICListenLoggingConfig> loggingConfig;
        std::optional<ICListenStreamingConfig> streamingConfig;
        std::optional<ICListenRecordingStats> recordingStats;

        void addConfiguration(Aspect aspect, const SerializableModule &module);
    };

    // Implementación de Payload
    [[nodiscard]] uint16_t getBytesSize() const override;

    [[nodiscard]] std::vector<uint8_t> toBytes() const override;

    [[nodiscard]] std::vector<Aspect> getAspectsAsEnums() const;

    [[nodiscard]] ICListenLoggingConfig getLoggingConfig() const {
        return loggingConfig.value();
    }

    [[nodiscard]] ICListenStreamingConfig getStreamingConfig() const {
        return streamingConfig.value();
    }

    [[nodiscard]] ICListenStatus getStatus() const {
        return status.value();
    }

    [[nodiscard]] ICListenRecordingStats getRecordingStats() const {
        return recordingStats.value();
    }


    static SetICListenConfigurationPayload fromBytes(const std::vector<uint8_t> &data);

private:
    explicit SetICListenConfigurationPayload(const std::bitset<8> &selectedAspects,
                                             const std::optional<ICListenStatus> &status,
                                             const std::optional<ICListenLoggingConfig> &loggingConfig,
                                             const std::optional<ICListenStreamingConfig> &streamingConfig,
                                             const std::optional<ICListenRecordingStats> &recordingStats);

    std::bitset<8> selectedAspects;
    std::optional<ICListenStatus> status;
    std::optional<ICListenLoggingConfig> loggingConfig;
    std::optional<ICListenStreamingConfig> streamingConfig;
    std::optional<ICListenRecordingStats> recordingStats;
};

#endif // ACOUSEA_MKR1310_NODES_SET_ICLISTEN_CONFIGURATION_PAYLOAD_H

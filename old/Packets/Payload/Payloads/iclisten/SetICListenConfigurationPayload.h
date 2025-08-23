#ifndef ACOUSEA_MKR1310_NODES_SET_ICLISTEN_CONFIGURATION_PAYLOAD_H
#define ACOUSEA_MKR1310_NODES_SET_ICLISTEN_CONFIGURATION_PAYLOAD_H

#include "Payload/Payload.h"
#include <bitset>
#include <vector>
#include <cstdint>
#include <optional>

#include "ICListenAspect.h"
#include "Payload/Modules/pamModules/ICListen/implementation/status/ICListenStatus.h"
#include "Payload/Modules/pamModules/ICListen/implementation/logging/ICListenLoggingConfig.h"
#include "Payload/Modules/pamModules/ICListen/implementation/streaming/ICListenStreamingConfig.h"
#include "Payload/Modules/pamModules/ICListen/implementation/stats/ICListenRecordingStats.h"

class SetICListenConfigurationPayload final : public Payload {
public:

    // Builder para construir el payload
    class Builder {
    public:
        Builder &includeAspect(ICListenAspect::Aspect aspect, const SerializableModule &module);

        [[nodiscard]] SetICListenConfigurationPayload build() const;

    private:
        std::bitset<8> aspects; // Representa los aspectos seleccionados
        std::optional<ICListenStatus> status;
        std::optional<ICListenLoggingConfig> loggingConfig;
        std::optional<ICListenStreamingConfig> streamingConfig;
        std::optional<ICListenRecordingStats> recordingStats;

        void addConfiguration(ICListenAspect::Aspect aspect, const SerializableModule &module);
    };

    // Implementación de Payload
    [[nodiscard]] uint16_t getBytesSize() const override;

    [[nodiscard]] std::vector<uint8_t> toBytes() const override;


    [[nodiscard]] std::optional<ICListenStatus> getStatus() const {
        return status;
    }

    [[nodiscard]] std::optional<ICListenLoggingConfig> getLoggingConfig() const {
        return loggingConfig;
    }

    [[nodiscard]] std::optional<ICListenStreamingConfig> getStreamingConfig() const {
        return streamingConfig;
    }

    [[nodiscard]] std::optional<ICListenRecordingStats> getRecordingStats() const {
        return recordingStats;
    }


    static SetICListenConfigurationPayload fromBytes(const std::vector<uint8_t> &data);

private:
    explicit SetICListenConfigurationPayload(const std::optional<ICListenStatus> &status,
                                             const std::optional<ICListenLoggingConfig> &loggingConfig,
                                             const std::optional<ICListenStreamingConfig> &streamingConfig,
                                             const std::optional<ICListenRecordingStats> &recordingStats);

    std::optional<ICListenStatus> status;
    std::optional<ICListenLoggingConfig> loggingConfig;
    std::optional<ICListenStreamingConfig> streamingConfig;
    std::optional<ICListenRecordingStats> recordingStats;
};

#endif // ACOUSEA_MKR1310_NODES_SET_ICLISTEN_CONFIGURATION_PAYLOAD_H

#ifndef ACOUSEA_MKR1310_NODES_FETCH_ICLISTEN_STATUS_PAYLOAD_H
#define ACOUSEA_MKR1310_NODES_FETCH_ICLISTEN_STATUS_PAYLOAD_H

#include "Payload/Payload.h"
#include <vector>
#include <cstdint>
#include <bitset>
#include <ErrorHandler/ErrorHandler.h>

#include "ICListenAspect.h"

class FetchICListenConfigurationPayload final : public Payload {
public:
    // Enum para especificar los aspectos de la configuración
    // Builder para construir el payload
    class Builder {
    public:
        Builder& includeAspect(ICListenAspect::Aspect aspect) {
            aspects |= static_cast<uint8_t>(aspect);
            return *this;
        }

        Builder& all() {
            aspects |= static_cast<uint8_t>(ICListenAspect::Aspect::STATUS)
                    | static_cast<uint8_t>(ICListenAspect::Aspect::LOGGING)
                    | static_cast<uint8_t>(ICListenAspect::Aspect::STREAMING)
                    | static_cast<uint8_t>(ICListenAspect::Aspect::STATS);
            return *this;
        }

        [[nodiscard]] FetchICListenConfigurationPayload build() const {
            return FetchICListenConfigurationPayload(aspects);
        }

    private:
        std::bitset<8> aspects; // Representa los aspectos seleccionados
    };

    [[nodiscard]] std::bitset<8> getAspects() const;

    // Implementaciones del Payload
    [[nodiscard]] uint16_t getBytesSize() const override;

    [[nodiscard]] std::vector<uint8_t> toBytes() const override;

    static FetchICListenConfigurationPayload fromBytes(const std::vector<uint8_t> &data);

private:
    explicit FetchICListenConfigurationPayload(const std::bitset<8>& selectedAspects);

    std::bitset<8> selectedAspects;
};

#endif // ACOUSEA_MKR1310_NODES_FETCH_ICLISTEN_STATUS_PAYLOAD_H

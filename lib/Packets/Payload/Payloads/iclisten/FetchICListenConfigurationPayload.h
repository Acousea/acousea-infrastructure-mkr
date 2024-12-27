#ifndef ACOUSEA_MKR1310_NODES_FETCH_ICLISTEN_STATUS_PAYLOAD_H
#define ACOUSEA_MKR1310_NODES_FETCH_ICLISTEN_STATUS_PAYLOAD_H

#include "Payload/Payload.h"
#include <vector>
#include <cstdint>
#include <bitset>
#include <ErrorHandler/ErrorHandler.h>

class FetchICListenConfigurationPayload : public Payload {
public:
    // Enum para especificar los aspectos de la configuración
    enum class Aspect : uint8_t {
        STATUS = 0x01,
        LOGGING = 0x02,
        STREAMING = 0x04,
        STATS = 0x08
    };

    // Builder para construir el payload
    class Builder {
    public:
        Builder& includeAspect(Aspect aspect) {
            aspects.set(static_cast<uint8_t>(aspect));
            return *this;
        }

        Builder& all() {
            aspects.set(); // Incluye todos los bits
            return *this;
        }

        [[nodiscard]] FetchICListenConfigurationPayload build() const {
            return FetchICListenConfigurationPayload(aspects);
        }

    private:
        std::bitset<8> aspects; // Representa los aspectos seleccionados
    };

    [[nodiscard]] std::vector<Aspect> getAspectsAsEnums() const;

    // Implementaciones del Payload
    [[nodiscard]] uint16_t getBytesSize() const override;

    [[nodiscard]] std::vector<uint8_t> toBytes() const override;

    static FetchICListenConfigurationPayload fromBytes(const std::vector<uint8_t> &data);

private:
    explicit FetchICListenConfigurationPayload(const std::bitset<8>& selectedAspects);

    std::bitset<8> selectedAspects;
};

#endif // ACOUSEA_MKR1310_NODES_FETCH_ICLISTEN_STATUS_PAYLOAD_H
